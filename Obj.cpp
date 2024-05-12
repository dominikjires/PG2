#include <iostream>
#include <fstream>
#include <string>
#include "Miniball.hpp"
#include "Obj.hpp"
#include "Texture.hpp"


Obj::Obj(std::string name, const std::filesystem::path& path_main, const std::filesystem::path& path_tex, glm::vec3 position, float scale, glm::vec4 init_rotation, bool is_height_map, bool use_aabb) :
    name(std::move(name)),
    position(position),
    scale(scale),
    init_rotation(init_rotation),
    use_aabb(use_aabb)
{
    if (!is_height_map)
        LoadObj(path_main);
    else
        HeightMap(path_main);

    GLuint texture_id = textureInit(path_tex.string().c_str());
    mesh = Mesh(GL_TRIANGLES, out_vertices, out_uvs, texture_id);
}


void Obj::LoadObj(const std::filesystem::path& file_name)
{

    std::vector<GLuint> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    out_vertices.clear();
    out_uvs.clear();

    std::string first_two_chars, first_three_chars;
    glm::vec2 uv;
    glm::vec3 vertex_or_normal;

    bool line_success;

    std::string line;
    std::ifstream file_reader(file_name);
    while (getline(file_reader, line)) {
        if (!line.empty()) {
            line_success = true;
            first_two_chars = line.substr(0, 2);
            first_three_chars = line.substr(0, 3);
            if (first_two_chars == "v ") {
                vertex_or_normal = {};
                (void)sscanf_s(line.c_str(), "v %f %f %f", &vertex_or_normal.x, &vertex_or_normal.y, &vertex_or_normal.z);
                temp_vertices.push_back(vertex_or_normal);
            }
            else if (first_three_chars == "vt ") {
                uv = {};
                (void)sscanf_s(line.c_str(), "vt %f %f", &uv.x, &uv.y);
                uv.y = -uv.y;
                temp_uvs.push_back(uv);
            }
            else if (first_three_chars == "vn ") {
                vertex_or_normal = {};
                (void)sscanf_s(line.c_str(), "vn %f %f %f", &vertex_or_normal.x, &vertex_or_normal.y, &vertex_or_normal.z);
                temp_normals.push_back(vertex_or_normal);
            }
            else if (first_two_chars == "f ") {
                auto n = std::count(line.begin(), line.end(), '/');
                if (n == 0) {
                    unsigned int indices_temp[3]{};
                    (void)sscanf_s(line.c_str(), "f %d %d %d", &indices_temp[0], &indices_temp[1], &indices_temp[2]);
                    vertexIndices.insert(vertexIndices.end(), { indices_temp[0], indices_temp[1], indices_temp[2] });
                }
                else if (n == 3) {
                    unsigned int indices_temp[6]{};
                    (void)sscanf_s(line.c_str(), "f %d/%d %d/%d %d/%d", &indices_temp[0], &indices_temp[3], &indices_temp[1], &indices_temp[4], &indices_temp[2], &indices_temp[5]);
                    vertexIndices.insert(vertexIndices.end(), { indices_temp[0], indices_temp[1], indices_temp[2] });
                    uvIndices.insert(uvIndices.end(), { indices_temp[3], indices_temp[4], indices_temp[5] });
                }
                else if (n == 6) {

                    if (line.find("//") != std::string::npos) {
                        unsigned int indices_temp[6]{};
                        (void)sscanf_s(line.c_str(), "f %d//%d %d//%d %d//%d", &indices_temp[0], &indices_temp[3], &indices_temp[1], &indices_temp[4], &indices_temp[2], &indices_temp[5]);
                        vertexIndices.insert(vertexIndices.end(), { indices_temp[0], indices_temp[1], indices_temp[2] });
                        normalIndices.insert(normalIndices.end(), { indices_temp[3], indices_temp[4], indices_temp[5] });
                    }
                    else {
                        unsigned int indices_temp[9]{};
                        (void)sscanf_s(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d", &indices_temp[0], &indices_temp[3], &indices_temp[6], &indices_temp[1], &indices_temp[4], &indices_temp[7], &indices_temp[2], &indices_temp[5], &indices_temp[8]);
                        vertexIndices.insert(vertexIndices.end(), { indices_temp[0], indices_temp[1], indices_temp[2] });
                        uvIndices.insert(uvIndices.end(), { indices_temp[3], indices_temp[4], indices_temp[5] });
                        normalIndices.insert(normalIndices.end(), { indices_temp[6], indices_temp[7], indices_temp[8] });
                    }
                }
                else if (n == 8) {
                    unsigned int v[4]{};
                    unsigned int vt[4]{};
                    unsigned int vn[4]{};
                    (void)sscanf_s(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d", &v[0], &vt[0], &vn[0], &v[1], &vt[1], &vn[1], &v[2], &vt[2], &vn[2], &v[3], &vt[3], &vn[3]);
                    vertexIndices.insert(vertexIndices.end(), { v[0], v[1], v[2], v[0], v[2], v[3] });
                    uvIndices.insert(uvIndices.end(), { vt[0], vt[1], vt[2], vt[0], vt[2], vt[3] });
                    normalIndices.insert(normalIndices.end(), { vn[0], vn[1], vn[2], vn[0], vn[2], vn[3] });
                }
                else {
                    line_success = false;
                }
            }
            else {
                line_success = false;
            }
        }
    }
    file_reader.close();

    // If not using AABB collision detection
    if (!use_aabb) {
        // Dimension of points (x, y, z)
        int d = 3;
        // Number of vertices
        auto n = temp_vertices.size();
        // Vector to store coordinates of points
        std::vector<std::vector<float>> ap(n, std::vector<float>(d));
        // Extract x, y, z coordinates of each vertex and store them
        for (int i = 0; i < n; i++) {
            ap[i][0] = temp_vertices[i].x;
            ap[i][1] = temp_vertices[i].y;
            ap[i][2] = temp_vertices[i].z;
        }
        // Define types for Miniball algorithm
        typedef std::vector<float>::const_iterator CoordIterator;
        typedef Miniball::Miniball <Miniball::CoordAccessor<std::vector<std::vector<float>>::const_iterator, CoordIterator>> MB;
        // Compute bounding sphere using Miniball algorithm
        MB mb(d, ap.begin(), ap.end());
        // Get center of the bounding sphere
        const float* center = mb.center();
        // Assign center coordinates to collision_bs_center
        for (int i = 0; i < d; ++i, ++center) collision_bs_center[i] = *center;
        // Scale center and compute scaled radius
        collision_bs_center *= scale;
        collision_bs_radius = sqrt(mb.squared_radius()) * scale;
    }
    // If using AABB collision detection
    else {
        // Initialize AABB min and max points
        collision_aabb_min = temp_vertices[0];
        collision_aabb_max = temp_vertices[0];
        // Find minimum and maximum coordinates for each axis
        for (const auto& point : temp_vertices) {
            if (point.x < collision_aabb_min.x) collision_aabb_min.x = point.x;
            if (point.y < collision_aabb_min.y) collision_aabb_min.y = point.y;
            if (point.z < collision_aabb_min.z) collision_aabb_min.z = point.z;
            if (point.x > collision_aabb_max.x) collision_aabb_max.x = point.x;
            if (point.y > collision_aabb_max.y) collision_aabb_max.y = point.y;
            if (point.z > collision_aabb_max.z) collision_aabb_max.z = point.z;
        }
        // Scale AABB coordinates
        collision_aabb_min *= scale;
        collision_aabb_max *= scale;
    }

    // Initialize vectors to hold processed data
    std::vector<glm::vec3> vertices_direct;
    std::vector<glm::vec2> texture_coordinates_direct;
    std::vector<glm::vec3> vertex_normals_direct;

    // Process vertex, texture coordinate, and normal data
    for (unsigned int u = 0; u < vertexIndices.size(); u++) {
        vertices_direct.push_back(temp_vertices[vertexIndices[u] - 1]);
    }
    for (unsigned int u = 0; u < uvIndices.size(); u++) {
        texture_coordinates_direct.push_back(temp_uvs[uvIndices[u] - 1]);
    }
    for (unsigned int u = 0; u < normalIndices.size(); u++) {
        vertex_normals_direct.push_back(temp_normals[normalIndices[u] - 1]);
    }

    // Compute sizes for texture coordinates and normals
    auto n_direct_uvs = texture_coordinates_direct.size();
    auto n_direct_normals = vertex_normals_direct.size();

    // Populate output vertex data
    for (unsigned int u = 0; u < vertices_direct.size(); u++) {
        Vertex vertex{};
        vertex.position = vertices_direct[u];
        if (u < n_direct_uvs) vertex.tex_coords = texture_coordinates_direct[u];
        if (u < n_direct_normals) vertex.normal = vertex_normals_direct[u];
        out_vertices.push_back(vertex);
        out_uvs.push_back(u);
    }

    // Print loaded file name
    std::cout << "LoadObj: Loaded file: " << file_name << "\n";
}

void Obj::Draw(ShaderProgram& shader)
{
    // Initialize model matrix as identity matrix
    mx_model = glm::mat4(1.0f);

    // Apply translation
    mx_model = glm::translate(mx_model, position);

    // Apply scaling
    mx_model = glm::scale(mx_model, glm::vec3(scale));

    // Initialize rotation axis from initial rotation
    glm::vec3 init_rotation_axes(init_rotation);

    // Apply initial rotation
    mx_model = glm::rotate(mx_model, glm::radians(init_rotation.w), init_rotation_axes);

    // Apply current rotation
    glm::vec3 rotation_axes(rotation);
    mx_model = glm::rotate(mx_model, glm::radians(rotation.w), rotation_axes);

    // Draw the object using the current model matrix
    mesh.Draw(shader, mx_model);
}

void Obj::HeightMap(const std::filesystem::path& file_name)
{
    out_vertices.clear();
    out_uvs.clear();
    cv::Mat hmap = cv::imread(file_name.u8string(), cv::IMREAD_GRAYSCALE);
    const unsigned int mesh_step_size = 5;
    glm::vec3 a{};
    glm::vec3 b{};
    glm::vec3 c{};
    unsigned int indices_counter = 0;

    std::map<std::pair<unsigned int, unsigned int>, glm::vec3> normal_sums;
    std::pair<unsigned int, unsigned int> pair, pair0, pair1, pair2, pair3;

    if (hmap.empty())
        return;

    std::cout << "Note: heightmap size:" << hmap.size << ", channels: " << hmap.channels() << std::endl;


    if (hmap.channels() != 1)
    {
        std::cerr << "WARN: requested 1 channel, got: " << hmap.channels() << std::endl;
    }

    // Create heightmap mesh from TRIANGLES in XZ plane, Y is UP (right hand rule)
    //
    //   3-----2
    //   |    /|
    //   |  /  |
    //   |/    |
    //   0-----1
    //
    //   012,023

    for (unsigned int x_coord = 0; x_coord < (hmap.cols - mesh_step_size); x_coord += mesh_step_size) {
        for (unsigned int z_coord = 0; z_coord < (hmap.rows - mesh_step_size); z_coord += mesh_step_size) {
            // Get The (X, Y, Z) Value For The Bottom Left Vertex = 0
            glm::vec3 p0(x_coord, hmap.at<uchar>(cv::Point(x_coord, z_coord)), z_coord);
            // Get The (X, Y, Z) Value For The Bottom Right Vertex = 1
            glm::vec3 p1(x_coord + mesh_step_size, hmap.at<uchar>(cv::Point(x_coord + mesh_step_size, z_coord)), z_coord);
            // Get The (X, Y, Z) Value For The Top Right Vertex = 2
            glm::vec3 p2(x_coord + mesh_step_size, hmap.at<uchar>(cv::Point(x_coord + mesh_step_size, z_coord + mesh_step_size)), z_coord + mesh_step_size);
            // Get The (X, Y, Z) Value For The Top Left Vertex = 3
            glm::vec3 p3(x_coord, hmap.at<uchar>(cv::Point(x_coord, z_coord + mesh_step_size)), z_coord + mesh_step_size);

            // Get max normalized height for tile, set texture accordingly
            // Grayscale image returns 0..256, normalize to 0.0f..1.0f by dividing by 256 (255 ?)
            float max_h = std::max(hmap.at<uchar>(cv::Point(x_coord, z_coord)) / 256.0f,
                std::max(hmap.at<uchar>(cv::Point(x_coord, z_coord + mesh_step_size)) / 256.0f,
                    std::max(hmap.at<uchar>(cv::Point(x_coord + mesh_step_size, z_coord + mesh_step_size)) / 256.0f,
                        hmap.at<uchar>(cv::Point(x_coord + mesh_step_size, z_coord)) / 256.0f
                    )));


            // Get texture coords in vertices, bottom left of geometry == bottom left of texture
            // Get texture coords in vertices, bottom left of geometry == bottom left of texture
            glm::vec2 texture_coords = HeightMap_GetSubtex(max_h);
            glm::vec2 tc0 = texture_coords;
            glm::vec2 tc1 = texture_coords + glm::vec2(1.0f / 16, 0.0f);		      // add offset for bottom right corner
            glm::vec2 tc2 = texture_coords + glm::vec2(1.0f / 16, 1.0f / 16);    // add offset for top right corner
            glm::vec2 tc3 = texture_coords + glm::vec2(0.0f, 1.0f / 16);         // add offset for bottom left corner

            // compute normal vector
            glm::vec3 normal1 = glm::normalize(glm::cross(p1 - p0, p2 - p0));
            glm::vec3 normal2 = glm::normalize(glm::cross(p2 - p0, p3 - p0));
            glm::vec3 avgNormal = (normal1 + normal2) * 0.5f; // Multiplication is marginally faster than division

            // add vertices and texture coordinates to mesh
            out_vertices.emplace_back(Vertex{ p0, -avgNormal, tc0 });
            out_vertices.emplace_back(Vertex{ p1, -avgNormal, tc1 });
            out_vertices.emplace_back(Vertex{ p2, -avgNormal, tc2 });
            out_vertices.emplace_back(Vertex{ p3, -avgNormal, tc3 });

            // update indices
            indices_counter += 4;
            out_uvs.insert(out_uvs.end(), { indices_counter - 4, indices_counter - 2, indices_counter - 3,
                                            indices_counter - 4, indices_counter - 1, indices_counter - 2 });

            // average normals
            pair0 = { x_coord, z_coord };
            pair1 = { x_coord + mesh_step_size, z_coord };
            pair2 = { x_coord + mesh_step_size, z_coord + mesh_step_size };
            pair3 = { x_coord, z_coord + mesh_step_size };
            normal_sums[pair0] -= avgNormal;
            normal_sums[pair1] -= avgNormal;
            normal_sums[pair2] -= avgNormal;
            normal_sums[pair3] -= avgNormal;

        }
    }

    // averaging for normals, 2nd iteration
    for (auto& vertex : out_vertices) {
        // Calculate the index pair for the normal sum lookup
        auto pair = std::make_pair(static_cast<unsigned int>(vertex.position.x), static_cast<unsigned int>(vertex.position.z));

        // Calculate the normal by normalizing the summed normals directly
        vertex.normal = glm::normalize(normal_sums[pair]);

        // Store the vertex height for heightmap collision
        _heights[{vertex.position.x* HEIGHTMAP_SCALE, vertex.position.z* HEIGHTMAP_SCALE}] = vertex.position.y;
    }
}

glm::vec2 Obj::HeightMap_GetSubtex(const float height)
{
    if (height > 0.8)
        return glm::vec2(1.0f / 16, 4.0f / 16);
    else if (height > 0.5)
        return glm::vec2(7.0f / 16, 1.0f / 16);
    else if (height > 0.3)
        return glm::vec2(4.0f / 16, 1.0f / 16);
    else
        return glm::vec2(1.0f / 16, 1.0f / 16);
}

bool Obj::Collision_CheckPoint(glm::vec3 point) const
{
    // Bounding sphere
    if (!use_aabb) {
        return glm::distance(point, position + collision_bs_center) < collision_bs_radius;
    }
    // AABB
    else {
        return
            point.x <= position.x + collision_aabb_max.x &&
            point.x >= position.x + collision_aabb_min.x &&
            point.y <= position.y + collision_aabb_max.y &&
            point.y >= position.y + collision_aabb_min.y &&
            point.z <= position.z + collision_aabb_max.z &&
            point.z >= position.z + collision_aabb_min.z
            ;
    }
}

void Obj::Clear()
{
    mesh.Clear();
}