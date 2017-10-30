#include "vgfw.h"

#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

class TestVgfw : public Vgfw
{
    struct Vec2
    {
        float x, y;
    };

    float screen_distance;
    float player_facing = 0.0f;
    float player_x = 3.5f;
    float player_y = 2.5f;
    Vec2 screen_rays[screen_width];

    const float pi = 3.14159265f;
    const float player_radius = 0.3f;
    const float fov = 90.0f;
    const float turn_speed = 90.0f;     // degrees / second
    const float walk_speed = 5.0f;
    const float wall_height = 1.0f;

    static const int world_size_x = 32;
    static const int world_size_y = 32;
    const char world_map[world_size_y][world_size_x + 1] = 
    {
        "#########.......#########.......",
        "#...............#...............",
        "#.......#########.......########",
        "#..............##..............#",
        "#......##......##......##......#",
        "#......##..............##......#",
        "#..............##..............#",
        "###............####............#",
        "##.............###.............#",
        "#............####............###",
        "#..............................#",
        "#..............##..............#",
        "#..............##..............#",
        "#...........#####...........####",
        "#..............................#",
        "###..####....########....#######",
        "####.####.......######..........",
        "#...............#...............",
        "#.......#########.......##..####",
        "#..............##..............#",
        "#......##......##.......#......#",
        "#......##......##......##......#",
        "#..............##..............#",
        "###............####............#",
        "##.............###.............#",
        "#............####............###",
        "#..............................#",
        "#..............................#",
        "#..............##..............#",
        "#...........##..............####",
        "#..............##..............#",
        "################################"
    };

    const uint8_t sky_color = 5;
    const uint8_t floor_color = 3;

    uint8_t* wall_texture = nullptr;

    uint8_t attenuate(uint8_t color, float intensity)
    {
        intensity = (intensity > 1.0f) ? 1.0f : ((intensity < 0.0f) ? 0.0f : intensity);
        int idx = 7 - (int)(intensity * 7.0f + 0.5f);
        return color + idx * 32;
    }

    float degrees_to_radians(float d)
    {
        static const float radians_per_degree = pi / 180.0f;
        return d * radians_per_degree;
    }

    uint8_t map_color_to_palette(uint8_t r, uint8_t g, uint8_t b, uint8_t* palette)
    {
        for (int p = 0; p < 256; ++p)
        {
            if (palette[p * 3] == r && palette[(p * 3) + 1] == g && palette[(p * 3) + 2] == b)
            {
                return p;
            }
        }

        return 0;
    }

    uint8_t* load_texture(const char* path, uint8_t* palette)
    {
        int x, y, comp;
        stbi_uc* file_data = stbi_load(path, &x, &y, &comp, 3);

        if (!file_data)
        {
            return nullptr;
        }

        if (x != 64 || y != 64)
        {
            return nullptr;
        }

        uint8_t* texture = new uint8_t[64 * 64];

        for (int p = 0; p < 64 * 64; ++p)
        {
            uint8_t r = file_data[p * 3];
            uint8_t g = file_data[(p * 3) + 1];
            uint8_t b = file_data[(p * 3) + 2];
            texture[p] = map_color_to_palette(r, g, b, palette);
        }

        return texture;
    }

    bool on_create() override
    {
        // Work out field of view and screen distance
        float screen_aspect = (float)screen_width / (float)screen_height;
        float half_fov = degrees_to_radians(fov) * 0.5f;
        screen_distance = screen_aspect / tanf(half_fov);

        // Pre calculate screen space ray directions
        for (int col = 0; col < screen_width; ++col)
        {
            // For each column construct the ray from the player's position through the column.
            float rx = screen_distance;
            float ry = screen_aspect * (col - screen_width * 0.5f) / (float)screen_width;
            float rn = 1.0f / sqrtf(rx * rx + ry * ry);
            screen_rays[col].x = rx * rn;
            screen_rays[col].y = ry * rn;
        }

        // Load palette
        int x, y, comp;
        stbi_uc* palette_data = stbi_load("textures\\palette.png", &x, &y, &comp, 3);

        if (!palette_data)
        {
            return false;
        }

        set_palette(palette_data);

        wall_texture = load_texture("textures\\bricks.png", palette_data);

        stbi_image_free(palette_data);

        if (!wall_texture)
        {
            return false;
        }

        return true;
    }

    void on_destroy() override
    {
        delete [] wall_texture;
    }

    bool on_update(float delta) override
    {
        float cos_facing = cosf(player_facing);
        float sin_facing = sinf(player_facing);

        // Turn around
        if (m_keys[L'A'].down)
        {
            player_facing -= degrees_to_radians(turn_speed) * delta;
        }

        if (m_keys[L'D'].down)
        {
            player_facing += degrees_to_radians(turn_speed) * delta;
        }

        // Move
        float move_x = 0.0f;
        float move_y = 0.0f;

        if (m_keys[L'W'].down)
        {
            move_x += cos_facing * delta * walk_speed;
            move_y += sin_facing * delta * walk_speed;
        }

        if (m_keys[L'S'].down)
        {
            move_x -= cos_facing * delta * walk_speed;
            move_y -= sin_facing * delta * walk_speed;
        }

        if (m_keys[L'Q'].down)
        {
            move_x += sin_facing * delta * walk_speed;
            move_y -= cos_facing * delta * walk_speed;
        }

        if (m_keys[L'E'].down)
        {
            move_x -= sin_facing * delta * walk_speed;
            move_y += cos_facing * delta * walk_speed;
        }

        // Clip movement
        if (collision_check(player_x + move_x, player_y + move_y, player_radius))
        {
            if (!collision_check(player_x + move_x, player_y, player_radius))
            {
                move_y = 0.0f;
            }
            else if (!collision_check(player_x, player_y + move_y, player_radius))
            {
                move_x = 0.0f;
            }
            else
            {
                move_x = 0.0f;
                move_y = 0.0f;
            }
        }

        player_x += move_x;
        player_y += move_y;

        // Draw
        for (int col = 0; col < screen_width; ++col)
        {
            // Get world space ray direction for column
            float rx = cos_facing * screen_rays[col].x - sin_facing * screen_rays[col].y;
            float ry = sin_facing * screen_rays[col].x + cos_facing * screen_rays[col].y;

            // Cast ray through the map and find the nearest intersection with a solid tile
            float distance = FLT_MAX;
            int column = 0;

            if (rx > 0.0f)
            {
                float dydx = ry / rx;
                int tx = floorf(player_x + 1.0f);

                for ( ; tx < world_size_x; ++tx)
                {
                    float y = player_y + dydx * (tx - player_x);
                    int ty = floorf(y);

                    if (ty < 0 || ty >= world_size_y)
                    {
                        break;
                    }

                    if (world_map[ty][tx] != '.')
                    {
                        float hit_d = (tx - player_x) * (tx - player_x) + (y - player_y) * (y - player_y);

                        if (hit_d < distance)
                        {
                            distance = hit_d;
                            column = floorf(fmodf(y, 1.0f) * 64.0f);
                        }

                        break;
                    }
                }
            }
            else if (rx < 0.0f)
            {
                float dydx = ry / rx;
                int tx = floorf(player_x - 1.0f);

                for ( ; tx >= 0; --tx)
                {
                    float y = player_y + dydx * (tx + 1.0f - player_x);
                    int ty = floorf(y);

                    if (ty < 0 || ty >= world_size_y)
                    {
                        break;
                    }

                    if (world_map[ty][tx] != '.')
                    {
                        float hit_d = (tx + 1.0f - player_x) * (tx + 1.0f - player_x) + (y - player_y) * (y - player_y);

                        if (hit_d < distance)
                        {
                            distance = hit_d;
                            column = floorf(fmodf(y, 1.0f) * 64.0f);
                        }

                        break;
                    }
                }
            }

            if (ry > 0.0f)
            {
                float dxdy = rx / ry;
                int ty = floorf(player_y + 1.0f);

                for ( ; ty < world_size_y; ++ty)
                {
                    float x = player_x + dxdy * (ty - player_y);
                    int tx = floorf(x);

                    if (tx < 0 || tx >= world_size_x)
                    {
                        break;
                    }

                    if (world_map[ty][tx] != '.')
                    {
                        float hit_d = (x - player_x) * (x - player_x) + (ty - player_y) * (ty - player_y);

                        if (hit_d < distance)
                        {
                            distance = hit_d;
                            column = floorf(fmodf(x, 1.0f) * 64.0f);
                        }

                        break;
                    }
                }
            }
            else if (ry < 0.0f)
            {
                float dxdy = rx / ry;
                int ty = floorf(player_y - 1.0f);

                for ( ; ty >= 0; --ty)
                {
                    float x = player_x + dxdy * (ty + 1.0f - player_y);
                    int tx = floorf(x);

                    if (tx < 0 || tx >= world_size_x)
                    {
                        break;
                    }

                    if (world_map[ty][tx] != '.')
                    {
                        float hit_d = (x - player_x) * (x - player_x) + (ty + 1.0f - player_y) * (ty + 1.0f - player_y);

                        if (hit_d < distance)
                        {
                            distance = hit_d;
                            column = floorf(fmodf(x, 1.0f) * 64.0f);
                        }

                        break;
                    }
                }
            }

            if (distance == FLT_MAX)
            {
                // Ray hit nothing
                for (int y = 0; y < screen_height / 2; ++y)
                {
                    set_pixel(col, y, sky_color);
                    set_pixel(col, y + screen_height / 2, floor_color);
                }
            }
            else
            {
                // Project hit distance to view depth
                distance = sqrtf(distance) * screen_rays[col].x;

                // Scale wall column height by distance
                float column_height = wall_height * (screen_distance / distance);
                int ceiling = (int)((1.0f - column_height) * 0.5f * screen_height);
                int floor = screen_height - ceiling;

                for (int y = 0; y < screen_height && y < ceiling; ++y)
                {
                    set_pixel(col, y, sky_color);
                }

                for (int y = ceiling; y < screen_height && y < floor; ++y)
                {
                    float v = (float)(y - ceiling) / (float)(floor - ceiling);
                    int iv = floorf(fmodf(v, 1.0f) * 64.0f);
                    uint8_t texel = wall_texture[iv * 64 + column];
                    set_pixel(col, y, attenuate(texel, column_height));
                }

                for (int y = floor; y < screen_height; ++y)
                {
                    set_pixel(col, y, floor_color);
                }
            }
        }

        return true;
    }

    bool collision_check(float pos_x, float pos_y, float half_size)
    {
        int sx = floor(pos_x - half_size);
        int sy = floor(pos_y - half_size);
        int ex = floor(pos_x + half_size);
        int ey = floor(pos_y + half_size);

        if (sx < 0 || ex < 0 || ex >= world_size_x || ey >= world_size_y)
        {
            return true;
        }

        for (int x = sx; x <= ex; ++x)
        {
            for (int y = sy; y <= ey; ++y)
            {
                if (world_map[y][x] != '.')
                {
                    return true;
                }
            }
        }

        return false;
    }
};

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    TestVgfw test_app;

    if (!test_app.initialize(L"First Person Shooter"))
    {
        exit(EXIT_FAILURE);
    }

    test_app.run();

    return EXIT_SUCCESS;
}
