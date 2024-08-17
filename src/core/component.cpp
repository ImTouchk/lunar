#include <core/component.hpp>
#include <script/script_vm.hpp>

namespace Core
{
	ScriptComponent::ScriptComponent(const std::string_view& name)
		: scriptName(name.data()),
		instance(nullptr),
		startMethod(nullptr), stopMethod(nullptr), updateMethod(nullptr)
	{
		auto& vm = Script::getMainVm();
		jclass klass = vm.findClass(name);
		startMethod = vm.findVoidMethod(klass, "start");
		stopMethod = vm.findVoidMethod(klass, "stop");
		updateMethod = vm.findVoidMethod(klass, "update");

		instance = vm.createClassInstance(klass);
		vm.callVoidMethod(instance, startMethod);
	}

	ScriptComponent::~ScriptComponent()
	{
		Script::getMainVm()
			.callVoidMethod(instance, stopMethod);
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
