// NEŠAHAT KOMPLET
#include <iostream>
#include <string>
#include "App.hpp"

// Function to shoot projectiles
void App::Shoot()
{
	// Generate unique name for the projectile
	auto name = "obj_projectile_" + std::to_string(number_of_projectiles);
	// Set projectile position to camera position
	scene_opaque.find(name)->second->position = camera.position;
	// Set projectile direction
	projectile_directions[number_of_projectiles] = camera.front;
	// Set projectile state to moving
	is_projectile_moving[number_of_projectiles] = true;
	// Increment projectile counter and wrap around if exceeds limit
	number_of_projectiles = (number_of_projectiles + 1) % PROJECTILES_N;
}

// Function to check collision with objects in the scene
bool App::CheckCollision(const glm::vec3& position)
{
	// Iterate through each model in collisions vector
	for (const auto model : collisions) {
		// Check if collision occurs with the model
		if (model->Collision_CheckPoint(position)) {
			// Get the name of the collided model
			const auto& hit_name = model->name;
			// If the collided model is a glass cube
			if (hit_name.substr(0, 10) == "obj_sphere") {
				// Move the cube downwards to hide it
				model->position.y -= HIDE_CUBE_Y;
				// Play glass breaking sound
				audio.PlayShot("sound_glass");
			}
			// Return true indicating collision occurred
			return true;
		}
	}
	// Return false indicating no collision occurred
	return false;
}

// Function to update projectile positions and check for collisions
void App::UpdateProjectiles(float delta_time)
{
	// Iterate through each projectile
	for (int i = 0; i < PROJECTILES_N; i++) {
		// If projectile is moving
		if (is_projectile_moving[i]) {
			// Generate unique name for the projectile
			auto name = "obj_projectile_" + std::to_string(i);
			// Get projectile object
			auto projectile = scene_opaque.find(name)->second;
			// Get current position of projectile
			auto position = projectile->position;

			// Update projectile position based on speed and direction
			projectile->position += projectile_speed * delta_time * projectile_directions[i];

			// Check for collision with updated position
			bool hit = CheckCollision(position);

			// If collision occurred
			if (hit) {
				// Set projectile state to not moving
				is_projectile_moving[i] = false;
				// Move projectile below the scene to hide it
				projectile->position = glm::vec3(0.0f, -1.0f, 0.0f);
			}
		}
	}
}


