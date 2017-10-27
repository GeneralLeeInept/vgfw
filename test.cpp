#include "vgfw.h"

class TestVgfw : public Vgfw
{
public:
    bool on_create() override { return true; }

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

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    TestVgfw test_app;

    if (!test_app.initialize(L"Vgfw Test App"))
    {
        exit(EXIT_FAILURE);
    }

    test_app.run();

    return EXIT_SUCCESS;
}
