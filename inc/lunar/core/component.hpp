#pragma once
#include <lunar/api.hpp>
#include <glm/glm.hpp>
#include <string_view>
#include <string>
#include <jni.h>

namespace Core
{
	class LUNAR_API Component
	{
	public:
        Component() = default;

		virtual size_t getTypeHash();
		virtual const char* getType() = 0;
		virtual bool isUpdateable() = 0;
		virtual void update() = 0;
	};

	class LUNAR_API ScriptComponent : public Component
	{
	public:
		ScriptComponent(const std::string_view& name);
		ScriptComponent();
		~ScriptComponent();

		size_t getTypeHash() override;
		const char* getType() override;
		bool isUpdateable() override;
		void update() override;

		const std::string& getScriptName() const;

	private:
		std::string scriptName;
		jobject instance;
		jmethodID startMethod;
		jmethodID stopMethod;
		jmethodID updateMethod;
	};

	class LUNAR_API TransformComponent : public Component
	{
	public:
		glm::dvec3 position;
		glm::dvec3 rotation;
		glm::dvec3 scale;

		const char* getType() override;
		bool isUpdateable() override;
		void update() override;
	};
}