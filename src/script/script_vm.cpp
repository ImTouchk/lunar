#include <lunar/script/script_vm.hpp>
#include <lunar/script/script_api.hpp>
#include <lunar/file/filesystem.hpp>
#include <lunar/debug/log.hpp>
#include <format>
#include <memory>
#include <vector>

namespace Script
{
	VmWrapper& getVmFromEnv(JNIEnv* env)
	{
		// TODO
		return getMainVm();
	}

	VmWrapper& getMainVm()
	{
		static VmWrapper vm = {};
		return vm;
	}

	VmWrapper::VmWrapper()
		: env(nullptr),
		jvm(nullptr),
		nativeHandle()
	{
		auto loader = Fs::baseDirectory()
			.append("core.jar")
			.string();

		if (!Fs::fileExists(loader))
		{
			DEBUG_ERROR("File \"{}\" does not exist. Missing engine data...", loader);
			return;
		}

		std::vector<std::string> options = { 
			std::format("-Djava.class.path={}", loader),
			std::format("-Djava.system.class.loader=dev.lunar.ScriptLoader"),
            "--enable-preview"
		};

#		ifdef LUNAR_VERBOSE
		options.push_back("-verbose:class");
#		endif

		std::unique_ptr<JavaVMOption[]> option_list = std::make_unique<JavaVMOption[]>(options.size());
		for (size_t i = 0; i < options.size(); i++)
		{
			option_list[i] = { .optionString = options[i].data() };
		}

		JavaVMInitArgs vm_args = {
			.version = JNI_VERSION_1_6,
			.nOptions = (jint)options.size(),
			.options = option_list.get(),
			.ignoreUnrecognized = false
		};

		if (JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args) != JNI_OK)
			DEBUG_ERROR("Could not create a Java Virtual Machine.");
		else
			DEBUG_LOG("Java Virtual Machine created.");

		jclass main_class = env->FindClass("dev/lunar/Main");
		jmethodID main_id = env->GetStaticMethodID(main_class, "main", "([Ljava/lang/String;)V");

		nativeHandle.klass = env->FindClass("dev/lunar/core/internal/NativeObjectHandle");
		nativeHandle.baseCtor = env->GetMethodID(nativeHandle.klass, "<init>", "()V");
		nativeHandle.valueField = env->GetFieldID(nativeHandle.klass, "handle", "I");

		env->CallStaticVoidMethod(main_class, main_id, nullptr);
	}

	VmWrapper::~VmWrapper()
	{
		//if (jvm->DestroyJavaVM() != JNI_OK)
		//	DEBUG_ERROR("Failure to destroy instance of JVM.");
		//else
		//	DEBUG_LOG("Java Virtual Machine instance destroyed.");
	}

    JNIEnv* VmWrapper::getJniEnv()
    {
        return env;
    }

	Identifiable::NativeType VmWrapper::getNativeHandle(jobject& object)
	{
		return env->GetIntField(object, nativeHandle.valueField);
	}

	jobject VmWrapper::createNativeHandle(Identifiable::NativeType id)
	{
		jobject object = env->NewObject(nativeHandle.klass, nativeHandle.baseCtor);
		env->SetIntField(object, nativeHandle.valueField, id);
		return object;
	}
}
