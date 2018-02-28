#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/native_activity.h>

void ANativeActivity_onCreate(ANativeActivity* activity,
                              void* savedState, size_t savedStateSize) {
    int a = 1;
    while(true);
}

void android_main(struct android_app* state) {
    int a = 1;
    while(true);
}
