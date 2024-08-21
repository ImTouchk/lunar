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

    NativeObjectWrapper::NativeObjectWrapper(const char *name)
            : klass(getMainVm().findClass(name)),
              baseConstructor(getMainVm().findVoidMethod(klass, "<init>")),
              handleField(getMainVm().getFieldId(klass, "handle", "I"))
    {
    }

    int NativeObjectWrapper::getHandle(jobject &object) const
    {
        return getMainVm()
                .getJniEnv()
                ->GetIntField(object, handleField);
    }

    jobject NativeObjectWrapper::createHandle(int handle)
    {
        JNIEnv* env = getMainVm().getJniEnv();
        jobject object = env->NewObject(klass, baseConstructor);
        env->SetIntField(object, handleField, handle);
        return object;
    }

    NativeObjectWrapper& VmWrapper::getWrapper(int index)
    {
        return wrappers[index];
    }

    jstring createManagedString(const std::string& str)
    {
        return getMainVm().getJniEnv()->NewStringUTF(str.c_str());
    }
}
