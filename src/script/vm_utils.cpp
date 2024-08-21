#include <lunar/script/script_vm.hpp>
#include <lunar/script/script_api.hpp>
#include <lunar/script/api/math.hpp>
#include <lunar/debug/log.hpp>

namespace Script
{
    jclass JavaArrayList::klass;
    jmethodID JavaArrayList::addMethod;
    jmethodID JavaArrayList::constructor;
    bool JavaArrayList::initialized = false;

    JavaArrayList::JavaArrayList(int size)
    {
        JNIEnv* env = getMainVm().getJniEnv();
        if(!initialized)
        {
            klass = env->FindClass("java/util/ArrayList");
            constructor = env->GetMethodID(klass, "<init>", "(I)V");
            addMethod = env->GetMethodID(klass, "add", "(Ljava/lang/Object;)Z");
        }
        object = env->NewObject(klass, constructor, size);
    }

    void JavaArrayList::add(jobject elem)
    {
        getMainVm().getJniEnv()->CallBooleanMethod(object, addMethod, elem);
    }

    jstring createManagedString(const std::string& str)
    {
        return getMainVm().getJniEnv()->NewStringUTF(str.c_str());
    }
}
