#include <iostream>
#include "App.hpp"

void App::error_callback(int error, const char* description) {
    // Print error message to standard error stream
    std::cerr << "Error: " << description << std::endl;
}

void App::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto app = static_cast<App*>(glfwGetWindowUserPointer(window));

    // Check for key press or key repeat actions
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            // Set window to close if escape key is pressed
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;

        case GLFW_KEY_RIGHT_ALT:
            // Toggle fullscreen mode
            app->is_fullscreen_on = !app->is_fullscreen_on;
            if (app->is_fullscreen_on) {
                // Store window position and size before going fullscreen
                glfwGetWindowPos(window, &app->window_xcor, &app->window_ycor);
                glfwGetWindowSize(window, &app->window_width_return_from_fullscreen, &app->window_height_return_from_fullscreen);
                if (app->window_height_return_from_fullscreen == 0) app->window_height_return_from_fullscreen++;
                // Switch to fullscreen mode
                glfwSetWindowMonitor(window, app->monitor, 0, 0, app->mode->width, app->mode->height, app->mode->refreshRate);
            }
            else {
                // Restore window position and size after exiting fullscreen
                glfwSetWindowMonitor(window, nullptr, app->window_xcor, app->window_ycor, app->window_width_return_from_fullscreen, app->window_height_return_from_fullscreen, 0);
            }
            break;

        case GLFW_KEY_V:
            // Toggle VSync and print its status
            app->is_vsync_on = !app->is_vsync_on;
            glfwSwapInterval(app->is_vsync_on);
            std::cout << "VSync: " << app->is_vsync_on << "\n";
            break;
        }
    }

    // Check for left or right shift key press
    if (action == GLFW_PRESS && (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT)) {
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
            // Toggle sprint when shift key is pressed
            app->camera.ToggleSprint();
        }
    }
}

void App::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (auto* this_inst = static_cast<App*>(glfwGetWindowUserPointer(window))) {
        // Variables for better readability
        bool& is_mouselook_on = this_inst->is_mouselook_on;
        const int cursor_disabled_mode = GLFW_CURSOR_DISABLED;

        // Perform action based on mouse button press
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            if (is_mouselook_on) {
                // Shoot if mouselook is on
                this_inst->Shoot();
                this_inst->audio.PlayShot("sound_shoot");
            }
            else {
                // Enable mouselook and disable cursor if mouselook is off
                is_mouselook_on = true;
                glfwSetInputMode(window, GLFW_CURSOR, cursor_disabled_mode);
            }
        }
    }
}

void App::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));
    this_inst->window_width = width;
    this_inst->window_height = height;
    // set viewport
    glViewport(0, 0, width, height);
    // now your canvas has [0,0] in bottom left corner, and its size is [width x height] 
    this_inst->UpdateProjection();
}


