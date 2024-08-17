#include <script/script_vm.hpp>
#include <file/filesystem.hpp>
#include <debug/log.hpp>
#include <format>
#include <memory>
#include <vector>

namespace Script
{
	VmWrapper& getMainVm()
	{
		static VmWrapper vm = {};
		return vm;
	}

	VmWrapper::VmWrapper()
		: env(nullptr),
		jvm(nullptr)
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
			std::format("-Djava.system.class.loader=dev.mugur.ScriptLoader")
		};

#if DEBUG_JVM_VERBOSE
		options.push_back("-verbose:class");
#endif

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

		jclass main_class = findClass("dev/mugur/Main");
		jmethodID main_id = findMainMethod(main_class, "main");
		env->CallStaticVoidMethod(main_class, main_id, nullptr);
	}

	VmWrapper::~VmWrapper()
	{
		if (jvm->DestroyJavaVM() != JNI_OK)
			DEBUG_ERROR("Failure to destroy instance of JVM.");
		else
			DEBUG_LOG("Java Virtual Machine instance destroyed.");
	}

	jmethodID VmWrapper::findVoidMethod(jclass& klass, const std::string_view& name)
	{
		return env->GetMethodID(klass, name.data(), "()V");
	}

	jmethodID VmWrapper::findMainMethod(jclass& klass, const std::string_view& name)
	{
		jmethodID main_id = env->GetStaticMethodID(klass, name.data(), "([Ljava/lang/String;)V");
		if (main_id == nullptr)
		{
			DEBUG_ERROR("Could not find entry point of JVM class.");
		}
		return main_id;
	}

	jclass VmWrapper::findClass(const std::string_view& name)
	{
		jclass clss = env->FindClass(name.data());
		if (clss == nullptr)
		{
			DEBUG_ERROR("Could not find java class \"{}\".", name);
		}

		return clss;
	}

	jmethodID VmWrapper::getDefaultConstructor(jclass& klass)
	{
		return env->GetMethodID(klass, "<init>", "()V");
	}

	jobject VmWrapper::createClassInstance(jclass& klass)
	{
		return env->NewObject(klass, getDefaultConstructor(klass));
	}

	void VmWrapper::callVoidMethod(jobject& object, jmethodID& method)
	{
		env->CallVoidMethod(object, method);
	}
}
