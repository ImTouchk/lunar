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

        JNIEnv* getJniEnv();

		Identifiable::NativeType getNativeHandle(jobject& object);
		jobject createNativeHandle(Identifiable::NativeType id);
	private:
		JavaVM* jvm;
		JNIEnv* env;
		struct
		{
			jclass klass;
			jmethodID baseCtor;
			jfieldID valueField;
		} nativeHandle;
	};

	LUNAR_API VmWrapper& getMainVm();
	LUNAR_API VmWrapper& getVmFromEnv(JNIEnv* env);
}
