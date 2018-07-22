#include "vgfw.h"

#include <cmath>
#include <vector>

const float pi = atanf(1.0f) * 4.0f;

float deg_to_rad(float deg)
{
    return deg * pi / 180.0f;
}

union Vec2 {
    struct
    {
        float x;
        float y;
    };

    float v[2];

    Vec2()
        : Vec2(0.0f)
    {
    }

    Vec2(const float& a)
        : Vec2(a, a)
    {
    }

    Vec2(float x_, float y_)
        : x(x_)
        , y(y_)
    {
    }

    float& operator[](int ord) { return v[ord]; }
    const float& operator[](int ord) const { return v[ord]; }
};

union Vec3 {
    struct
    {
        float x, y, z;
    };

    float v[3];

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

    float& operator[](int ord) { return v[ord]; }
    const float& operator[](int ord) const { return v[ord]; }

    Vec2 xy() const { return Vec2(x, y); }
};

union Vec4 {
    struct
    {
        float x, y, z, w;
    };

    float v[4];

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

    float& operator[](int ord) { return v[ord]; }
    const float& operator[](int ord) const { return v[ord]; }

    Vec3 xyz() const { return Vec3(x, y, z); }
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
        float f = 1.0f / tanf(deg_to_rad(fov) * 0.5f);
        Mat4 proj;
        proj[0][0] = f / aspect;
        proj[1][1] = f;
        proj[2][2] = -(zfar + znear) / (zfar - znear);
        proj[2][3] = -1.0f;
        proj[3][3] = -(2.0f * zfar * znear) / (zfar - znear);
        return proj;
    }

    static Mat4 rotate_x(float theta)
    {
        float a = deg_to_rad(theta);
        float cosa = cosf(a);
        float sina = sinf(a);
        Mat4 m;
        m[0][0] = 1.0f;
        m[1][1] = cosa;
        m[2][1] = -sina;
        m[1][2] = sina;
        m[2][2] = cosa;
        m[3][3] = 1.0f;
        return m;
    }

    static Mat4 rotate_y(float theta)
    {
        float a = deg_to_rad(theta);
        float cosa = cosf(a);
        float sina = sinf(a);
        Mat4 m;
        m[0][0] = cosa;
        m[2][0] = sina;
        m[1][1] = 1.0f;
        m[0][2] = -sina;
        m[2][2] = cosa;
        m[3][3] = 1.0f;
        return m;
    }

    static Mat4 rotate_z(float theta)
    {
        float a = deg_to_rad(theta);
        float cosa = cosf(a);
        float sina = sinf(a);
        Mat4 m;
        m[0][0] = cosa;
        m[1][0] = -sina;
        m[0][1] = sina;
        m[1][1] = cosa;
        m[2][2] = 1.0f;
        m[3][3] = 1.0f;
        return m;
    }

    static Mat4 scale(float s)
    {
        Mat4 m;

        for (int i = 0; i < 3; ++i)
        {
            m[i][i] = s;
        }

        m[3][3] = 1.0f;

        return m;
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
            m[j][i] = dot(r, b[j]);
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

Vec2 operator-(const Vec2& a, const Vec2& b)
{
    return Vec2(a.x - b.x, a.y - b.y);
}

Vec3 transform(const Mat4& m, const Vec3& v)
{
    Vec4 r = m * Vec4(v, 1.0f);
    return r.xyz();
}

Vec3 rotate(const Mat4& m, const Vec3& v)
{
    Vec4 r = m * Vec4(v, 0.0f);
    return r.xyz();
}

// Given a directed line segment from a->b and a point c check
// which side of a->b c lies on:
//  -1 = right side
//   0 = colinear
//   1 = left side
int orient2d(const Vec2& a, const Vec2& b, const Vec2& c)
{
    Vec2 ab = b - a;
    Vec2 ac = c - a;
    float det = ab.x * ac.y - ac.x * ab.y;
    return (det > 0.0f) ? 1 : ((det < 0.0f) ? -1 : 0);
}

// Classify triangle by signed area after projection to XY plane; useful for culling:
//  -1 = clockwise winding / backface
//   0 = degenerate
//   1 = counter-clockwise winding / frontface
int classify(const Triangle& t)
{
    return orient2d(t.p[0].xy(), t.p[1].xy(), t.p[2].xy());
}

struct Edge
{
    Vec2 v[2];
    float a, b, c;

    Edge() = default;

    Edge(const Vec2& v0, const Vec2& v1)
    {
        v[0] = v0;
        v[1] = v1;

        a = v0.y - v1.y;
        b = v1.x - v0.x;
        c = v0.x * v1.y - v1.x * v0.y;
    }

    float operator()(const Vec2& p)
    {
        return a * p.x + b * p.y + c;
    }
};

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
            { B, A, D },
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
        if (m_keys[L' '].pressed)
        {
            anim = !anim;
        }
        else if (m_keys[L'Z'].pressed)
        {
            anim = false;

            if (m_keys[VK_LSHIFT].down)
            {
                model = Mat4::identity();
            }
            else
            {
                model = Mat4::rotate_y(180.0f);
            }
        }
        else if (m_keys[L'X'].pressed)
        {
            anim = false;

            if (m_keys[VK_LSHIFT].down)
            {
                model = Mat4::rotate_y(270.0f);
            }
            else
            {
                model = Mat4::rotate_y(90.0f);
            }
        }
        else if (m_keys[L'Y'].pressed)
        {
            anim = false;

            if (m_keys[VK_LSHIFT].down)
            {
                model = Mat4::rotate_z(270.0f);
            }
            else
            {
                model = Mat4::rotate_z(90.0f);
            }
        }

        if (anim)
        {
            time += delta;
            model = Mat4::rotate_y(time * 60.0f * 1.5f) * Mat4::rotate_z(time * 30.0f * 1.5f) * Mat4::rotate_x(-time * 45.0f * 1.5f);
        }

        if (m_keys[VK_F1].pressed)
        {
            wireframe = !wireframe;
        }

        draw_scene();
        return true;
    }

    void on_destroy() override {}

    void draw_scene()
    {
        clear_screen(0);

        Mat4 mvp = proj * view * model;
        uint8_t c = 1;

        for (const Mesh& m : meshes)
        {
            for (const Triangle& t : m.tris)
            {
                draw_triangle(mvp, t, c++);
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

        Vec2 screen_coords[3];

        for (int i = 0; i < 3; ++i)
        {
            screen_coords[i].x = ((ndc_coords[i].x + 1.0f) * screen_width / 2.0f);
            screen_coords[i].y = ((ndc_coords[i].y + 1.0f) * screen_height / 2.0f);
        }

        if (orient2d(screen_coords[0], screen_coords[1], screen_coords[2]) < 0)
        {
            if (wireframe)
            {
                draw_line(screen_coords[0].x, screen_coords[0].y, screen_coords[1].x, screen_coords[1].y, c);
                draw_line(screen_coords[1].x, screen_coords[1].y, screen_coords[2].x, screen_coords[2].y, c);
                draw_line(screen_coords[2].x, screen_coords[2].y, screen_coords[0].x, screen_coords[0].y, c);
            }
            else
            {
                fill_triangle(screen_coords, c);
            }
        }
    }

    void fill_triangle(Vec2* screen_coords, uint8_t c)
    {
        Edge e01(screen_coords[0], screen_coords[1]);
        Edge e12(screen_coords[1], screen_coords[2]);
        Edge e20(screen_coords[2], screen_coords[0]);

        int bounds_min_x = INT_MAX;
        int bounds_min_y = INT_MAX;
        int bounds_max_x = INT_MIN;
        int bounds_max_y = INT_MIN;

        for (int i = 0; i < 3; ++i)
        {
            bounds_min_x = floorf(screen_coords[i].x) < bounds_min_x ? floorf(screen_coords[i].x) : bounds_min_x;
            bounds_min_y = floorf(screen_coords[i].y) < bounds_min_y ? floorf(screen_coords[i].y) : bounds_min_y;
            bounds_max_x = ceilf(screen_coords[i].x) > bounds_max_x ? ceilf(screen_coords[i].x) : bounds_max_x;
            bounds_max_y = ceilf(screen_coords[i].y) > bounds_max_y ? ceilf(screen_coords[i].y) : bounds_max_y;
        }

        bounds_min_x = bounds_min_x > 0 ? bounds_min_x : 0;
        bounds_min_y = bounds_min_y > 0 ? bounds_min_y : 0;
        bounds_max_x = bounds_max_x < screen_width ? bounds_max_x : screen_width;
        bounds_max_y = bounds_max_y < screen_height ? bounds_max_y : screen_height;

        for (int y = bounds_min_y; y < bounds_max_y; ++y)
        {
            for (int x = bounds_min_x; x < bounds_max_x; ++x)
            {
                Vec2 p(x, y);

                if (e01(p) <= 0.0f && e12(p) <= 0.0f && e20(p) <= 0.0f)
                {
                    set_pixel(x, y, c);
                }
            }
        }
    }

    std::vector<Mesh> meshes;
    Mat4 model;
    Mat4 view;
    Mat4 proj;
    float time = 0.0f;
    bool anim = true;
    bool wireframe = false;
};

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    TestVgfw test_app;

    if (!test_app.initialize(L"Vgfw 3D Renderer", 1280, 720, 1))
    {
        exit(EXIT_FAILURE);
    }

    test_app.run();

    return EXIT_SUCCESS;
}
