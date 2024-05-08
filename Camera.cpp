// NEŠAHAT
#include <iostream>

#include <glm/ext/matrix_transform.hpp>

#include "Camera.hpp"

Camera::Camera(glm::vec3 position) : position(position) {
    this->up_global = glm::vec3(0.0f, 1.0f, 0.0f);
    // initialization of the camera reference system
    this->UpdateCameraVectors();
    sprint = false;
}

glm::mat4 Camera::GetViewMatrix() {
    return glm::lookAt(this->position, this->position + this->front, this->up_global);
}

glm::vec3 Camera::ProcessInput(GLFWwindow* window, GLfloat delta_time)
{
    glm::vec3 direction(0, 0, 0);
    glm::vec3 zero(0, 0, 0);

    glm::vec3 horizontal(front.x, 0, front.z);
    glm::vec3 forward(right.x, 0, right.z);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        direction += horizontal;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        direction += -horizontal;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        direction += -forward;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        direction += forward;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        direction += up_global;
    }

    if (direction == zero) {
        sprint = false;
    }

    float movement_speed = movement_speed_normal;
    if (sprint) {
        movement_speed = movement_speed_sprint;
    }

    if (direction == zero) {
        return zero;
    }
    else {
        return glm::normalize(direction) * movement_speed * delta_time;
    }
}


void Camera::ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constraintPitch) {
    xoffset *= this->mouse_sensitivity_horizontal;
    yoffset *= this->mouse_sensitivity_vertical;

    this->yaw -= xoffset;
    this->pitch += yoffset;

    if (constraintPitch)
    {
        if (this->pitch > 89.0f)
            this->pitch = 89.0f;
        if (this->pitch < -89.0f)
            this->pitch = -89.0f;
    }

    this->UpdateCameraVectors();
}

void Camera::ToggleSprint() {
    sprint = !sprint;
}

void Camera::UpdateListenerPosition(AudioSlave& audio)
{
    audio.UpdateListenerPosition(position, front, up_global);
}

void Camera::UpdateCameraVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
    front.y = sin(glm::radians(this->pitch));
    front.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));

    this->front = glm::normalize(front);
    this->right = glm::normalize(glm::cross(this->front, glm::vec3(0.0f, 1.0f, 0.0f)));
    this->up_global = glm::normalize(glm::cross(this->right, this->front));
}
