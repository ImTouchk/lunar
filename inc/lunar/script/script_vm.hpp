#pragma once
#include <lunar/script/script_api.hpp>
#include <lunar/api.hpp>
#include <string_view>
#include <string>
#include <jni.h>

namespace Script
{
	struct LUNAR_API NativeHandle
	{
		jclass klass;
		jmethodID baseCtor;
		jfieldID valueField;
	};

	class LUNAR_API VirtualMachine
	{
	public:
		VirtualMachine(std::vector<std::string>& launchOptions);
		VirtualMachine();
		~VirtualMachine() = default;

		JNIEnv* getJniEnv();

	private:
		JavaVM* jvm;
		JNIEnv* env;
	};

	struct LUNAR_API VirtualMachineBuilder
	{
	public:
		VirtualMachineBuilder() = default;
		~VirtualMachineBuilder() = default;

		VirtualMachineBuilder& enableVerbose(bool value = true);
		VirtualMachineBuilder& loadPackage(const Fs::Path& path);
		VirtualMachineBuilder& setPackageLoader(const std::string_view& name);
		VirtualMachineBuilder& useNativePackageLoader();
		VirtualMachineBuilder& useDynamicPackageLoader();
		VirtualMachine create();
	private:
		std::string packageLoader = {};
		std::vector<Fs::Path> packages = { Fs::baseDirectory().append("core.jar") };
		bool verbose = false;
	};

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
