#include <iostream>
#include <chrono>
#include <stack>
#include <random>
#include <sstream>

#include <opencv2/opencv.hpp>
#include <GL/glew.h> 
#include <GL/wglew.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "App.hpp"
#include "gl_err_callback.hpp"
#include "ShaderProgram.hpp"

// Constructor
App::App()
{
    std::cout << "Constructed\n";
}

// Initialization function
bool App::Init()
{
    try {
        // Set GLFW error callback
        glfwSetErrorCallback(error_callback);

        // Initialize GLFW
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW\n";
            return false;
        }

        // Set OpenGL version and profile hints
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        // Create GLFW window
        window = glfwCreateWindow(window_width, window_height, "PG2", nullptr, nullptr);
        if (!window) {
            std::cerr << "Failed to create GLFW window\n";
            glfwTerminate();
            return false;
        }

        // Set user pointer for callbacks
        glfwSetWindowUserPointer(window, this);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // Get primary monitor and video mode
        monitor = glfwGetPrimaryMonitor();
        mode = glfwGetVideoMode(monitor);

        // Make the OpenGL context current
        glfwMakeContextCurrent(window);
        glfwSetKeyCallback(window, key_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        // Enable Vsync
        glfwSwapInterval(1);
        is_vsync_on = true;

        // Initialize GLEW
        GLenum err = glewInit();
        if (GLEW_OK != err) {
            std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << "\n";
            glfwTerminate();
            return false;
        }

        // Initialize WGLEW
        if (wglewInit() != GLEW_OK) {
            std::cerr << "Failed to initialize WGLEW\n";
            glfwTerminate();
            return false;
        }

        // Enable OpenGL debug output if supported
        if (GLEW_ARB_debug_output) {
            glDebugMessageCallback(MessageCallback, nullptr);
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            std::cout << "GL_DEBUG enabled.\n";
        }
        else {
            std::cout << "GL_DEBUG NOT SUPPORTED!\n";
        }

        // Enable OpenGL features
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_POLYGON_SMOOTH);
        glEnable(GL_CULL_FACE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Initialize scene
        InitScene();
        glfwShowWindow(window);

        std::cout << "Initialized\n";
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception during initialization: " << e.what() << "\n";
        if (window) {
            glfwDestroyWindow(window);
        }
        glfwTerminate();
        return false;
    }
    catch (...) {
        std::cerr << "Unknown exception during initialization\n";
        if (window) {
            glfwDestroyWindow(window);
        }
        glfwTerminate();
        return false;
    }
}

// Main application loop
int App::Run(void) {
    try {
        // Initialize timing variables
        double currentFrameTime = glfwGetTime();
        double lastFrameTime = currentFrameTime;
        double fpsCounterSeconds = 0;
        int fpsCounterFrames = 0;

        // Update projection matrix and viewport
        UpdateProjection();
        glViewport(0, 0, window_width, window_height);

        // Initialize camera position and movement
        camera.position = { 1.0f, 1.0f, 1.0f };
        glm::vec3 camera_movement{};


        double cursor_x, cursor_y;

        double lastWalkTime = currentFrameTime;
        const double walkingDelay = 0.4;
        const double sprintingDelay = 0.2;

        float fallingSpeed = 0;
        float jumpingSpeed = 0;
        bool isGrounded = true;


        // Main loop
        while (!glfwWindowShouldClose(window)) {
            // Timing
            currentFrameTime = glfwGetTime();
            auto fps_frame_start_timestamp = std::chrono::steady_clock::now();

            // Clear buffers
            glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Calculate delta time
            float delta_time = static_cast<float>(currentFrameTime - lastFrameTime);
            lastFrameTime = currentFrameTime;

            // Process camera input
            camera_movement = camera.ProcessInput(window, delta_time);
            camera.position += glm::vec3(camera_movement.x, 0.0f, camera_movement.z);

            bool is_space_pressed = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
            auto heightmap_y = GetHeightmapY(camera.position.x, camera.position.z);
            float min_height = heightmap_y + PLAYER_HEIGHT;

            // Handle walking and sprinting
            if ((camera_movement.x != 0 || camera_movement.z != 0) && isGrounded) {
                if ((!camera.sprint && currentFrameTime > lastWalkTime + walkingDelay) ||
                    (camera.sprint && currentFrameTime > lastWalkTime + sprintingDelay)) {
                    audio.PlayWalk();
                    lastWalkTime = currentFrameTime;
                }
            }
            else {
                lastWalkTime = currentFrameTime;
            }

            // Handle mouselook
            if (is_mouselook_on) {
                glfwGetCursorPos(window, &cursor_x, &cursor_y);
                camera.ProcessMouseMovement(static_cast<GLfloat>(window_width / 2.0 - cursor_x),
                    static_cast<GLfloat>(window_height / 2.0 - cursor_y));
                glfwSetCursorPos(window, window_width / 2.0, window_height / 2.0);
            }

            // Handle jumping and falling
            if (is_space_pressed && isGrounded) {
                jumpingSpeed = 5.0f;
                audio.PlayJump();
            }

            if (jumpingSpeed > 0.0f) {
                fallingSpeed = 0;
                jumpingSpeed -= delta_time * 9.81f;
                camera.position.y += delta_time * jumpingSpeed;
                if (camera.position.y < min_height) {
                    camera.position.y = min_height;
                }
                isGrounded = false;
            }
            else {
                fallingSpeed += delta_time * 9.81f;
                camera.position.y -= delta_time * fallingSpeed;
                if (camera.position.y < min_height) {
                    camera.position.y = min_height;
                    fallingSpeed = 0;
                    if (!isGrounded) {
                        audio.PlayLand();
                    }
                    isGrounded = true;
                }
                else if (isGrounded && camera.position.y - min_height > 1.0f) {
                    isGrounded = false;
                }
            }

            // Update view matrix
            glm::mat4 mx_view = camera.GetViewMatrix();
            UpdateModel(delta_time);
            UpdateProjectiles(delta_time);

            // Activate shader program and set uniforms
            my_shader.Activate();
            my_shader.SetUniform("u_mx_view", mx_view);
            my_shader.SetUniform("u_mx_projection", mx_projection);
            my_shader.SetUniform("u_ambient_alpha", 0.0f);
            my_shader.SetUniform("u_diffuse_alpha", 0.7f);
            my_shader.SetUniform("u_camera_position", camera.position);
            my_shader.SetUniform("u_material.ambient", glm::vec3(0.1f));
            my_shader.SetUniform("u_material.specular", glm::vec3(1.0f));
            my_shader.SetUniform("u_material.shininess", 96.0f);
            my_shader.SetUniform("u_directional_light.direction", glm::vec3(0.0f, -0.9f, -0.17f));
            my_shader.SetUniform("u_directional_light.diffuse", glm::vec3(0.8f));
            my_shader.SetUniform("u_directional_light.specular", glm::vec3(0.14f));

            // Draw opaque objects
            for (auto& [key, value] : scene_opaque) {
                value->Draw(my_shader);
            }

            // Handle transparent objects
            glEnable(GL_BLEND);
            glDisable(GL_CULL_FACE);
            glDepthMask(GL_FALSE);

            // Sort transparent objects
            for (auto& transparent_pair : scene_transparent_pairs) {
                transparent_pair->second->_distance_from_camera = glm::length(camera.position - transparent_pair->second->position);
            }

            std::sort(scene_transparent_pairs.begin(), scene_transparent_pairs.end(),
                [](std::pair<const std::string, Obj*>*& a, std::pair<const std::string, Obj*>*& b) {
                    return a->second->_distance_from_camera > b->second->_distance_from_camera;
                });

            // Draw transparent objects
            for (auto& transparent_pair : scene_transparent_pairs) {
                transparent_pair->second->Draw(my_shader);
            }

            // Reset OpenGL state
            glDisable(GL_BLEND);
            glEnable(GL_CULL_FACE);
            glDepthMask(GL_TRUE);

            // Swap buffers and poll events
            glfwSwapBuffers(window);
            glfwPollEvents();

            // Calculate FPS
            auto fps_frame_end_timestamp = std::chrono::steady_clock::now();
            std::chrono::duration<double> fps_elapsed_seconds = fps_frame_end_timestamp - fps_frame_start_timestamp;
            fpsCounterSeconds += fps_elapsed_seconds.count();
            fpsCounterFrames++;

            // Update FPS counter
            if (fpsCounterSeconds >= 1) {
                FPS = fpsCounterFrames;
                fpsCounterSeconds = 0;
                fpsCounterFrames = 0;
            }

            // Set window title with FPS
            std::stringstream ss;
            ss << FPS << " FPS";
            glfwSetWindowTitle(window, ss.str().c_str());
        }
    }
    catch (const std::exception& e) {
        std::cerr << "App failed: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    // Output OpenGL information
    std::cout << "\nOpenGL Information\n";
    std::cout << "Vendor:\t\t" << glGetString(GL_VENDOR) << "\n";
    std::cout << "Renderer:\t" << glGetString(GL_RENDERER) << "\n";
    std::cout << "OpenGL Version:\t" << glGetString(GL_VERSION) << "\n";
    std::cout << "GLSL Version:\t" << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";

    GLint profile;
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);
    if (profile & GL_CONTEXT_CORE_PROFILE_BIT) {
        std::cout << "Profile:\tCore Profile\n";
    }
    else {
        std::cout << "Profile:\tCompatibility Profile\n";
    }

    if (const auto errorCode = glGetError()) {
        std::cout << "Pending GL error while obtaining profile: " << errorCode << "\n";
    }

    std::cout << "Finished\n";
    return EXIT_SUCCESS;
}


App::~App()
{
    my_shader.Clear();

    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();

    exit(EXIT_SUCCESS);
    std::cout << "Bye...\n";
}

// Update the projection matrix based on window dimensions
void App::UpdateProjection() {
    const float minWindowHeight = 1.0f;
    window_height = std::max(window_height, static_cast<int>(minWindowHeight));
    const float aspectRatio = static_cast<float>(window_width) / window_height;
    const float nearClipPlane = 0.1f;
    const float farClipPlane = 20000.0f;
    const float verticalFOV = glm::radians(FOV);
    mx_projection = glm::perspective(verticalFOV, aspectRatio, nearClipPlane, farClipPlane);
}

