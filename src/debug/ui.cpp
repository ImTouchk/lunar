//#include <lunar/debug/ui.hpp>
//#include <lunar/core/time.hpp>
//#include <lunar/core/input.hpp>
//
//namespace Debug
//{
//    void DrawComponentPanel(Render::RenderContext& ctx, Core::Component* component)
//    {
//        auto title = std::format("Component: {}", component->_getClassName());
//        if (ImGui::CollapsingHeader(title.c_str()))
//        {
//            ImGui::Indent();
//            component->drawDebugUI(ctx);
//            ImGui::Unindent();
//        }
//    }
//
//    void DrawGameObjectPanel(Render::RenderContext& ctx, Core::GameObject& object)
//    {
//        auto title = std::format("GameObject: {} (ID: {})", object.getName(), object.getId());
//
//        ImGui::TreePush(object.getName().c_str());
//        if (ImGui::CollapsingHeader(title.c_str()))
//        {
//            ImGui::Indent();
//
//            if (ImGui::Button("Delete"))
//            {
//                auto* scene = object.getParentScene();
//                scene->deleteGameObject(object.getId());
//            }
//
//            ImGui::SeparatorText("Transform");
//
//            auto& transform = object.getTransform();
//
//            ImGui::InputFloat3("Position", &transform.position.x);
//            ImGui::InputFloat3("Rotation", &transform.rotation.x);
//            ImGui::InputFloat3("Scale", &transform.scale.x);
//
//            ImGui::SeparatorText("Components");
//            auto components = object.getComponents();
//            for (auto& component : components)
//                DrawComponentPanel(ctx, component.get());
//
//            if (components.size() == 0)
//                ImGui::Text("This object has no components.");
//
//            auto children = object.getChildren();
//            ImGui::SeparatorText("Children");
//            for (auto& child : children)
//                DrawGameObjectPanel(ctx, *child);
//
//            if (children.size() == 0)
//                ImGui::Text("This object has no children.");
//
//            ImGui::Unindent();
//        }
//        ImGui::TreePop();
//    }
//
//	void DrawSceneHierarchyPanel(Render::RenderContext& context, Core::Scene& scene)
//	{
//        auto& objects = scene.getGameObjects();
//        auto  title   = std::format("Scene: {}", scene.getName());
//
//        ImGui::SetCurrentContext(context.getImGuiContext());
//        ImGui::Begin(title.c_str());
//
//        ImGui::Text("Scene ID: %d", scene.getId());
//        ImGui::Text("Scene objects: %d", objects.size());
//
//        ImGui::SeparatorText("GameObjects");
//        for (auto& object : objects)
//            if (object.getParentId() == -1)
//                DrawGameObjectPanel(context, object);
//
//        ImGui::End();
//	}
//
//    void DrawGeneralInfoPanel(Render::RenderContext& context)
//    {
//        ImGui::SetCurrentContext(context.getImGuiContext());
//        ImGui::Begin("General Info");
//        ImGui::SeparatorText("General");
//        ImGui::Text("Application: %s (v%d.%d.%d)", APP_NAME, APP_VER_MAJOR, APP_VER_MINOR, APP_VER_PATCH);
//        ImGui::Text("Engine: lunar (v%d.%d.%d)", LUNAR_VER_MAJOR, LUNAR_VER_MINOR, LUNAR_VER_PATCH);
//        
//        ImGui::Text("Debug build: ");
//        ImGui::SameLine();
//        ImGui::TextColored(ImVec4(0.5f, 1.f, 0.5f, 1.f), "%s", LUNAR_DEBUG_BUILD ? "TRUE" : "FALSE");
//
//        ImGui::SeparatorText("Rendering");
//        ImGui::Text("Render backend: ");
//        ImGui::SameLine();
//        ImGui::TextColored(ImVec4(0.5f, 1.f, 0.5f, 1.f), "%s", LUNAR_RENDER_BACKEND);
//
//        auto* target = context.getCurrentTarget();
//        ImGui::Text("Render resolution: ");
//        ImGui::SameLine();
//        ImGui::TextColored(ImVec4(0.5f, 1.f, 0.5f, 1.f), "%dx%d", target->getRenderWidth(), target->getRenderHeight());
//
//        ImGui::Text("Average FPS: ");
//        ImGui::SameLine();
//
//        auto* time_context = Time::GetGlobalContext();
//        auto  fps_color = (time_context->fps < 30)
//            ? ImVec4(1, 0, 0, 1) 
//            : (
//                (time_context->fps < 60) 
//                    ? ImVec4(1, 1, 0, 1) 
//                    : ImVec4(0, 1, 0, 1)
//            );
//
//        ImGui::TextColored(fps_color, "%d", time_context->fps.load());
//
//        auto& handler  = lunar::Input::GetGlobalHandler();
//        auto  axis     = handler.getAxis();
//        auto  rotation = handler.getRotation();
//        ImGui::SeparatorText("Input");
//        ImGui::Text("Axis: { %f, %f }", axis.x, axis.y);
//        ImGui::Text("Rotation: { %f, %f }", rotation.x, rotation.y);
//
//        ImGui::End();
//    }
//}
