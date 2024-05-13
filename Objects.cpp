#include <iostream>
#include <filesystem>
#include "App.hpp"
#include <opencv2/opencv.hpp>

// Function to create a model object
Obj* App::CreateModel(const std::string& name, const std::string& obj, const std::string& tex, bool is_opaque, const glm::vec3& position, float scale, const glm::vec4& rotation, bool collision, bool use_aabb)
{
    // Construct paths for model and texture
    std::filesystem::path modelpath("./resources/objects/" + obj);
    std::filesystem::path texturepath("./resources/textures/" + tex);

    // Create a new Obj instance
    auto model = new Obj(name, modelpath, texturepath, position, scale, rotation, false, use_aabb);

    // Insert the model into the appropriate scene container based on its opacity
    if (is_opaque) {
        scene_opaque.insert({ name, model });
    }
    else {
        scene_transparent.insert({ name, model });
    }

    // Add the model to the collisions vector if collision is enabled
    if (collision) {
        collisions.push_back(model);
    }

    return model;
}

// Function to update model rotations
void App::UpdateModel(float delta_time)
{
    auto updateRotation = [this](const std::string& name, float angle) {
        auto it_opaque = scene_opaque.find(name);
        auto it_transparent = scene_transparent.find(name);

        if (it_opaque != scene_opaque.end()) {
            it_opaque->second->rotation = glm::vec4(0.0f, 1.0f, 0.0f, angle);
        }
        else if (it_transparent != scene_transparent.end()) {
            it_transparent->second->rotation = glm::vec4(0.0f, 1.0f, 0.0f, angle);
        }
        };

    float angle = glfwGetTime() * 23.0f;
    for (int i = 1; i <= 10; ++i) {
        std::string name = "obj_sphere_" + std::to_string(i);
        updateRotation(name, angle * (i % 3 == 0 ? 2 : 1));
    }
}

// Function to initialize the scene
void App::InitScene()
{
    // Load shader programs
    std::filesystem::path VS_path("./resources/shaders/uber.vert");
    std::filesystem::path FS_path("./resources/shaders/uber.frag");
    my_shader = ShaderProgram(VS_path, FS_path);

    glm::vec3 position;
    float scale;
    glm::vec4 rotation;

    // Load heightmap object
    std::filesystem::path heightspath("./resources/textures/heights.png");
    std::filesystem::path texturepath("./resources/textures/tex_256.png");
    position = glm::vec3(-HEIGHTMAP_SHIFT, 0.0f, -HEIGHTMAP_SHIFT);
    scale = HEIGHTMAP_SCALE;
    rotation = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    auto obj_heightmap = new Obj("heightmap", heightspath, texturepath, position, scale, rotation, true, false);
    scene_opaque.insert({ "obj_heightmap", obj_heightmap });
    _heights = &obj_heightmap->_heights;

    // Create boxes
    position = glm::vec3(4.0f, 0.5f, 15.0f);
    scale = 0.2f;
    rotation = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    CreateModel("obj_box1", "box.obj", "box.png", true, position, scale, rotation, true, true);

    position = glm::vec3(2.98f, 0.5f, 15.0f);
    CreateModel("obj_box2", "box.obj", "box.png", true, position, scale, rotation, true, true);

    position = glm::vec3(3.5f, 1.5f, 15.0f);
    CreateModel("obj_box3", "box.obj", "box.png", true, position, scale, rotation, true, true);

    // Create spheres in a circular pattern
    scale = 0.5f;
    rotation = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

    float radius = 4.0f;
    float centerX = 3.5f;
    float centerY = 3.0f;
    float centerZ = 9.2f;
    float angleIncrement = glm::radians(360.0f / 10.0f);

    for (int i = 0; i < 10; ++i) {
        float angle = i * angleIncrement;
        float x = centerX + radius * cos(angle);
        float z = centerZ + radius * sin(angle);
        position = glm::vec3(x, centerY, z);

        std::string modelName = "obj_sphere_" + std::to_string(i + 1);
        CreateModel(modelName, "sphere_tri_vnt.obj", "disco.jpg", false, position, scale, rotation, true, false);
    }

    // Create projectiles
    glm::vec3 initialPosition(0.0f, 0.0f, 0.0f);
    float initialScale = 0.1f;
    glm::vec4 initialRotation(0.0f, 1.0f, 0.0f, 0.0f);

    for (int i = 0; i < PROJECTILES_N; ++i) {
        std::string projectileName = "obj_projectile_" + std::to_string(i);

        auto projectileModel = CreateModel(projectileName, "sphere_tri_vnt.obj", "ball.jpg", true,
            initialPosition, initialScale, initialRotation, false, false);

        projectiles[i] = projectileModel;
    }

    // Populate transparent scene pairs for rendering
    for (auto& pair : scene_transparent) {
        scene_transparent_pairs.push_back(&pair);
    }
}
