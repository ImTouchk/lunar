#pragma once
#include <lunar/core/component.hpp>
#include <lunar/file/json_file.hpp>
#include <lunar/utils/identifiable.hpp>
#include <lunar/debug.hpp>
#include <lunar/api.hpp>
#include <concepts>
#include <string>
#include <memory>
#include <vector>

namespace Render { class LUNAR_API RenderContext; }

namespace Core
{
	// TODO: sort components based on nameHash so binary search can be done
	class LUNAR_API Scene;

	class LUNAR_API GameObject : public Identifiable
	{
	public:
		GameObject(const std::string_view& name, Scene* scene, GameObject* parent = nullptr);
		GameObject();
		~GameObject();

		void                      update();
		void                      renderUpdate(Render::RenderContext& context);
        size_t                    getNameHash() const;
        const std::string&        getName() const;
		TransformComponent&       getTransform();
		const TransformComponent& getTransform() const;

		std::vector<GameObject*>  getChildren();
		GameObject*               getParent();
		Identifiable::NativeType  getParentId() const;
		Scene*                    getParentScene();
		std::span<std::shared_ptr<Component>> getComponents();
		void                      addComponent(std::shared_ptr<Component> constructed);

		template<typename T> requires IsDerivedComponent<T>
		T* getComponent()
		{
			return static_cast<T*>(getComponent(typeid(T)));
		}

		template<typename T> requires IsDerivedComponent<T>
		const T* getComponent() const
		{
			return static_cast<const T*>(getComponent(typeid(T)));
		}

		template <typename T, class... _Valty> requires IsDerivedComponent<T>
		T& addComponent(_Valty&&... ctor_values)
		{	
			DEBUG_ASSERT(getComponent<T>() == nullptr, "There can exist only one component of type <T> on a single gameobject.");
			std::shared_ptr<T> new_component = std::make_shared<T>(std::forward<_Valty>(ctor_values)...);
			addComponent(new_component);
			return *new_component;
		}
		
	private:
		Component* getComponent(const std::type_info& ty);

        std::string name;
		size_t nameHash;

		TransformComponent transform;
		std::vector<std::shared_ptr<Component>> components;

		Scene* scene;
		Identifiable::NativeType parent;	
		
		friend class Scene;
	};
}
