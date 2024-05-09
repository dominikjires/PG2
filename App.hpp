#pragma once

#include <map>

#include "Obj.hpp"
#include "ShaderProgram.hpp"
#include "Camera.hpp"
#include "Audio.hpp"

#define PLAYER_HEIGHT 1.0f      // Camera above ground
#define HEIGHTMAP_SHIFT 50.0f   // Heightmap is shifted by this value on x and z coordinates
#define N_PROJECTILES 10        // How many projectiles are there in the pool

#define HIDE_CUBES_INSTEAD_DESTROY true // If hit by projectile, glass cubes are hidden under ground instead of removed from scene ('R' key does nothing if false)
#define HIDE_CUBE_Y 10.0f               // Hide cubes by subtracting this from their Y coordinate

class App {
public:
    App();

    bool Init();
    void InitAssets();
    int Run(); // Run every frame
    Obj* CreateModel(std::string name, std::string obj, std::string tex, bool is_opaque, glm::vec3 position, float scale, glm::vec4 rotation, bool collision, bool use_aabb);
    void UpdateModels(float delta_time); // Inside Run(); time based update of objects in the scene

    ~App();
private:
    std::map<std::string, Obj*> scene_opaque;
    std::map<std::string, Obj*> scene_transparent;
    std::vector<std::pair<const std::string, Obj*>*> scene_transparent_pairs; // Used for sorting transparent scene

    bool is_vsync_on{};
    bool is_fullscreen_on = false;
    bool is_mouselook_on = true;

    GLFWmonitor* monitor{};
    const GLFWvidmode* mode{};
    int window_xcor{};
    int window_ycor{};
    int window_width = 1280;
    int window_height = 800;
    int window_width_return_from_fullscreen{};
    int window_height_return_from_fullscreen{};

    float FOV = 110.0f;
    int FPS = 0;
    glm::mat4 mx_projection = glm::identity<glm::mat4>();
    Camera camera = Camera(glm::vec3(0, 0, 0));

    GLFWwindow* window = nullptr;
    glm::vec4 clear_color = glm::vec4(243 / 255.0f, 196 / 255.0f, 128 / 255.0f, 0.0f);

    void UpdateProjectionMatrix();

    void PrintGLInfo();

    static void error_callback(int error, const char* description);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    ShaderProgram my_shader;

    Audio audio;

    int is_flashlight_on = 1;

    // Jukebox
    Obj* obj_jukebox{};
    glm::vec2 jukebox_to_player{};
    glm::vec2 jukebox_to_player_n{};
    int is_jukebox_on = 1;

    // Heightmap
    std::map<std::pair<float, float>, float>* _heights{};
    float GetHeightmapY(float position_x, float position_z) const;

    // Collision
    std::vector<Obj*> collisions; // All objects projectile can collide with

    // Projectiles
    const float projectile_speed = 20.0f;
    int projectile_n = 0;                   // Currently used projectile
    Obj* projectiles[N_PROJECTILES]{};    // Pool of projectiles
    glm::vec3 projectile_directions[N_PROJECTILES]{};
    bool is_projectile_moving[N_PROJECTILES]{};
    void Shoot();
    void UpdateProjectiles(float delta_time);
};