//
// Created by LENOVO on 06-08-2026.
//
#include <algorithm>
#include "LevelLoader.h"
#include "LevelTypeConverter.h"

void LevelLoader::parseLine(const std::string& line) {
    std::istringstream stream(line);

    char recordType;

    if (!(stream >> recordType)) {
        return; // empty or whitespace-only line, nothing to parse
    }


//    S spawn
//    W walls
//    P Platforms
//    G Grounds
//    B Blocks
//    F Fruits
//    T Traps
//    C CheckPoint
    switch(recordType){
        case 'S':
            parseSpawn(stream);
            break;
        case 'W':
            parseWalls(stream);
            break;
        case 'P':
            parsePlatforms(stream);
            break;
        case 'G':
            parseGrounds(stream);
            break;
        case 'B':
            parseBlocks(stream);
            break;
        case 'F':
            parseFruits(stream);
            break;
        case 'T':
            parseTraps(stream);
            break;
        case 'E':
            parseEnemy(stream);
            break;
        case 'I':
            parseInteractable(stream);
            break;
        case 'K':
            parseBoss(stream);
            break;
        case 'A':
            parseBossArena(stream);
            break;
        case '#':
            break;   // comment line
        case 'C':
            parseCheckPoint(stream);
            break;
        case 'R':
            parseWallCollisionRect(stream);
            break;
        case 'X':
            parseBackGround(stream);
            break;
        case 'Y':
            parseForeGround(stream);
            break;
        case 'D':
            parseAmbient(stream);
            break;
        case 'L':
            parseLight(stream);
            break;
        case 'M':
            parseMusic(stream);
            break;
        default:
            LOGI("Failed to decode:%c",recordType);
    }

}

void LevelLoader::parseSpawn(std::istringstream& ss) {

    float spawnX =0;float spawnY =0;

    ss>>spawnX>>spawnY;

    if (ss.fail()) {
        LOGE("Malformed trap line: failed parsing fixed fields");
        return;
    }

    m_player.x=spawnX;
    m_player.y=spawnY;
    m_hasSpawn = true;

}

void LevelLoader::parseWalls(std::istringstream& ss) {
    float x=0.00f,y=0.00f,w=0.00f,h=0.00f;
    std::string spriteType;
    std::string colliderType;

    ss>>x>>y>>w>>h>>spriteType>>colliderType;

    if (ss.fail()) {
        LOGE("Malformed wall line: failed parsing fixed fields");
        return;
    }

    SpriteType sprite = toSpriteType(spriteType);
    ColliderType collider = toColliderType(colliderType);
    if(sprite == SpriteType::COUNT){
        LOGI("invalid type for wall");
        return;
    }
    if(collider == ColliderType::NONE){
        LOGI("invalid type for wall");
        return;
    }
    m_levelWalls.emplace_back(x,y,w,h,sprite,collider);
}

void LevelLoader::parsePlatforms(std::istringstream& ss) {
    float x=0.00f,y=0.00f,w=0.00f,h=0.00f;
    std::string spriteType;
    std::string colliderType;

    ss>>x>>y>>w>>h>>colliderType>>spriteType;

    if (ss.fail()) {
        LOGE("Malformed platform line: failed parsing fixed fields");
        return;
    }

    SpriteType sprite = toSpriteType(spriteType);
    ColliderType collider = toColliderType(colliderType);
    if(sprite == SpriteType::COUNT){
        LOGI("invalid type for platform");
        return;
    }
    if(collider == ColliderType::NONE){
        LOGI("invalid type for platform");
        return;
    }
    m_platforms.emplace_back(x,y,w,h,collider,sprite);
}

void LevelLoader::parseGrounds(std::istringstream& ss) {
    float x=0.00f,y=0.00f,w=0.00f,h=0.00f;
    std::string typeStr;
    std::string colliderTypeStr;

    ss>>x>>y>>w>>h>>typeStr>>colliderTypeStr;

    if (ss.fail()) {
        LOGE("Malformed ground line: failed parsing fixed fields");
        return;
    }

    SpriteType sprite = toSpriteType(typeStr);
    ColliderType collider = toColliderType(colliderTypeStr);

    if(sprite == SpriteType::COUNT){
        LOGI("invalid type for ground");
        return;
    }
    if(collider == ColliderType::NONE){
        LOGI("invalid type for ground");
        return;
    }
    m_grounds.emplace_back(x,y,w,h,sprite,collider);
}

void LevelLoader::parseBlocks(std::istringstream& ss) {
    float x=0.00f,y=0.00f,w=0.00f,h=0.00f;
    std::string variantStr;
    std::string typeStr;

    ss>>x>>y>>w>>h>>variantStr>>typeStr;

    if (ss.fail()) {
        LOGE("Malformed blocks line: failed parsing fixed fields");
        return;
    }

    BlockVariant variant = toBlockVariant(variantStr);
    BlockType type = toBlockType(typeStr);
    if(variant == BlockVariant::NONE){
        LOGI("invalid type for block");
        return;
    }
    if(type == BlockType::NONE){
        LOGI("invalid type for block");
        return;
    }
    m_blocks.emplace_back(x,y,w,h,variant,type);
}

void LevelLoader::parseFruits(std::istringstream& ss) {
    float x=0.00f,y=0.00f;
    std::string typeStr;

    ss>>x>>y>>typeStr;

    if (ss.fail()) {
        LOGE("Malformed fruits line: failed parsing fixed fields");
        return;
    }

    FruitType type = toFruitType(typeStr);
    if(type == FruitType::NONE){
        LOGI("invalid type for fruit");
        return;
    }
    m_fruits.emplace_back(x,y,type);
}

void LevelLoader::parseBackGround(std::istringstream& ss){
    float x=0.00f,y=0.00f,w = 0.00f,h=0.00f;
    float parallaxFactorX=0.00f, parallaxFactorY=0.00f;
    std::string typeStr;
    bool isAnimated = false;
    ss>>x>>y>>w>>h>>parallaxFactorX>>parallaxFactorY>>std::boolalpha>>isAnimated>>typeStr;

    BackGroundType type = toBackGroundType(typeStr);
    if(type == BackGroundType::NONE){
        LOGI("invalid type for background");
        return;
    }
    m_backGroundElements.emplace_back(x,y,w,h,parallaxFactorX,parallaxFactorY,isAnimated,type);
}

void LevelLoader::parseForeGround(std::istringstream &ss) {
    float x=0.00f,y=0.00f,w = 0.00f,h=0.00f;
    float parallaxFactorX=0.00f, parallaxFactorY=0.00f;
    std::string typeStr;
    bool isAnimated = false;
    ss>>x>>y>>w>>h>>parallaxFactorX>>parallaxFactorY>>std::boolalpha>>isAnimated>>typeStr;

    ForeGroundType type = toForeGroundType(typeStr);
    if(type == ForeGroundType::NONE){
        LOGI("invalid type for background");
        return;
    }
    m_foreGroundElements.emplace_back(x,y,w,h,parallaxFactorX,parallaxFactorY,isAnimated,type);
}

void LevelLoader::parseTraps(std::istringstream& ss) {
    float x = 0.f, y = 0.f, startPath = 0.f, endPath = 0.f, speed = 0.f;
    std::string typeStr, statusStr, axisStr, shapeStr, colliderStr;

    ss >> x >> y >> typeStr >> statusStr >> startPath >> endPath >> speed >> axisStr >> shapeStr >> colliderStr;
    if (ss.fail()) {
        LOGE("Malformed trap line: failed parsing fixed fields");
        return;
    }

    TrapType type = toTrapType(typeStr);
    if (type == TrapType::NONE) {
        LOGE("Skipping trap with unknown type: %s", typeStr.c_str());
        return;
    }

    Trap trap;
    trap.x = x;
    trap.y = y;
    trap.baseX = x;//these are used as coords for starting corner of rect path must initialze them as spawn
    trap.baseY = y;
    trap.previousX = x;
    trap.previousY = y;
    trap.type = type;
    if(type == TrapType::MOVING_PLATFORM_GREY || type == TrapType::MOVING_PLATFORM_BROWN){
        LOGI("parseTraps: type=%d x=%.2f y=%.2f prevX=%.2f prevY=%.2f", (int)type, trap.x, trap.y, trap.previousX, trap.previousY);
    }
    trap.status = toTrapStatus(statusStr);
    trap.startPath = startPath;
    trap.endPath = endPath;
    trap.movingSpeed = speed;
    trap.axis = toPathAxis(axisStr);
    trap.pathShape = toPathShape(shapeStr);
    trap.colliderType = toColliderType(colliderStr);

    switch(type){
        case TrapType::SAW:
            parseSawTail(ss,trap);
            break;
        case TrapType::SPIKE_BALL:
            parseSpikeBallTail(ss,trap);
            break;
        default:
            parseGenericTail(ss,trap);
    }
    if(ss.fail()){
        LOGE("Malformed trap line: failed parsing tailing fields");
        return;
    }
    m_traps.push_back(trap);
}

// I x y w h SPEAKER line one|line two|line three
//   trigger area in world px; SPEAKER is one word (underscores -> spaces); each '|' piece is one dialog page
void LevelLoader::parseInteractable(std::istringstream& ss) {
    float x = 0.f, y = 0.f, w = 0.f, h = 0.f;
    std::string speaker;
    ss >> x >> y >> w >> h >> speaker;
    if(ss.fail()){
        LOGE("Malformed interactable (I) line: failed parsing fixed fields");
        return;
    }
    std::string rest;
    std::getline(ss, rest);
    std::vector<std::string> lines;
    size_t start = 0;
    while(start <= rest.size()){
        size_t bar = rest.find('|', start);
        std::string piece = rest.substr(start, bar == std::string::npos ? std::string::npos : bar - start);
        const size_t a = piece.find_first_not_of(" \t\r");
        const size_t b = piece.find_last_not_of(" \t\r");
        if(a != std::string::npos) lines.push_back(piece.substr(a, b - a + 1));
        if(bar == std::string::npos) break;
        start = bar + 1;
    }
    if(lines.empty()){
        LOGE("Interactable has no dialog text, skipped");
        return;
    }
    m_interactables.emplace_back(x, y, w, h, speaker, lines);
}

// E x y ENEMY_TYPE pathStartX pathEndX speed [AERIAL]
//   x y            spawn position (world px). Ground enemies fall onto the floor, aerial ones hover at y
//   pathStartX/End the horizontal patrol line (px); x should lie between them
//   speed          patrol/chase speed in px/s
//   AERIAL         optional: flies (no gravity) instead of walking
// The enemy's starting attack is just its shortest-reach one; updateAI picks per attack from there.
void LevelLoader::parseEnemy(std::istringstream& ss) {
    float x = 0.f, y = 0.f, startPath = 0.f, endPath = 0.f, speed = 0.f;
    std::string typeStr;
    ss >> x >> y >> typeStr >> startPath >> endPath >> speed;
    if(ss.fail()){
        LOGE("Malformed enemy (E) line: failed parsing fixed fields");
        return;
    }
    EnemyType type = toEnemyType(typeStr);
    if(type == EnemyType::NONE) return;

    combatType combat = combatType::GROUND;
    std::string mode;
    if(ss >> mode && mode == "AERIAL") combat = combatType::AERIAL;

    const EnemyAIConfig* cfg = getEnemyAIConfig(type);
    AttackType attack = cfg ? cfg->attacks[0] : AttackType::NONE;
    m_enemies.emplace_back(x, y, type, EnemyAction::PATROL, attack, startPath, endPath, speed,
                           PathAxis::HORIZONTAL, PathShape::LINE, 0.0f, combat);
}

void LevelLoader::parseSawTail(std::istringstream& ss,Trap& trap) {
    float radius;
    bool showChain;

    ss>>radius>>std::boolalpha>>showChain;

    if(ss.fail()){
        LOGE("Malformed trap line: failed parsing tailing fields");
        return;
    }
    trap.radius =radius;
    trap.showChain =showChain;
}

void LevelLoader::parseSpikeBallTail(std::istringstream& ss,Trap& trap) {
    float radius;
    bool showChain;

    ss>>radius>>std::boolalpha>>showChain;

    if(ss.fail()){
        LOGE("Malformed trap line: failed parsing tailing fields");
        return;
    }
    trap.radius =radius;
    trap.showChain =showChain;
}

// optional last token on the trap line: UP | RIGHT | DOWN | LEFT (default UP) - see TrapOrientation
void LevelLoader::parseGenericTail(std::istringstream& ss,Trap& trap) {
    std::string orientation;
    if(ss>>orientation){
        trap.orientation = toTrapOrientation(orientation);
        if(!trapSupportsOrientation(trap.type) && trap.orientation != TrapOrientation::UP){
            LOGI("trap type %d can't be rotated, orientation %s ignored",(int)trap.type,orientation.c_str());
            trap.orientation = TrapOrientation::UP;
        }
    } else {
        ss.clear();   // no token = normal end of line, not a parse failure (parseTraps checks ss.fail())
    }
}

void LevelLoader::parseCheckPoint(std::istringstream& ss) {
    float x=0.00f,y=0.00f,w=0.00f,h=0.00f;
    std::string checkPointFlagAniType;

    ss>>x>>y>>w>>h>>checkPointFlagAniType;

    if(ss.fail()){
        LOGE("Malformed checkpoint line: failed parsing fixed fields");
        return;
    }

    CheckPointAni flagType = toCheckPointAniType(checkPointFlagAniType);

    m_checkPoint={x,y,w,h,flagType};
}
void LevelLoader::parseWallCollisionRect(std::istringstream& ss){
    float x=0.00f,y=0.00f,w=0.00f,h=0.00f;
    ss>>x>>y>>w>>h;
    if(ss.fail()){
        LOGE("Malformed wallCollisionRect line: failed parsing fixed fields");
        return;
    }
    m_wallCollisionRect = {x,y,w,h};
}
void LevelLoader::parseAmbient(std::istringstream& ss) {
    float darkness = 0.0f;
    ss>>darkness;
    if(ss.fail()){
        LOGE("Malformed ambient (D) line");
        return;
    }
    m_ambient.darkness = darkness;
    float radius = 0.0f, softness = 0.0f;
    if(ss>>radius>>softness){            // optional player light size
        m_ambient.playerRadius = radius;
        m_ambient.playerSoftness = softness;
    }
}

void LevelLoader::parseLight(std::istringstream& ss) {
    LightSource l;
    int r=255,g=200,b=120;
    ss>>l.x>>l.y>>l.radius>>r>>g>>b>>l.intensity;
    if(ss.fail()){
        LOGE("Malformed light (L) line: failed parsing fixed fields");
        return;
    }
    l.r = (uint8_t)std::clamp(r,0,255);
    l.g = (uint8_t)std::clamp(g,0,255);
    l.b = (uint8_t)std::clamp(b,0,255);
    float flicker = 0.0f;
    int visible = 1;
    if(ss>>flicker){
        l.flicker = flicker;
        if(ss>>visible) l.visible = visible != 0;
    }
    m_lights.push_back(l);
}

// M file [gain]
//   background music of this level (looped). file is relative to the assets folder; a bare name like
//   cave.ogg means Sound/Music/cave.ogg. gain 0..1 trims this track (default 1). `M NONE` = silent level.
void LevelLoader::parseMusic(std::istringstream& ss) {
    std::string file;
    ss>>file;
    if(ss.fail()){
        LOGE("Malformed music (M) line");
        return;
    }
    float gain = 1.0f;
    if(ss>>gain) gain = std::clamp(gain,0.0f,2.0f);
    if(file == "NONE"){
        m_musicPath.clear();
        m_musicGain = 1.0f;
        m_musicExplicit = true;
        return;
    }
    if(file.find('/') == std::string::npos) file = "Sound/Music/" + file;
    m_musicPath = file;
    m_musicGain = gain;
    m_musicExplicit = true;
}

void LevelLoader::loadLevel(int level) {
    m_musicPath.clear();
    m_musicGain = 1.0f;
    m_musicExplicit = false;
    m_lights.clear();
    m_enemies.clear();
    m_interactables.clear();
    m_bosses.clear();
    m_hasBossArena = false;
    m_ambient = LevelAmbient{};
    m_hasSpawn = false;
    std::string content = loadFromFile("Levels/level_"+std::to_string(level)+".txt");

    std::istringstream stream(content);

    std::string line;
    while(std::getline(stream,line)){
        parseLine(line);
    }
    // no M record: use the convention Sound/Music/level_<n>.ogg (silent if that file doesn't exist)
    if(!m_musicExplicit) m_musicPath = "Sound/Music/level_" + std::to_string(level) + ".ogg";

    // the level's 'A' arena (if any) belongs to every boss in the level
    if(m_hasBossArena)
        for(Boss& b : m_bosses){ b.hasArena = true; b.arena = m_bossArena; }
}

// K x y BOSS_TYPE [phase1Music [phase2Music [gain]]]
//   a boss. x y = top-left of its hitbox (it falls onto the floor). The fight starts when the player walks
//   into the level's arena (see 'A'); the arena then locks (camera fixed, player can't leave) until the boss dies.
//   Music for the fight: phase 1 / phase 2 track. A bare file name means Sound/Music/Boss/<name>; '-' = use
//   the default Sound/Music/Boss/<biome>_<mini|main>_phase<1|2>.ogg. gain trims both tracks (default: per boss).
//   The level's own M record (or level_<n>.ogg) is the music BEFORE the fight and after the boss dies.
//   example:  K 3400 400 MOSS_BROODMOTHER
//   example:  K 3400 400 PEAK_REX_EMPEROR rex_p1.ogg rex_p2.ogg 0.9
void LevelLoader::parseBoss(std::istringstream& ss) {
    float x = 0.0f, y = 0.0f;
    std::string typeStr;
    ss >> x >> y >> typeStr;
    if(ss.fail()){ LOGE("Malformed boss (K) line"); return; }
    const BossType type = bossTypeFromName(typeStr);
    if(type == BossType::NONE){ LOGE("Unknown boss type: %s", typeStr.c_str()); return; }

    Boss boss(x, y, type);
    auto readTrack = [&](std::string& out){
        std::string f;
        if(!(ss >> f)) return false;
        if(f == "-") return true;                                        // keep the default
        if(f.find('/') == std::string::npos) f = "Sound/Music/Boss/" + f;
        out = f;
        return true;
    };
    if(readTrack(boss.musicPhase1) && readTrack(boss.musicPhase2)){
        float gain = 1.0f;
        if(ss >> gain) boss.musicGain = std::clamp(gain, 0.0f, 2.0f);
    }
    m_bosses.push_back(boss);
}

// A x y w h
//   the boss arena: the world rectangle the player is locked into during the fight. While it is locked the camera
//   stops following the player and sits centred on this rectangle, so it must fit on one screen
//   (<= 1600 x 720; wider than that is trimmed). Put it at the end of the level so the camera's normal clamp and
//   the lock agree, e.g. a 4000 px wide level -> A 2400 0 1600 704.
//   Without an 'A' record the arena is one screen wide, centred on the boss.
void LevelLoader::parseBossArena(std::istringstream& ss) {
    float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
    ss >> x >> y >> w >> h;
    if(ss.fail() || w <= 0.0f || h <= 0.0f){ LOGE("Malformed arena (A) line"); return; }
    m_bossArena = {x, y, w, h};
    m_hasBossArena = true;
}

LevelLoader::LevelLoader() {
}

std::string LevelLoader::loadFromFile(const std::string& filepath){

    SDL_IOStream* stream = SDL_IOFromFile(filepath.c_str(),"r");
    if(!stream){
        LOGE("Failure to load level files");
        return {};
    }
    Sint64 size = SDL_GetIOSize(stream);
    if(size < 0){
        LOGE("Invalid file size from the IO stream");
        return {};
    }
    std::string buffer(size,'\0');
    size_t read = SDL_ReadIO(stream,buffer.data(),size);
    SDL_CloseIO(stream);
    if (read != static_cast<size_t>(size)) {
        LOGE("Short read on level file: %s (got %zu of %lld)", filepath.c_str(), read, size);
    }
    return buffer;
}

LevelLoader::~LevelLoader() {

}
