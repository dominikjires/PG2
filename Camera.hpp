#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "AudioSlave.hpp"

class Camera
{
public:
    // Camera Attributes
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up; // camera local UP vector

    GLfloat yaw = 0.0f;
    GLfloat pitch = 0.0f;
    GLfloat roll = 0.0f;

    // Player options
    const GLfloat movement_speed_normal = 3.0f;
    const GLfloat movement_speed_sprint = 6.2f;
    
    // Mouse options
    const GLfloat mouse_sensitivity_horizontal = 0.18f;
    const GLfloat mouse_sensitivity_vertical = 0.175f;

    Camera(glm::vec3 position);
    glm::mat4 GetViewMatrix();
    glm::vec3 ProcessInput(GLFWwindow* window, GLfloat deltaTime);
    void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset);

    void ToggleSprint();

    void UpdateListenerPosition(AudioSlave& audio);

    bool is_sprint_toggled;
private:
    glm::vec3 world_up;

    void UpdateCameraVectors();
};
