#pragma once
#include <lunar/core/common.hpp>
#include <lunar/core/component.hpp>
#include <lunar/core/handle.hpp>
#include <lunar/file/json_file.hpp>
#include <lunar/utils/identifiable.hpp>
#include <lunar/debug/assert.hpp>
#include <lunar/api.hpp>
#include <concepts>
#include <string>
#include <memory>
#include <vector>

namespace lunar
{
	class LUNAR_API GameObject_T
	{
	public:
		GameObject_T(Scene* scene, const std::string_view& name, GameObject parent = nullptr) noexcept;
		GameObject_T()  noexcept = default;
		~GameObject_T() noexcept = default;

		void                   update();
		size_t                 getId()             const;
		std::string_view       getName()           const;
		Scene*                 getScene();
		GameObject             getParent();
		std::vector<Component> getComponents();
		Component              getComponent(const std::type_info& ty);
		const Transform&       getTransform()      const;
		Transform&             getTransform();
		glm::vec3              getWorldPos()       const;
		glm::quat              getWorldRotation()  const;
		glm::vec3              getWorldScale()     const;
		glm::mat4              getWorldTransform() const;
		glm::vec3              getLocalPos()       const;
		glm::vec3              getLocalRotation()  const;
		glm::vec3              getLocalScale()     const;
		void                   setWorldPos(glm::vec3 pos);
		void                   setLocalPos(glm::vec3 pos);

		template<typename T> requires IsComponentType<T>
		T*                     getComponent() { return static_cast<T*>(getComponent(typeid(T)).get()); }

		Component_T*           addComponent(Component created);
		template <typename T, class... _Valty> requires IsComponentType<T>
		T*                     addComponent(_Valty&&... ctor_values)
		{
			DEBUG_ASSERT(getComponent<T>() == nullptr, "There can exist only one component of type <T> on a single gameobject.");
			auto new_component = std::make_shared<T>(std::forward<_Valty>(ctor_values)...);
			addComponent(new_component);
			return new_component.get();
		}

		std::vector<GameObject> getChildren();
		GameObject              createChildObject(const std::string_view& name);
	private:
		size_t      id        = 0;
		Scene*      scene     = nullptr;
		GameObject  parent    = nullptr;
		std::string name      = "GameObject";
		size_t      nameHash  = 0;
		Transform   transform = {};
	};
}

//namespace Core
//{
//	// TODO: sort components based on nameHash so binary search can be done
//	class LUNAR_API Scene_T;
//
//
//	class LUNAR_API GameObject : public Identifiable
//	{
//	public:
//		GameObject(const std::string_view& name, Scene* scene, GameObject* parent = nullptr);
//		GameObject();
//		~GameObject();
//
//		void                      update();
//		void                      renderUpdate(Render::RenderContext& context);
//        size_t                    getNameHash() const;
//        const std::string&        getName() const;
//		TransformComponent&       getTransform();
//		const TransformComponent& getTransform() const;
//
//		std::vector<GameObject*>  getChildren();
//		GameObject*               getParent();
//		Identifiable::NativeType  getParentId() const;
//		Scene*                    getParentScene();
//		std::span<std::shared_ptr<Component>> getComponents();
//		void                      addComponent(std::shared_ptr<Component> constructed);
//
//		template<typename T> requires IsDerivedComponent<T>
//		T* getComponent()
//		{
//			return static_cast<T*>(getComponent(typeid(T)));
//		}
//
//		template<typename T> requires IsDerivedComponent<T>
//		const T* getComponent() const
//		{
//			return static_cast<const T*>(getComponent(typeid(T)));
//		}
//
//		template <typename T, class... _Valty> requires IsDerivedComponent<T>
//		T& addComponent(_Valty&&... ctor_values)
//		{	
//			DEBUG_ASSERT(getComponent<T>() == nullptr, "There can exist only one component of type <T> on a single gameobject.");
//			std::shared_ptr<T> new_component = std::make_shared<T>(std::forward<_Valty>(ctor_values)...);
//			addComponent(new_component);
//			return *new_component;
//		}
//		
//	private:
//		Component* getComponent(const std::type_info& ty);
//
//        std::string name;
//		size_t nameHash;
//
//		TransformComponent transform;
//		std::vector<std::shared_ptr<Component>> components;
//
//		Scene* scene;
//		Identifiable::NativeType parent;	
//		
//		friend class Scene;
//	};
//}
