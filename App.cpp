// C++
#include <iostream>
#include <chrono>
#include <stack>
#include <random>
#include <sstream>

// OpenCV � GL independent
#include <opencv2/opencv.hpp>

// OpenGL Extension Wrangler: allow all multiplatform GL functions
#include <GL/glew.h> 
// WGLEW = Windows GL Extension Wrangler :: platform specific functions (in this case Windows)
#include <GL/wglew.h> 

// GLFW toolkit
// Uses GL calls to open GL context, i.e. GLEW must be first.
#include <GLFW/glfw3.h>

// OpenGL math
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Our app
#include "App.hpp"
#include "gl_err_callback.hpp"
#include "ShaderProgram.hpp"

#define print(x)// std::cout << x << "\n"

App::App()
{
    std::cout << "Constructed...\n--------------\n";
}

// App initialization, if returns true then run run()
bool App::Init()
{
    try {
        // Set GLFW error callback
        glfwSetErrorCallback(error_callback);

        // Init GLFW :: https://www.glfw.org/documentation.html
        if (!glfwInit()) {
            return false;
        }

        // Set OpenGL version
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        // Set OpenGL profile
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Core, comment this line for Compatible

        // Window is hidden until everything is initialized
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        // Open window (GL canvas) with no special properties :: https://www.glfw.org/docs/latest/quick.html#quick_create_window
        window = glfwCreateWindow(window_width, window_height, "Moje krasne okno", NULL, NULL);
        if (!window) {
            glfwTerminate();
            return false;
        }
        glfwSetWindowUserPointer(window, this);

        // Hide cursor
        //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); // <- weird mouselook behavior
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // <- ok

        // Fullscreen On/Off
        monitor = glfwGetPrimaryMonitor(); // Get primary monitor
        mode = glfwGetVideoMode(monitor); // Get resolution of the monitor

        // Setup callbacks
        glfwMakeContextCurrent(window);
        glfwSetKeyCallback(window, key_callback);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetCursorPosCallback(window, cursor_position_callback);
        glfwSetScrollCallback(window, scroll_callback);

        // Set V-Sync ON.
        glfwSwapInterval(1);
        is_vsync_on = true;

        // Init GLEW :: http://glew.sourceforge.net/basic.html
        GLenum err = glewInit();
        if (GLEW_OK != err) {
            fprintf(stderr, "Error: %s\n", glewGetErrorString(err)); /* Problem: glewInit failed, something is seriously wrong. */
        }
        wglewInit();

        //...after ALL GLFW & GLEW init ...
        if (GLEW_ARB_debug_output)
        {
            glDebugMessageCallback(MessageCallback, 0);
            glEnable(GL_DEBUG_OUTPUT);

            //default is asynchronous debug output, use this to simulate glGetError() functionality
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

            std::cout << "GL_DEBUG enabled.\n";
        }
        else std::cout << "GL_DEBUG NOT SUPPORTED!\n";

        // Set GL params
        glEnable(GL_DEPTH_TEST);

        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_POLYGON_SMOOTH);

        glEnable(GL_CULL_FACE);

        // Transparency blending function
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // First init OpenGL, THAN init assets: valid context MUST exist
        InitAssets();

        // Show window after everything loads        
        glfwShowWindow(window);
    }
    catch (std::exception const& e) {
        std::cerr << "Init failed : " << e.what() << "\n";
        exit(-1);
    }
    std::cout << "--------------\nInitialized...\n";
    return true;
}

int App::Run(void)
{
    try {
        double current_timestamp = glfwGetTime();
        double last_frame_time = current_timestamp;

        // FPS counting
        double fps_counter_seconds = 0;
        int fps_counter_frames = 0;

        // Init view
        UpdateProjectionMatrix();
        glViewport(0, 0, window_width, window_height);

        // Camera
        camera.position.x = 1.0f;
        camera.position.y = 1.0f;
        camera.position.z = 1.0f;

        glm::vec3 camera_movement{};

        // Mouselook
        double cursor_x, cursor_y;

        // Walking sound
        double walk_last_played_timestamp = current_timestamp;
        const double walk_play_delay_normal = 0.4;
        const double walk_play_delay_sprint = 0.25;

        // Jetpack
        float falling_speed = 0;
        float jumping_speed = 0;
        bool is_grounded = true;

        // Jukebox
        audio.PlayMusic3D();

        // Main loop
        while (!glfwWindowShouldClose(window)) {
            current_timestamp = glfwGetTime();

            // Time/FPS measure start
            auto fps_frame_start_timestamp = std::chrono::steady_clock::now();

            // Clear OpenGL canvas, both color buffer and Z-buffer
            glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // === After clearing the canvas ===
            float delta_time = static_cast<float>(current_timestamp - last_frame_time);
            last_frame_time = current_timestamp;

            // Player movement
            camera_movement = camera.ProcessInput(window, delta_time);
            camera.position.x += camera_movement.x;
            camera.position.z += camera_movement.z;

            // Movement sound
            if ((camera_movement.x != 0 || camera_movement.z != 0) && is_grounded) {
                if ((!camera.sprint && current_timestamp > walk_last_played_timestamp + walk_play_delay_normal)
                    || (camera.sprint && current_timestamp > walk_last_played_timestamp + walk_play_delay_sprint)) {
                    audio.PlayWalk(); // Play step sound if grounded and walking and we didn't play the sound for the duration of delay (sprinting == shorter delay)
                    walk_last_played_timestamp = current_timestamp;
                }
            }
            else {
                walk_last_played_timestamp = current_timestamp; // Consistent delay for first step sound after movement starts
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
            // Jetpack
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
            audio.UpdateFallVolume(falling_speed > 1.0f && !is_grounded);

            // Create View Matrix according to camera settings
            glm::mat4 mx_view = camera.GetViewMatrix();

            // Update objects

            UpdateModels(delta_time);
            UpdateProjectiles(delta_time);

            // 3D Audio
            camera.UpdateListenerPosition(audio);
            audio.UpdateMusicPosition(obj_jukebox->position);

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

            // - POINT LIGHT :: JUKEBOX
            my_shader.SetUniform("u_point_lights[0].diffuse", glm::vec3(0.0f, 1.0f, 1.0f));
            my_shader.SetUniform("u_point_lights[0].specular", glm::vec3(0.07f));
            my_shader.SetUniform("u_point_lights[0].on", is_jukebox_on);
            glm::vec3 point_light_pos = obj_jukebox->position; // Light position infront of the jukebox
            point_light_pos.y += 1.0f;
            point_light_pos.x += 0.7f * jukebox_to_player_n.x;
            point_light_pos.z += 0.7f * jukebox_to_player_n.y;
            my_shader.SetUniform("u_point_lights[0].position", point_light_pos);
            my_shader.SetUniform("u_point_lights[0].constant", 1.0f);
            my_shader.SetUniform("u_point_lights[0].linear", 1.0f);
            my_shader.SetUniform("u_point_lights[0].exponent", 0.5f);

            // - SPOTLIGHT
            my_shader.SetUniform("u_spotlight.diffuse", glm::vec3(0.7f));
            my_shader.SetUniform("u_spotlight.specular", glm::vec3(0.56f));
            my_shader.SetUniform("u_spotlight.position", camera.position);
            my_shader.SetUniform("u_spotlight.direction", camera.front);
            my_shader.SetUniform("u_spotlight.cos_inner_cone", glm::cos(glm::radians(20.0f)));
            my_shader.SetUniform("u_spotlight.cos_outer_cone", glm::cos(glm::radians(27.0f)));
            my_shader.SetUniform("u_spotlight.constant", 1.0f);
            my_shader.SetUniform("u_spotlight.linear", 0.07f);
            my_shader.SetUniform("u_spotlight.exponent", 0.017f);
            my_shader.SetUniform("u_spotlight.on", is_flashlight_on);

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
            ss << FPS << " FPS | " << FOV << " FOV | X" << camera.position.x << " Y" << camera.position.y << " Z" << camera.position.z;
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

void App::UpdateProjectionMatrix(void)
{
    if (window_height < 1) window_height = 1; // avoid division by 0

    float ratio = static_cast<float>(window_width) / window_height;

    mx_projection = glm::perspective(
        glm::radians(FOV),   // The vertical Field of View
        ratio,               // Aspect Ratio. Depends on the size of your window.
        0.1f,                // Near clipping plane. Keep as big as possible, or you'll get precision issues.
        20000.0f             // Far clipping plane. Keep as little as possible.
    );
}

void App::PrintGLInfo()
{
    std::cout << "\n=================== :: GL Info :: ===================\n";
    std::cout << "GL Vendor:\t" << glGetString(GL_VENDOR) << "\n";
    std::cout << "GL Renderer:\t" << glGetString(GL_RENDERER) << "\n";
    std::cout << "GL Version:\t" << glGetString(GL_VERSION) << "\n";
    std::cout << "GL Shading ver:\t" << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n\n";

    GLint profile;
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);
    if (const auto errorCode = glGetError()) {
        std::cout << "[!] Pending GL error while obtaining profile: " << errorCode << "\n";
        //return;
    }
    if (profile & GL_CONTEXT_CORE_PROFILE_BIT) {
        std::cout << "Core profile" << "\n";
    }
    else {
        std::cout << "Compatibility profile" << "\n";
    }
    std::cout << "=====================================================\n\n";
}
