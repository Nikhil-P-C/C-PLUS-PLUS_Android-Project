//
// Created by LENOVO on 30-08-2026.
//
#include "PostProcessor.h"
#include <SDL3/SDL.h>

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

void PostProcessor::beginScene(SDL_Renderer* renderer) {
    SDL_RenderClear(renderer);

    SDL_SetRenderTarget(renderer, m_sceneTex);

}

void PostProcessor::endSceneAndComposite(SDL_Renderer* r, SDL_Window* window) {
    // --- bright pass: sceneTex -> brightTex ---
    SDL_SetRenderTarget(r, m_brightTex);
    float threshold = 0.7f;
    SDL_SetGPURenderStateFragmentUniforms(m_brightPassState, 0, &threshold, sizeof(threshold));
    SDL_SetGPURenderState(r, m_brightPassState);
    SDL_RenderTexture(r, m_sceneTex, nullptr, nullptr);
    SDL_SetGPURenderState(r, nullptr);

    // --- blur: two passes, ping-ponging brightTex -> A -> B ---
    struct { float texelSize[2]; float direction[2]; } blurParams{};
    blurParams.texelSize[0] = 1.0f / 800.0f;
    blurParams.texelSize[1] = 1.0f / 360.0f;

    SDL_SetRenderTarget(r, m_pingPongA);
    blurParams.direction[0] = 1.0f; blurParams.direction[1] = 0.0f; // horizontal
    SDL_SetGPURenderStateFragmentUniforms(m_blurState, 0, &blurParams, sizeof(blurParams));
    SDL_SetGPURenderState(r, m_blurState);
    SDL_RenderTexture(r, m_brightTex, nullptr, nullptr);

    SDL_SetRenderTarget(r, m_pingPongB);
    blurParams.direction[0] = 0.0f; blurParams.direction[1] = 1.0f; // vertical
    SDL_SetGPURenderStateFragmentUniforms(m_blurState, 0, &blurParams, sizeof(blurParams));
    SDL_RenderTexture(r, m_pingPongA, nullptr, nullptr);
    SDL_SetGPURenderState(r, nullptr);

    // --- composite: draw scene, then additively blend blurred bloom on top ---
    SDL_SetRenderTarget(r, nullptr); // back to the window
    SDL_RenderTexture(r, m_sceneTex, nullptr, nullptr);
    SDL_SetTextureBlendMode(m_pingPongB, SDL_BLENDMODE_ADD);
    SDL_RenderTexture(r, m_pingPongB, nullptr, nullptr);


}

SDL_Texture *PostProcessor::createTarget(SDL_Renderer *renderer, int w, int h) {
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                       SDL_TEXTUREACCESS_TARGET, w, h);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);
    return texture;
}

void PostProcessor::init(SDL_Renderer *renderer, SDL_GPUDevice *device) {
    m_sceneTex   = createTarget(renderer, 1600, 720);
    m_brightTex  = createTarget(renderer, 800, 360);   // half-res for cheaper blur
    m_pingPongA  = createTarget(renderer, 800, 360);
    m_pingPongB  = createTarget(renderer, 800, 360);

    ShaderBytes brightSrc = loadSpirv("Shaders/brightpass.frag.spv");
    SDL_GPUShaderCreateInfo brightShaderInfo{};

    brightShaderInfo.code_size = brightSrc.size;
    brightShaderInfo.code = static_cast<Uint8*>(brightSrc.data);
    brightShaderInfo.entrypoint = "main";
    brightShaderInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
    brightShaderInfo.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    brightShaderInfo.num_samplers = 1;
    brightShaderInfo.num_uniform_buffers = 1;
    brightShaderInfo.num_storage_buffers = 0;
    brightShaderInfo.num_storage_textures = 0;
    brightShaderInfo.props = 0;

    SDL_GPUShader* brightPassShader = SDL_CreateGPUShader(device, &brightShaderInfo);
    SDL_free(brightSrc.data);

    ShaderBytes blurSrc = loadSpirv("Shaders/blur.frag.spv");
    SDL_GPUShaderCreateInfo blurShaderInfo{};

    blurShaderInfo.code_size = blurSrc.size;
    blurShaderInfo.code = static_cast<Uint8*>(blurSrc.data);
    blurShaderInfo.entrypoint = "main";
    blurShaderInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
    blurShaderInfo.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    blurShaderInfo.num_samplers = 1;
    blurShaderInfo.num_uniform_buffers = 1;
    blurShaderInfo.num_storage_buffers = 0;
    blurShaderInfo.num_storage_textures = 0;
    blurShaderInfo.props = 0;
    
    SDL_GPUShader* blurShader = SDL_CreateGPUShader(device, &blurShaderInfo);
    SDL_free(blurSrc.data);

    SDL_GPURenderStateCreateInfo brightInfo = {};
    brightInfo.fragment_shader = brightPassShader;
    m_brightPassState = SDL_CreateGPURenderState(renderer, &brightInfo);

    SDL_GPURenderStateCreateInfo blurInfo = {};
    blurInfo.fragment_shader = blurShader;
    m_blurState = SDL_CreateGPURenderState(renderer, &blurInfo);

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

void PostProcessor::beginGlowLayer(SDL_Renderer* r) {
    SDL_SetRenderTarget(r, m_glowTex);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 0); // fully transparent
    SDL_RenderClear(r);
}

void PostProcessor::compositeGlowLayer(SDL_Renderer* r) {
    SDL_Texture* blurred = blurTexture(r, m_glowTex, 1.5f);

    SDL_SetRenderTarget(r, m_sceneTex);
    SDL_SetTextureBlendMode(blurred, SDL_BLENDMODE_ADD);
    SDL_RenderTexture(r, blurred, nullptr, nullptr);
    SDL_SetTextureBlendMode(blurred, SDL_BLENDMODE_BLEND); // reset for next frame's ping-pong reuse
}

void PostProcessor::present(SDL_Renderer* r, SDL_Window* window) {
    SDL_SetRenderTarget(r, nullptr);
    SDL_RenderTexture(r, m_sceneTex, nullptr, nullptr);
    SDL_RenderPresent(r);
}
PostProcessor::PostProcessor() =default;