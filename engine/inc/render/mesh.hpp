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

class MeshWrapper
{
public:
    MeshWrapper() = default;
    ~MeshWrapper() = default;

    void set_vertices(const std::vector<Vertex>& vertices);
    void set_indices(const std::vector<Index>& indices);
    void set_active(bool new_state);
    void use_shader(Shader shader);

    void create(void* pMeshVector, unsigned handle);
    void destroy();

    [[nodiscard]] bool active() const;
private:
    void* pMeshVector = nullptr;
    unsigned handle = 0;
    bool is_active = false;
};
