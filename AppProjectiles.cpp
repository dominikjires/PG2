#include <iostream>
#include <string>

#include "App.hpp"

#define print(x) //std::cout << x << "\n"

void App::Shoot()
{
	auto name = "obj_projectile_" + std::to_string(projectile_n);	// Name of currently used projectile

	scene_opaque.find(name)->second->position = camera.position;	// Teleport it to camera's position

	projectile_directions[projectile_n] = camera.front;				// Projectile direction == camera's look direction
	is_projectile_moving[projectile_n] = true;						// Projectile is marked as moving (not idle)

	projectile_n = (projectile_n + 1) % N_PROJECTILES;				// Switch to next projectile
}

void App::UpdateProjectiles(float delta_time)
{
	for (int i = 0; i < N_PROJECTILES; i++) { // Every frame
		if (is_projectile_moving[i]) {		  // for every projectile that's not idle
			auto name = "obj_projectile_" + std::to_string(i);
			auto projectile = scene_opaque.find(name)->second;
			auto position = projectile->position;

			// Projectile movement
			projectile->position += projectile_speed * delta_time * projectile_directions[i];

			// Projectile collision check			
			bool hit = false;

			// - Objects collision check
			for (const auto model : collisions) {		// For every model that has been marked as being able to collide with a projectile
				if (model->Collision_CheckPoint(position)) { // If projectile's center inside collider
					const auto& hit_name = model->name;
					print("PROJECTILE HIT " << hit_name);
					hit = true;

					// Projectile hit glass cube – destroy it & play sound
					if (hit_name.substr(0, 15) == "obj_glass_cube_") {
						if (!HIDE_CUBES_INSTEAD_DESTROY) {
							// Remove cube from possible collisions vector
							collisions.erase(std::remove(collisions.begin(), collisions.end(), model), collisions.end());
							// Remove cube from the scene
							scene_transparent.erase(hit_name);
							// Remove cube from helper vector for sorting transparent objects (by clearing the whole thing and regenerating it)
							scene_transparent_pairs.clear();
							for (auto i = scene_transparent.begin(); i != scene_transparent.end(); i++) {
								scene_transparent_pairs.push_back(&*i);
							}
							// Cleanup
							model->Clear();
						}
						else {
							// Only hide the cube, so it can be easily respawned by pressing the 'R' key
							model->position.y -= HIDE_CUBE_Y;
						}
						// Play broken glass audio
						audio.Play3DOneShot("snd_glass", position);
					}
					// Projectile hit jukebox – on/off light+music
					else if (hit_name == "obj_jukebox") {
						is_jukebox_on = (is_jukebox_on + 1) % 2;
						audio.UpdateMusicVolume(static_cast<float>(is_jukebox_on));
						audio.Play3DOneShot("snd_hit", position);
					}
					// other non-ground impact (obj_table)
					else {
						audio.Play3DOneShot("snd_hit", position);
					}

					break; // No need to check for other collisions
				}
			}

			// - Heightmap collision check – if hits ground hide and play sound
			if (position.y < GetHeightmapY(position.x, position.z)) {
				print("PROJECTILE HIT ground");
				hit = true;
				audio.Play3DOneShot("snd_hit", position);
			}

			// - Hide if hit and set as idle
			if (hit) {
				is_projectile_moving[i] = false;
				projectile->position = glm::vec3(0.0f, -10.0f, 0.0f);
			}
		}
	}
}
