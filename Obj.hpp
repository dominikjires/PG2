#pragma once

#include <filesystem>
#include <map>

#include "Vertex.hpp"
#include "Mesh.hpp"
#include "ShaderProgram.hpp"

#define HEGHTMAP_SCALE 0.1f

class Obj
{
public:
    std::string name;

    Obj(std::string name, const std::filesystem::path& path_main, const std::filesystem::path& path_tex, glm::vec3 position, float scale, glm::vec4 init_rotation, bool is_height_map, bool use_aabb);
    void Draw(ShaderProgram& shader);
    void Clear();

    // Transformations
    glm::vec3 position{};
    float scale{};
    glm::vec4 rotation = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f); // axes xyz + angle (deg)

    //
    float _distance_from_camera; // for sorting transparent objects
    std::map<std::pair<float, float>, float> _heights; // for heightmap collision

    // Collision
    bool use_aabb;
    // - Bounding sphere
    glm::vec3 collision_bs_center{};
    float collision_bs_radius{};
    // - AABB
    glm::vec3 collision_aabb_min{};
    glm::vec3 collision_aabb_max{};
    // - Mehods
    bool Collision_CheckPoint(glm::vec3 point) const; // Check if point is inside this Model's collider
private:
    Mesh mesh;
    std::vector<Vertex> out_vertices{};
    std::vector<GLuint> out_uvs{};

    // For storing values in Draw()
    glm::mat4 mx_model{};
    glm::vec3 rotation_axes{};
    glm::vec3 init_rotation_axes{};

    // Transformations
    glm::vec4 init_rotation{}; // axes xyz + angle (deg); if model is weirdly rotated, it can be fixed with this rotation and other rotations are relative to this

    void LoadObj(const std::filesystem::path& file_name);

    // HeightMap
    void HeightMap(const std::filesystem::path& file_name);
    glm::vec2 HeightMap_GetSubtex(const float height);

};