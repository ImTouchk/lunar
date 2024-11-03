#pragma once
#include <lunar/api.hpp>
#include <glm/glm.hpp>
#include <string_view>
#include <string>
#include <jni.h>

namespace Core
{
	enum class LUNAR_API ComponentUpdatePriority
	{
		eNormal = 0,
		eLow = 1,
		eHigh = 2,
	};

	class LUNAR_API Component
	{
	public:
        Component() = default;

		virtual bool isUpdateable() = 0;
		virtual void update() = 0;

	protected:
		ComponentUpdatePriority priority = ComponentUpdatePriority::eNormal;
	};

	class LUNAR_API ScriptComponent : public Component
	{
	public:
		ScriptComponent(const std::string_view& name);
		ScriptComponent();
		~ScriptComponent();

		bool isUpdateable() override;
		void update() override;

		const std::string& getScriptName() const;

	private:
		std::string scriptName;
		jobject instance;
		jmethodID onLoad;
		jmethodID onUnload;
		jmethodID onUpdate;
	};

	class LUNAR_API TransformComponent : public Component
	{
	public:
		glm::dvec3 position;
		glm::dvec3 rotation;
		glm::dvec3 scale;

		bool isUpdateable() override;
		void update() override;
	};

	template<typename T>
	concept IsDerivedComponent = std::derived_from<T, Component> && !std::is_same_v<T, Component>;
}
