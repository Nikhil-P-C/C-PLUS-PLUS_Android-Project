//
// Created by LENOVO on 06-08-2026.
//
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
        case 'C':
            parseCheckPoint(stream);
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
    trap.type = type;
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

void LevelLoader::parseGenericTail(std::istringstream& ss,Trap& trap) {
    //do nothing : might remove this function later
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

void LevelLoader::loadLevel(int level) {
    std::string content = loadFromFile("Levels/level_"+std::to_string(level)+".txt");

    std::istringstream stream(content);

    std::string line;
    while(std::getline(stream,line)){
        parseLine(line);
    }
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
