#include <lunar/core/scene.hpp>
#include <lunar/core/gameobject.hpp>
#include <lunar/core/component.hpp>
#include <lunar/script/script_vm.hpp>

namespace Core
{
	Scene& Component::getScene()
	{
		DEBUG_ASSERT(_scene != nullptr, "Component object was not properly initialized");
		return *_scene;
	}

	const Scene& Component::getScene() const
	{
		DEBUG_ASSERT(_scene != nullptr, "Component object was not properly initialized");
		return *_scene;
	}

	GameObject& Component::getGameObject()
	{
		return getScene().getGameObject(_gameObject);
	}

	const GameObject& Component::getGameObject() const
	{
		return getScene().getGameObject(_gameObject);
	}

	TransformComponent& Component::getTransform()
	{
		return getGameObject().getTransform();
	}

	const TransformComponent& Component::getTransform() const
	{
		return getGameObject().getTransform();
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

	void ScriptComponent::update()
	{
		Script::getMainVm()
			.getJniEnv()
			->CallVoidMethod(instance, onUpdate);
	}
}
