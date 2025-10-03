#include <jni.h>
#include <string>
#include "game.cpp"

Game game(1,2);

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myapp_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "the sum of to number"+std::to_string(game.add());
    return env->NewStringUTF(hello.c_str());
}