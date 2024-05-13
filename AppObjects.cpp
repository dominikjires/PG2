#include <opencv2/opencv.hpp>

#include "App.hpp"

#define print(x) std::cout << x << "\n"

#define FASTER_JUKEBOX false	// If true, replace jukebox model with box (faster loading)
#define DEBUG_BOUNDINGS false	// If true, show "gizmos" that visualize bounding sphere / AABB

#define JUKEBOX_SPEED 2.0f

Obj* App::CreateModel(std::string name, std::string obj, std::string tex, bool is_opaque, glm::vec3 position, float scale, glm::vec4 rotation, bool collision, bool use_aabb)
{
	if (name.substr(0, 15) != "obj_projectile_") print("Loading " << name << ":"); // Print object name we're currently loading except projectiles

	std::filesystem::path modelpath("./resources/objects/" + obj);
	std::filesystem::path texturepath("./resources/textures/" + tex);
	auto model = new Obj(name, modelpath, texturepath, position, scale, rotation, false, use_aabb);

	if (is_opaque) {
		scene_opaque.insert({ name, model });
	}
	else {
		scene_transparent.insert({ name, model });
	}

	if (collision) { // Can projectile collide with this object ?
		collisions.push_back(model);
	}

	return model;
}

// Load models, load textures, load shaders, initialize level, etc.
void App::InitAssets()
{
	print("RAM OK\nROM OK");
	// == SHADERS ==
	// Load shaders and create ShaderProgram
	std::filesystem::path VS_path("./resources/shaders/uber.vert");
	std::filesystem::path FS_path("./resources/shaders/uber.frag");
	my_shader = ShaderProgram(VS_path, FS_path);

	// == MODELS ==
	glm::vec3 position{};
	float scale{};
	glm::vec4 rotation{};

	// = OPAQUE MODELS =
	// Jukebox
	position = glm::vec3(4.0f, 0.0f, 8.0f);
	scale = 0.125f;
	rotation = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	if (!FASTER_JUKEBOX) {
		obj_jukebox = CreateModel("obj_jukebox", "jukebox.obj", "jukebox.jpg", true, position, scale, rotation, true, false);
	}
	else {
		obj_jukebox = CreateModel("obj_jukebox", "cube_triangles_normals_tex.obj", "jukebox.jpg", true, position, 0.5f, rotation, true, false); // for testing (faster loading)	
	}
	// Table
	position = glm::vec3(1.0f, 0.0f, 6.0f);
	scale = 0.015f;
	rotation = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	CreateModel("obj_table", "table.obj", "table.png", true, position, scale, rotation, true, true);
	// Box1
	position = glm::vec3(4.0f, 0.5f, 15.0f);
	scale = 0.2f;
	rotation = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	CreateModel("obj_box1", "box.obj", "box.png", true, position, scale, rotation, true, true);
	// Box2
	position = glm::vec3(2.98f, 0.5f, 15.0f);
	scale = 0.2f;
	rotation = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	CreateModel("obj_box2", "box.obj", "box.png", true, position, scale, rotation, true, true);
	// Box3
	position = glm::vec3(3.5f, 1.5f, 15.0f);
	scale = 0.2f;
	rotation = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	CreateModel("obj_box3", "box.obj", "box.png", true, position, scale, rotation, true, true);
	// Projectiles
	print("Loading projectiles:");
	position = glm::vec3(0.0f, -10.0f, 0.0f); // Hidden
	scale = 0.05f;
	rotation = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	for (int i = 0; i < NUMBER_OF_PROJECTILES; i++) {
		auto name = "obj_projectile_" + std::to_string(i);
		auto obj_projectile_x = CreateModel(name, "sphere_tri_vnt.obj", "Red.png", true, position, scale, rotation, false, false);
		projectiles[i] = obj_projectile_x;
	}
	// Testing AABB spheres (visualize AABB collider around object (table))
	if (DEBUG_BOUNDINGS) {
		auto obj_table = scene_opaque.find("obj_table")->second;
		CreateModel("obj_test_0", "sphere_tri_vnt.obj", "Green.png", true, obj_table->position + obj_table->collision_aabb_min, scale, rotation, false, false);
		CreateModel("obj_test_1", "sphere_tri_vnt.obj", "Green.png", true, obj_table->position + obj_table->collision_aabb_max, scale, rotation, false, false);
	}

	// = TRANSPARENT MODELS =
	// Glass cubes on the table
	scale = 0.5f;
	rotation = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	position = glm::vec3(0.2f, 1.0f, 6.0f);
	CreateModel("obj_glass_cube_r", "cube_triangles_normals_tex.obj", "Red.png", false, position, scale, rotation, true, false);
	position.x += 0.8f;
	CreateModel("obj_glass_cube_g", "cube_triangles_normals_tex.obj", "Green.png", false, position, scale, rotation, true, false);
	position.x += 0.8f;
	CreateModel("obj_glass_cube_b", "cube_triangles_normals_tex.obj", "Blue.png", false, position, scale, rotation, true, false);
	// Testing bounding sphere (visualize bounding sphere collider around object)
	if (DEBUG_BOUNDINGS) {
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		scale = 0.2f;
		rotation = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
		CreateModel("obj_sphere", "sphere_tri_vnt.obj", "Green.png", false, position, scale, rotation, false, false);
	}

	// == HEIGHTMAP ==
	print("Loading heightmap ...");
	std::filesystem::path heightspath("./resources/textures/heights.png");
	std::filesystem::path texturepath("./resources/textures/tex_256.png");
	position = glm::vec3(-HEIGHTMAP_SHIFT, 0.0f, -HEIGHTMAP_SHIFT);
	scale = HEIGHTMAP_SCALE;
	rotation = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	auto obj_heightmap = new Obj("heightmap", heightspath, texturepath, position, scale, rotation, true, false);
	scene_opaque.insert({ "obj_heightmap", obj_heightmap });
	_heights = &obj_heightmap->_heights;

	// == for TRANSPARENT OBJECTS sorting ==	
	for (auto i = scene_transparent.begin(); i != scene_transparent.end(); i++) {
		scene_transparent_pairs.push_back(&*i); // Add pairs to vector because map cannot be sorted
	}
}

void App::UpdateModels(float delta_time)
{
	glm::vec3 position{};
	float scale{};
	glm::vec4 rotation{};

	// GLASS CUBES rotation
	auto cube_pair = scene_transparent.find("obj_glass_cube_r");
	if (cube_pair != scene_transparent.end()) cube_pair->second->rotation = glm::vec4(0.0f, 1.0f, 0.0f, 23 * glfwGetTime());
	cube_pair = scene_transparent.find("obj_glass_cube_g");
	if (cube_pair != scene_transparent.end()) cube_pair->second->rotation = glm::vec4(0.0f, 1.0f, 0.0f, 45 * glfwGetTime());
	cube_pair = scene_transparent.find("obj_glass_cube_b");
	if (cube_pair != scene_transparent.end()) cube_pair->second->rotation = glm::vec4(0.0f, 1.0f, 0.0f, 90 * glfwGetTime());

	// JUKEBOX
	// - rotate towards player
	float angles = glm::degrees(atan2(-jukebox_to_player.y, jukebox_to_player.x)) + 90;
	rotation = glm::vec4(0.0f, 1.0f, 0.0f, angles);
	obj_jukebox->rotation = rotation;
	// - move towards player
	if (glm::length(jukebox_to_player) > 2.0f) {
		position = obj_jukebox->position;
		position.x += jukebox_to_player_n.x * delta_time * JUKEBOX_SPEED;
		position.z += jukebox_to_player_n.y * delta_time * JUKEBOX_SPEED;
		position.y = GetHeightmapY(position.x, position.z);
		obj_jukebox->position = position;
	}

	// --------------------
	// Bounding sphere test (visualize bounding sphere collider around object)
	if (DEBUG_BOUNDINGS) {
		auto model_to_check = scene_opaque.find("obj_jukebox")->second;
		position = model_to_check->position + model_to_check->collision_bs_center;
		scale = model_to_check->collision_bs_radius;
		scene_transparent.find("obj_sphere")->second->position = position;
		scene_transparent.find("obj_sphere")->second->scale = scale;
	}
}