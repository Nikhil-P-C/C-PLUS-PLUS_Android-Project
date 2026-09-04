//
// Created by LENOVO on 30-08-2026.
//
#include "PostProcessor.h"
#include <SDL3/SDL.h>

// "Screen" blend: result = src + dst*(1-src). Lightens like additive but
// asymptotically approaches white instead of clipping/oversaturating past
// it — the right blend for bloom highlights on an 8-bit LDR target.
static const SDL_BlendMode kScreenBlend = SDL_ComposeCustomBlendMode(
    SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_COLOR, SDL_BLENDOPERATION_ADD,
    SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD);

ShaderBytes loadSpirv(const char* path) {
    SDL_IOStream* io = SDL_IOFromFile(path, "rb");
    if (!io) {
        SDL_Log("Failed to open shader file %s: %s", path, SDL_GetError());
        return { nullptr, 0 };
    }

    size_t size = 0;
    void* data = SDL_LoadFile_IO(io, &size, true); // true = closes io for you
    if (!data) {
        SDL_Log("Failed to read shader file %s: %s", path, SDL_GetError());
        return { nullptr, 0 };
    }
    return { data, size };
}

SDL_Texture *PostProcessor::createTarget(SDL_Renderer *renderer, int w, int h) {
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                       SDL_TEXTUREACCESS_TARGET, w, h);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    return texture;
}

static SDL_GPUShader* compileFragmentShader(SDL_GPUDevice* device, const char* spirvPath,
                                             int numSamplers, int numUniformBuffers) {
    ShaderBytes src = loadSpirv(spirvPath);
    if (!src.data) return nullptr;

    SDL_GPUShaderCreateInfo info{};
    info.code_size            = src.size;
    info.code                 = static_cast<Uint8*>(src.data);
    info.entrypoint           = "main";
    info.format                = SDL_GPU_SHADERFORMAT_SPIRV;
    info.stage                 = SDL_GPU_SHADERSTAGE_FRAGMENT;
    info.num_samplers          = numSamplers;
    info.num_uniform_buffers   = numUniformBuffers;
    info.num_storage_buffers   = 0;
    info.num_storage_textures  = 0;
    info.props                 = 0;

    SDL_GPUShader* shader = SDL_CreateGPUShader(device, &info);
    SDL_free(src.data);
    return shader;
}

void PostProcessor::init(SDL_Renderer *renderer, SDL_GPUDevice *device) {
    m_bloomLayerTex = createTarget(renderer, 1600, 720);
    m_brightTex     = createTarget(renderer, 800, 360);   // half-res: bright-pass output feeds straight into the blur
    m_pingPongA     = createTarget(renderer, 800, 360);
    m_pingPongB     = createTarget(renderer, 800, 360);

    SDL_GPUShader* brightPassShader = compileFragmentShader(device, "Shaders/brightpass.frag.spv", 1, 1);
    SDL_GPURenderStateCreateInfo brightInfo{};
    brightInfo.fragment_shader = brightPassShader;
    m_brightPassState = SDL_CreateGPURenderState(renderer, &brightInfo);

    SDL_GPUShader* blurShader = compileFragmentShader(device, "Shaders/blur.frag.spv", 1, 1);
    SDL_GPURenderStateCreateInfo blurInfo{};
    blurInfo.fragment_shader = blurShader;
    m_blurState = SDL_CreateGPURenderState(renderer, &blurInfo);

    m_whitePixel = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, 1, 1);
    SDL_SetTextureBlendMode(m_whitePixel, SDL_BLENDMODE_BLEND);
    Uint32 whitePixelData = 0xFFFFFFFF;
    SDL_UpdateTexture(m_whitePixel, nullptr, &whitePixelData, sizeof(Uint32));

    SDL_GPUShader* lightMaskShader = compileFragmentShader(device, "Shaders/lightmask.frag.spv", 1, 1);
    SDL_GPURenderStateCreateInfo lightMaskInfo{};
    lightMaskInfo.fragment_shader = lightMaskShader;
    m_lightMaskState = SDL_CreateGPURenderState(renderer, &lightMaskInfo);
}

SDL_Texture* PostProcessor::blurTexture(SDL_Renderer* r, SDL_Texture* src, float radiusTexels) {
    struct { float texelSize[2]; float direction[2]; } params{};
    float w, h;
    SDL_GetTextureSize(src, &w, &h);
    params.texelSize[0] = radiusTexels / w;
    params.texelSize[1] = radiusTexels / h;

    SDL_SetGPURenderState(r, m_blurState);

    SDL_SetRenderTarget(r, m_pingPongA);
    params.direction[0] = 1.0f; params.direction[1] = 0.0f; // horizontal
    SDL_SetGPURenderStateFragmentUniforms(m_blurState, 0, &params, sizeof(params));
    SDL_RenderTexture(r, src, nullptr, nullptr);

    SDL_SetRenderTarget(r, m_pingPongB);
    params.direction[0] = 0.0f; params.direction[1] = 1.0f; // vertical
    SDL_SetGPURenderStateFragmentUniforms(m_blurState, 0, &params, sizeof(params));
    SDL_RenderTexture(r, m_pingPongA, nullptr, nullptr);

    SDL_SetGPURenderState(r, nullptr);
    return m_pingPongB;
}

void PostProcessor:: beginBloomGroup(SDL_Renderer* r) {
    SDL_SetRenderTarget(r, m_bloomLayerTex);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 0); // fully transparent
    SDL_RenderClear(r);
}

void PostProcessor::endBloomGroup(SDL_Renderer* r) {
    // bright-pass: keep only genuinely bright pixels, downsampled to half-res
    SDL_SetRenderTarget(r, m_brightTex);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
    SDL_RenderClear(r);
    float threshold = 0.55f; // raise toward 1.0 for a more selective/subtle bloom, lower for more of the layer to glow
    SDL_SetGPURenderStateFragmentUniforms(m_brightPassState, 0, &threshold, sizeof(threshold));
    SDL_SetGPURenderState(r, m_brightPassState);
    SDL_RenderTexture(r, m_bloomLayerTex, nullptr, nullptr);
    SDL_SetGPURenderState(r, nullptr);

    // blur just the bright-pass result
    SDL_Texture* blurred = blurTexture(r, m_brightTex, 1.55f);

    // the group's normal/sharp appearance never reached the window on
    // its own (it was captured offscreen) — draw it first
    SDL_SetRenderTarget(r, nullptr);
    SDL_RenderTexture(r, m_bloomLayerTex, nullptr, nullptr);

    //then screen-blend the blurred highlights on top.
    SDL_SetTextureBlendMode(blurred, kScreenBlend);
    SDL_RenderTexture(r, blurred, nullptr, nullptr);
    SDL_SetTextureBlendMode(blurred, SDL_BLENDMODE_BLEND); // reset for next group/frame's ping-pong reuse
}

void PostProcessor::applyPlayerLight(SDL_Renderer* r, float centerX, float centerY,
                                     float radiusPx, float softnessPx, float darkness) {
    const float screenW = 1600.0f, screenH = 720.0f; // matches the game's fixed logical presentation size

    struct {
        float center[2];
        float radius;
        float softness;
        float darkness;
        float aspect;
        float _pad[2];
    } params{};
    params.center[0] = centerX / screenW;
    params.center[1] = centerY / screenH;
    params.radius    = radiusPx / screenH;
    params.softness  = softnessPx / screenH;
    params.darkness  = darkness;
    params.aspect    = screenW / screenH;

    SDL_SetRenderTarget(r, nullptr); // draw straight over whatever's already on the window
    SDL_SetGPURenderState(r, m_lightMaskState);
    SDL_SetGPURenderStateFragmentUniforms(m_lightMaskState, 0, &params, sizeof(params));

    SDL_FRect fullScreen{0.0f, 0.0f, screenW, screenH};
    SDL_RenderTexture(r, m_whitePixel, nullptr, &fullScreen);

    SDL_SetGPURenderState(r, nullptr);
}
PostProcessor::PostProcessor() = default;
