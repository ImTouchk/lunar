#pragma once
#include <any>
#include <vector>
#include <cstdint>

#include "render/mesh.hpp"
#include "render/texture.hpp"
#include "math/vec.hpp"

class CGameWindow;

struct RendererCreateInfo
{
    CGameWindow* pWindow;
};

class CRenderer
{
public:
    CRenderer() = default;
    ~CRenderer() = default;

    void create(RendererCreateInfo&& createInfo);
    void destroy();

    [[nodiscard]] std::vector<ShaderWrapper> create_shaders(GraphicsShaderCreateInfo* pCreateInfos, unsigned count);
    [[nodiscard]] std::vector<ShaderWrapper> create_shaders(ComputeShaderCreateInfo* pCreateInfos, unsigned count);

    [[nodiscard]] MeshWrapper create_object(MeshCreateInfo&& meshCreateInfo);
    [[nodiscard]] TextureWrapper create_texture(TextureCreateInfo&& textureCreateInfo);

    void draw();

private:
    std::any backend_data = 0;
    CGameWindow* window_handle = nullptr;
    bool is_active = false;
    bool is_window_minimized = false;
};