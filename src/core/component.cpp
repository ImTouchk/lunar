#include <lunar/core/component.hpp>
#include <lunar/script/script_vm.hpp>

namespace Core
{
	size_t Component::getTypeHash()
	{
		return std::hash<std::string>{}(getType());
	}

	const char* TransformComponent::getType()
	{
		return "core.transformComponent";
	}

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
		startMethod(nullptr), stopMethod(nullptr), updateMethod(nullptr)
	{
		auto& vm = Script::getMainVm();
		jclass klass = vm.findClass(name);
		startMethod = vm.findVoidMethod(klass, "onStart");
		stopMethod = vm.findVoidMethod(klass, "onStop");
		updateMethod = vm.findVoidMethod(klass, "onUpdate");

		instance = vm.createClassInstance(klass);
		vm.callVoidMethod(instance, startMethod);
	}

	ScriptComponent::ScriptComponent()
		: scriptName(""),
		instance(nullptr),
		startMethod(nullptr), stopMethod(nullptr), updateMethod(nullptr)
	{
	}

	ScriptComponent::~ScriptComponent()
	{
		if (scriptName != "")
			Script::getMainVm()
				.callVoidMethod(instance, stopMethod);
	}

	const std::string& ScriptComponent::getScriptName() const
	{
		return scriptName;
	}

	const char* ScriptComponent::getType()
	{
		return "core.scriptComponent";
	}

	size_t ScriptComponent::getTypeHash()
	{
		static size_t hash = std::hash<std::string>{}(getType());
		return hash;
	}

	bool ScriptComponent::isUpdateable()
	{
		return true;
	}

	void ScriptComponent::update()
	{
		Script::getMainVm()
			.callVoidMethod(instance, updateMethod);
	}
}
