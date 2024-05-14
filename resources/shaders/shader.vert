#version 460 core

// Vertex attributes
layout (location = 0) in vec4 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_texture_coordinate;

// Matrices
uniform mat4 u_mx_model;         // Object local coor space -> World space
uniform mat4 u_mx_view;          // World space -> Camera space
uniform mat4 u_mx_projection;    // Camera space -> Screen

// VS -> FS
out vec3 o_fragment_position;
out vec3 o_normal;
out vec2 o_texture_coordinate;

void main()
{
    o_fragment_position = vec3(u_mx_model * a_position);

    // https://computergraphics.stackexchange.com/questions/1502/why-is-the-transposed-inverse-of-the-model-view-matrix-used-to-transform-the-nor
    o_normal = mat3(transpose(inverse(u_mx_model))) * a_normal;

    o_texture_coordinate = a_texture_coordinate;

    gl_Position = u_mx_projection * u_mx_view * u_mx_model * a_position;
}
