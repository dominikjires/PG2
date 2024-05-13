#include "App.hpp"

float App::GetHeightmapY(float position_x, float position_z) const
{
    float hm_x_f = position_x + HEIGHTMAP_SHIFT;
    float hm_z_f = position_z + HEIGHTMAP_SHIFT;
    float hm_y_f = 0.0f;
    float hm_x_i = std::floor(hm_x_f);
    float hm_z_i = std::floor(hm_z_f);
    if (hm_x_f - hm_x_i < 0.5f && hm_z_f - hm_z_i < 0.5f) {
        // In the lower-left triangle
        float x_fraction = hm_x_f - hm_x_i;
        float y_fraction = hm_z_f - hm_z_i;
        float common_height = (*_heights)[{hm_x_i, hm_z_i}];
        float x_difference = (*_heights)[{hm_x_i + 1, hm_z_i}] - common_height;
        float y_difference = (*_heights)[{hm_x_i, hm_z_i + 1}] - common_height;
        hm_y_f = common_height + x_fraction * x_difference + y_fraction * y_difference;
    }
    else {
        // In the upper-right triangle
        float x_fraction = hm_x_i + 1 - hm_x_f;
        float y_fraction = hm_z_i + 1 - hm_z_f;
        float common_height = (*_heights)[{hm_x_i + 1, hm_z_i + 1}];
        float x_difference = common_height - (*_heights)[{hm_x_i, hm_z_i + 1}];
        float y_difference = common_height - (*_heights)[{hm_x_i + 1, hm_z_i}];
        hm_y_f = common_height - x_fraction * x_difference - y_fraction * y_difference;
    }
    return hm_y_f * HEIGHTMAP_SCALE;
}