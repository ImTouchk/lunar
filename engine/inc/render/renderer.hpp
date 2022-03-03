#pragma once
#include <any>
#include <vector>
#include <cstdint>

#include "render/mesh.hpp"
#include "math/vec.hpp"

class GameWindow;

struct RendererCreateInfo
{
    GameWindow* pWindow;
};

class GameRenderer
{
public:
    GameRenderer() = default;
    ~GameRenderer() = default;

    void create(RendererCreateInfo&& createInfo);
    void destroy();

    [[nodiscard]] std::vector<Shader> create_shaders(GraphicsShaderCreateInfo* pCreateInfos, unsigned count);
    [[nodiscard]] MeshWrapper create_object(MeshCreateInfo&& meshCreateInfo);

    void draw();

private:
    std::any backend_data = 0;
    GameWindow* window_handle = nullptr;
    bool is_active = false;
    bool is_window_minimized = false;
};