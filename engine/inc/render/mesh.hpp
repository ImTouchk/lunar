#pragma once
#include "utils/identifier.hpp"
#include "render/texture.hpp"
#include "render/shader.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <any>

#ifdef VULKAN_RENDERER
namespace Vk
{
    class ObjectManager;  
}
#elif OPENGL_RENDERER
namespace GL
{
    struct RendererInternalData;
}
#endif

enum class MeshType
{
    eUnknown = 0,
    eStatic,
    eDynamic
};

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_uv;
};

using Index = uint16_t;

struct MeshCreateInfo
{
    MeshType type;
    const std::vector<Vertex>& vertices;
    const std::vector<Index>& indices;
    ShaderWrapper shader;
};

class MeshWrapper
{
public:
#ifdef VULKAN_RENDERER
    explicit MeshWrapper(Vk::ObjectManager& objectManager, Identifier handle);
#elif OPENGL_RENDERER
    explicit MeshWrapper(GL::RendererInternalData& internalData, Identifier handle);
#endif
    ~MeshWrapper() = default;

    void set_vertices(std::vector<Vertex>&& vertices);
    void set_indices(std::vector<Index>&& indices);
    void set_active(bool new_state);

    void set_transform(glm::mat4&& transform);

    void use_shader(ShaderWrapper& shader);
    void use_texture(TextureWrapper&& texture);

    void create(void* pMeshVector, unsigned handle);
    void destroy();

    [[nodiscard]] bool active() const;
private:
#ifdef VULKAN_RENDERER
    Vk::ObjectManager& objectManager;
#elif OPENGL_RENDERER
    GL::RendererInternalData& rendererData;
#endif
    Identifier identifier;
};
