#pragma once

#include <chrono>

#include <Windows.h>

class Vgfw
{
public:
    static const int screen_width = 320;
    static const int screen_height = 240;
    static const int window_width = screen_width * 2;
    static const int window_height = screen_height * 2;

    virtual ~Vgfw() = default;

    virtual bool on_create() = 0;
    virtual void on_destroy() = 0;
    virtual bool on_update(float delta) = 0;

    bool initialize(const wchar_t* name)
    {
        m_title = _wcsdup(name);

        // Create window
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = window_proc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.lpszClassName = m_classname;

        if (!RegisterClassEx(&wc))
        {
            return false;
        }

        DWORD dwExStyle = WS_EX_OVERLAPPEDWINDOW;
        DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
        RECT client_rect = { 0, 0, window_width, window_height };
        AdjustWindowRectEx(&client_rect, dwStyle, FALSE, dwExStyle);
        int width = client_rect.right - client_rect.left;
        int height = client_rect.bottom - client_rect.top;
        m_hwnd = CreateWindowEx(dwExStyle, m_classname, m_title, dwStyle, 0, 0, width, height, NULL, NULL, GetModuleHandle(NULL), NULL);
        SetProp(m_hwnd, m_classname, (HANDLE)this);

        if (m_hwnd == NULL)
        {
            return false;
        }

        int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
        int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
        SetWindowPos(m_hwnd, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

        // Initialise key states
        for (int i = 0; i < 2; ++i)
        {
            m_keystate[i] = new short[256];
            memset(m_keystate[i], 0, sizeof(short) * 256);
        }

        // Initialise frame buffer
        for (int i = 0; i < 2; ++i)
        {
            m_framebuffer[i] = new uint8_t[screen_width * screen_height];
            memset(m_framebuffer[i], 0, screen_width * screen_height);
        }

        // Set default palette
        static uint32_t default_palette[256] = {
            0x000000, 0x0000a8, 0x00a800, 0x00a8a8, 0xa80000, 0xa800a8, 0xa85400, 0xa8a8a8, 0x545454, 0x5454fc, 0x54fc54, 0x54fcfc, 0xfc5454,
            0xfc54fc, 0xfcfc54, 0xfcfcfc, 0x000000, 0x141414, 0x202020, 0x2c2c2c, 0x383838, 0x444444, 0x505050, 0x606060, 0x707070, 0x808080,
            0x909090, 0xa0a0a0, 0xb4b4b4, 0xc8c8c8, 0xe0e0e0, 0xfcfcfc, 0x0000fc, 0x4000fc, 0x7c00fc, 0xbc00fc, 0xfc00fc, 0xfc00bc, 0xfc007c,
            0xfc0040, 0xfc0000, 0xfc4000, 0xfc7c00, 0xfcbc00, 0xfcfc00, 0xbcfc00, 0x7cfc00, 0x40fc00, 0x00fc00, 0x00fc40, 0x00fc7c, 0x00fcbc,
            0x00fcfc, 0x00bcfc, 0x007cfc, 0x0040fc, 0x7c7cfc, 0x9c7cfc, 0xbc7cfc, 0xdc7cfc, 0xfc7cfc, 0xfc7cdc, 0xfc7cbc, 0xfc7c9c, 0xfc7c7c,
            0xfc9c7c, 0xfcbc7c, 0xfcdc7c, 0xfcfc7c, 0xdcfc7c, 0xbcfc7c, 0x9cfc7c, 0x7cfc7c, 0x7cfc9c, 0x7cfcbc, 0x7cfcdc, 0x7cfcfc, 0x7cdcfc,
            0x7cbcfc, 0x7c9cfc, 0xb4b4fc, 0xc4b4fc, 0xd8b4fc, 0xe8b4fc, 0xfcb4fc, 0xfcb4e8, 0xfcb4d8, 0xfcb4c4, 0xfcb4b4, 0xfcc4b4, 0xfcd8b4,
            0xfce8b4, 0xfcfcb4, 0xe8fcb4, 0xd8fcb4, 0xc4fcb4, 0xb4fcb4, 0xb4fcc4, 0xb4fcd8, 0xb4fce8, 0xb4fcfc, 0xb4e8fc, 0xb4d8fc, 0xb4c4fc,
            0x000070, 0x1c0070, 0x380070, 0x540070, 0x700070, 0x700054, 0x700038, 0x70001c, 0x700000, 0x701c00, 0x703800, 0x705400, 0x707000,
            0x547000, 0x387000, 0x1c7000, 0x007000, 0x00701c, 0x007038, 0x007054, 0x007070, 0x005470, 0x003870, 0x001c70, 0x383870, 0x443870,
            0x543870, 0x603870, 0x703870, 0x703860, 0x703854, 0x703844, 0x703838, 0x704438, 0x705438, 0x706038, 0x707038, 0x607038, 0x547038,
            0x447038, 0x387038, 0x387044, 0x387054, 0x387060, 0x387070, 0x386070, 0x385470, 0x384470, 0x505070, 0x585070, 0x605070, 0x685070,
            0x705070, 0x705068, 0x705060, 0x705058, 0x705050, 0x705850, 0x706050, 0x706850, 0x707050, 0x687050, 0x607050, 0x587050, 0x507050,
            0x507058, 0x507060, 0x507068, 0x507070, 0x506870, 0x506070, 0x505870, 0x000040, 0x100040, 0x200040, 0x300040, 0x400040, 0x400030,
            0x400020, 0x400010, 0x400000, 0x401000, 0x402000, 0x403000, 0x404000, 0x304000, 0x204000, 0x104000, 0x004000, 0x004010, 0x004020,
            0x004030, 0x004040, 0x003040, 0x002040, 0x001040, 0x202040, 0x282040, 0x302040, 0x382040, 0x402040, 0x402038, 0x402030, 0x402028,
            0x402020, 0x402820, 0x403020, 0x403820, 0x404020, 0x384020, 0x304020, 0x284020, 0x204020, 0x204028, 0x204030, 0x204038, 0x204040,
            0x203840, 0x203040, 0x202840, 0x2c2c40, 0x302c40, 0x342c40, 0x3c2c40, 0x402c40, 0x402c3c, 0x402c34, 0x402c30, 0x402c2c, 0x40302c,
            0x40342c, 0x403c2c, 0x40402c, 0x3c402c, 0x34402c, 0x30402c, 0x2c402c, 0x2c4030, 0x2c4034, 0x2c403c, 0x2c4040, 0x2c3c40, 0x2c3440,
            0x2c3040, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000
        };

        set_palette(default_palette);

        return true;
    }

    void run()
    {
        if (!on_create())
        {
            shutdown();
            return;
        }

        ShowWindow(m_hwnd, SW_SHOW);

        bool active = true;

        auto prev_time = std::chrono::system_clock::now();

        while (active)
        {
            auto current_time = std::chrono::system_clock::now();
            std::chrono::duration<float> elapsed_time = current_time - prev_time;
            prev_time = current_time;
            float delta = elapsed_time.count();

            wchar_t title[256];
            swprintf(title, 256, L"%s - %llu us", m_title, (uint64_t)(delta * 1000000.0f));
            SetWindowText(m_hwnd, title);

            // Process Windows messages
            MSG msg;

            while (PeekMessage(&msg, m_hwnd, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            // Update keyboard
            short* current_keystate = m_keystate[m_current_keystate];
            short* prev_keystate = m_keystate[m_current_keystate ^ 1];

            for (int i = 0; i < 256; i++)
            {
                current_keystate[i] = GetAsyncKeyState(i);

                m_keys[i].pressed = false;
                m_keys[i].released = false;

                if (current_keystate[i] != prev_keystate[i])
                {
                    if (current_keystate[i] & 0x8000)
                    {
                        m_keys[i].pressed = !m_keys[i].down;
                        m_keys[i].down = true;
                    }
                    else
                    {
                        m_keys[i].released = true;
                        m_keys[i].down = false;
                    }
                }
            }

            m_current_keystate ^= 1;

            // User update
            if (!on_update(delta))
            {
                quit();
            }

            // Present
            m_frontbuffer ^= 1;
            InvalidateRect(m_hwnd, NULL, FALSE);

            // Check if Window closed
            active = !!IsWindow(m_hwnd);
        }

        on_destroy();
        shutdown();
    }

    void quit() { PostQuitMessage(0); }

    void set_pixel(uint32_t x, uint32_t y, uint8_t p)
    {
        if (x < screen_width && y < screen_height)
        {
            uint8_t* backbuffer = m_framebuffer[m_frontbuffer ^ 1];
            backbuffer[x + y * screen_width] = p;
        }
    }

    void set_palette(uint32_t rgbx[256])
    {
        for (int p = 0; p < 256; ++p)
        {
            RGBQUAD entry = {};
            entry.rgbRed = (rgbx[p] >> 16) & 255;
            entry.rgbGreen = (rgbx[p] >> 8) & 255;
            entry.rgbBlue = rgbx[p] & 255;
            m_palette[p] = entry;
        }
    }

    void clear_screen(uint8_t c)
    {
        uint8_t* backbuffer = m_framebuffer[m_frontbuffer ^ 1];
        memset(backbuffer, c, screen_width * screen_height);
    }

protected:
    struct KeyState
    {
        bool pressed;
        bool released;
        bool down;
    };

    KeyState m_keys[256] = {};

private:
    void shutdown()
    {
        for (int i = 0; i < 2; ++i)
        {
            delete[] m_framebuffer[i];
        }

        for (int i = 0; i < 2; ++i)
        {
            delete[] m_keystate[i];
        }

        UnregisterClass(m_classname, GetModuleHandle(NULL));

        free(m_title);
    }

    LRESULT on_paint()
    {
        PAINTSTRUCT ps = {};
        HDC hDC = BeginPaint(m_hwnd, &ps);

        char bmi_memory[sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD)];
        BITMAPINFO* bmi = (BITMAPINFO*)bmi_memory;
        ZeroMemory(bmi, sizeof(BITMAPINFO));
        bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi->bmiHeader.biBitCount = 8;
        bmi->bmiHeader.biHeight = -screen_height;
        bmi->bmiHeader.biWidth = screen_width;
        bmi->bmiHeader.biPlanes = 1;
        memcpy(bmi->bmiColors, m_palette, 256 * sizeof(RGBQUAD));

        HDC hMemDC = CreateCompatibleDC(hDC);
        HBITMAP hBitmap = CreateDIBSection(hMemDC, bmi, DIB_RGB_COLORS, NULL, NULL, 0);
        SetDIBits(hDC, hBitmap, 0, screen_height, m_framebuffer[m_frontbuffer], bmi, DIB_RGB_COLORS);

        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
        StretchBlt(hDC, 0, 0, window_width, window_height, hMemDC, 0, 0, screen_width, screen_height, SRCCOPY);

        SelectObject(hMemDC, hBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hMemDC);

        EndPaint(m_hwnd, &ps);

        return (LRESULT)0;
    }

    static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        Vgfw* vgfw = (Vgfw*)GetProp(hwnd, L"Vgfw");

        switch (msg)
        {
            case WM_PAINT:
            {
                return vgfw->on_paint();
            }
        }

        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    const wchar_t* m_classname = L"Vgfw";

    HWND m_hwnd = NULL;
    short* m_keystate[2] = {};
    int m_current_keystate = 0;
    uint8_t* m_framebuffer[2] = {};
    int m_frontbuffer = 0;
    wchar_t* m_title = nullptr;
    RGBQUAD m_palette[256] = {};
};
