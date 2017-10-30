#include "vgfw.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

class TestVgfw : public Vgfw
{
public:
    bool on_create() override 
    {
        int x, y, comp;
        stbi_uc *palette_data = stbi_load("textures\\palette.png", &x, &y, &comp, 3);

        if (!palette_data)
        {
            return false;
        }

        set_palette(palette_data);

        return true; 
    }

    bool on_update(float delta) override
    {
        for (int x = 0; x < 320; ++x)
        {
            for (int y = 0; y < 240; ++y)
            {
                set_pixel(x, y, (x / 20) + (y / 15) * 16);
            }
        }

        return true;
    }

    void on_destroy() override {}
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
