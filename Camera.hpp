// NEŠAHAT AŽ NA AUDIO
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

    GLfloat yaw = -90.0f;
    GLfloat pitch = 0.0f;
    GLfloat roll = 0.0f;

    // Player options
    const GLfloat movement_speed_normal = 1.0f;
    const GLfloat movement_speed_sprint = 5.0f;
    
    // Mouse options
    const GLfloat mouse_sensitivity_horizontal = 0.25f;
    const GLfloat mouse_sensitivity_vertical = 0.25f;

    Camera(glm::vec3 position);
    glm::mat4 GetViewMatrix();
    glm::vec3 ProcessInput(GLFWwindow* window, GLfloat deltaTime);
    void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constraintPitch = GL_TRUE);
    void ToggleSprint();
    void UpdateListenerPosition(AudioSlave& audio);
    bool sprint;
private:
    glm::vec3 up_global;
    void UpdateCameraVectors();
};
