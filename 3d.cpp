#include "vgfw.h"

#include <cmath>
#include <vector>

union Vec2i {
    Vec2i()
        : Vec2i(0)
    {
    }

    Vec2i(int i)
        : Vec2i(i, i)
    {
    }

    Vec2i(int x_, int y_)
        : x(x_), y(y_)
    {
    }

    struct
    {
        int x, y;
    };

    int v[2];

    int& operator[](int ord) { return v[ord]; }
    const int& operator[](int ord) const { return v[ord]; }
};

union Vec3 {
    Vec3()
        : Vec3(0.0f)
    {
    }

    Vec3(float f)
        : Vec3(f, f, f)
    {
    }

    Vec3(float x_, float y_, float z_)
        : x(x_)
        , y(y_)
        , z(z_)
    {
    }

    struct
    {
        float x, y, z;
    };

    float v[3];

    float& operator[](int ord) { return v[ord]; }
    const float& operator[](int ord) const { return v[ord]; }
};

union Vec4 {
    Vec4()
        : Vec4(0.0f)
    {
    }

    Vec4(float f)
        : Vec4(f, f, f, f)
    {
    }

    Vec4(float x_, float y_, float z_, float w_)
        : x(x_)
        , y(y_)
        , z(z_)
        , w(w_)
    {
    }

    Vec4(const Vec3& v3, float w_)
        : Vec4(v3.x, v3.y, v3.z, w_)
    {
    }

    struct
    {
        float x, y, z, w;
    };

    float v[4];

    float& operator[](int ord) { return v[ord]; }
    const float& operator[](int ord) const { return v[ord]; }
};

union Mat4 {
    Mat4()
        : X()
        , Y()
        , Z()
        , P()
    {
    }

    struct
    {
        Vec4 X, Y, Z, P;
    };

    Vec4 m[4];

    Vec4& operator[](int ord) { return m[ord]; }
    const Vec4& operator[](int ord) const { return m[ord]; }

    Vec4 row(int ord) const { return Vec4(X[ord], Y[ord], Z[ord], P[ord]); }

    static Mat4 identity()
    {
        Mat4 m;

        for (int i = 0; i < 4; ++i)
        {
            m[i][i] = 1.0f;
        }

        return m;
    }

    static Mat4 projection(float fov, float aspect, float znear, float zfar)
    {
        float f = 1.0f / tanf(fov * 3.1415927410f / 360.0f);
        Mat4 proj;
        proj[0][0] = f / aspect;
        proj[1][1] = f;
        proj[2][2] = -(zfar + znear) / (zfar - znear);
        proj[2][3] = -1.0f;
        proj[3][3] = -(2.0f * zfar * znear) / (zfar - znear);
        return proj;
    }
};

struct Triangle
{
    Vec3 p[3];
};

struct Mesh
{
    std::vector<Triangle> tris;
};

float dot(const Vec4& a, const Vec4& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

Mat4 operator*(const Mat4& a, const Mat4& b)
{
    Mat4 m;

    for (int i = 0; i < 4; ++i)
    {
        Vec4 r = a.row(i);

        for (int j = 0; j < 4; ++j)
        {
            m[i][j] = dot(r, b[j]);
        }
    }

    return m;
}

Vec4 operator*(const Mat4& m, const Vec4& v)
{
    Vec4 v2;

    for (int i = 0; i < 4; ++i)
    {
        v2[i] = dot(m.row(i), v);
    }

    return v2;
}

Vec4 operator/(const Vec4& v, float s)
{
    Vec4 r;

    for (int i = 0; i < 4; ++i)
    {
        r[i] = v[i] / s;
    }

    return r;
}

Vec3 transform(const Mat4& m, const Vec3& v)
{
    Vec4 r = m * Vec4(v, 1.0f);
    return Vec3(r.x, r.y, r.z);
}

Vec3 rotate(const Mat4& m, const Vec3& v)
{
    Vec4 r = m * Vec4(v, 0.0f);
    return Vec3(r.x, r.y, r.z);
}

class TestVgfw : public Vgfw
{
public:
    bool on_create() override
    {
        model = Mat4::identity();
        view = Mat4::identity();
        view.P.z = 5.0f;
        proj = Mat4::projection(90.0f, screen_width / (float)screen_height, 0.1f, 10.0f);

        // clang-format off
        Vec3 A(-1.0f,  1.0f,  1.0f);
        Vec3 B( 1.0f,  1.0f,  1.0f);
        Vec3 C( 1.0f, -1.0f,  1.0f);
        Vec3 D(-1.0f, -1.0f,  1.0f);
        Vec3 E(-1.0f,  1.0f, -1.0f);
        Vec3 F( 1.0f,  1.0f, -1.0f);
        Vec3 G( 1.0f, -1.0f, -1.0f);
        Vec3 H(-1.0f, -1.0f, -1.0f);

        Mesh cube;
        cube.tris =
        {
            { A, B, D },
            { B, D, C },
            { E, F, G },
            { E, G, H },
            { F, E, B },
            { E, A, B },
            { A, E, D },
            { E, H, D },
            { F, B, G },
            { B, C, G },
            { H, G, D },
            { G, C, D }
        };
        meshes.push_back(cube);

        return true;
    }

    bool on_update(float delta) override
    {
        draw_scene();
        return true;
    }

    void on_destroy() override {}

    void draw_scene()
    {
        clear_screen(0);

        Mat4 mvp = proj * view * model;

        for (const Mesh& m : meshes)
        {
            for (const Triangle& t : m.tris)
            {
                draw_triangle(mvp, t, 15);
            }
        }
    }

    void draw_triangle(const Mat4& mvp, const Triangle& tri, uint8_t c)
    {
        Vec4 clip_coords[3];

        for (int i = 0; i < 3; ++i)
        {
            clip_coords[i] = mvp * Vec4(tri.p[i], 1.0f);
        }

        Vec4 ndc_coords[3];

        for (int i = 0; i < 3; ++i)
        {
            ndc_coords[i] = clip_coords[i] / clip_coords[i].w;
        }

        Vec2i screen_coords[3];

        for (int i = 0; i < 3; ++i)
        {
            screen_coords[i].x = (ndc_coords[i].x + 1.0f) * screen_width / 2.0f;
            screen_coords[i].y = (ndc_coords[i].y + 1.0f) * screen_height / 2.0f;
        }

        draw_line(screen_coords[0].x, screen_coords[0].y, screen_coords[1].x, screen_coords[1].y, c);
        draw_line(screen_coords[1].x, screen_coords[1].y, screen_coords[2].x, screen_coords[2].y, c);
        draw_line(screen_coords[2].x, screen_coords[2].y, screen_coords[0].x, screen_coords[0].y, c);
    }

    std::vector<Mesh> meshes;
    Mat4 model;
    Mat4 view;
    Mat4 proj;
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
