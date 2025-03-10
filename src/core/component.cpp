#include <lunar/core/scene.hpp>
#include <lunar/core/gameobject.hpp>
#include <lunar/core/component.hpp>
#include <lunar/script/script_vm.hpp>

namespace lunar
{
	Component_T::Component_T(GameObject parent) noexcept
		: gameObject(parent),
		scene(parent->getScene())
	{
	}

	const char* Component_T::getClassName() const
	{
		return typeid(*this).name();
	}

	GameObject Component_T::getGameObject()
	{
		return gameObject;
	}

	const GameObject Component_T::getGameObject() const
	{
		return gameObject;
	}

	Scene* Component_T::getScene()
	{
		return scene;
	}

	const Scene* Component_T::getScene() const
	{
		return scene;
	}

	const Transform& Component_T::getTransform() const
	{
		return getGameObject()->getTransform();
	}

	Transform& Component_T::getTransform()
	{
		return getGameObject()->getTransform();
	}
}

//namespace Core
//{
//	void Component::drawDebugUI(Render::RenderContext& context)
//	{
//		ImGui::SetCurrentContext(context.getImGuiContext());
//		ImGui::Text("This component does not have a dedicated debug UI menu.");
//	}
//
//	Scene& Component::getScene()
//	{
//		DEBUG_ASSERT(_scene != nullptr, "Component object was not properly initialized");
//		return *_scene;
//	}
//
//	const Scene& Component::getScene() const
//	{
//		DEBUG_ASSERT(_scene != nullptr, "Component object was not properly initialized");
//		return *_scene;
//	}
//
//	GameObject& Component::getGameObject()
//	{
//		return getScene().getGameObject(_gameObject);
//	}
//
//	const GameObject& Component::getGameObject() const
//	{
//		return getScene().getGameObject(_gameObject);
//	}
//
//	TransformComponent& Component::getTransform()
//	{
//		return getGameObject().getTransform();
//	}
//
//	const TransformComponent& Component::getTransform() const
//	{
//		return getGameObject().getTransform();
//	}
//
//	ScriptComponent::ScriptComponent(const std::string_view& name)
//		: scriptName(name),
//		instance(nullptr),
//		onLoad(nullptr), 
//		onUnload(nullptr), 
//		onUpdate(nullptr)
//	{
//		auto& vm = Script::getMainVm();
//		auto* env = vm.getJniEnv();
//		jclass klass = env->FindClass(name.data());
//		onLoad = env->GetMethodID(klass, "onLoad", "()V");
//		onUnload = env->GetMethodID(klass, "onUnload", "()V");
//		onUpdate = env->GetMethodID(klass, "onUpdate", "()V");
//
//		jmethodID constructor = env->GetMethodID(klass, "<init>", "()V");
//		instance = env->NewObject(klass, constructor);
//		env->CallVoidMethod(instance, onLoad);
//	}
//
//	ScriptComponent::ScriptComponent()
//		: scriptName(""),
//		instance(nullptr),
//		onLoad(nullptr), 
//		onUnload(nullptr), 
//		onUpdate(nullptr)
//	{
//	}
//
//	ScriptComponent::~ScriptComponent()
//	{
//		if (scriptName != "")
//			Script::getMainVm()
//				.getJniEnv()
//					->CallVoidMethod(instance, onUnload);
//	}
//
//	const std::string& ScriptComponent::getScriptName() const
//	{
//		return scriptName;
//	}
//
//	void ScriptComponent::update()
//	{
//		Script::getMainVm()
//			.getJniEnv()
//			->CallVoidMethod(instance, onUpdate);
//	}
//}
