#pragma once

// Standard library includes
#include <map>
#include <vector>
#include <string>
#include <utility>

// Project-specific includes
#include "Obj.hpp"
#include "ShaderProgram.hpp"
#include "Camera.hpp"
#include "Audio.hpp"

// Constants used in the application
constexpr float PLAYER_HEIGHT = 1.0f; // Height of the player in the game world
constexpr float HEIGHTMAP_SHIFT = 50.0f; // Offset for the heightmap
constexpr int PROJECTILES_COUNT = 10; // Maximum number of projectiles
constexpr bool USE_HIDE_CUBES = true; // Flag to determine if hide cubes are used
constexpr float SPHERE_HIDE_DISTANCE = 30.0f; // Distance at which spheres become hidden

// Main application class
class App {
public:
    App(); // Constructor
    ~App(); // Destructor

    bool Init(); // Initializes the application
    void InitScene(); // Initializes the scene
    int Run(); // Runs the main application loop

    // Creates a new model in the scene
    Obj* CreateModel(const std::string& name, const std::string& obj, const std::string& tex, bool is_opaque,
        const glm::vec3& position, float scale, const glm::vec4& rotation, bool collision, bool use_aabb);
    void UpdateModel(float delta_time); // Updates the model with the given delta time

private:
    // Containers for the scene objects
    std::map<std::string, Obj*> opaque_scene; // Opaque objects in the scene
    std::map<std::string, Obj*> transparent_scene; // Transparent objects in the scene
    std::vector<std::pair<const std::string, Obj*>*> transparent_scene_pairs; // Pairs of transparent objects

    // Application settings
    bool vsync_enabled = false; // VSync setting
    bool fullscreen_enabled = false; // Fullscreen setting
    bool mouselook_enabled = true; // Mouse look setting
    int flashlight_enabled = 1; // Flashlight setting
    float light_intensity = 0.7f; // Light intensity
    GLFWmonitor* primary_monitor = nullptr; // Pointer to the primary monitor
    const GLFWvidmode* video_mode = nullptr; // Pointer to the video mode
    int window_xcor = 0; // Window x-coordinate
    int window_ycor = 0; // Window y-coordinate
    int window_width = 1280; // Window width
    int window_height = 800; // Window height
    int window_width_return_from_fullscreen = 0; // Window width when returning from fullscreen
    int window_height_return_from_fullscreen = 0; // Window height when returning from fullscreen

    float FOV = 110.0f; // Field of view
    int FPS = 0; // Frames per second
    glm::mat4 mx_projection = glm::identity<glm::mat4>(); // Projection matrix
    Camera camera = Camera(glm::vec3(0, 0, 0)); // Camera object

    GLFWwindow* window = nullptr; // Pointer to the GLFW window
    glm::vec4 clear_color = glm::vec4(243.0f / 255.0f, 196.0f / 255.0f, 128.0f / 255.0f, 0.0f); // Clear color

    ShaderProgram my_shader; // Shader program object
    Audio audio; // Audio object

    std::map<std::pair<float, float>, float>* _heights = nullptr; // Pointer to the heightmap
    float GetHeightmapY(float position_x, float position_z) const; // Gets the height at a specific position

    std::vector<Obj*> collisions; // List of objects involved in collisions

    const float projectile_speed = 10.0f; // Speed of projectiles
    Obj* projectiles[PROJECTILES_COUNT]{}; // Array of projectile objects
    glm::vec3 projectile_directions[PROJECTILES_COUNT]{}; // Directions of projectiles
    bool is_projectile_moving[PROJECTILES_COUNT]{}; // Flags indicating if projectiles are moving
    int number_of_projectiles = 0; // Number of projectiles

    void Shoot(); // Shoots a projectile
    void UpdateProjectiles(float delta_time); // Updates the projectiles
    bool CheckCollision(const glm::vec3& position); // Checks for collisions at a specific position

    void UpdateProjection(); // Updates the projection matrix
    static void error_callback(int error, const char* description); // GLFW error callback
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods); // GLFW key callback
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods); // GLFW mouse button callback
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height); // GLFW framebuffer size callback
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset); // GLFW scroll callback
};
