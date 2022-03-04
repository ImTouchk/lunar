#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <any>

using Shader = unsigned int;

enum class MeshType
{
    eUnknown = 0,
    eStatic,
    eDynamic
};

struct GraphicsShaderCreateInfo
{
    std::vector<char>& vertexCode;
    std::vector<char>& fragmentCode;
};

using Vertex = glm::vec3;
using Index = uint16_t;

struct MeshCreateInfo
{
    MeshType type;
    const std::vector<Vertex>& vertices;
    const std::vector<Index>& indices;
    Shader shader;
};

namespace Vk
{
    class ObjectManager;
}

class MeshWrapper
{
public:
    MeshWrapper(Vk::ObjectManager& objectManager, unsigned handle);
    ~MeshWrapper() = default;

    void set_vertices(std::vector<Vertex>&& vertices);
    void set_indices(std::vector<Index>&& indices);
    void set_active(bool new_state);
    void use_shader(Shader shader);

    void create(void* pMeshVector, unsigned handle);
    void destroy();

    [[nodiscard]] bool active() const;
private:
    Vk::ObjectManager& objectManager;
    unsigned identifier;
};
