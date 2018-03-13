#include <jni.h>
#include <string>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <dlfcn.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))

extern "C" {

void android_main(android_app *app)
{
    auto lib_vulkan = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    if (!lib_vulkan)
    {
        __android_log_print(ANDROID_LOG_INFO, "native-activity", "%s", "Vulkan Not Support");
        return;
    }

    int frames = 0;

    while(true)
    {
        int ident;
        int events;
        android_poll_source *source;
        while ((ident = ALooper_pollAll(0, nullptr, &events, (void**)&source)) >= 0)
        {
            if (source != nullptr)
                source->process(app, source);

            if (app->destroyRequested)
                return;
        }

        LOGI("%d", frames);
        frames++;
    }
}

}
