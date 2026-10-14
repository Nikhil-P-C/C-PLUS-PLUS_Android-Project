#include "GameState.h"
#include "States/MenuState/TitleScreenState.h"
#include "States/TransitionState/TransitionState.h"
#include "States/PauseState/PauseState.h"
#include "States/RespawnState/RespawnState.h"
#include "States/DebugState/DebugState.h"
#include "States/HUDOverlayState/HUDOverlayState.h"
#include "utils/utils.h"
#include <cmath>
#include <algorithm>


//
// Created by LENOVO on 27-04-2026.
//
GameState::GameState(SDL_Renderer *renderer,int level) {

    this->Name = "GameState";
    SDL_AddEventWatch(HandleBackgroundEvents,this);
    m_renderer = renderer;
    LOGI("gamestate constructor:%p",this);
    m_windowH =GameData::getInstance().getWinHeight();
    m_windowW =GameData::getInstance().getWinWidth();

    m_level = level;
    setLevel(m_level);

    //init player attributes
    m_player.setSize(SPRITE_WIDTH*P_scale-80.00f,SPRITE_HEIGHT*P_scale-35.00f);
    m_player.setPosition(200.00f,400.00f,P_scale);

    m_player.setSpriteOffset(-40.00f,-20.00f);
    m_player.setSpriteSize(SPRITE_WIDTH*P_scale,SPRITE_HEIGHT*P_scale);
    Camera::getInstance().setSize((float)m_windowW,(float)m_windowH);
    Camera::getInstance().resetShake();      // Camera is a singleton - don't carry shake into a new level

    //font loading
    m_font = TTF_OpenFontIO(m_fontFile, false, 36);
    m_interactableBuilder.setFont(m_font);
    //background sprite loading;
    m_backGround = Engine::Get().getAssetManager().getTexture(TextureType::BG_SKY);
    // m_tileset loading
    m_tileset = Engine::Get().getAssetManager().getTexture(TextureType::TILESET_SPRITE);
    //player sprite loading
    m_yellowSkin = Engine::Get().getAssetManager().getTexture(TextureType::YELLOW_SKIN);
    m_blueSkin  = Engine::Get().getAssetManager().getTexture(TextureType::BLUE_SKIN);
    m_redSkin  = Engine::Get().getAssetManager().getTexture(TextureType::RED_SKIN);
    m_greenSkin  = Engine::Get().getAssetManager().getTexture(TextureType::GREEN_SKIN);

    //from player static data
    if(PlayerDetail::getInstance().getPlayerSkin() == PlayerSkin::YELLOW)
        m_playerTexture = m_yellowSkin;
    else if(PlayerDetail::getInstance().getPlayerSkin()  == PlayerSkin::BLUE)
        m_playerTexture = m_blueSkin;
    else if(PlayerDetail::getInstance().getPlayerSkin()  == PlayerSkin::RED)
        m_playerTexture = m_redSkin;
    else if(PlayerDetail::getInstance().getPlayerSkin()  == PlayerSkin::GREEN)
        m_playerTexture = m_greenSkin;

    //player name loading
    SDL_Surface* playerNameSurface = TTF_RenderText_Solid(m_font,PlayerDetail::getInstance().getPlayerName().c_str(),
                                                          PlayerDetail::getInstance().getPlayerName().length(),{255,255,255,255});
    m_playerNameTextue = SDL_CreateTextureFromSurface(renderer,playerNameSurface);
    SDL_DestroySurface(playerNameSurface);


}

void GameState::render(SDL_Renderer* renderer)  {

    int camX = (int)std::round(Camera::getInstance().getCamera().x);
    int camY = (int)std::round(Camera::getInstance().getCamera().y);


    // Group 1: background + walls + particles + traps + fruits, all drawn
    // before the player. One offscreen capture, bright-pass + blur applied
    // once to the whole group (see PostProcessor::beginBloomGroup).
    Engine::Get().getPostProcessor().beginBloomGroup(m_renderer);

    m_backgroundBuilder.render(m_renderer);
    m_ambientParticles.renderBack(m_renderer);

    for(const auto& tile : m_wallTiles)
    {
        SDL_FRect dst = {
                tile.x - (float)camX,
                tile.y - (float)camY,
                TILE_SIZE * SCALE, TILE_SIZE * SCALE};

        if (tile.flip) {
            SDL_RenderTextureRotated(renderer, m_tileset, &tile.src, &dst, 0.0f, nullptr, SDL_FLIP_HORIZONTAL_AND_VERTICAL);
        } else {
            SDL_RenderTexture(renderer, m_tileset, &tile.src, &dst);
        }
    }

    for(auto const& tile:m_wallShape.tiles){
        SDL_FRect src = tile.src;
        SDL_FRect dst{tile.x-(float)camX,tile.y-(float)camY,tile.w,tile.h};
        SDL_RenderTexture(renderer,m_tileset,&src,&dst);
    }

    for(const auto& platform : m_platforms){
        int tileSize =TILE_SIZE;

        const int platformWidth = (int)platform.w;
        const int platformHeight = (int)platform.h;

        int widthTiles = platformWidth / tileSize;
        int heightTiles = platformHeight / tileSize;

        for (int y = 0; y < heightTiles; y++) {
            for (int x = 0; x < widthTiles; x++) {

                SDL_FRect src;

                bool left = (x == 0);
                bool right = (x == widthTiles - 1);
                bool top = (y == 0);
                bool bottom = (y == heightTiles - 1);

                if(heightTiles ==1){

                    if(platform.platformType == SpriteType::GOLD_PLATFORM){

                        if (left) src = {SpriteCollection::goldPlatform.x, 0.00f, TILE_SIZE, TILE_SIZE};

                        else if (right)
                            src = {SpriteCollection::goldPlatform.x + 32.00f, 0.00f, TILE_SIZE,
                                   TILE_SIZE};

                        else
                            src = {SpriteCollection::goldPlatform.x + TILE_SIZE, 0.00f, TILE_SIZE,
                                   TILE_SIZE};
                    }

                    else if(platform.platformType == SpriteType::WOODEN_PLATFORM){
                        if (left) src = {SpriteCollection::woodenPlatform.x,
                                         SpriteCollection::woodenPlatform.y, TILE_SIZE, TILE_SIZE};

                        else if (right)
                            src = {SpriteCollection::woodenPlatform.x + 32.00f,
                                   SpriteCollection::woodenPlatform.y, TILE_SIZE,TILE_SIZE};

                        else
                            src = {SpriteCollection::woodenPlatform.x + TILE_SIZE,
                                   SpriteCollection::woodenPlatform.y, TILE_SIZE,
                                   TILE_SIZE};

                    }

                    else if(platform.platformType == SpriteType::STONE_PLATFORM) {
                        if (left) src = {SpriteCollection::stonePlatform.x,
                                         SpriteCollection::stonePlatform.y, TILE_SIZE, TILE_SIZE};

                        else if (right)
                            src = {SpriteCollection::stonePlatform.x + 32.00f,
                                   SpriteCollection::stonePlatform.y, TILE_SIZE,TILE_SIZE};

                        else
                            src = {SpriteCollection::stonePlatform.x + TILE_SIZE,
                                   SpriteCollection::stonePlatform.y, TILE_SIZE,
                                   TILE_SIZE};
                    }

                }

                SDL_FRect dst = {(platform.x+x* (TILE_SIZE * SCALE)) - camX,
                                 ( platform.y+y * (TILE_SIZE * SCALE)) - camY,
                                 TILE_SIZE * SCALE, TILE_SIZE * SCALE};

                SDL_RenderTexture(renderer, m_tileset, &src, &dst);
            }

        }

    }
    SDL_FRect checkPointDst{m_checkPoint.x-camX,m_checkPoint.y-camY,m_checkPoint.w,m_checkPoint.h};
    if(m_checkPoint.aniType == CheckPointAni::NO_FLAG)
    {
        SDL_FRect checkPointSrc{0.00f,0.00f,64.00f,64.00f};
        SDL_RenderTexture(renderer,Engine::Get().getAssetManager().getTexture(TextureType::CHECKPOINT_FLAG_NO),
                          &checkPointSrc,&checkPointDst);
    }
    if(m_checkPoint.aniType == CheckPointAni::FLAG_OUT)
    {
        SDL_FRect checkPointSrc{0.00f+64.00f*m_checkPoint.currentFrame,0.00f,64.00f,64.00f};
        SDL_RenderTexture(renderer,Engine::Get().getAssetManager().getTexture(TextureType::CHECKPOINT_FLAG_OUT),
                          &checkPointSrc,&checkPointDst);
    }
    if(m_checkPoint.aniType == CheckPointAni::FLAG_IDLE)
    {
        SDL_FRect checkPointSrc{0.00f+64.00f*m_checkPoint.currentFrame,0.00f,64.00f,64.00f};
        SDL_RenderTexture(renderer,Engine::Get().getAssetManager().getTexture(TextureType::CHECKPOINT_FLAG_IDLE),
                          &checkPointSrc,&checkPointDst);
    }

    m_particleSystem.render(m_renderer);
    m_trapBuilder.render(m_renderer);
    m_fruitBuilder.render(m_renderer);
    if(m_isDoubleJumping){

        SDL_FRect doubleJumpDst{
                m_player.x + m_player.spriteOffsetX - (float)camX,
                m_player.y + m_player.spriteOffsetY - (float)camY,
                m_player.spriteW, m_player.spriteH};
        SDL_FRect doubleJumpSrc{
                static_cast<float>(0 + (m_doubleJumpFrame * DOUBLE_JUMP_FRAME_W)), 0,
                (float)DOUBLE_JUMP_FRAME_W, (float)DOUBLE_JUMP_FRAME_H};
        if(!m_isPlayerfacingRight){
            SDL_RenderTextureRotated(renderer, Engine::Get().getAssetManager().getTexture(
                                             TextureType::PLAYER_DOUBLE_JUMP), &doubleJumpSrc, &doubleJumpDst, 0.0f, nullptr,
                                     SDL_FLIP_HORIZONTAL);
        } else {
            SDL_RenderTexture(renderer, Engine::Get().getAssetManager().getTexture(
                    TextureType::PLAYER_DOUBLE_JUMP), &doubleJumpSrc, &doubleJumpDst);
        }
    }
    Engine::Get().getPostProcessor().endBloomGroup(m_renderer);
    m_enemyBuilder.render(m_renderer);
    m_interactableBuilder.renderWorld(m_renderer);

    SDL_FRect dst = {m_player.x+m_player.spriteOffsetX-(float)camX,
                     m_player.y+m_player.spriteOffsetY-(float)camY,
                     m_player.spriteW,m_player.spriteH};

    SDL_FRect src = {(float) (0 + (SPRITE_WIDTH * m_currentFrame)), 0, SPRITE_WIDTH, SPRITE_HEIGHT};
    if(m_isDead){
        // white silhouette flash, then the hurt pose tumbling away (rotation starts when the pop does)
        int deathFrame = (m_deathTimer < PLAYER_DEATH_WHITE_S) ? PLAYER_DEATH_FRAME_WHITE : PLAYER_DEATH_FRAME_POSE;
        src.x = (float)(SPRITE_WIDTH * deathFrame);
        double angle = (m_deathTimer > PLAYER_DEATH_FREEZE_S)
                       ? m_deathDir * PLAYER_DEATH_SPIN_DEG_S * (m_deathTimer - PLAYER_DEATH_FREEZE_S) : 0.0;
        SDL_SetTextureAlphaMod(m_playerTexture, 255);
        SDL_RenderTextureRotated(renderer, m_playerTexture, &src, &dst, angle, nullptr,
                                 m_isPlayerfacingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL);
    }
    else {
        if(PlayerDetail::getInstance().isInvincible())
            SDL_SetTextureAlphaMod(m_playerTexture, isBlinkFrame() ? 80 : 150);
        else
            SDL_SetTextureAlphaMod(m_playerTexture,255);

        if(!m_isPlayerfacingRight)
        {
            SDL_RenderTextureRotated(renderer, m_playerTexture, &src, &dst, 0.0f, nullptr,
                                     SDL_FLIP_HORIZONTAL);
        }
        else
        {
            SDL_RenderTexture(renderer, m_playerTexture, &src, &dst);
        }
        if(SDL_GetTicks() < m_playerFlashEnd){
            // damage flash: second additive pass pushes the sprite toward white
            SDL_SetTextureBlendMode(m_playerTexture, SDL_BLENDMODE_ADD);
            SDL_SetTextureAlphaMod(m_playerTexture, 230);
            SDL_RenderTextureRotated(renderer, m_playerTexture, &src, &dst, 0.0f, nullptr,
                                     m_isPlayerfacingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL);
            SDL_SetTextureAlphaMod(m_playerTexture, 255);
            SDL_SetTextureBlendMode(m_playerTexture, SDL_BLENDMODE_BLEND);
        }
    }
    //player name rendering (hidden while dead - it would hover in place while the body tumbles)
    SDL_FRect playerNameDst{m_player.x+m_player.spriteOffsetX-(float)camX+(12.50f*P_scale)
                            -(50+1*static_cast<float>(PlayerDetail::getInstance().getPlayerName().length()))/2,
                            m_player.y+m_player.spriteOffsetY-(float)camY-(2.00f*P_scale) -10,
                            50+1*static_cast<float>(PlayerDetail::getInstance().getPlayerName().length()),
                            45.00f};
    if(!m_isDead) SDL_RenderTexture(renderer,m_playerNameTextue, nullptr,&playerNameDst);


    m_blockBuilder.render(m_renderer);

    // Group 2: foreground only, drawn after the player/blocks so it composites on top of them.
    Engine::Get().getPostProcessor().beginBloomGroup(m_renderer);
    if(m_isAttacking)
    {
        SDL_FRect slashDst{m_player.x - 40 - camX, m_player.y - 20 - camY, 48 * P_scale,
                           24 * P_scale};
        SDL_FRect slashSrc{static_cast<float>(0 + (m_tempInc * 48)), 0, 48, 24};
        if (!m_isPlayerfacingRight) {
            slashDst.x = slashDst.x - 120;
            SDL_RenderTextureRotated(renderer, Engine::Get().getAssetManager().getTexture(
                                             TextureType::ATTACK_PLAYER_SLASH), &slashSrc, &slashDst, 0.0f, nullptr,
                                     SDL_FLIP_HORIZONTAL);
        } else {

            SDL_RenderTexture(renderer, Engine::Get().getAssetManager().getTexture(
                    TextureType::ATTACK_PLAYER_SLASH), &slashSrc, &slashDst);
        }
    }

    if(m_isBeaming && m_beamHitBox.w > 0.0f){
        constexpr float BEAM_SRC_CAP_W = 37.0f;
        constexpr float BEAM_SRC_MID_W = 119.0f;
        constexpr float BEAM_SRC_H     = 40.0f;

        SDL_Texture* beamTexture = Engine::Get().getAssetManager().getTexture(TextureType::ATTACK_PLAYER_BEAM);
        SDL_FlipMode beamFlip = m_isPlayerfacingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

        float scaleY = m_beamHitBox.h / BEAM_SRC_H;
        float capW = BEAM_SRC_CAP_W * scaleY;
        if(capW * 2.0f > m_beamHitBox.w) capW = m_beamHitBox.w * 0.5f; // beam too short for both caps at full size - shrink them to meet in the middle, skip the mid segment
        float midW = m_beamHitBox.w - capW * 2.0f;

        float beamScreenX = m_beamHitBox.x - camX;
        float beamScreenY = m_beamHitBox.y - camY;

        // "near" cap sits at the player's edge (the origin of the beam), "far" cap is the tip,
        // and the stretched middle segment fills the gap between them.
        float nearCapX, farCapX, midX;
        if(m_isPlayerfacingRight){
            nearCapX = beamScreenX;
            midX     = beamScreenX + capW;
            farCapX  = beamScreenX + m_beamHitBox.w - capW;
        } else {
            farCapX  = beamScreenX;
            midX     = beamScreenX + capW;
            nearCapX = beamScreenX + m_beamHitBox.w - capW;
        }

        SDL_FRect nearCapSrc{0.0f, 0.0f, BEAM_SRC_CAP_W, BEAM_SRC_H};
        SDL_FRect nearCapDst{nearCapX, beamScreenY, capW, m_beamHitBox.h};
        SDL_RenderTextureRotated(renderer, beamTexture, &nearCapSrc, &nearCapDst, 0.0f, nullptr, beamFlip);

        if(midW > 0.0f){
            SDL_FRect midSrc{BEAM_SRC_CAP_W, 0.0f, BEAM_SRC_MID_W, BEAM_SRC_H};
            SDL_FRect midDst{midX, beamScreenY, midW, m_beamHitBox.h};
            SDL_RenderTextureRotated(renderer, beamTexture, &midSrc, &midDst, 0.0f, nullptr, beamFlip);
        }

        SDL_FRect farCapSrc{BEAM_SRC_CAP_W + BEAM_SRC_MID_W, 0.0f, BEAM_SRC_CAP_W, BEAM_SRC_H};
        SDL_FRect farCapDst{farCapX, beamScreenY, capW, m_beamHitBox.h};
        SDL_RenderTextureRotated(renderer, beamTexture, &farCapSrc, &farCapDst, 0.0f, nullptr, beamFlip);
    }


    m_fxParticles.render(m_renderer);   // hit sparks / flashes (additive, glow via this bloom group)

    m_foregroundBuilder.render(m_renderer);
    m_ambientParticles.renderFront(m_renderer);
    Engine::Get().getPostProcessor().endBloomGroup(m_renderer);

    // ambient darkness + player light + light objects, all described by the level file (D / L lines)
    m_lighting.render(m_renderer, (float)camX, (float)camY,
                      m_player.x + m_player.spriteOffsetX + m_player.spriteW * 0.5f - (float) camX,
                      m_player.y + m_player.spriteOffsetY + m_player.spriteH * 0.5f - (float) camY);

    renderComboCounter(renderer);   // after the light/vignette so it isn't darkened
    m_interactableBuilder.renderDialog(renderer);   // screen-space dialog box, on top of the world

    updateHurtAudio();              // wall-clock based: the audio also recovers while the pause menu is up
    if(m_hurtVignette > 0.0f){
        // red frame, clear middle: outer rectangle red, inner rectangle transparent, vertex colours blend between
        const float t = std::min(1.0f, m_hurtVignette / 0.9f);
        const float a = t * t * 0.6f;
        const float W = Camera::getInstance().getCamera().w, H = Camera::getInstance().getCamera().h;
        const float ix = W * 0.22f, iy = H * 0.22f;
        const SDL_FColor edge{0.85f, 0.04f, 0.04f, a}, clear{0.85f, 0.04f, 0.04f, 0.0f};
        SDL_Vertex v[8] = {
                {{0,0},edge,{0,0}}, {{W,0},edge,{0,0}}, {{W,H},edge,{0,0}}, {{0,H},edge,{0,0}},
                {{ix,iy},clear,{0,0}}, {{W-ix,iy},clear,{0,0}}, {{W-ix,H-iy},clear,{0,0}}, {{ix,H-iy},clear,{0,0}}
        };
        int idx[24];
        for(int i = 0; i < 4; i++){
            const int n = (i + 1) % 4;
            idx[i*6+0] = i;     idx[i*6+1] = n;     idx[i*6+2] = 4 + n;
            idx[i*6+3] = i;     idx[i*6+4] = 4 + n; idx[i*6+5] = 4 + i;
        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderGeometry(renderer, nullptr, v, 8, idx, 24);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }
    if(m_impactFlash > 0.0f){
        // impact frame: near-solid white on the first frames, fading out fast
        const float a = std::min(1.0f, m_impactFlash / 0.10f);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, (Uint8)(a * a * 235.0f));
        SDL_FRect full{0.0f, 0.0f, Camera::getInstance().getCamera().w, Camera::getInstance().getCamera().h};
        SDL_RenderFillRect(renderer, &full);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }
}

void GameState::update(float dt){

    if(PlayerDetail::getInstance().getPlayerHP() <= 0 && !m_isDead){
        startPlayerDeath();
    }
    if(m_isDead){
        updateDeath(dt);
        return;     // no input, collisions, hits or camera follow while the death animation plays
    }
    // hit-stop: freeze the simulation for a few ms on impact. Shake + sparks keep running on real dt,
    // and the wall-clock (SDL_GetTicks) timers are pushed forward so the freeze doesn't eat them.
    if(m_hitStop > 0.0f){
        m_hitStop -= dt;
        const unsigned int frozenMs = (unsigned int)(dt * 1000.0f + 0.5f);
        m_aniLastTime = SDL_GetTicks();                 // pauses the player/slash frame animation
        m_enemyBuilder.shiftTimers(frozenMs);
        if(m_hurtAnimEndTime)  m_hurtAnimEndTime  += frozenMs;
        if(m_knockbackEndTime) m_knockbackEndTime += frozenMs;
        m_fxParticles.update(dt);
        updateRealtimeFx(dt);
        return;
    }

    // slow motion (after the freeze): scale the simulation dt, and push the wall-clock (SDL_GetTicks) timers
    // forward by the time we "didn't" simulate so animations / cooldowns / enemy AI slow down with it.
    const float realDt = dt;
    if(m_slowMoLeft > 0.0f){
        m_slowMoLeft -= realDt;
        const float t = std::min(1.0f, std::max(0.0f, 1.0f - m_slowMoLeft / m_slowMoTotal));
        const float e = t * t * (3.0f - 2.0f * t);                         // smoothstep back to normal speed
        m_timeScale = m_slowMoMin + (1.0f - m_slowMoMin) * e;
    } else {
        m_timeScale = 1.0f;
    }
    if(m_timeScale < 0.999f){
        dt *= m_timeScale;
        m_timeDebtMs += (1.0f - m_timeScale) * realDt * 1000.0f;
        const unsigned int ms = (unsigned int)m_timeDebtMs;
        m_timeDebtMs -= (float)ms;
        if(ms > 0){
            m_aniLastTime = std::min<uint32_t>(m_aniLastTime + ms, SDL_GetTicks());
            m_enemyBuilder.shiftTimers(ms);
            if(m_hurtAnimEndTime)  m_hurtAnimEndTime  += ms;
            if(m_knockbackEndTime) m_knockbackEndTime += ms;
            if(m_recoilEndTime)    m_recoilEndTime    += ms;
        }
    }

    if(!m_isBeaming){
        PlayerDetail::getInstance().addMagic(MAGIC_REGEN_PER_SEC * dt);   // no regen while channeling the beam
    }
    m_fruitBuilder.update(dt);

    // interactables: open/advance dialogs, and hold the player still while one is open
    if(InputDispatcher::getInstance().consumeInteract()) m_interactableBuilder.interact();
    m_interactableBuilder.update(dt, SDL_FRect{m_player.x, m_player.y, m_player.w, m_player.h});
    if(m_interactableBuilder.isDialogOpen()) InputDispatcher::getInstance().inputLogClear();

    m_previousY =m_player.y;

    handlePhysicAndInput(dt);

    if(m_isAttacking){
        m_playerHitBox = {m_player.x+40,m_player.y +15,29* P_scale,15 * P_scale};
        if (!m_isPlayerfacingRight) {
            m_playerHitBox.x = m_playerHitBox.x - 185;
        }
    }
    else {
        m_playerHitBox ={0.00f};
    }

    m_isGrounded=false;

    const float xBeforeCollision = m_player.x;
    const float speedBeforeCollision = std::fabs(m_velocityX);
    handleCollision();
    {
        // slammed into a wall/block at speed: dust puff + small shake, once per impact
        const float push = xBeforeCollision - m_player.x;      // >0 = shoved left (wall on the right)
        if(std::fabs(push) > 1.0f){
            if(!m_wallBump && speedBeforeCollision >= 350.0f){
                m_wallBump = true;
                const float contactX = push > 0.0f ? m_player.x + m_player.w : m_player.x;
                m_particleSystem.emitBurst(contactX, m_player.y + m_player.h * 0.7f, 5, 80.0f, 220.0f,
                                           0.25f, 0.40f, 40.0f, 220, 220, 220);
                Camera::getInstance().addTrauma(0.08f);
            }
        } else {
            m_wallBump = false;
        }
    }

    for(int i =0;i<m_traps.size();i++)
    {
        //fire
        if(m_traps[i].type == TrapType::FIRE)
            m_trapBuilder.checkFireCollision(i,m_player.x,m_player.y,m_player.w,m_player.h);
        if(m_trapBuilder.isSolid(i)){
            gameMath::collisionSide side =m_trapBuilder.resolveTrapCollision(i, m_player.x, m_player.y,
                                                                             m_player.w, m_player.h,
                                                                             m_previousY,m_velocityY);


            if(side != gameMath::collisionSide::NONE&&(m_trapBuilder.trapHasPath(m_traps[i].type)||m_traps[i].type==TrapType::FALLING_PLATFORM)){
                SDL_FPoint trapDelta =m_trapBuilder.getTrapDelta(i);
                m_player.x+=trapDelta.x;
                if(m_traps[i].colliderType != ColliderType::ONE_WAY && side ==gameMath::collisionSide::TOP)
                    m_player.y+=trapDelta.y;
            }
            switch (side) {
                case gameMath::collisionSide::TOP:
                    if(m_traps[i].type == TrapType::FALLING_PLATFORM) {
                        m_trapBuilder.triggerFall(i);
                    }
                    m_isGrounded =true;
                    m_velocityY =0.0f;
                    break;
                case gameMath::collisionSide::BOTTOM:
                    m_velocityY =0.0f;
                case gameMath::collisionSide::LEFT:
                case gameMath::collisionSide::RIGHT:
                case gameMath::collisionSide::NONE:
                    break;
            }
        }
        //trampoline
        bool trampolineJump =false;
        if(m_traps[i].type == TrapType::TRAMPOLINE)
            trampolineJump=m_trapBuilder.checkTrampolineBounce(i,m_player.x,m_player.y,m_player.w,m_player.h,m_particleSystem);
        if(trampolineJump){
            m_velocityY=0.0f;
            m_isGrounded = false;
            m_velocityY += -1500;
            m_jumpsUsed = 1; // counts as the first jump - one air jump still available after
        }

    }

    m_trapBuilder.updatePath(dt);
    SDL_FRect playerRect{m_player.x,m_player.y,m_player.w,m_player.h};
    m_enemyBuilder.update(dt,playerRect,m_wallCollisionRect,m_grounds,m_platforms,m_blocks);
    flushEnemyDeathEffects();

    // player's weapons vs enemies: m_playerHitBox (melee) was just computed above,
    // m_beamHitBox (beam) was computed earlier this frame inside handlePhysicAndInput->tryBeam
    // the hit that would land now is combo hit #n; every COMBO_FINISHER_EVERY-th one shoves harder
    const uint32_t comboNow = SDL_GetTicks();
    const int comboIfHit = (comboNow - m_lastComboHit > COMBO_WINDOW_MS) ? 1 : m_combo + 1;
    const float knockMul = (comboIfHit % COMBO_FINISHER_EVERY == 0) ? 2.0f : 1.0f;

    int enemiesKilled = m_enemyBuilder.applyPlayerDamage(m_playerHitBox, m_isAttacking,
                                                         m_beamHitBox, m_isBeaming, dt, m_attackId, knockMul);
    if(enemiesKilled > 0){
        PlayerDetail::getInstance().addScore(enemiesKilled * ENEMY_KILL_SCORE);
    }
    flushEnemyHitEffects();

    // Sword hits something solid (not an enemy): a hit spark whose look depends on WHAT it hit -
    // wall/ground/platform = warm sparks flung back, object (block/trap) = cold metallic sparks.
    // Once per swing, skipped if the swing already hit an enemy.
    if(m_isAttacking && m_tempInc >= 2 && m_wallHitId != m_attackId && m_enemyHitId != m_attackId){
        const float originX = m_isPlayerfacingRight ? (m_player.x + m_player.w) : m_player.x;
        int kind = 0;
        const float dist = probeSolidAhead(originX, m_playerHitBox.y, m_playerHitBox.h, m_isPlayerfacingRight, kind);
        if(dist <= m_playerHitBox.w){
            m_wallHitId = m_attackId;
            const float contactX = m_isPlayerfacingRight ? originX + dist : originX - dist;
            const float contactY = m_playerHitBox.y + m_playerHitBox.h * 0.5f;
            const float backDir = m_isPlayerfacingRight ? -1.0f : 1.0f;     // sparks bounce back at the player
            if(kind == 1) m_fxParticles.emitObjectSpark(contactX, contactY, backDir);
            else          m_fxParticles.emitWallSpark(contactX, contactY, backDir);
            Camera::getInstance().addTrauma(0.08f);
            requestHitStop(0.03f);
            applyRecoil(backDir, 300.0f);
            Engine::Get().getAssetManager().playSound(SoundType::SFX_ATTACK_SLASH_COLLISION);
        }
    }

    // beam: constant light rumble + sparks where it ends against a solid
    if(m_isBeaming && m_beamHitBox.w > 0.0f){
        Camera::getInstance().rumble(0.08f);
        m_beamFxTimer -= dt;
        if(m_beamFxTimer <= 0.0f){
            m_beamFxTimer = 0.06f;
            const float endX = m_isPlayerfacingRight ? (m_beamHitBox.x + m_beamHitBox.w) : m_beamHitBox.x;
            m_fxParticles.emitSparkBurst(endX, m_beamHitBox.y + m_beamHitBox.h * 0.5f,
                                         m_isPlayerfacingRight ? -1.0f : 1.0f, 4, 0.9f,
                                         250.0f, 650.0f, 40.0f, 80.0f, 140, 220, 255, 70.0f);
        }
    } else {
        m_beamFxTimer = 0.0f;
    }

    TrapType type;
    gameMath::collisionSide side =gameMath::collisionSide::NONE;

    bool hazardColl =m_trapBuilder.checkHazard(m_player.x,m_player.y,
                                               m_player.w,m_player.h,type ,side);


    unsigned int hitNow =SDL_GetTicks();

    const bool canBeHit = hitNow-PlayerDetail::getInstance().getLastHitTime() >m_invincibilityTimer;
    if(canBeHit)
        PlayerDetail::getInstance().setInvincibility(false);

    // Always run the enemy check, even while invincible: a swing that overlaps the player during
    // i-frames is spent (attackHasHit) instead of waiting and landing the moment i-frames end.
    gameMath::collisionSide enemySide = gameMath::collisionSide::NONE;
    bool enemyHit = m_enemyBuilder.checkAttackOnPlayer(m_player.x, m_player.y, m_player.w, m_player.h,
                                                       hitNow, canBeHit && !hazardColl, enemySide);
    if(canBeHit){
        if(hazardColl)
        {
            handlePlayerHit(type, side, hitNow);
        }
        else if(enemyHit)
        {
            // reuses the same hp/i-frame logic as a trap hit, but with a lighter knockback
            handlePlayerHit(TrapType::NONE, enemySide, hitNow,
                            PLAYER_ENEMY_KNOCKBACK_SPEED, PLAYER_ENEMY_KNOCKBACK_MS);
        }
    }


    int score = m_fruitBuilder.onCollision(m_player.x,m_player.y,m_player.w,m_player.h);
    PlayerDetail::getInstance().addScore(score);


    //camera
    Camera::getInstance().lockCameraOn(m_player.x,m_player.y,m_player.h,m_player.w);

    SDL_FRect cameraBounds{m_wallCollisionRect.x,m_wallCollisionRect.y,m_wallCollisionRect.w,m_wallCollisionRect.h+TILE_SIZE*SCALE};
    Camera::getInstance().cameraClamp(cameraBounds);
    m_fxParticles.update(dt);            // scaled dt: sparks hang in the air during slow motion
    updateRealtimeFx(realDt);
    if(m_combo > 0 && SDL_GetTicks() - m_lastComboHit > COMBO_WINDOW_MS) m_combo = 0;

    float Force = m_trapBuilder.checkFanForce(m_player.x,m_player.y,m_player.w,m_player.h,m_particleSystem);
    if(Force <0)
    {
        m_velocityY=0.0f;
        m_velocityY += Force;
        m_isGrounded = false;
    }


    if(m_isGrounded){
        m_jumpsUsed = 0;
    }
    else if(m_jumpsUsed == 0 && m_coyoteTimer <= 0.0f){
        m_jumpsUsed = 1;   // coyote window over: the first jump is gone, only the air jump is left
    }

    if(gameMath::checkcollision(m_player.x,m_player.y,m_checkPoint.x,m_checkPoint.y,
                                m_player.h,m_player.w,m_checkPoint.h,m_checkPoint.w)){
        triggerCheckpoint();

        if(!m_isCompleted)
            m_lastTransitionTime = SDL_GetTicks();
        m_isCompleted =true;
    }

    if(m_isCompleted && SDL_GetTicks() - m_lastTransitionTime>m_transitionDelay){
        if(m_levelTransitioning)return;
        m_levelTransitioning =true;

        Engine::Get().popOverlayState();
        Engine::Get().popOverlayState();
        if(GameData::getInstance().isDebugEnabled())
            Engine::Get().popOverlayState();
        InputDispatcher::getInstance().inputLogClear();
        Engine::Get().pushOverlayState(std::make_unique<TransitionState>(m_renderer,m_level+1));
    }

    updateAnimation();

    m_particleSystem.update(dt);
    m_ambientParticles.update(dt);
    m_fruitBuilder.update(dt);
    m_trapBuilder.update(dt);

}

bool GameState::handleEvents(SDL_Event& event) {
//    if (m_transitioning)return true;

    if(InputUtils::IsPointerDown(event)){
        float px = 0.f, py = 0.f;
        InputUtils::GetPointerPosition(event, m_renderer, px, py);
        const int camX = (int)std::round(Camera::getInstance().getCamera().x);
        const int camY = (int)std::round(Camera::getInstance().getCamera().y);
        if(m_interactableBuilder.onPointerDown(px, py, camX, camY)) return true;
    }

    if(InputUtils::IsBackKey(event)) {
        InputDispatcher::getInstance().setInputReleased(true);
        InputDispatcher::getInstance().inputLogClear();
        GameData::getInstance().setPaused(true);
        m_transitioning = true;
        Engine::Get().popOverlayState();//controls
        Engine::Get().popOverlayState();//HUD
        if (GameData::getInstance().isDebugEnabled())
            Engine::Get().popOverlayState();
        Engine::Get().pushState(std::make_unique<PauseState>(m_renderer, this));
        return true;
    }

    return false;
}

void GameState::handleCollision() {
    //Walls

    const float renderedHeight = (std::ceil(m_wallCollisionRect.h / (SCALE*TILE_SIZE)) *(SCALE *TILE_SIZE));
    gameMath::collisionSide wallCollisionSide;

    wallCollisionSide = gameMath::checkcollisionXY(m_player.x,m_player.y,m_wallCollisionRect.x,
                                                   m_wallCollisionRect.y,
                                                   m_player.h,m_player.w,
                                                   TILE_SIZE * SCALE,m_wallCollisionRect.w);
    if(wallCollisionSide == gameMath::collisionSide::BOTTOM)
        m_velocityY =0.0f;
    wallCollisionSide =gameMath::checkcollisionXY(m_player.x, m_player.y, m_wallCollisionRect.x,
                                                  m_wallCollisionRect.y,
                                                  m_player.h, m_player.w, renderedHeight,
                                                  TILE_SIZE * SCALE);
    if(wallCollisionSide == gameMath::collisionSide::BOTTOM)
        m_velocityY =0.0f;
    wallCollisionSide =gameMath::checkcollisionXY(m_player.x, m_player.y,
                                                  m_wallCollisionRect.x + m_wallCollisionRect.w -
                                                  TILE_SIZE * SCALE, m_wallCollisionRect.y,
                                                  m_player.h, m_player.w, renderedHeight,
                                                  TILE_SIZE * SCALE);
    if(wallCollisionSide == gameMath::collisionSide::BOTTOM)
        m_velocityY =0.0f;
    wallCollisionSide =gameMath::checkcollisionXY(m_player.x, m_player.y, m_wallCollisionRect.x,
                                                  m_wallCollisionRect.y + renderedHeight -
                                                  TILE_SIZE * SCALE,
                                                  m_player.h, m_player.w, TILE_SIZE * SCALE,
                                                  m_wallCollisionRect.w);
    if (wallCollisionSide == gameMath::collisionSide::BOTTOM)
        m_velocityY = 0.0f;
    if (wallCollisionSide == gameMath::collisionSide::TOP) {
        m_isGrounded = true;
        m_velocityY = 0.0f;
    }


    //platforms
    for(const auto& platform : m_platforms) {
        if (platform.colliderType == ColliderType::SOLID) {

            gameMath::collisionSide side = gameMath::checkcollisionXY(m_player.x, m_player.y,
                                                                      platform.x,
                                                                      platform.y,
                                                                      m_player.h, m_player.w,
                                                                      platform.h * SCALE,
                                                                      platform.w * SCALE);
            if (side == gameMath::collisionSide::TOP) {

                m_velocityY = 0.0f;
                m_isGrounded =true;
            }
            if (side == gameMath::collisionSide::BOTTOM) {
                m_velocityY = 0.0f;
            }
        } else if (platform.colliderType == ColliderType::ONE_WAY) {
            float previousBottom = m_previousY + m_player.h;
            float currentBottom = m_player.y + m_player.h;
            float platformTop = platform.y;

            if (m_velocityY > 0 && previousBottom <= platformTop
                && currentBottom >= platformTop
                && gameMath::checkcollisionX(m_player.x, m_player.y, platform.x,
                                             platform.y,
                                             m_player.h, m_player.w, platform.h * SCALE,
                                             platform.w * SCALE)) {

                m_player.y = platformTop - m_player.h;
                m_velocityY = 0.0f;
                m_isGrounded = true;
            }

        }

    }

    //ground
    for(const auto& ground : m_grounds){
        gameMath::collisionSide groundCollisionSide=
                gameMath::checkcollisionXY(m_player.x,m_player.y,ground.x,ground.y,
                                           m_player.h,m_player.w,ground.h*SCALE,ground.w*SCALE);

        if (groundCollisionSide == gameMath::collisionSide::TOP) {
            m_isGrounded = true;
            m_velocityY = 0.0f;
        }

        if(groundCollisionSide == gameMath::collisionSide::BOTTOM){
            m_velocityY =0.0f;
        }
    }

    for(const auto& block : m_blocks){
        gameMath::collisionSide blockCollisionSide=
                gameMath::checkcollisionXY(m_player.x,m_player.y,block.rect.x,block.rect.y,
                                           m_player.h,m_player.w,block.rect.h,block.rect.w);
        if (blockCollisionSide == gameMath::collisionSide::TOP) {
            m_isGrounded = true;
            m_velocityY = 0.0f;
        }
        if(blockCollisionSide == gameMath::collisionSide::BOTTOM){
            m_velocityY =0.0f;
        }
    }
}

// poof where a finished enemy vanished
void GameState::flushEnemyDeathEffects() {
    for(const SDL_FPoint& p : m_enemyBuilder.consumeDeathEvents()){
        m_particleSystem.emitBurst(p.x, p.y, 14, 120.0f, 340.0f, 0.35f, 0.60f, 46.0f, 255, 240, 200);
    }
}

void GameState::applyRecoil(float dirX, float speed) {
    const uint32_t now = SDL_GetTicks();
    if(now < m_knockbackEndTime && now >= m_recoilEndTime) return;   // real hurt-knockback in progress: don't override
    m_velocityX = dirX * speed;
    m_recoilEndTime = now + 120;
    m_knockbackEndTime = now + 120;      // also stops the "no input -> velocityX = 0" idle reset
}

// Impact frame: the same hit spark art as on enemies but ~2x bigger, a radial burst of sharp streaks,
// and a white full-screen flash that lands on the first frame of the hit-stop.
void GameState::playerImpactEffect(float dirX) {
    const float cx = m_player.x + m_player.w * 0.5f;
    const float cy = m_player.y + m_player.h * 0.5f;
    m_fxParticles.emitHitSpark(cx, cy, dirX, 10, 2.0f);       // the enemy hit-spark art, doubled in size
    m_fxParticles.emitDamageBurst(cx, cy, dirX);               // shock rings, streaks, falling shards, embers
    m_impactFlash = 0.10f;                                     // white impact frame
    m_hurtVignette = 0.9f;                                     // red screen-edge pulse
    m_playerFlashEnd = SDL_GetTicks() + 220;                   // player sprite flashes white
}

void GameState::triggerSlowMo(float seconds, float minScale) {
    m_slowMoTotal = seconds;
    m_slowMoLeft = seconds;
    m_slowMoMin = minScale;
}

void GameState::updateRealtimeFx(float realDt) {
    Camera::getInstance().updateShake(realDt);
    m_comboPop = std::max(0.0f, m_comboPop - realDt * 6.0f);
    m_impactFlash = std::max(0.0f, m_impactFlash - realDt);
    m_hurtVignette = std::max(0.0f, m_hurtVignette - realDt);
    updateHurtAudio();
}

void GameState::triggerHurtAudio(float intensity, unsigned int ms, bool heartbeat) {
    m_audioFxStart = SDL_GetTicks();
    m_audioFxMs = ms;
    m_audioFxIntensity = intensity;
    m_audioFxHeartbeat = heartbeat;
    m_audioFxActive = true;
    Engine::Get().getAssetManager().playBoom(0.5f + 0.5f * intensity);   // the big low thud on impact
    updateHurtAudio();
}

// Envelope (Hollow Knight feel):
//   0 - 0.25s      : fully muffled + slowed ("stunned")
//   then           : settles into a heavy, slowed, half-muffled world with the heartbeat running
//   last 0.7s      : everything eases back to normal, exactly when the invincibility ends
void GameState::updateHurtAudio() {
    if(!m_audioFxActive) return;
    AssetManager& am = Engine::Get().getAssetManager();
    const float elapsed = (float)(SDL_GetTicks() - m_audioFxStart);
    const float total = (float)m_audioFxMs;
    if(elapsed >= total){
        m_audioFxActive = false;
        am.setHearingDamage(0.0f);
        am.setHeartbeat(0.0f);
        return;
    }
    const float HOLD = 250.0f, SETTLE = 500.0f, RELEASE = 700.0f;
    const float SUSTAIN_MUFFLE = 0.55f;                                  // muffle level while sustained (no pitch/speed change on purpose)
    float level = 1.0f;                                                // 1 = full "stunned", blends down to sustain
    float envelope = 1.0f;                                             // 1 during the effect, ->0 on release
    if(elapsed > HOLD){
        const float t = std::min(1.0f, (elapsed - HOLD) / SETTLE);
        level = 1.0f - t * (1.0f - SUSTAIN_MUFFLE);
    }
    const float releaseStart = std::max(0.0f, total - RELEASE);
    if(elapsed > releaseStart){
        const float t = (elapsed - releaseStart) / std::max(1.0f, total - releaseStart);
        envelope = (1.0f - t) * (1.0f - t);
    }
    const float muffle = level * envelope * m_audioFxIntensity;
    am.setHearingDamage(muffle);
    // heartbeat only once the initial hit has passed, fading out with the release
    am.setHeartbeat((m_audioFxHeartbeat && elapsed > HOLD) ? envelope : 0.0f);
}

void GameState::requestHitStop(float seconds) {
    if(seconds > m_hitStop) m_hitStop = seconds;
}

void GameState::flushEnemyHitEffects() {
    for(const EnemyHitEvent& e : m_enemyBuilder.consumeHitEvents()){
        onEnemyHit(e);
    }
}

// One landed player hit. Melee hits build the combo and escalate sparks / shake / hit-stop with it;
// beam ticks only spark + rumble (they fire ~11x/s and must not count as combo hits).
void GameState::onEnemyHit(const EnemyHitEvent& e) {
    Camera& cam = Camera::getInstance();

    if(e.isBeam){
        m_fxParticles.emitSparkBurst(e.x, e.y, e.dirX, 5, 0.9f, 300.0f, 800.0f, 50.0f, 90.0f,
                                     120, 230, 255, 90.0f);
        cam.rumble(0.10f);
        return;
    }

    const uint32_t now = SDL_GetTicks();
    if(now - m_lastComboHit > COMBO_WINDOW_MS) m_combo = 0;
    applyRecoil(m_isPlayerfacingRight ? -1.0f : 1.0f, (m_combo + 1) % COMBO_FINISHER_EVERY == 0 ? 450.0f : 300.0f);
    m_combo++;
    m_lastComboHit = now;
    m_comboPop = 1.0f;
    m_enemyHitId = m_attackId;

    const float k = std::min(m_combo, 10) / 10.0f;          // 0.1 .. 1.0

    m_fxParticles.emitHitSpark(e.x, e.y, e.dirX, m_combo);
    Engine::Get().getAssetManager().playSound(SoundType::SFX_ATTACK_SLASH_HIT);
    cam.kick(e.dirX * (6.0f + 8.0f * k));                   // view lurches the way the enemy is shoved
    cam.addTrauma(0.15f + 0.25f * k);
    float stop = e.killed ? 0.10f : 0.045f + 0.02f * k;

    if(m_combo % COMBO_FINISHER_EVERY == 0){                // finisher beat
        cam.addTrauma(0.35f);
        stop = std::max(stop, 0.09f);
        // (the finisher ring burst art is played from ParticleSystem::emitHitSpark on every 5th hit)
    }
    requestHitStop(stop);
}

// "x7" counter, top-right, pops on every hit and fades out when the combo window is about to close
void GameState::renderComboCounter(SDL_Renderer* renderer) {
    if(m_combo < 2 || m_isDead || !m_font) return;
    const uint32_t age = SDL_GetTicks() - m_lastComboHit;
    if(age > COMBO_WINDOW_MS) return;

    if(!m_comboTexture || m_comboTexValue != m_combo){
        if(m_comboTexture){ SDL_DestroyTexture(m_comboTexture); m_comboTexture = nullptr; }
        char buf[16];
        SDL_snprintf(buf, sizeof(buf), "x%d", m_combo);
        SDL_Surface* s = TTF_RenderText_Solid(m_font, buf, SDL_strlen(buf), {255,255,255,255});
        if(!s) return;
        m_comboTexture = SDL_CreateTextureFromSurface(renderer, s);
        m_comboTexW = (float)s->w;
        m_comboTexH = (float)s->h;
        SDL_DestroySurface(s);
        m_comboTexValue = m_combo;
        if(m_comboTexture) SDL_SetTextureScaleMode(m_comboTexture, SDL_SCALEMODE_NEAREST);
    }
    if(!m_comboTexture) return;

    const int c = std::min(m_combo, 10);
    SDL_SetTextureColorMod(m_comboTexture, 255, (Uint8)std::max(120, 255 - c * 14), (Uint8)std::max(40, 220 - c * 18));
    const unsigned int fadeMs = 300;
    float alpha = 1.0f;
    if(age > COMBO_WINDOW_MS - fadeMs) alpha = (float)(COMBO_WINDOW_MS - age) / (float)fadeMs;
    SDL_SetTextureAlphaMod(m_comboTexture, (Uint8)(alpha * 255.0f));

    const float scale = 1.8f + 0.7f * m_comboPop;
    const float w = m_comboTexW * scale, h = m_comboTexH * scale;
    const float screenW = Camera::getInstance().getCamera().w;
    SDL_FRect dst{screenW - 60.0f - w, 100.0f, w, h};
    SDL_RenderTexture(renderer, m_comboTexture, nullptr, &dst);
}

void GameState::startPlayerDeath() {
    Camera::getInstance().addTrauma(0.7f);
    m_slowMoLeft = 0.0f; m_timeScale = 1.0f;
    triggerHurtAudio(1.0f, 2400, false);
    m_hitStop = 0.0f;
    m_combo = 0;
    m_isDead = true;
    m_deathPopped = false;
    m_deathTimer = 0.0f;
    m_deathDir = m_isPlayerfacingRight ? -1.0f : 1.0f;   // tumble backwards

    // cancel everything the player was in the middle of
    m_isAttacking = false;
    m_tempInc = 0;
    m_isBeaming = false;
    m_beamHitBox = {0.0f,0.0f,0.0f,0.0f};
    m_playerHitBox = {0.0f,0.0f,0.0f,0.0f};
    m_isDoubleJumping = false;
    m_doubleJumpFrame = 0;
    m_velocityX = 0.0f;
    m_velocityY = 0.0f;
    m_knockbackEndTime = 0;
}

void GameState::updateDeath(float dt) {
    m_deathTimer += dt;
    m_fxParticles.update(dt);
    updateRealtimeFx(dt);

    // swallow buffered presses so they don't fire the instant the player respawns
    InputDispatcher::getInstance().consumeJumpPressed();
    InputDispatcher::getInstance().consumeAttack();
    InputDispatcher::getInstance().consumeHeal();

    if(m_deathTimer >= PLAYER_DEATH_FREEZE_S){
        if(!m_deathPopped){
            m_deathPopped = true;
            m_velocityY = -PLAYER_DEATH_POP_SPEED;
            float cx = m_player.x + m_player.w * 0.5f, cy = m_player.y + m_player.h * 0.5f;
            uint8_t r = 255, g = 255, b = 255;
            switch(PlayerDetail::getInstance().getPlayerSkin()){
                case PlayerSkin::YELLOW: r = 255; g = 220; b = 90;  break;
                case PlayerSkin::BLUE:   r = 90;  g = 170; b = 255; break;
                case PlayerSkin::RED:    r = 255; g = 90;  b = 90;  break;
                case PlayerSkin::GREEN:  r = 110; g = 230; b = 120; break;
            }
            m_particleSystem.emitBurst(cx, cy, 22, 180.0f, 480.0f, 0.45f, 0.80f, 56.0f, r, g, b);
        }
        // no collision: the body falls straight through the floor and off the bottom of the screen
        m_velocityY += m_gravity * dt;
        m_player.y += m_velocityY * dt;
    }

    // the world keeps running. Enemies are handed a far-away "player" so they drop aggro and walk
    // home instead of attacking the body.
    m_trapBuilder.updatePath(dt);
    SDL_FRect farAway{-100000.0f, -100000.0f, m_player.w, m_player.h};
    m_enemyBuilder.update(dt, farAway, m_wallCollisionRect, m_grounds, m_platforms, m_blocks);
    flushEnemyDeathEffects();

    m_particleSystem.update(dt);
    m_ambientParticles.update(dt);
    m_fruitBuilder.update(dt);
    m_trapBuilder.update(dt);

    if(m_deathTimer >= PLAYER_DEATH_TOTAL_S) openRespawnScreen();
}

void GameState::openRespawnScreen() {
    if(m_respawnScreenOpen) return;
    m_respawnScreenOpen = true;
    // same hand-over as the pause menu: release input, drop the gameplay overlays, put a menu state on top
    InputDispatcher::getInstance().setInputReleased(true);
    InputDispatcher::getInstance().inputLogClear();
    GameData::getInstance().setPaused(true);
    Engine::Get().popOverlayState();//controls
    Engine::Get().popOverlayState();//HUD
    if (GameData::getInstance().isDebugEnabled())
        Engine::Get().popOverlayState();
    Engine::Get().pushState(std::make_unique<RespawnState>(m_renderer, this, hasReachedCheckpoint()));
}

void GameState::completeRespawn() {
    m_respawnScreenOpen = false;
    respawnPlayer();
}

void GameState::respawnPlayer() {
    m_combo = 0;
    m_slowMoLeft = 0.0f; m_timeScale = 1.0f;
    m_isDead = false;
    m_deathTimer = 0.0f;
    m_deathPopped = false;

    PlayerDetail::getInstance().setPlayerHP(5);
    if(hasReachedCheckpoint()){
        // stand on the checkpoint that was reached
        m_player.setPosition(m_checkPoint.x + m_checkPoint.w * 0.5f - m_player.w * 0.5f,
                             m_checkPoint.y + m_checkPoint.h - m_player.h - 6.0f, P_scale);
    }else{
        m_player.setPosition(m_respawnX, m_respawnY, P_scale);
    }
    m_coyoteTimer = 0.0f;
    m_jumpBufferTimer = 0.0f;

    m_velocityX = 0.0f;
    m_velocityY = 0.0f;
    m_isGrounded = false;
    m_jumpsUsed = 0;
    m_playerAction = PlayerAction::IDLE;
    m_hurtAnimEndTime = 0;
    m_knockbackEndTime = 0;
}

void GameState::handlePlayerHit(TrapType hazardType, gameMath::collisionSide side, unsigned int now,
                                float knockbackStrength, unsigned int knockbackDurationMs) {

    PlayerDetail::getInstance().setLastHitTime(now);

    PlayerDetail::getInstance().subPlayerHP(1);

    PlayerDetail::getInstance().setInvincibility(true);
    m_hurtAnimEndTime = now + HURT_ANIM_MS;
    m_knockbackEndTime = now + knockbackDurationMs;
    const float KNOCK_H = knockbackStrength, KNOCK_V = knockbackStrength;
    switch(side){
        case gameMath::collisionSide::TOP:    m_velocityY = -KNOCK_V; break; // pushed up
        case gameMath::collisionSide::BOTTOM: m_velocityY =  KNOCK_V; break; // pushed down
        case gameMath::collisionSide::LEFT:   m_velocityX = -KNOCK_H; break; // pushed left
        case gameMath::collisionSide::RIGHT:  m_velocityX =  KNOCK_H; break; // pushed right
        default: break;
    }
    m_isGrounded = false;

    // feedback: shake (+ a kick the way the player is shoved), brief freeze, combo is broken
    Camera::getInstance().addTrauma(0.6f);
    Camera::getInstance().kick(side == gameMath::collisionSide::LEFT ? -12.0f :
                               side == gameMath::collisionSide::RIGHT ? 12.0f : 0.0f);
    requestHitStop(0.12f);
    m_combo = 0;
    playerImpactEffect(side == gameMath::collisionSide::LEFT ? -1.0f : 1.0f);
    triggerSlowMo(1.1f, 0.30f);        // after the 0.12s freeze: ~1s of slow motion easing back to normal
    triggerHurtAudio(0.9f, (unsigned int)m_invincibilityTimer, true);   // lasts exactly as long as the invincibility
}

void GameState::triggerCheckpoint(){
    if(m_checkPoint.aniType == CheckPointAni::FLAG_OUT)return;
    m_checkPoint.aniType = m_checkPoint.aniType == CheckPointAni::NO_FLAG ? CheckPointAni::FLAG_OUT:CheckPointAni::FLAG_IDLE;
}

bool GameState::tryHeal(unsigned int now) {
    if(now < m_healCooldownEndTime) return false;
    if(PlayerDetail::getInstance().getMagic() < HEAL_MAGIC_COST) return false;
    if(PlayerDetail::getInstance().getPlayerHP() >= 5) return false;

    PlayerDetail::getInstance().subMagic(HEAL_MAGIC_COST);
    PlayerDetail::getInstance().addPlayerHP(HEAL_AMOUNT);
    m_healCooldownEndTime = now + HEAL_COOLDOWN_MS;
    return true;
}

namespace {
    inline bool verticalOverlap(float y1, float h1, float y2, float h2) {
        return y1 < y2 + h2 && y1 + h1 > y2;
    }
}

float GameState::computeBeamLength(float originX, float beamY, float beamH, bool facingRight) {

    const float leftWall  = m_wallCollisionRect.x + TILE_SIZE * SCALE;
    const float rightWall = m_wallCollisionRect.x + m_wallCollisionRect.w - TILE_SIZE * SCALE;

    float maxLength = facingRight ? (rightWall - originX) : (originX - leftWall);
    if(maxLength < 0.0f) maxLength = 0.0f;

    auto clip = [&](float solidX, float solidW, float solidY, float solidH){
        if(!verticalOverlap(beamY, beamH, solidY, solidH)) return;
        if(facingRight){
            if(solidX >= originX){
                float dist = solidX - originX;
                if(dist < maxLength) maxLength = dist;
            }
        } else {
            float solidRight = solidX + solidW;
            if(solidRight <= originX){
                float dist = originX - solidRight;
                if(dist < maxLength) maxLength = dist;
            }
        }
    };

    for(const auto& ground : m_grounds){
        clip(ground.x, ground.w * SCALE, ground.y, ground.h * SCALE);
    }
    for(const auto& block : m_blocks){
        clip(block.rect.x, block.rect.w, block.rect.y, block.rect.h);
    }
    for(const auto& platform : m_platforms){
        if(platform.colliderType != ColliderType::SOLID) continue;
        clip(platform.x, platform.w * SCALE, platform.y, platform.h * SCALE);
    }

    return maxLength;
}

float GameState::probeSolidAhead(float originX, float y, float h, bool facingRight, int& kind) {
    const float leftWall  = m_wallCollisionRect.x + TILE_SIZE * SCALE;
    const float rightWall = m_wallCollisionRect.x + m_wallCollisionRect.w - TILE_SIZE * SCALE;
    float best = facingRight ? (rightWall - originX) : (originX - leftWall);
    if(best < 0.0f) best = 0.0f;
    kind = 0;

    auto clip = [&](float sx, float sw, float sy, float sh, int k){
        if(!verticalOverlap(y, h, sy, sh)) return;
        float dist;
        if(facingRight){ if(sx < originX) return; dist = sx - originX; }
        else { const float sr = sx + sw; if(sr > originX) return; dist = originX - sr; }
        if(dist < best){ best = dist; kind = k; }
    };
    for(const auto& ground : m_grounds)  clip(ground.x, ground.w * SCALE, ground.y, ground.h * SCALE, 0);
    for(const auto& platform : m_platforms){
        // solid AND one-way platforms: the sword hits them either way (one-way is only passable for the player)
        if(platform.colliderType != ColliderType::SOLID && platform.colliderType != ColliderType::ONE_WAY) continue;
        clip(platform.x, platform.w * SCALE, platform.y, platform.h * SCALE, 0);
    }
    for(const auto& block : m_blocks)    clip(block.rect.x, block.rect.w, block.rect.y, block.rect.h, 1);
    // use the TrapBuilder's own traps: they hold the live (moving) positions, GameState::m_traps is a stale copy
    const std::vector<Trap>& liveTraps = m_trapBuilder.getTraps();
    for(int i = 0; i < (int)liveTraps.size(); i++){
        // any trap that is solid by type (moving/falling platform, rock head, fire) OR set to a SOLID / ONE_WAY collider
        const Trap& t = liveTraps[i];
        const bool hittable = m_trapBuilder.isSolid(i) || t.colliderType == ColliderType::SOLID ||
                              t.colliderType == ColliderType::ONE_WAY;
        if(!hittable) continue;
        SDL_FRect box = m_trapBuilder.getTrapCollisionBox(t);
        clip(box.x, box.w, box.y, box.h, 1);
    }
    return best;
}

void GameState::tryBeam(float dt) {
    float drain = BEAM_MAGIC_DRAIN_PER_SEC * dt;
    if(PlayerDetail::getInstance().getMagic() < drain){
        // ran out mid-channel - force it to stop
        m_isBeaming = false;
        InputDispatcher::getInstance().setBeaming(false);
        m_beamHitBox = {0.0f,0.0f,0.0f,0.0f};
        return;
    }
    PlayerDetail::getInstance().subMagic(drain);
    m_isBeaming = true;

    float beamY = m_player.y;
    float beamH = m_player.h;
    float originX = m_isPlayerfacingRight ? (m_player.x + m_player.w) : m_player.x;

    float length = computeBeamLength(originX, beamY, beamH, m_isPlayerfacingRight);

    m_beamHitBox.y = beamY;
    m_beamHitBox.h = beamH;
    m_beamHitBox.w = length;
    m_beamHitBox.x = m_isPlayerfacingRight ? originX : (originX - length);

    // TODO: no beam sprite yet - resolve damage against m_enemies here once
    // enemy hp hookup exists.
}

void GameState::updateAnimation() {
    if(SDL_GetTicks() < m_hurtAnimEndTime) m_playerAction = PlayerAction::HURT;
    switch(m_playerAction){
        case IDLE:
            m_Animation.startIndex=0;
            m_Animation.lastIndex =3;
            break;
        case MOVINGLEFT:
        case MOVINGRIGHT:
            m_Animation.startIndex=4;
            m_Animation.lastIndex =10;
            break;
        case JUMP:
            m_Animation.startIndex=11;
            m_Animation.lastIndex =13;
            break;
        case HURT:
            m_Animation.startIndex=14;
            m_Animation.lastIndex =16;
            break;
        case CROUCHING:
            m_Animation.startIndex=17;
            m_Animation.lastIndex =23;
            break;

    }

    m_aniNowTime = SDL_GetTicks();
    if(m_aniNowTime - m_aniLastTime > m_aniframeDelay){
        if(m_isAttacking){
            m_tempInc++;
            if(m_tempInc >4) {
                m_tempInc = 0;
                m_isAttacking =false;
            }

        }

        if(m_isDoubleJumping){
            m_doubleJumpFrame++;
            if(m_doubleJumpFrame > DOUBLE_JUMP_LAST_FRAME){
                m_doubleJumpFrame = 0;
                m_isDoubleJumping = false;
            }
        }

        if(m_currentFrame < m_Animation.startIndex)
            m_currentFrame = m_Animation.startIndex;
        if(m_currentFrame < m_Animation.lastIndex)
            m_currentFrame++;
        else
            m_currentFrame = m_Animation.startIndex;
        m_aniLastTime = m_aniNowTime;
    }


    switch(m_checkPoint.aniType){
        case CheckPointAni::NO_FLAG:
            m_checkPoint.ani.startIndex = 0;
            m_checkPoint.ani.lastIndex = 0;
            break;
        case CheckPointAni::FLAG_OUT:
            m_checkPoint.ani.startIndex =0;
            m_checkPoint.ani.lastIndex =24;
            break;
        case CheckPointAni::FLAG_IDLE:
            m_checkPoint.ani.startIndex =0;
            m_checkPoint.ani.lastIndex =9;
    }
    uint32_t nowTime = SDL_GetTicks();
    if(nowTime - m_checkPoint.aniLastTime > m_aniframeDelay){
        m_checkPoint.aniLastTime = nowTime;

        if(m_checkPoint.currentFrame < m_checkPoint.ani.startIndex)
            m_checkPoint.currentFrame = m_checkPoint.ani.startIndex;

        if(m_checkPoint.currentFrame < m_checkPoint.ani.lastIndex){
            m_checkPoint.currentFrame++;
        }
        else{
            if(m_checkPoint.aniType == CheckPointAni::FLAG_OUT){
                m_checkPoint.aniType = CheckPointAni::FLAG_IDLE;
                m_checkPoint.currentFrame = 0;
            }
            else if(m_checkPoint.aniType == CheckPointAni::FLAG_IDLE){
                m_checkPoint.currentFrame = m_checkPoint.ani.startIndex;
            }
        }
    }

}

void GameState::handlePhysicAndInput(float dt) {



    if(m_wasGrounded && m_isGrounded){
        m_particleSystem.emitLandDust(m_player.x+20.00f, m_player.y+m_player.h-50.00f,
                                      m_player.x-40.00f, m_player.y+m_player.h-50.00f);
        m_wasGrounded =false;
    }
    bool inKnockback = SDL_GetTicks() < m_knockbackEndTime;
    const bool inRecoil = SDL_GetTicks() < m_recoilEndTime;
    if(inRecoil) m_velocityX *= std::exp(-8.0f * dt);   // push-back eases out

    if(!inKnockback)
    {
        if (!InputDispatcher::getInstance().movingLeft &&
            !InputDispatcher::getInstance().movingRight) {
            m_playerAction = PlayerAction::IDLE;
            m_velocityX = 0.0f;
        }
    }

    if(InputDispatcher::getInstance().movingLeft && !inRecoil){
        m_isPlayerfacingRight =false;
        m_playerAction=PlayerAction::MOVINGLEFT;
        m_velocityX = -400.00f;
        m_walkTimer += dt;
        if(m_isGrounded){
            if(m_walkTimer >0.2){
                m_walkTimer =0.00f;
                m_particleSystem.emitLeftDust(m_player.x+20.00f, m_player.y+m_player.h-50.00f);
                m_particleSystem.emitLeftDust(m_player.x+20.00f, m_player.y+m_player.h-50.00f);
                m_particleSystem.emitLeftDust(m_player.x+20.00f, m_player.y+m_player.h-50.00f);
            }
        }
        else{
            m_walkTimer =0.00f;
        }
    }

    if(InputDispatcher::getInstance().movingRight && !inRecoil){
        m_isPlayerfacingRight =true;
        m_playerAction=PlayerAction::MOVINGRIGHT;
        m_velocityX=400.00f;

        m_walkTimer += dt;
        if(m_isGrounded){
            if(m_walkTimer >0.2){
                m_walkTimer =0.00f;
                m_particleSystem.emitRightDust(m_player.x-40.00f, m_player.y+m_player.h-50.00f);
                m_particleSystem.emitRightDust(m_player.x-40.00f, m_player.y+m_player.h-50.00f);
                m_particleSystem.emitRightDust(m_player.x-40.00f, m_player.y+m_player.h-50.00f);
            }
        }
        else{
            m_walkTimer =0.00f;
        }
    }

    // Footsteps: only while actually walking on the ground. playSoundIfIdle() ignores the call
    // while the previous step clip is still playing, so steps queue up one after another and
    // every clip plays for its full length (no per-frame spam, no cut-offs).
    if((InputDispatcher::getInstance().movingLeft || InputDispatcher::getInstance().movingRight)
       && m_isGrounded && !inKnockback){
        Engine::Get().getAssetManager().playSoundIfIdle(SoundType::SFX_FOOTSTEPS);
    }

    const bool jumpPressed = InputDispatcher::getInstance().consumeJumpPressed();

    // Coyote time: for COYOTE_TIME_S after walking off a ledge the first (full) jump still works.
    // Jump buffer: a press made up to JUMP_BUFFER_S before landing fires the moment the player touches down.
    if(m_isGrounded)               m_coyoteTimer = COYOTE_TIME_S;
    else if(m_coyoteTimer > 0.0f)  m_coyoteTimer = std::max(0.0f, m_coyoteTimer - dt);
    if(jumpPressed)                      m_jumpBufferTimer = JUMP_BUFFER_S;
    else if(m_jumpBufferTimer > 0.0f)    m_jumpBufferTimer = std::max(0.0f, m_jumpBufferTimer - dt);

    // m_jumpsUsed == 0 while airborne only happens in the coyote window (it is set to 1 once it ends,
    // and by every real jump / trampoline), so this can't be used to jump twice off one ledge.
    const bool canGroundJump = m_isGrounded || (m_coyoteTimer > 0.0f && m_jumpsUsed == 0);

    if(m_jumpBufferTimer > 0.0f && canGroundJump){
        Engine::Get().getAssetManager().playSoundIfIdle(SoundType::SFX_JUMP);
        m_playerAction=PlayerAction::JUMP;
        m_velocityY =-m_jumpVelocity;
        m_wasGrounded =true;
        m_jumpsUsed =1;
        m_jumpBufferTimer = 0.0f;
        m_coyoteTimer = 0.0f;
        m_particleSystem.emitJumpDust(m_player.x, m_player.y + m_player.h - 40.00f);
    }
    else if(jumpPressed && m_jumpsUsed < m_maxJumps){
        // Air jump - body keeps whatever pose it was already in (same idea as
        // attack not touching the base animation); the double-jump sheet is
        // drawn as its own overlay pass in render().
        // (If no air jump is left the press just stays in the buffer for a moment, see above.)
        Engine::Get().getAssetManager().playSoundIfIdle(SoundType::SFX_JUMP);
        m_velocityY = -m_jumpVelocity * 0.85f;
        m_jumpsUsed++;
        m_jumpBufferTimer = 0.0f;
        m_isDoubleJumping = true;
        m_doubleJumpFrame = 0;
        m_particleSystem.emitJumpDust(m_player.x, m_player.y + m_player.h - 40.00f);
    }

    if(InputDispatcher::getInstance().consumeAttack()){
        if(!m_isAttacking) m_attackId++;   // new swing -> new id; enemies take at most one hit per id
        m_isAttacking =true;
    }

    m_player.x +=m_velocityX * dt;
    m_velocityY+=m_gravity*dt;
    m_player.y +=m_velocityY*dt;

    const unsigned int nowTicks = SDL_GetTicks();
    if(InputDispatcher::getInstance().consumeHeal()){
        // A heal request that arrives while beaming, or right after the beam was released, is
        // the release of the beam button - not a tap. Drop it.
        constexpr unsigned int HEAL_AFTER_BEAM_LOCKOUT_MS = 250;
        bool justBeamed = m_isBeaming || (nowTicks - m_lastBeamTime) < HEAL_AFTER_BEAM_LOCKOUT_MS;
        if(!justBeamed && tryHeal(nowTicks)){
            m_particleSystem.emitHealSparkle(m_player.x + m_player.w * 0.5f, m_player.y);
        }
    }

    if(InputDispatcher::getInstance().beaming){
        m_lastBeamTime = nowTicks;
        tryBeam(dt);
    } else {
        m_isBeaming = false;
        m_beamHitBox = {0.0f,0.0f,0.0f,0.0f};
    }


}

void GameState::setLevel(int level) {

    m_blocks.clear();
    m_platforms.clear();
    m_grounds.clear();
    m_wallTiles.clear();
    m_levelWalls.clear();
    m_traps.clear();
    m_fruits.clear();
    m_enemies.clear();
    m_backgroundElements.clear();
    m_foregroundElements.clear();

    m_levelLoader.loadLevel(level);

    m_checkPoint=m_levelLoader.getCheckPoint();
    m_blocks =m_levelLoader.getBlocks();
    m_levelWalls =m_levelLoader.getLevelWalls();
    m_platforms =m_levelLoader.getPlatforms();
    m_grounds =m_levelLoader.getLevelGrounds();
    m_fruits =m_levelLoader.getFruits();
    m_traps =m_levelLoader.getTraps();
    m_enemies =m_levelLoader.getEnemies();
    m_backgroundElements = m_levelLoader.getBackGroundElements();
    m_foregroundElements = m_levelLoader.getForeGroundElements();
    m_wallCollisionRect=m_levelLoader.getWallCollisionRect();

    m_blockBuilder.init(m_blocks,TILE_SIZE,SCALE);
    m_fruitBuilder.init(m_fruits);
    m_trapBuilder.init(m_traps);
    m_enemyBuilder.init(m_enemies);
    m_interactableBuilder.init(m_levelLoader.getInteractables());
    GroundShapeBuilder builder;
    m_wallShape = builder.build(m_grounds,TILE_SIZE,(int)SCALE);
    m_backgroundBuilder.init(m_backgroundElements);
    m_foregroundBuilder.init(m_foregroundElements);
    m_ambientParticles.init(m_renderer, AmbientParticles::themeFromBackground(m_backgroundElements));
    m_lighting.init(m_renderer);
    m_lighting.setLevel(m_levelLoader.getAmbient(), m_levelLoader.getLights());
    if(m_levelLoader.hasSpawn()){
        m_respawnX = m_levelLoader.getPlayer().x;
        m_respawnY = m_levelLoader.getPlayer().y;
    }

    buildWallTiles();

}

void GameState::buildWallTiles() {
    m_wallTiles.clear();

    for(const auto& level : m_levelWalls)
    {
        int tileSize =(int)(TILE_SIZE*SCALE);
        const int platformWidth = (int)level.w;
        const int platformHeight = (int)level.h;
        int widthTiles = static_cast<int>(std::ceil((float)platformWidth  / (float)tileSize));
        int heightTiles = static_cast<int>(std::ceil((float)platformHeight / (float)tileSize));
        for (int i = 0; i < heightTiles; i++) {
            for (int j = 0; j < widthTiles; j++) {
                SDL_FRect src;
                float x =level.x + (float)j * (TILE_SIZE * SCALE);
                float y =level.y + (float)i* (TILE_SIZE * SCALE);
                bool left = (j == 0);
                bool right = (j == widthTiles - 1);
                bool top = (i == 0);
                bool bottom = (i == heightTiles - 1);
                bool edge =false;
                SpriteValue CurrentSpriteValue;
                SpriteValue CurrentEdgeSpriteValue;
                if(level.spriteType == SpriteType::STONE_BRICK_WALL) {
                    CurrentSpriteValue =SpriteCollection::stoneBrickWall;
                    CurrentEdgeSpriteValue =SpriteCollection::stoneBrickWallEdges;
                }
                else if(level.spriteType == SpriteType::MOSS_WALL) {
                    CurrentSpriteValue =SpriteCollection::mossWall;
                    CurrentEdgeSpriteValue =SpriteCollection::mossWallEdges;
                }
                if(hasWallAbove(x,y)){
                    top =false;
                }
                if(hasWallBelow(x,y)){
                    bottom =false;
                }
                if(hasWallRight(x,y)){
                    right =false;
                }
                if(hasWallLeft(x,y)){
                    left =false;
                }
                if (top) {
                    if (left) {
                        src = {CurrentEdgeSpriteValue.x,
                               CurrentEdgeSpriteValue.y, TILE_SIZE, TILE_SIZE};
                        edge =true;
                    }
                    else if (right) {
                        src = {CurrentEdgeSpriteValue.x + TILE_SIZE,
                               CurrentEdgeSpriteValue.y, TILE_SIZE, TILE_SIZE};
                        edge =true;
                    }
                    else {
                        src = {CurrentSpriteValue.x + TILE_SIZE,
                               CurrentSpriteValue.y, TILE_SIZE, TILE_SIZE};
                        edge = false;
                    }
                }
                else if (bottom) {
                    if (left) {
                        src = {CurrentEdgeSpriteValue.x,
                               CurrentEdgeSpriteValue.y+TILE_SIZE, TILE_SIZE, TILE_SIZE};
                        edge =true;
                    }
                    else if (right) {
                        src = {CurrentEdgeSpriteValue.x + TILE_SIZE,
                               CurrentEdgeSpriteValue.y+TILE_SIZE, TILE_SIZE, TILE_SIZE};
                        edge= true;
                    }
                    else {
                        src = {CurrentSpriteValue.x + TILE_SIZE,
                               CurrentSpriteValue.y+32.00f, TILE_SIZE, TILE_SIZE};
                        edge =false;
                    }
                } else {
                    edge =false;
                    if (left) src = {CurrentSpriteValue.x,
                                     CurrentEdgeSpriteValue.y+ TILE_SIZE, TILE_SIZE, TILE_SIZE};
                    else if (right)
                        src = {CurrentSpriteValue.x + 32.00f,
                               CurrentEdgeSpriteValue.y+TILE_SIZE, TILE_SIZE, TILE_SIZE};
                    else src = {CurrentSpriteValue.x + TILE_SIZE,
                                CurrentSpriteValue.y+TILE_SIZE, TILE_SIZE, TILE_SIZE};
                }

                // Mirrors the original per-frame draw condition exactly:
                // single-side tiles are drawn flipped, corner tiles drawn
                // plain, and pure-interior tiles (all four flags false) are
                // never drawn at all — so we simply don't cache an entry for them.
                if ((top || bottom || left || right) && !edge) {
                    m_wallTiles.push_back({src, x, y, /*flip=*/true});
                }
                else if ((top || bottom) && (left || right) && edge) {
                    m_wallTiles.push_back({src, x, y, /*flip=*/false});
                }
            }
        }
    }
}

bool GameState::hasWallAbove(float x, float y) {
    float checkX = x;
    float checkY = y - (TILE_SIZE * SCALE);
    for(const auto& wall : m_levelWalls){
        if(checkX >= wall.x&& checkX < wall.x + wall.w &&
           checkY >=wall.y&& checkY<wall.y+wall.h)
            return true;
    }
    return false;
}

bool GameState::hasWallBelow(float x, float y) {
    float checkX = x;
    float checkY = y + (TILE_SIZE * SCALE);
    for(const auto& wall : m_levelWalls){
        if(checkX >= wall.x&& checkX < wall.x + wall.w &&
           checkY >=wall.y&& checkY<wall.y+wall.h)
            return true;
    }
    return false;
}

bool GameState::hasWallRight(float x, float y) {
    float checkX = x+ (TILE_SIZE * SCALE);
    float checkY = y;
    for(const auto& wall : m_levelWalls){
        if(checkX >= wall.x&& checkX < wall.x + wall.w &&
           checkY >=wall.y&& checkY<wall.y+wall.h)
            return true;
    }
    return false;
}

bool GameState::hasWallLeft(float x, float y) {
    float checkX = x- (TILE_SIZE * SCALE);
    float checkY = y;
    for(const auto& wall : m_levelWalls){
        if(checkX >= wall.x&& checkX < wall.x + wall.w &&
           checkY >=wall.y&& checkY<wall.y+wall.h)
            return true;
    }
    return false;
}

bool GameState::isBlinkFrame() const {
    unsigned int elapsed = SDL_GetTicks() - PlayerDetail::getInstance().getLastHitTime();
    return (elapsed / m_blinkTimer) % 2 == 0;
}

bool SDLCALL GameState::HandleBackgroundEvents(void *userdata, SDL_Event *event) {
    auto* gameState = static_cast<GameState*>(userdata);
    if (event->type == SDL_EVENT_WILL_ENTER_BACKGROUND && !GameData::getInstance().isPaused()) {
        InputDispatcher::getInstance().setInputReleased(true);
        InputDispatcher::getInstance().inputLogClear();
        GameData::getInstance().setPaused(true);
        gameState->m_transitioning = true;
        Engine::Get().popOverlayState();
        Engine::Get().popOverlayState();
        if(GameData::getInstance().isDebugEnabled())
            Engine::Get().popOverlayState();
        Engine::Get().pushState(std::make_unique<PauseState>(gameState->m_renderer,gameState));
        return true;

    }
    else if (event->type == SDL_EVENT_DID_ENTER_BACKGROUND) {

    }
    else if (event->type == SDL_EVENT_WILL_ENTER_FOREGROUND){

    }
    return true; // Return true to keep the event in the queue for other systems
}