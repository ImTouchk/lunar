#pragma once
#include "math/vec.hpp"
#include <vector>
#include <any>

using Shader = unsigned int;

class CMesh
{
public:
    CMesh() = default;
    ~CMesh() = default;

    void set_vertices(const std::vector<vec3f> vertices);
    void set_indices(const std::vector<unsigned> indices);
    void set_active(bool new_state);
    void use_shader(Shader shader);

    [[nodiscard]] bool active() const;
    [[nodiscard]] std::any& internal_data() const;
private:
    std::any data = 0;
    bool is_active = false;
};