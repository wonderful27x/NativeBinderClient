#include <jni.h>
#include <string>
#include "test.h"
#include "wonderful_binder.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_nativebinderclient_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";

    int num = test_add(2,25);
    hello += std::to_string(num);

    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_nativebinderclient_MainActivity_addService(JNIEnv *env, jobject thiz) {
    // TODO: implement addService()
    addDemoService();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_nativebinderclient_MainActivity_alert(JNIEnv *env, jobject thiz) {
    // TODO: implement alert()
    executeAlert();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_nativebinderclient_MainActivity_push(JNIEnv *env, jobject thiz, jint data) {
    // TODO: implement push()
    executePush(data);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_nativebinderclient_MainActivity_add(JNIEnv *env, jobject thiz, jint v1, jint v2) {
    // TODO: implement add()
    return executeAdd(v1,v2);
}