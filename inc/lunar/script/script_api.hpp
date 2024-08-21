#pragma once
#include <lunar/script/api/scene.hpp>
#include <lunar/api.hpp>
#include <string_view>
#include <string>
#include <jni.h>

namespace Script
{
    class NativeObjectWrapper
    {
    public:
        NativeObjectWrapper(const char* name);
        NativeObjectWrapper() = default;

        int getHandle(jobject& object) const;
        jobject createHandle(int handle);

    private:
        jclass klass;
        jmethodID baseConstructor;
        jfieldID handleField;
    };

    class JavaArrayList
    {
    public:
        JavaArrayList(int size);

        void add(jobject elem);

        jobject object;
    private:
        static jclass klass;
        static jmethodID constructor;
        static jmethodID addMethod;
        static bool initialized;
    };

    enum NOWrapperTypes {
        VM_SCENE_WRAPPER = 0,
        VM_GAMEOBJ_WRAPPER = 1,
        VM_OBJECT_COUNT = 2,
    };

    jstring createManagedString(const std::string& str);
}
