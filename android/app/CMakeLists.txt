cmake_minimum_required(VERSION 3.4.1)

add_library(app SHARED src/main/cpp/app.cpp
            C:/Users/Administrator/AppData/Local/Android/Sdk/ndk-bundle/sources/android/native_app_glue/android_native_app_glue.c)

find_library(log-lib log)

target_link_libraries(app ${log-lib})

find_library(android-lib android)

target_link_libraries(app ${android-lib})

target_include_directories(app PUBLIC C:/Users/Administrator/AppData/Local/Android/Sdk/ndk-bundle/sources/android/native_app_glue)

add_subdirectory(../../ext build)