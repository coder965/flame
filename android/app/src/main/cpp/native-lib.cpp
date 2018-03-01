#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/native_activity.h>
#include <dlfcn.h>

void ANativeActivity_onCreate(ANativeActivity* activity,
                              void* savedState, size_t savedStateSize) {
    auto libvulkan = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    if (!libvulkan)
        return;
    int a = 1;
    while(true);
}
