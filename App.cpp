
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

#define print(x)// std::cout << x << "\n"

App::App()
{
    std::cout << "Constructed\n";
}

bool App::Init()
{
    try {
        // Set GLFW error callback
        glfwSetErrorCallback(error_callback);

        // Init GLFW
        if (!glfwInit()) {
            return false;
        }

        // Set OpenGL version
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Window is hidden until everything is initialized
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        // Open window
        window = glfwCreateWindow(window_width, window_height, "PG2", NULL, NULL);
        if (!window) {
            glfwTerminate();
            return false;
        }
        glfwSetWindowUserPointer(window, this);

        // Hide cursor
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // Fullscreen On/Off
        monitor = glfwGetPrimaryMonitor();
        mode = glfwGetVideoMode(monitor);

        // Setup callbacks
        glfwMakeContextCurrent(window);
        glfwSetKeyCallback(window, key_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        // Set V-Sync ON.
        glfwSwapInterval(1);
        is_vsync_on = true;

        // Init GLEW
        GLenum err = glewInit();
        if (GLEW_OK != err) {
            // Handle initialization error
        }
        wglewInit();

        // Debug output setup
        if (GLEW_ARB_debug_output)
        {
            glDebugMessageCallback(MessageCallback, 0);
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

            std::cout << "GL_DEBUG enabled.\n";
        }
        else std::cout << "GL_DEBUG NOT SUPPORTED!\n";

        // OpenGL configuration
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_POLYGON_SMOOTH);
        glEnable(GL_CULL_FACE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // First init OpenGL, THAN init assets
        InitScene();

        // Show window after everything loads        
        glfwShowWindow(window);
    }
    catch (std::exception const& e) {
        // Handle exception
        exit(-1);
    }
    std::cout << "Initialized...\n";
    return true;
}


int App::Run(void)
{
    try {
        double current_walk = glfwGetTime();
        double last_frame_time = current_walk;

        // FPS counting
        double fps_counter_seconds = 0;
        int fps_counter_frames = 0;

        // Init view
        UpdateProjection();
        glViewport(0, 0, window_width, window_height);

        // Camera
        camera.position.x = 1.0f;
        camera.position.y = 1.0f;
        camera.position.z = 1.0f;

        glm::vec3 camera_movement{};

        // Mouselook
        double cursor_x, cursor_y;

        // Walking sound
        double last_walk = current_walk;
        const double walking_delay = 0.4;
        const double sprinting_delay = 0.2;

        // Jumping
        float falling_speed = 0;
        float jumping_speed = 0;
        bool is_grounded = true;

        // Main loop
        while (!glfwWindowShouldClose(window)) {
            current_walk = glfwGetTime();

            // Time/FPS measure start
            auto fps_frame_start_timestamp = std::chrono::steady_clock::now();

            // Clear OpenGL canvas, both color buffer and Z-buffer
            glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // === After clearing the canvas ===
            float delta_time = static_cast<float>(current_walk - last_frame_time);
            last_frame_time = current_walk;

            // Player movement
            camera_movement = camera.ProcessInput(window, delta_time);
            camera.position.x += camera_movement.x;
            camera.position.z += camera_movement.z;

            // Movement sound
            if ((camera_movement.x != 0 || camera_movement.z != 0) && is_grounded) {
                if ((!camera.sprint && current_walk > last_walk + walking_delay)
                    || (camera.sprint && current_walk > last_walk + sprinting_delay)) {
                    audio.PlayWalk();
                    last_walk = current_walk;
                }
            }
            else {
                last_walk = current_walk;
            }

            // Mouselook � get cursor's offset from window center and the move it back to center
            if (is_mouselook_on) {
                glfwGetCursorPos(window, &cursor_x, &cursor_y);
                camera.ProcessMouseMovement(static_cast<GLfloat>(window_width / 2.0 - cursor_x), static_cast<GLfloat>(window_height / 2.0 - cursor_y));
                glfwSetCursorPos(window, window_width / 2.0, window_height / 2.0);
            }
            bool is_space_pressed = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
            // Heightmap collision � for our X and Z get Y coordinate for ground level
            auto heightmap_y = GetHeightmapY(camera.position.x, camera.position.z);
            // Jumping
            float min_hei = heightmap_y + PLAYER_HEIGHT;// Camera's smallest Y coordinate possible
            if (is_space_pressed && is_grounded) {
                jumping_speed = 5.0f;
                audio.PlayJump();
            }
            if (jumping_speed > 0.0f) {             // If holding space
                falling_speed = 0;
                jumping_speed -= delta_time * 9.81f;
                camera.position.y += delta_time * jumping_speed;
                if (camera.position.y < min_hei) {      // For going up steep hills, so we cannot go into the hill
                    camera.position.y = min_hei;
                }
                is_grounded = false;
            }
            else {
                // If not holding space
                falling_speed += delta_time * 9.81f;            // Gravity
                camera.position.y -= delta_time * falling_speed;// Fall
                if (camera.position.y < min_hei) {              // Do not fall through ground
                    camera.position.y = min_hei;
                    falling_speed = 0;
                    if (!is_grounded) {                         // Landing sound
                        audio.PlayLand();
                    }
                    is_grounded = true;
                }
                else if (is_grounded && camera.position.y - min_hei > 1.0f) {
                    is_grounded = false;
                }
            }

            // Create View Matrix according to camera settings
            glm::mat4 mx_view = camera.GetViewMatrix();

            // Update objects

            UpdateModel(delta_time);
            UpdateProjectiles(delta_time);

            // Activate shader
            my_shader.Activate();

            // Set shader uniform variables
            my_shader.SetUniform("u_mx_view", mx_view); // World space -> Camera space
            my_shader.SetUniform("u_mx_projection", mx_projection); // Camera space -> Screen

            // UBER
            my_shader.SetUniform("u_ambient_alpha", 0.0f);
            my_shader.SetUniform("u_diffuse_alpha", 0.7f);
            my_shader.SetUniform("u_camera_position", camera.position);

            // - AMBIENT
            my_shader.SetUniform("u_material.ambient", glm::vec3(0.1f));

            // - MATERIAL SPECULAR
            my_shader.SetUniform("u_material.specular", glm::vec3(1.0f));
            my_shader.SetUniform("u_material.shininess", 96.0f);

            // - DIRECTION :: SUN O)))
            my_shader.SetUniform("u_directional_light.direction", glm::vec3(0.0f, -0.9f, -0.17f));
            my_shader.SetUniform("u_directional_light.diffuse", glm::vec3(0.8f));
            my_shader.SetUniform("u_directional_light.specular", glm::vec3(0.14f));

            // Draw the scene
            // - Draw opaque objects
            for (auto& [key, value] : scene_opaque) {
                value->Draw(my_shader);
            }
            // - Draw transparent objects
            glEnable(GL_BLEND);         // enable blending
            glDisable(GL_CULL_FACE);    // no polygon removal
            glDepthMask(GL_FALSE);      // set Z to read-only
            // - - Calculate distace from camera for all transparent objects
            for (auto& transparent_pair : scene_transparent_pairs) {
                transparent_pair->second->_distance_from_camera = glm::length(camera.position - transparent_pair->second->position);
            }
            // - - Sort all transparent objects in vector by their distance from camera (far to near)
            std::sort(scene_transparent_pairs.begin(), scene_transparent_pairs.end(), [](std::pair<const std::string, Obj*>*& a, std::pair<const std::string, Obj*>*& b) {
                return a->second->_distance_from_camera > b->second->_distance_from_camera;
                });
            // - - Draw all transparent objects in sorted order
            for (auto& transparent_pair : scene_transparent_pairs) {
                transparent_pair->second->Draw(my_shader);
            }
            glDisable(GL_BLEND);
            glEnable(GL_CULL_FACE);
            glDepthMask(GL_TRUE);

            // === End of frame ===
            // Swap front and back buffers
            glfwSwapBuffers(window);

            // Poll for and process events
            glfwPollEvents();

            // Time/FPS measure end
            auto fps_frame_end_timestamp = std::chrono::steady_clock::now();
            std::chrono::duration<double> fps_elapsed_seconds = fps_frame_end_timestamp - fps_frame_start_timestamp;
            fps_counter_seconds += fps_elapsed_seconds.count();
            fps_counter_frames++;
            if (fps_counter_seconds >= 1) {
                FPS = fps_counter_frames;
                fps_counter_seconds = 0;
                fps_counter_frames = 0;
            }
            // Window title
            std::stringstream ss;
            ss << FPS << " FPS";
            glfwSetWindowTitle(window, ss.str().c_str());
        }
    }
    catch (std::exception const& e) {
        std::cerr << "App failed : " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    PrintGLInfo();

    std::cout << "Finished OK...\n";
    return EXIT_SUCCESS;
}

App::~App()
{
    // clean-up
    my_shader.Clear();

    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();

    exit(EXIT_SUCCESS);
    std::cout << "Bye...\n";
}

void App::UpdateProjection() {
    // Ensure window height is not zero to avoid division by zero
    const float minWindowHeight = 1.0f;
    window_height = std::max(window_height, static_cast<int>(minWindowHeight));

    // Calculate aspect ratio
    const float aspectRatio = static_cast<float>(window_width) / window_height;

    // Define clipping planes
    const float nearClipPlane = 0.1f;
    const float farClipPlane = 20000.0f;

    // Calculate vertical field of view in radians
    const float verticalFOV = glm::radians(FOV);

    // Generate perspective projection matrix
    mx_projection = glm::perspective(verticalFOV, aspectRatio, nearClipPlane, farClipPlane);
}

void App::PrintGLInfo()
{
    std::cout << "\nOpenGL Information\n";

    // Vendor information
    std::cout << "Vendor:\t\t" << glGetString(GL_VENDOR) << "\n";

    // Renderer information
    std::cout << "Renderer:\t" << glGetString(GL_RENDERER) << "\n";

    // OpenGL version
    std::cout << "OpenGL Version:\t" << glGetString(GL_VERSION) << "\n";

    // GLSL (OpenGL Shading Language) version
    std::cout << "GLSL Version:\t" << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";

    // Check OpenGL context profile
    GLint profile;
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);
    if (profile & GL_CONTEXT_CORE_PROFILE_BIT) {
        std::cout << "Profile:\tCore Profile\n";
    }
    else {
        std::cout << "Profile:\tCompatibility Profile\n";
    }

    // Check for OpenGL errors
    if (const auto errorCode = glGetError()) {
        std::cout << "Pending GL error while obtaining profile" << errorCode << "\n";
    }
}

