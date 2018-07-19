#include "vgfw.h"

class TestVgfw : public Vgfw
{
public:
    bool on_create() override { return true; }

    bool on_update(float delta) override
    {
        draw_line(0, 0, screen_width, screen_height, 14);
        draw_line(screen_width, 0, 0, screen_height, 2);
        draw_line(10, 0, 14, screen_height, 4);
        draw_line(0, 10, screen_width, 12, 8);

        return true;
    }

    void on_destroy() override {}
};

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    TestVgfw test_app;

    if (!test_app.initialize(L"Vgfw 3D Renderer"))
    {
        exit(EXIT_FAILURE);
    }

    test_app.run();

    return EXIT_SUCCESS;
}
