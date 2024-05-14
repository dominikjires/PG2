#pragma once

#include <filesystem>
#include <map>

#include "Vertex.hpp"
#include "Mesh.hpp"
#include "ShaderProgram.hpp"

#define HEIGHTMAP_SCALE 0.1f 

class Obj
{
public:
    std::string name; // Name of the object

    Obj(std::string name, const std::filesystem::path& path_main, const std::filesystem::path& path_tex, glm::vec3 position, float scale, glm::vec4 init_rotation, bool is_height_map, bool use_aabb); // Constructor
    void Draw(ShaderProgram& shader); // Method to draw the object
    void Clear(); // Method to clear object data

    glm::vec3 position{}; // Position of the object
    float scale{}; // Scale of the object
    glm::vec4 rotation = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f); // Rotation of the object

    float distance_from_camera; // Distance of the object from the camera
    std::map<std::pair<float, float>, float> _heights; // Map to store height data

    bool use_aabb; // Flag indicating whether to use axis-aligned bounding box for collision
    glm::vec3 collision_bs_center{}; // Center of the bounding sphere for collision
    float collision_bs_radius{}; // Radius of the bounding sphere for collision
    glm::vec3 collision_aabb_min{}; // Minimum point of the axis-aligned bounding box
    glm::vec3 collision_aabb_max{}; // Maximum point of the axis-aligned bounding box
    bool CheckCollisionWithPoint(glm::vec3 point) const; // Method to check collision with a point

private:
    Mesh mesh; // Mesh object
    std::vector<Vertex> vertices{}; // Vector to store output vertices
    std::vector<GLuint> uv_coords{}; // Vector to store output texture coordinates

    glm::mat4 model_matrix{}; // Model matrix
    glm::vec3 rotation_axes{}; // Rotation axes
    glm::vec3 initial_rotation_axes{}; // Initial rotation axes

    glm::vec4 initial_rotation{}; // Initial rotation

    void LoadObj(const std::filesystem::path& file_name); // Method to load OBJ file
    void LoadHeightMap(const std::filesystem::path& file_name); // Method to load heightmap
    glm::vec2 HeightMap_GetSubtex(const float height); // Method to get subtexture from height
};
