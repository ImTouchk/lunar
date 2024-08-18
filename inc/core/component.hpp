#pragma once
#include <jni.h>
#include <string_view>
#include <glm/glm.hpp>

namespace Core
{
	class Component
	{
	public:
		virtual size_t getTypeHash();
		virtual const char* getType() = 0;
		virtual bool isUpdateable() = 0;
		virtual void update() = 0;
	};

	class ScriptComponent : public Component
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

	class TransformComponent : public Component
	{
	public:
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;

		const char* getType() override;
		bool isUpdateable() override;
		void update() override;
	};
}