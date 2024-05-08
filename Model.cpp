#include <iostream>
#include <fstream>
#include <string>

#include "Miniball.hpp"

#include "Model.hpp"
#include "Texture.hpp"

#define print(x) //std::cout << x << "\n"
#define print_loading(x) std::cout << x

Model::Model(std::string name, const std::filesystem::path& path_main, const std::filesystem::path& path_tex, glm::vec3 position, float scale, glm::vec4 init_rotation, bool is_height_map, bool use_aabb) :
    name(name),
    position(position),
    scale(scale),
    init_rotation(init_rotation),
    use_aabb(use_aabb)
{
    if (!is_height_map) {
        LoadOBJFile(path_main);
    }
    else {
        HeightMap(path_main);        
    }

    GLuint texture_id = textureInit(path_tex.string().c_str());
    mesh = Mesh(GL_TRIANGLES, mesh_vertices, mesh_vertex_indices, texture_id);
}

void Model::Draw(ShaderProgram& shader)
{
    // Einheitsmatrix
    mx_model = glm::identity<glm::mat4>();
    // Move object
    mx_model = glm::translate(mx_model, position);
    // Scale object (scale in all three dimensions must be the same in this "engine")
    mx_model = glm::scale(mx_model, glm::vec3(scale));
    // Initial rotation (should be set only once when creating the Model)
    init_rotation_axes = glm::vec3(init_rotation.x, init_rotation.y, init_rotation.z);
    mx_model = glm::rotate(mx_model, glm::radians(init_rotation.w), init_rotation_axes);
    // Additional rotation
    rotation_axes = glm::vec3(rotation.x, rotation.y, rotation.z);
    mx_model = glm::rotate(mx_model, glm::radians(rotation.w), rotation_axes);
    // Draw
    mesh.Draw(shader, mx_model);
}

void Model::LoadOBJFile(const std::filesystem::path& file_name)
{
    mesh_vertices.clear();
    mesh_vertex_indices.clear();

    // [1] Read the file and fill vectors
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> texture_coordinates;
    std::vector<glm::vec3> vertex_normals;
    std::vector<GLuint> indices_vertex, indices_texture_coordinate, indices_vertex_normal;

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
            // v -1.183220029 4.784470081 47.4618988
            if (first_two_chars == "v ") {
                vertex_or_normal = {};
                (void)sscanf_s(line.c_str(), "v %f %f %f", &vertex_or_normal.x, &vertex_or_normal.y, &vertex_or_normal.z);
                vertices.push_back(vertex_or_normal);
            }
            // vt 0.5000 0.7500
            else if (first_three_chars == "vt ") {
                uv = {};
                (void)sscanf_s(line.c_str(), "vt %f %f", &uv.x, &uv.y);
                uv.y = -uv.y; // DDS textures are inverted
                texture_coordinates.push_back(uv);
            }
            // vn 0.7235898972 -0.6894102097 -0.03363365307
            else if (first_three_chars == "vn ") {
                vertex_or_normal = {};
                (void)sscanf_s(line.c_str(), "vn %f %f %f", &vertex_or_normal.x, &vertex_or_normal.y, &vertex_or_normal.z);
                vertex_normals.push_back(vertex_or_normal);
            }
            else if (first_two_chars == "f ") {
                auto n = std::count(line.begin(), line.end(), '/');
                // f 1 2 3
                if (n == 0) {
                    unsigned int indices_temp[3]{};
                    (void)sscanf_s(line.c_str(), "f %d %d %d", &indices_temp[0], &indices_temp[1], &indices_temp[2]);
                    indices_vertex.insert(indices_vertex.end(), { indices_temp[0], indices_temp[1], indices_temp[2] });
                }
                // f 3/1 4/2 5/3
                else if (n == 3) {
                    unsigned int indices_temp[6]{};
                    (void)sscanf_s(line.c_str(), "f %d/%d %d/%d %d/%d", &indices_temp[0], &indices_temp[3], &indices_temp[1], &indices_temp[4], &indices_temp[2], &indices_temp[5]);
                    indices_vertex.insert(indices_vertex.end(), { indices_temp[0], indices_temp[1], indices_temp[2] });
                    indices_texture_coordinate.insert(indices_texture_coordinate.end(), { indices_temp[3], indices_temp[4], indices_temp[5] });
                }
                else if (n == 6) {
                    // f 7//1 8//2 9//3
                    if (line.find("//") != std::string::npos) {
                        unsigned int indices_temp[6]{};
                        (void)sscanf_s(line.c_str(), "f %d//%d %d//%d %d//%d", &indices_temp[0], &indices_temp[3], &indices_temp[1], &indices_temp[4], &indices_temp[2], &indices_temp[5]);
                        indices_vertex.insert(indices_vertex.end(), { indices_temp[0], indices_temp[1], indices_temp[2] });
                        indices_vertex_normal.insert(indices_vertex_normal.end(), { indices_temp[3], indices_temp[4], indices_temp[5] });
                    }                
                    // f 6/4/1 3/5/3 7/6/5
                    else {
                        unsigned int indices_temp[9]{};
                        (void)sscanf_s(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d", &indices_temp[0], &indices_temp[3], &indices_temp[6], &indices_temp[1], &indices_temp[4], &indices_temp[7], &indices_temp[2], &indices_temp[5], &indices_temp[8]);
                        indices_vertex.insert(indices_vertex.end(), { indices_temp[0], indices_temp[1], indices_temp[2] });
                        indices_texture_coordinate.insert(indices_texture_coordinate.end(), { indices_temp[3], indices_temp[4], indices_temp[5] });
                        indices_vertex_normal.insert(indices_vertex_normal.end(), { indices_temp[6], indices_temp[7], indices_temp[8] });
                    }
                }
                // f 1/1/1 2/2/2 22/23/3 21/22/4
                else if (n == 8) {
                    unsigned int v[4]{};
                    unsigned int vt[4]{};
                    unsigned int vn[4]{};
                    (void)sscanf_s(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d", &v[0], &vt[0], &vn[0], &v[1], &vt[1], &vn[1], &v[2], &vt[2], &vn[2], &v[3], &vt[3], &vn[3]);
                    indices_vertex.insert(indices_vertex.end(), { v[0], v[1], v[2], v[0], v[2], v[3] });
                    indices_texture_coordinate.insert(indices_texture_coordinate.end(), { vt[0], vt[1], vt[2], vt[0], vt[2], vt[3] });
                    indices_vertex_normal.insert(indices_vertex_normal.end(), { vn[0], vn[1], vn[2], vn[0], vn[2], vn[3] });
                }
                else {
                    line_success = false;
                }
            }
            else {
                line_success = false;
            }

            if (!line_success && first_two_chars != "# ") {
                print("LoadOBJFile: Ignoring line '" << line << "' in file '" << file_name << "'");
            }
        }
    }
    file_reader.close();
    print_loading("#");

    // [2] Calculate collision sphere/box
    // - Bounding sphere
    if (!use_aabb) {
        int d = 3; // dimension
        auto n = vertices.size(); // number of points
        float** ap = new float* [n];
        for (int i = 0; i < n; i++) {
            float* p = new float[d];
            p[0] = vertices[i].x;
            p[1] = vertices[i].y;
            p[2] = vertices[i].z;
            ap[i] = p;
        }
        typedef float* const* PointIterator;
        typedef const float* CoordIterator;
        typedef Miniball::Miniball <Miniball::CoordAccessor<PointIterator, CoordIterator>> MB;
        MB mb(d, ap, ap + n);
        const float* center = mb.center();
        for (int i = 0; i < d; ++i, ++center) collision_bs_center[i] = *center;
        collision_bs_center *= scale;
        collision_bs_radius = sqrt(mb.squared_radius()) * scale;
    }
    // - AABB
    else {
        collision_aabb_min = vertices[0];
        collision_aabb_max = vertices[0];
        for (const auto& point : vertices) {
            if (point.x < collision_aabb_min.x) collision_aabb_min.x = point.x;
            if (point.y < collision_aabb_min.y) collision_aabb_min.y = point.y;
            if (point.z < collision_aabb_min.z) collision_aabb_min.z = point.z;
            if (point.x > collision_aabb_max.x) collision_aabb_max.x = point.x;
            if (point.y > collision_aabb_max.y) collision_aabb_max.y = point.y;
            if (point.z > collision_aabb_max.z) collision_aabb_max.z = point.z;
        }
        collision_aabb_min *= scale;
        collision_aabb_max *= scale;
    }
    print_loading("#");

    // RETARDED DRAW ™ 2.0
    // - [3] Indirect -> direct
    std::vector<glm::vec3> vertices_direct;
    std::vector<glm::vec2> texture_coordinates_direct;
    std::vector<glm::vec3> vertex_normals_direct;

    for (unsigned int u = 0; u < indices_vertex.size(); u++) {
        vertices_direct.push_back(vertices[indices_vertex[u] - 1]);
    }
    for (unsigned int u = 0; u < indices_texture_coordinate.size(); u++) {
        texture_coordinates_direct.push_back(texture_coordinates[indices_texture_coordinate[u] - 1]);
    }
    for (unsigned int u = 0; u < indices_vertex_normal.size(); u++) {
        vertex_normals_direct.push_back(vertex_normals[indices_vertex_normal[u] - 1]);
    }

    ///* Uncomment these if you don't like to live dangerously
    auto n_direct_uvs = texture_coordinates_direct.size();
    auto n_direct_normals = vertex_normals_direct.size();
    /**/
    print_loading("#");

    // - [4] vectors to Vertex vector
    for (unsigned int u = 0; u < vertices_direct.size(); u++) {
        Vertex vertex{};
        vertex.position = vertices_direct[u];
        if (u < n_direct_uvs) vertex.tex_coords = texture_coordinates_direct[u];
        if (u < n_direct_normals) vertex.normal = vertex_normals_direct[u];
        mesh_vertices.push_back(vertex);
        mesh_vertex_indices.push_back(u);
    }
    print("LoadOBJFile: Loaded OBJ file " << file_name << "\n");
    print_loading("#\n");
}

void Model::HeightMap(const std::filesystem::path& file_name)
{
    mesh_vertices.clear();
    mesh_vertex_indices.clear();

    cv::Mat hmap = cv::imread(file_name.u8string(), cv::IMREAD_GRAYSCALE);
    if (hmap.empty()) std::cerr << "HeightMap: [!] Height map empty? File: " << file_name << "\n";

    const unsigned int mesh_step_size = 10;

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

    glm::vec3 normalA{};
    glm::vec3 normalB{};
    glm::vec3 normal{};
    unsigned int indices_counter = 0;

    std::map<std::pair<unsigned int, unsigned int>, glm::vec3> normal_sums;
    std::pair<unsigned int, unsigned int> pair, pair0, pair1, pair2, pair3;

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
			float max_h = std::max(hmap.at<uchar>(cv::Point(x_coord, z_coord)) / 255.0f,
				std::max(hmap.at<uchar>(cv::Point(x_coord, z_coord + mesh_step_size)) / 255.0f,
					std::max(hmap.at<uchar>(cv::Point(x_coord + mesh_step_size, z_coord + mesh_step_size)) / 255.0f,
						hmap.at<uchar>(cv::Point(x_coord + mesh_step_size, z_coord)) / 255.0f
					)));

            // Get texture coords in vertices, bottom left of geometry == bottom left of texture
            glm::vec2 tc0 = HeightMap_GetSubtexByHeight(max_h);
            glm::vec2 tc1 = tc0 + glm::vec2((1.0f / 16), 0.0f);		    // add offset for bottom right corner
            glm::vec2 tc2 = tc0 + glm::vec2((1.0f / 16), (1.0f / 16));  // add offset for top right corner
            glm::vec2 tc3 = tc0 + glm::vec2(0.0f, (1.0f / 16));         // add offset for bottom left corner

            // RETARDED HEIGHT MAP ™ 2.0
            // - calculate normal vector            
            normalA = glm::normalize(glm::cross(p1 - p0, p2 - p0));
            normalB = glm::normalize(glm::cross(p2 - p0, p3 - p0));
            normal = (normalA + normalB) / 2.0f;
            
            // - place vertices and ST to mesh
            mesh_vertices.emplace_back(Vertex{ p0, -normal, tc0 });
            mesh_vertices.emplace_back(Vertex{ p1, -normal, tc1 });
            mesh_vertices.emplace_back(Vertex{ p2, -normal, tc2 });
            mesh_vertices.emplace_back(Vertex{ p3, -normal, tc3 });

            // - place indices
            indices_counter += 4;
            mesh_vertex_indices.emplace_back(indices_counter - 4);
            mesh_vertex_indices.emplace_back(indices_counter - 2);
            mesh_vertex_indices.emplace_back(indices_counter - 3);
            mesh_vertex_indices.emplace_back(indices_counter - 4);
            mesh_vertex_indices.emplace_back(indices_counter - 1);
            mesh_vertex_indices.emplace_back(indices_counter - 2);

            // - normal averaging
            pair0 = { x_coord, z_coord };
            pair1 = { x_coord + mesh_step_size, z_coord };
            pair2 = { x_coord + mesh_step_size, z_coord + mesh_step_size };
            pair3 = { x_coord, z_coord + mesh_step_size };
            normal_sums[pair0] -= normal;
            normal_sums[pair1] -= normal;
            normal_sums[pair2] -= normal;
            normal_sums[pair3] -= normal;
        }
    }

    // - normal averaging, 2nd iter
    for (auto& vertex : mesh_vertices) {
        pair = { static_cast<unsigned int>(vertex.position.x), static_cast<unsigned int>(vertex.position.z) };
        vertex.normal = glm::normalize(normal_sums[pair]); // no need to divide by four, we can just normalize

        _heights[{vertex.position.x * HEGHTMAP_SCALE, vertex.position.z * HEGHTMAP_SCALE}] = vertex.position.y; // for heightmap collision
    }

    print("HeightMap: height map vertices: " << mesh_vertices.size());
}

glm::vec2 Model::HeightMap_GetSubtexST(const int x, const int y)
{
    return glm::vec2((x * 1.0f / 16), (y * 1.0f / 16)); // Expects tilemap with 16 rows&cols; interpolation is dealt with via bleeding pixels
}

glm::vec2 Model::HeightMap_GetSubtexByHeight(float height)
{
    if (height > 0.9) return HeightMap_GetSubtexST(4, 4);
    else if (height > 0.8) return HeightMap_GetSubtexST(1, 4);
    else if (height > 0.5) return HeightMap_GetSubtexST(7, 1);
    else if (height > 0.3) return HeightMap_GetSubtexST(4, 1);
    else return HeightMap_GetSubtexST(1, 1);
}

bool Model::Collision_CheckPoint(glm::vec3 point) const
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

void Model::Clear()
{
    mesh.Clear();
}
