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
            app->fullscreen_enabled = !app->fullscreen_enabled;
            if (app->fullscreen_enabled) {
                // Store window position and size before going fullscreen
                glfwGetWindowPos(window, &app->window_xcor, &app->window_ycor);
                glfwGetWindowSize(window, &app->window_width_return_from_fullscreen, &app->window_height_return_from_fullscreen);
                if (app->window_height_return_from_fullscreen == 0) app->window_height_return_from_fullscreen++;
                // Switch to fullscreen mode
                glfwSetWindowMonitor(window, app->primary_monitor, 0, 0, app->video_mode->width, app->video_mode->height, app->video_mode->refreshRate);
            }
            else {
                // Restore window position and size after exiting fullscreen
                glfwSetWindowMonitor(window, nullptr, app->window_xcor, app->window_ycor, app->window_width_return_from_fullscreen, app->window_height_return_from_fullscreen, 0);
            }
            break;

        case GLFW_KEY_V:
            // Toggle VSync and print its status
            app->vsync_enabled = !app->vsync_enabled;
            glfwSwapInterval(app->vsync_enabled);
            std::cout << "VSync: " << app->vsync_enabled << "\n";
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
        bool& is_mouselook_on = this_inst->mouselook_enabled;
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

void App::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    auto app = static_cast<App*>(glfwGetWindowUserPointer(window));

    // Adjust light intensity based on mouse scroll direction
    if (yoffset > 0) {
        // Increase light intensity
        app->light_intensity += 0.1f;
        if (app->light_intensity > 1.0f)
            app->light_intensity = 1.0f; // Cap intensity to 1.0
    }
    else {
        // Decrease light intensity
        app->light_intensity -= 0.1f;
        if (app->light_intensity < 0.0f)
            app->light_intensity = 0.0f; // Cap intensity to 0.0
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


