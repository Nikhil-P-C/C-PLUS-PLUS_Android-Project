//
// Created by LENOVO on 23-07-2026.
//
#include "ParticleSystem.h"
#include "engine/Engine.h"
#include <SDL3/SDL.h>
#include "utils/utils.h"
#include <random>
#include <algorithm>
#include <cmath>

static std::mt19937 rng(std::random_device{}());
void ParticleSystem::render(SDL_Renderer* renderer)
{

    int camX = m_screenSpace ? 0 : (int)std::round(Camera::getInstance().getCamera().x);
    int camY = m_screenSpace ? 0 : (int)std::round(Camera::getInstance().getCamera().y);
    for(const auto& fx : m_spriteFx){
        if(!fx.alive) continue;
        const int frame = std::min(fx.frames - 1, (int)(fx.age / fx.frameDur));
        SDL_FRect fsrc{0.0f, (float)(frame * fx.frameH), (float)fx.frameW, (float)fx.frameH};
        const float dw = fx.frameW * fx.scale, dh = fx.frameH * fx.scale;
        SDL_FRect fdst{fx.x - camX - dw * 0.5f, fx.y - camY - dh * 0.5f, dw, dh};
        SDL_RenderTextureRotated(renderer, fx.tex, &fsrc, &fdst, fx.angleDeg, nullptr,
                                 fx.flipX ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    }

    SDL_FRect src {0,0,16,16};
    for(const auto& particle:m_particles)
    {
        if (!particle.isAlive)
            continue;
        float alpha = particle.life / particle.maxLife;
        if(particle.size<0)continue;

        if(particle.kind != ParticleKind::PUFF){
            // Hard-edged vector shapes (no soft texture): additive, x/y is the centre.
            SDL_BlendMode prevBlend = SDL_BLENDMODE_NONE;
            SDL_GetRenderDrawBlendMode(renderer, &prevBlend);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
            const float a = alpha;
            const float cx = particle.x - camX, cy = particle.y - camY;
            const SDL_FColor col{particle.colorR/255.0f, particle.colorG/255.0f, particle.colorB/255.0f, a};
            const SDL_FColor hot{1.0f, 1.0f, 1.0f, a};          // white-hot core

            if(particle.kind == ParticleKind::RING){
                // shock ring: thick bright leading edge, thinning as it expands, fading inward
                const int N = 56;
                const float rOut = particle.size * 0.5f;
                const float rIn  = std::max(0.0f, rOut - (4.0f + 26.0f * a));
                SDL_Vertex v[2 * N];
                int idx[6 * N];
                const SDL_FColor inner{col.r, col.g, col.b, a * 0.15f};
                for(int i = 0; i < N; i++){
                    const float ang = i * (6.2831853f / N);
                    const float ca = std::cos(ang), sa = std::sin(ang);
                    v[2*i]     = {{cx + ca * rOut, cy + sa * rOut}, col,   {0,0}};
                    v[2*i + 1] = {{cx + ca * rIn,  cy + sa * rIn},  inner, {0,0}};
                    const int n = (i + 1) % N;
                    idx[6*i + 0] = 2*i;     idx[6*i + 1] = 2*i + 1; idx[6*i + 2] = 2*n;
                    idx[6*i + 3] = 2*n;     idx[6*i + 4] = 2*i + 1; idx[6*i + 5] = 2*n + 1;
                }
                SDL_RenderGeometry(renderer, nullptr, v, 2 * N, idx, 6 * N);
            } else if(particle.kind == ParticleKind::STREAK || particle.kind == ParticleKind::SCRATCH){
                // thin pointed shard: sharp tip at the head, widest just behind it, tapering to a point
                // (scratch = same shape, fixed angle, longer + thinner, symmetric)
                const bool scratch = particle.kind == ParticleKind::SCRATCH;
                const float ang = scratch ? particle.angle : std::atan2(particle.vY, particle.vX);
                const float dx = std::cos(ang), dy = std::sin(ang);
                const float h = particle.size * 0.5f;
                const float halfW = scratch ? (4.0f + 4.0f * a) : (3.5f + 2.5f * a);
                const float px = -dy, py = dx;
                const SDL_FPoint tip {cx + dx*h,        cy + dy*h};
                const SDL_FPoint tail{cx - dx*h,        cy - dy*h};
                const SDL_FPoint mid {cx + dx*h*(scratch ? 0.0f : 0.35f),  cy + dy*h*(scratch ? 0.0f : 0.35f)};
                SDL_Vertex v[4] = {
                        {tip,  col, {0,0}},
                        {{mid.x + px*halfW, mid.y + py*halfW}, hot, {0,0}},
                        {{mid.x - px*halfW, mid.y - py*halfW}, hot, {0,0}},
                        {tail, col, {0,0}},
                };
                const int idx[6] = {0,1,2, 3,1,2};
                SDL_RenderGeometry(renderer, nullptr, v, 4, idx, 6);
            } else {
                // impact flash: 4 long + 4 short sharp spikes
                const float R = particle.size * 0.5f;
                SDL_Vertex v[17];
                v[0] = {{cx, cy}, hot, {0,0}};
                int idx[48];
                for(int i = 0; i < 16; i++){
                    const float ang = i * (6.2831853f / 16.0f) + 0.3927f;   // rotated 22.5deg
                    const float r = (i % 4 == 0) ? R : ((i % 2 == 0) ? R * 0.55f : R * 0.18f);
                    SDL_FColor rim = col; rim.a = a * 0.0f + (i % 2 == 0 ? a : a * 0.6f);
                    v[i+1] = {{cx + std::cos(ang)*r, cy + std::sin(ang)*r}, rim, {0,0}};
                    idx[i*3] = 0; idx[i*3+1] = i + 1; idx[i*3+2] = (i + 1) % 16 + 1;
                }
                SDL_RenderGeometry(renderer, nullptr, v, 17, idx, 48);
            }
            SDL_SetRenderDrawBlendMode(renderer, prevBlend);
            continue;
        }
        SDL_SetTextureColorMod(m_particleTexture, particle.colorR, particle.colorG, particle.colorB);
        SDL_SetTextureAlphaMod(
                m_particleTexture,
                static_cast<Uint8>(alpha * 255)
        );
        SDL_FRect dustDst{particle.x-camX,particle.y-camY,particle.size,particle.size};
        SDL_RenderTexture(renderer,m_particleTexture, &src,&dustDst);
    }
}

bool ParticleSystem::emitSpriteFx(TextureType type, float x, float y, float dirX, float angleDeg,
                                  float frameDur, float scale)
{
    SDL_Texture* tex = Engine::Get().getAssetManager().getTexture(type);
    if(!tex || tex->h < 32) return false;
    for(auto& fx : m_spriteFx){
        if(fx.alive) continue;
        fx.alive = true;
        fx.tex = tex;
        fx.frameW = tex->w;          // vertical strip: width = frame width, every frame is 32px tall
        fx.frameH = 32;
        fx.frames = std::max(1, tex->h / fx.frameH);
        fx.x = x; fx.y = y;
        fx.flipX = dirX < 0.0f;
        fx.angleDeg = fx.flipX ? -angleDeg : angleDeg;
        fx.scale = scale;
        fx.age = 0.0f;
        fx.frameDur = frameDur;
        SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);   // keep pixel art crisp
        return true;
    }
    return true;   // pool full: drop this one, but art exists so don't fall back to generated sparks
}

void ParticleSystem::update(float dt)
{
    for(auto& fx : m_spriteFx){
        if(!fx.alive) continue;
        fx.age += dt;
        if(fx.age >= fx.frames * fx.frameDur) fx.alive = false;
    }
    for(auto& particle:m_particles)
    {
        float sizefactor = particle.life /particle.maxLife;
        // sparks shrink to nothing; the flash does the opposite (pops out while fading)
        if(particle.kind == ParticleKind::FLASH)
            particle.size = particle.baseSize * (1.4f - 0.9f * sizefactor);
        else if(particle.kind == ParticleKind::RING)
            particle.size = particle.baseSize * (1.0f - sizefactor * sizefactor);   // fast out, then settles
        else if(particle.kind == ParticleKind::SCRATCH)
            particle.size = particle.baseSize * (1.2f - 0.4f * sizefactor);   // snaps out, then fades
        else
            particle.size = particle.baseSize*sizefactor;
        if(!particle.isAlive)
            continue;
        particle.life -= dt;
        if(particle.life <=0)
        {
            particle.isAlive = false;
            // back to plain-dust defaults so a reused slot never inherits spark behaviour
            particle.kind = ParticleKind::PUFF;
            particle.drag = 0.0f;
            particle.gravity = 0.0f;
            continue;
        }
        if(particle.drag > 0.0f){
            const float d = std::exp(-particle.drag * dt);
            particle.vX *= d;
            particle.vY *= d;
        }
        particle.vY += particle.gravity * dt;
        particle.x += particle.vX*dt;
        particle.y += particle.vY*dt;
    }
}

void ParticleSystem::emitOneJumpParticle(float feetX, float feetY)
{
    for(auto& particle:m_particles){
        if(!particle.isAlive){
            particle.isAlive = true;
            particle.x =feetX;
            particle.y =feetY;
            particle.size =70.00f;
            particle.baseSize=particle.size;
            particle.vX =randomFloat(-100.00f,100.00f);
            particle.vY =randomFloat(-300.00f,100.00f);

            particle.life = 0.4f;
            particle.maxLife=0.4f;
            particle.colorR=255; particle.colorG=255; particle.colorB=255;
            return;
        }
    }

}

void ParticleSystem::emitJumpDust(float feetX, float feetY)
{
    int burst =randomInt(2,4);
    for(int i = 0; i< burst;i++)
    {
        emitOneJumpParticle(feetX, feetY);
    }
}
void ParticleSystem::emitLeftDust(float feetX, float feetY)
{
    for(auto& particle:m_particles){
        if(!particle.isAlive){
            particle.isAlive = true;
            particle.x =feetX;
            particle.y =feetY;
            particle.size =70.00f;
            particle.baseSize=particle.size;
            particle.vX =randomFloat(0,200);
            particle.vY =randomFloat(-100,100);
            particle.life = 0.4f;
            particle.maxLife=0.4f;
            particle.colorR=255; particle.colorG=255; particle.colorB=255;
            return;
        }
    }
}

void ParticleSystem::emitRightDust(float feetX, float feetY) {
    for(auto& particle:m_particles){
        if(!particle.isAlive){
            particle.isAlive = true;
            particle.x =feetX;
            particle.y =feetY;
            particle.size =70.00f;
            particle.baseSize=particle.size;
            particle.vX =randomFloat(-100,0);
            particle.vY =randomFloat(-100,100);
            particle.life = 0.4f;
            particle.maxLife=0.4f;
            particle.colorR=255; particle.colorG=255; particle.colorB=255;
            return;
        }
    }
}

void ParticleSystem::emitLandDust(float leftFeetX, float leftFeetY,float rightFeetX,float rightFeetY)
{
    int burst =3;
    for(int i =0; i<burst;i++)
    {
        float randomX1 = randomFloat(rightFeetX,leftFeetX);
        float randomX2 = randomFloat(rightFeetX,leftFeetX);


        emitLeftDust(randomX1, leftFeetY);
        emitRightDust(randomX2, rightFeetY);
    }
}


void ParticleSystem::emitHealSparkle(float x, float y)
{
    int burst = randomInt(15,20);
    for(int i = 0; i<burst; i++)
    {
        for(auto& particle:m_particles){
            if(!particle.isAlive){
                particle.isAlive = true;
                particle.x = x + randomFloat(-50.00f,50.00f);
                particle.y = y+ randomFloat(-50.00f,50.00f);
                particle.size = 50.00f;
                particle.baseSize = particle.size;
                particle.vX = randomFloat(-50.00f,50.00f);
                particle.vY = randomFloat(-50.00f,-20.00f);
                particle.life = 1.0f;
                particle.maxLife = 1.0f;
                particle.colorR = (uint8_t)randomInt(130,255);
                particle.colorG = 255;
                particle.colorB = 150;
                break;
            }
        }
    }
}

void ParticleSystem::emitRing(float x, float y, float diameter, float life, uint8_t r, uint8_t g, uint8_t b)
{
    if(Particle* p = findFree()){
        *p = Particle{};
        p->isAlive = true;
        p->kind = ParticleKind::RING;
        p->x = x; p->y = y;
        p->baseSize = diameter; p->size = 0.0f;
        p->life = p->maxLife = life;
        p->colorR = r; p->colorG = g; p->colorB = b;
    }
}

void ParticleSystem::emitDamageBurst(float x, float y, float dirX)
{
    // 1) shock rings: a big red one and a fast white one inside it
    emitRing(x, y, 640.0f, 0.42f, 255, 70, 60);
    emitRing(x, y, 360.0f, 0.26f, 255, 255, 255);

    // 2) fast radial streaks, full circle
    emitSparkBurst(x, y, dirX, 22, 3.14159f, 700.0f, 1700.0f, 80.0f, 170.0f, 255, 120, 100, 0.0f);

    // 3) debris shards: short, thrown out + up, heavy gravity so they fall back down, red/white mix
    for(int i = 0; i < 18; i++){
        Particle* p = findFree();
        if(!p) break;
        const float a  = randomFloat(0.0f, 6.2831853f);
        const float sp = randomFloat(250.0f, 900.0f);
        *p = Particle{};
        p->isAlive = true;
        p->kind = ParticleKind::STREAK;
        p->x = x + randomFloat(-20.0f, 20.0f); p->y = y + randomFloat(-30.0f, 30.0f);
        p->vX = std::cos(a) * sp;
        p->vY = std::sin(a) * sp - 350.0f;
        p->baseSize = p->size = randomFloat(18.0f, 42.0f);
        p->life = p->maxLife = randomFloat(0.45f, 0.9f);
        p->drag = 1.6f;
        p->gravity = 1700.0f;
        const bool white = (i % 3 == 0);
        p->colorR = 255; p->colorG = white ? 235 : 70; p->colorB = white ? 225 : 60;
    }

    // 4) embers: slow, floaty, linger
    for(int i = 0; i < 12; i++){
        Particle* p = findFree();
        if(!p) break;
        const float a  = randomFloat(0.0f, 6.2831853f);
        const float sp = randomFloat(60.0f, 260.0f);
        *p = Particle{};
        p->isAlive = true;
        p->kind = ParticleKind::STREAK;
        p->x = x; p->y = y;
        p->vX = std::cos(a) * sp;
        p->vY = std::sin(a) * sp - 120.0f;
        p->baseSize = p->size = randomFloat(10.0f, 20.0f);
        p->life = p->maxLife = randomFloat(0.8f, 1.4f);
        p->drag = 2.5f;
        p->gravity = -60.0f;                 // drift upward slightly
        p->colorR = 255; p->colorG = 150; p->colorB = 90;
    }
}

Particle* ParticleSystem::findFree()
{
    for(auto& p : m_particles) if(!p.isAlive) return &p;
    return nullptr;
}

void ParticleSystem::emitSparkBurst(float x, float y, float dirX, int count, float spread,
                                    float speedMin, float speedMax, float lengthMin, float lengthMax,
                                    uint8_t r, uint8_t g, uint8_t b, float flashSize)
{
    const float base = dirX >= 0.0f ? 0.0f : 3.14159265f;
    for(int i = 0; i < count; i++){
        Particle* p = findFree();
        if(!p) break;
        const float a  = base + randomFloat(-spread, spread);
        const float sp = randomFloat(speedMin, speedMax);
        *p = Particle{};
        p->isAlive = true;
        p->kind = ParticleKind::STREAK;
        p->x = x; p->y = y;
        p->vX = std::cos(a) * sp;
        p->vY = std::sin(a) * sp;
        p->baseSize = p->size = randomFloat(lengthMin, lengthMax);   // streak length
        p->life = p->maxLife = randomFloat(0.12f, 0.22f);
        p->drag = 9.0f;
        p->gravity = 400.0f;
        p->colorR = r; p->colorG = g; p->colorB = b;
    }
    if(flashSize > 0.0f){
        if(Particle* p = findFree()){
            *p = Particle{};
            p->isAlive = true;
            p->kind = ParticleKind::FLASH;
            p->x = x; p->y = y;
            p->baseSize = p->size = flashSize;
            p->life = p->maxLife = 0.08f;
            p->colorR = r; p->colorG = g; p->colorB = b;
        }
    }
}

void ParticleSystem::emitScratch(float x, float y, float angle, float length)
{
    if(Particle* p = findFree()){
        *p = Particle{};
        p->isAlive = true;
        p->kind = ParticleKind::SCRATCH;
        p->x = x; p->y = y;
        p->angle = angle;
        p->baseSize = p->size = length;
        p->life = p->maxLife = 0.16f;
        p->colorR = 255; p->colorG = 255; p->colorB = 255;
    }
}

// Enemy hit (Silksong-style): big white starburst, two long crossing scratch lines, white streaks
// spraying the way the enemy is shoved. Warms toward orange only at high combo.
void ParticleSystem::emitHitSpark(float x, float y, float dirX, int combo, float scale)
{
    const int c = combo < 1 ? 1 : (combo > 10 ? 10 : combo);
    const uint8_t g = (uint8_t)std::max(150, 255 - c * 10);
    const uint8_t b = (uint8_t)std::max(90, 255 - c * 16);
    // art first: spark burst + two crossing thrust scratches (+ ring burst on finisher hits).
    // generated flash/scratches below are only the fallback when the PNGs are missing.
    if(emitSpriteFx(TextureType::FX_SPARK_ENEMY, x, y, dirX, randomFloat(-10.0f, 10.0f), 0.035f, 5.0f * scale)){
        emitSpriteFx(TextureType::FX_THRUST_1, x, y, dirX, 16.0f + randomFloat(-6.0f, 6.0f), 0.035f, 5.0f * scale);
        emitSpriteFx(TextureType::FX_THRUST_2, x, y, dirX, -20.0f + randomFloat(-6.0f, 6.0f), 0.035f, 5.0f * scale);
        if((combo > 0 && combo % 5 == 0) || scale > 1.0f)
            emitSpriteFx(TextureType::FX_SPARK_FINISHER, x, y, dirX, 0.0f, 0.04f, 7.0f * scale);
        return;
    }
    emitSparkBurst(x, y, dirX, 6 + c, 0.9f, (500.0f + c * 30.0f) * scale, (1100.0f + c * 40.0f) * scale,
                   (60.0f + c * 3.0f) * scale, (110.0f + c * 6.0f) * scale, 255, g, b, (190.0f + c * 8.0f) * scale);
    const float base = dirX >= 0.0f ? 0.0f : 3.14159265f;
    emitScratch(x, y, base + 0.45f, (340.0f + c * 8.0f) * scale);
    emitScratch(x, y, base - 0.45f, (260.0f + c * 6.0f) * scale);
}

// Wall / ground / platform: small, warm, flung back off the surface, then they fall
void ParticleSystem::emitWallSpark(float x, float y, float dirX)
{
    if(emitSpriteFx(TextureType::FX_SPARK_WALL, x, y, dirX, 0.0f, 0.035f, 5.0f)) return;
    const float base = dirX >= 0.0f ? 0.0f : 3.14159265f;
    for(int i = 0; i < 9; i++){
        Particle* p = findFree();
        if(!p) break;
        const float a  = base + randomFloat(-1.3f, 1.3f);
        const float sp = randomFloat(250.0f, 650.0f);
        *p = Particle{};
        p->isAlive = true;
        p->kind = ParticleKind::STREAK;
        p->x = x; p->y = y;
        p->vX = std::cos(a) * sp; p->vY = std::sin(a) * sp;
        p->baseSize = p->size = randomFloat(25.0f, 55.0f);
        p->life = p->maxLife = randomFloat(0.15f, 0.30f);
        p->drag = 5.0f;
        p->gravity = 1100.0f;
        p->colorR = 255; p->colorG = (uint8_t)randomInt(170, 215); p->colorB = 70;
    }
    if(Particle* p = findFree()){
        *p = Particle{};
        p->isAlive = true; p->kind = ParticleKind::FLASH;
        p->x = x; p->y = y;
        p->baseSize = p->size = 80.0f;
        p->life = p->maxLife = 0.07f;
        p->colorR = 255; p->colorG = 200; p->colorB = 90;
    }
}

// Block / trap / other object: cold white-blue, fewer and straighter, no gravity drop
void ParticleSystem::emitObjectSpark(float x, float y, float dirX)
{
    if(emitSpriteFx(TextureType::FX_SPARK_OBJECT, x, y, dirX, 0.0f, 0.035f, 5.0f)) return;
    emitSparkBurst(x, y, dirX, 7, 0.7f, 350.0f, 800.0f, 45.0f, 90.0f, 170, 220, 255, 100.0f);
}

ParticleSystem::ParticleSystem()
{
    m_particleTexture = Engine::Get().getAssetManager().getTexture(TextureType::DUST_PARTICLE);

}
void ParticleSystem::emitBurst(float x, float y, int count, float speedMin, float speedMax,
                               float lifeMin, float lifeMax, float size,
                               uint8_t r, uint8_t g, uint8_t b)
{
    for(int i = 0; i < count; i++){
        for(auto& particle : m_particles){
            if(particle.isAlive) continue;
            float angle = randomFloat(0.0f, 6.2831853f);
            float speed = randomFloat(speedMin, speedMax);
            float s = size * randomFloat(0.6f, 1.0f);
            particle.isAlive = true;
            particle.size = s;
            particle.baseSize = s;
            particle.x = x - s * 0.5f;          // particles are drawn from their top-left corner
            particle.y = y - s * 0.5f;
            particle.vX = std::cos(angle) * speed;
            particle.vY = std::sin(angle) * speed;
            particle.life = randomFloat(lifeMin, lifeMax);
            particle.maxLife = particle.life;
            particle.colorR = r; particle.colorG = g; particle.colorB = b;
            break;
        }
    }
}

void ParticleSystem::emitDust(float pointX, float pointY,int velocityX,int velocityY,float life,float size)
{
    for(auto& particle:m_particles){
        if(!particle.isAlive){
            particle.isAlive = true;
            particle.x =pointX;
            particle.y =pointY;
            particle.size =size;
            particle.baseSize=particle.size;
            particle.vX =velocityX;
            particle.vY =velocityY;

            particle.life = life;
            particle.maxLife=life;
            particle.colorR=255; particle.colorG=255; particle.colorB=255;
            return;
        }
    }
}

void ParticleSystem::emitParticleWProps(int count, int rangeX1, int rangeX2, int rangeY1, int rangeY2,
                                        int velocityX1,int velocityX2 , int velocityY1,int velocityY2,
                                        float minLife, float maxLife, int dirX, int dirY,float size)
{
    if(count <=0)return;
    for(int i =0; i<count;i++)
    {
        int randomX = randomInt(rangeX1,rangeX2);
        int randomY = randomInt(rangeY1,rangeY2);
        int randomVx = randomInt(velocityX1,velocityX2);
        float life = randomFloat(minLife,maxLife);


        emitDust(randomX,randomY,randomVx,dirY,life,size);

    }
}



float ParticleSystem::randomFloat(float min, float max)
{
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

int ParticleSystem::randomInt(int min, int max)
{
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}