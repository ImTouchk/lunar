#pragma once
#include <jni.h>
#include <string_view>

namespace Core
{
	class Component
	{
	public:
		virtual size_t getTypeHash() = 0;
		virtual const char* getType() = 0;
		virtual bool isUpdateable() = 0;
		virtual void update() = 0;
	};

	class ScriptComponent : public Component
	{
	public:
		ScriptComponent(const std::string_view& name);
		ScriptComponent() = default;
		~ScriptComponent();

		size_t getTypeHash() override;
		const char* getType() override;
		bool isUpdateable() override;
		void update() override;

	private:
		const char* scriptName;
		jobject instance;
		jmethodID startMethod;
		jmethodID stopMethod;
		jmethodID updateMethod;
	};
}