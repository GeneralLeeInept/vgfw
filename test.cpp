#include "vgfw.h"

class TestVgfw : public Vgfw
{
public:
    uint8_t make_color(float r, float g, float b)
    {
        int red_bits = (r == 1.0f) ? 3 : static_cast<int>(r * 4);
        int green_bits = (g == 1.0f) ? 7 : static_cast<int>(g * 8);
        int blue_bits = (b == 1.0f) ? 7 : static_cast<int>(b * 8);
        return (red_bits << 6) | (green_bits << 3) | blue_bits;
    }

    bool on_create() override
    {
        uint8_t r2g3b3[768];

        for (int i = 0; i < 256; ++i)
        {
            int red_bits = (i >> 6) & 3;
            int green_bits = (i >> 3) & 7;
            int blue_bits = i & 7;

            r2g3b3[(i * 3) + 0] = (red_bits * 255) / 3;
            r2g3b3[(i * 3) + 1] = (green_bits * 255) / 7;
            r2g3b3[(i * 3) + 2] = (blue_bits * 255) / 7;
        }

        set_palette(r2g3b3);
        return true;
    }

    bool on_update(float delta) override
    {
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

                    float dr = sqrtf((0.25f - fx) * (0.25f - fx) + (0.25f - fy) * (0.25f - fy));
                    dr = dr < 1.0f ? dr : 1.0f;
                    float r = 1.0f - dr;

                    float dg = sqrtf((0.75f - fx) * (0.75f - fx) + (0.25f - fy) * (0.25f - fy));
                    dg = dg < 1.0f ? dg : 1.0f;
                    float g = 1.0f - dg;

                    float db = sqrtf((0.25f - fx) * (0.25f - fx) + (0.75f - fy) * (0.75f - fy));
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
