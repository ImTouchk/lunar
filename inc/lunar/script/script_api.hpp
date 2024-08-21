#pragma once
#include <lunar/script/api/scene.hpp>
#include <lunar/api.hpp>
#include <string_view>
#include <string>
#include <jni.h>

namespace Script
{
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

    jstring createManagedString(const std::string& str);
}
