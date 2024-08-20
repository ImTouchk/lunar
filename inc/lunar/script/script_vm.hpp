#pragma once
#include <lunar/script/script_api.hpp>
#include <lunar/api.hpp>
#include <string_view>
#include <string>
#include <jni.h>

namespace Script
{
	class VmWrapper
	{
	public:
		VmWrapper();
		~VmWrapper();

		jclass findClass(const std::string_view& name);
		jmethodID findMainMethod(jclass& klass, const std::string_view& name);
		jmethodID findVoidMethod(jclass& klass, const std::string_view& name);
        jfieldID getFieldId(jclass& klass, const char* name, const char* signature);
		jmethodID getDefaultConstructor(jclass& klass);
		jobject createClassInstance(jclass& klass);
		void callVoidMethod(jobject& object, jmethodID& method);

        JNIEnv* getJniEnv();

        void initialize();
        NativeObjectWrapper& getWrapper(int index);
	private:
		JavaVM* jvm;
		JNIEnv* env;

        NativeObjectWrapper wrappers[VM_OBJECT_COUNT];
	};

	VmWrapper& getMainVm();
}
