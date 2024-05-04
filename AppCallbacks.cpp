#include <iostream>

#include "App.hpp"

void App::error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void App::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));
    if ((action == GLFW_PRESS) || (action == GLFW_REPEAT)) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            // Exit The App
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;

        case GLFW_KEY_F11:
            // Fullscreen on/off
            this_inst->is_fullscreen_on = !this_inst->is_fullscreen_on;
            if (this_inst->is_fullscreen_on) { // Remember window info and set fullscreen
                glfwGetWindowPos(window, &this_inst->window_xcor, &this_inst->window_ycor);
                glfwGetWindowSize(window, &this_inst->window_width_return_from_fullscreen, &this_inst->window_height_return_from_fullscreen);
                if (this_inst->window_height_return_from_fullscreen == 0) this_inst->window_height_return_from_fullscreen++; // Prevent zero height window
                glfwSetWindowMonitor(window, this_inst->monitor, 0, 0, this_inst->mode->width, this_inst->mode->height, this_inst->mode->refreshRate);
            }
            else { // Set windowed with remembered window info
                glfwSetWindowMonitor(window, nullptr, this_inst->window_xcor, this_inst->window_ycor, this_inst->window_width_return_from_fullscreen, this_inst->window_height_return_from_fullscreen, 0);
            }
            break;

        case GLFW_KEY_V:
            // Vsync on/off
            this_inst->is_vsync_on = !this_inst->is_vsync_on;
            glfwSwapInterval(this_inst->is_vsync_on);
            std::cout << "VSync: " << this_inst->is_vsync_on << "\n";
            break;

        case GLFW_KEY_F:
            // Flashlight on/off
            this_inst->is_flashlight_on = (this_inst->is_flashlight_on + 1) % 2;
            break;

        case GLFW_KEY_R:
            // Reset glass cubes
            if (HIDE_CUBES_INSTEAD_DESTROY) {
                for (const auto& pair : this_inst->scene_transparent) {
                    if (pair.first.substr(0, 15) == "obj_glass_cube_" && pair.second->position.y < 0.0f) {
                        pair.second->position.y += HIDE_CUBE_Y; // Move them up if they're under ground
                    }
                }
            }
            break;
        }
    }
    
    // Minecraft-like sprint
    if (action == GLFW_PRESS && (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL)) {
        this_inst->camera.ToggleSprint();
    }
}

void App::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));    
    this_inst->FOV -= 10.0f * static_cast<float>(yoffset);      // Scrollwheel down == FOV++
    this_inst->FOV = std::clamp(this_inst->FOV, 70.0f, 160.0f); // Limit FOV to "reasonable" values
    this_inst->UpdateProjectionMatrix();
}

void App::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));
    this_inst->window_width = width;
    this_inst->window_height = height;
    // set viewport
    glViewport(0, 0, width, height);
    // now your canvas has [0,0] in bottom left corner, and its size is [width x height] 
    this_inst->UpdateProjectionMatrix();
}

void App::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));
    // LMB to shoot
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (this_inst->is_mouselook_on) {
            this_inst->Shoot();
            this_inst->audio.Play2DOneShot("snd_shoot");
        }
        else {
            this_inst->is_mouselook_on = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
    // RMB to toggle mouselook
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        this_inst->is_mouselook_on = !this_inst->is_mouselook_on;
        if (this_inst->is_mouselook_on) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

void App::cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    // Logic was moved to App::Run because of mouse look stutter

    /*
    auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));
    this_inst->camera.ProcessMouseMovement(static_cast<GLfloat>(this_inst->window_width / 2.0 - xpos), static_cast<GLfloat>(this_inst->window_height / 2.0 - ypos));
    glfwSetCursorPos(window, this_inst->window_width / 2.0, this_inst->window_height / 2.0);
    /**/
}
