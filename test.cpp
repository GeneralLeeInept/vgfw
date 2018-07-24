#include "vgfw.h"

class TestVgfw : public Vgfw
{
public:
    uint8_t make_color(float r, float g, float b)
    {
        int red_bits = (r == 1.0f) ? 7 : static_cast<int>(r * 8);
        int green_bits = (g == 1.0f) ? 7 : static_cast<int>(g * 8);
        int blue_bits = (b == 1.0f) ? 3 : static_cast<int>(b * 4);
        return (red_bits << 5) | (green_bits << 2) | blue_bits;
    }

    bool on_create() override
    {
        uint8_t r3g3b2[768];

        for (int i = 0; i < 256; ++i)
        {
            int red_bits = (i >> 5) & 7;
            int green_bits = (i >> 2) & 7;
            int blue_bits = i & 0x3;

            r3g3b2[(i * 3) + 0] = (red_bits * 255) / 7;
            r3g3b2[(i * 3) + 1] = (green_bits * 255) / 7;
            r3g3b2[(i * 3) + 2] = (blue_bits * 255) / 3;
        }

        set_palette(r3g3b2);

        red_spot[0] = 0.25f;
        red_spot[1] = 0.25f;
        green_spot[0] = 0.75f;
        green_spot[1] = 0.25f;
        blue_spot[0] = 0.5f;
        blue_spot[1] = 0.75f;

        red_vel[0] = 0.2f;
        red_vel[1] = 0.3f;
        green_vel[0] = -0.14f;
        green_vel[1] = 0.2f;
        blue_vel[0] = 0.3;
        blue_vel[1] = 0.14;

        return true;
    }

    bool on_update(float delta) override
    {
        for (int i = 0; i < 2; ++i)
        {
            red_spot[i] += red_vel[i] * delta;
            if (red_spot[i] < 0.0f)
            {
                red_spot[i] = 0.0f;
                red_vel[i] = -red_vel[i];
            }
            else if (red_spot[i] > 1.0f)
            {
                red_spot[i] = 1.0f;
                red_vel[i] = -red_vel[i];
            }

            green_spot[i] += green_vel[i] * delta;
            if (green_spot[i] < 0.0f)
            {
                green_spot[i] = 0.0f;
                green_vel[i] = -green_vel[i];
            }
            else if (green_spot[i] > 1.0f)
            {
                green_spot[i] = 1.0f;
                green_vel[i] = -green_vel[i];
            }

            blue_spot[i] += blue_vel[i] * delta;
            if (blue_spot[i] < 0.0f)
            {
                blue_spot[i] = 0.0f;
                blue_vel[i] = -blue_vel[i];
            }
            else if (blue_spot[i] > 1.0f)
            {
                blue_spot[i] = 1.0f;
                blue_vel[i] = -blue_vel[i];
            }
        }

        if (m_keys[L' '].pressed)
        {
            greyscale = !greyscale;
        }

        if (!greyscale)
        {
            for (int x = 0; x < 320; ++x)
            {
                for (int y = 0; y < 240; ++y)
                {
                    float fx = x / 320.0f;
                    float fy = y / 240.0f;

                    float dr = sqrtf((red_spot[0] - fx) * (red_spot[0] - fx) + (red_spot[1] - fy) * (red_spot[1] - fy));
                    dr = dr < 1.0f ? dr : 1.0f;
                    float r = 1.0f - dr;

                    float dg = sqrtf((green_spot[0] - fx) * (green_spot[0] - fx) + (green_spot[1] - fy) * (green_spot[1] - fy));
                    dg = dg < 1.0f ? dg : 1.0f;
                    float g = 1.0f - dg;

                    float db = sqrtf((blue_spot[0] - fx) * (blue_spot[0] - fx) + (blue_spot[1] - fy) * (blue_spot[1] - fy));
                    db = db < 1.0f ? db : 1.0f;
                    float b = 1.0f - db;

                    set_pixel(x, y, make_color(r * r, g * g, b * b));
                }
            }
        }
        else
        {
            for (int x = 0; x < 320; ++x)
            {
                for (int y = 0; y < 240; ++y)
                {
                    float fx = x / 320.0f;
                    float fy = y / 240.0f;

                    float dr = sqrtf((0.5f - fx) * (0.5f - fx) + (0.5f - fy) * (0.5f - fy));
                    dr = dr < 1.0f ? dr : 1.0f;
                    float r = (1.0f - dr) * (1.0f - dr);

                    set_pixel(x, y, make_color(r, r, r));
                }
            }
        }
        return true;
    }

    void on_destroy() override {}

    bool greyscale = false;
    float red_spot[2];
    float green_spot[2];
    float blue_spot[2];
    float red_vel[2];
    float green_vel[2];
    float blue_vel[2];
};

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    TestVgfw test_app;

    if (!test_app.initialize(L"Vgfw Test App"))
    {
        exit(EXIT_FAILURE);
    }

    test_app.run();

    return EXIT_SUCCESS;
}
