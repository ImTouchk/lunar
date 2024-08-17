#pragma once
#include <jni.h>
#include <string>
#include <string_view>

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
		jmethodID getDefaultConstructor(jclass& klass);
		jobject createClassInstance(jclass& klass);
		void callVoidMethod(jobject& object, jmethodID& method);
	private:
		JavaVM* jvm;
		JNIEnv* env;
	};

	VmWrapper& getMainVm();
}
