//
// Created by LENOVO on 30-08-2026.
//
#pragma once

#include <SDL3/SDL.h>
struct ShaderBytes {
    void* data;
    size_t size;
};
ShaderBytes loadSpirv(const char* path);
class PostProcessor{
public:
    PostProcessor();

    void init(SDL_Renderer* renderer,SDL_GPUDevice* device);

    SDL_Texture* createTarget(SDL_Renderer* r, int w, int h);

    void beginScene(SDL_Renderer* r);

    void endSceneAndComposite(SDL_Renderer* r, SDL_Window* window);

    void present(SDL_Renderer* r, SDL_Window* window); // draws m_sceneTex to window, presents

    SDL_Texture* blurTexture(SDL_Renderer* r, SDL_Texture* src, float radiusTexels);

    // Isolated emissive layer — only glow-flagged objects draw here.
    void beginGlowLayer(SDL_Renderer* r);      // target = m_glowTex, TRANSPARENT clear
    void compositeGlowLayer(SDL_Renderer* r);  // blurs m_glowTex, additively blits onto m_sceneTex, restores target = m_sceneTex

private:
    SDL_Texture* m_sceneTex= nullptr;
    SDL_Texture* m_brightTex= nullptr;
    SDL_Texture* m_glowTex  = nullptr;

    SDL_Texture* m_pingPongA= nullptr;
    SDL_Texture* m_pingPongB= nullptr;

    SDL_GPURenderState* m_brightPassState= nullptr;
    SDL_GPURenderState* m_blurState= nullptr;
};