#include <lunar/core/component.hpp>
#include <lunar/script/script_vm.hpp>

namespace Core
{
	bool TransformComponent::isUpdateable()
	{
		return false;
	}

	void TransformComponent::update() 
	{
	}

	ScriptComponent::ScriptComponent(const std::string_view& name)
		: scriptName(name),
		instance(nullptr),
		onLoad(nullptr), 
		onUnload(nullptr), 
		onUpdate(nullptr)
	{
		auto& vm = Script::getMainVm();
		auto* env = vm.getJniEnv();
		jclass klass = env->FindClass(name.data());
		onLoad = env->GetMethodID(klass, "onLoad", "()V");
		onUnload = env->GetMethodID(klass, "onUnload", "()V");
		onUpdate = env->GetMethodID(klass, "onUpdate", "()V");

		jmethodID constructor = env->GetMethodID(klass, "<init>", "()V");
		instance = env->NewObject(klass, constructor);
		env->CallVoidMethod(instance, onLoad);
	}

	ScriptComponent::ScriptComponent()
		: scriptName(""),
		instance(nullptr),
		onLoad(nullptr), 
		onUnload(nullptr), 
		onUpdate(nullptr)
	{
	}

	ScriptComponent::~ScriptComponent()
	{
		if (scriptName != "")
			Script::getMainVm()
				.getJniEnv()
					->CallVoidMethod(instance, onUnload);
	}

	const std::string& ScriptComponent::getScriptName() const
	{
		return scriptName;
	}

	bool ScriptComponent::isUpdateable()
	{
		return true;
	}

	void ScriptComponent::update()
	{
		Script::getMainVm()
			.getJniEnv()
			->CallVoidMethod(instance, onUpdate);
	}
}
