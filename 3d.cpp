#include "vgfw.h"

#include <cmath>
#include <vector>

//===================================================================================================================================================
//
// Angles and trigonometry
//
//===================================================================================================================================================
const float pi = atanf(1.0f) * 4.0f;

float deg_to_rad(float deg)
{
    return deg * pi / 180.0f;
}

//===================================================================================================================================================
//
// 2d vector
//
//===================================================================================================================================================
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

Vec2 operator-(const Vec2& a, const Vec2& b)
{
    return Vec2(a.x - b.x, a.y - b.y);
}

//===================================================================================================================================================
//
// 3d vector
//
//===================================================================================================================================================
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

//===================================================================================================================================================
//
// 4d vector
//
//===================================================================================================================================================
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
    Vec2 xy() const { return Vec2(x, y); }
};

float dot(const Vec4& a, const Vec4& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

Vec4 operator*(const Vec4& v, float s)
{
    return Vec4(v.x * s, v.y * s, v.z * s, v.w * s);
}

Vec4 operator/(const Vec4& v, float s)
{
    float denom = 1.0f / s;
    return v * denom;
}

//===================================================================================================================================================
//
// 2x2 matrix
//
//===================================================================================================================================================
union Mat2 {
    Mat2()
        : X()
        , Y()
    {
    }

    struct
    {
        Vec2 X, Y;
    };

    Vec2 m[2];

    Vec2& operator[](int ord) { return m[ord]; }
    const Vec2& operator[](int ord) const { return m[ord]; }

    Vec2 row(int ord) const { return Vec2(X[ord], Y[ord]); }
};

float det(const Mat2& m)
{
    return m.X.x * m.Y.y - m.Y.x * m.X.y;
}

//===================================================================================================================================================
//
// 3x3 matrix
//
//===================================================================================================================================================
union Mat3 {
    Mat3()
        : X()
        , Y()
        , Z()
    {
    }

    struct
    {
        Vec3 X, Y, Z;
    };

    Vec3 m[3];

    Vec3& operator[](int ord) { return m[ord]; }
    const Vec3& operator[](int ord) const { return m[ord]; }

    Vec3 row(int ord) const { return Vec3(X[ord], Y[ord], Z[ord]); }
};

// The 2x2 submatrix formed by removing column i and row j from m
Mat2 submatrix(const Mat3& m, int i, int j)
{
    Mat2 s;
    int dest_col = 0;

    for (int src_col = 0; src_col < 3; ++src_col)
    {
        if (src_col == i)
        {
            continue;
        }

        int dest_row = 0;

        for (int src_row = 0; src_row < 3; ++src_row)
        {
            if (src_row == j)
            {
                continue;
            }

            s[dest_col][dest_row] = m[src_col][src_row];
            ++dest_row;
        }

        ++dest_col;
    }

    return s;
}

float det(const Mat3& m)
{
    float minors[3];

    for (int i = 0; i < 3; ++i)
    {
        minors[i] = det(submatrix(m, 0, i));
    }

    return m[0][0] * minors[0] - m[0][1] * minors[1] + m[0][2] * minors[2];
}

//===================================================================================================================================================
//
// 4x4 matrix
//
//===================================================================================================================================================
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

    /*
          f
        ------                 0                    0                    0
        aspect

          0                    f                    0                    0

                                              zfar + znear        2 * zfar * znear
          0                    0              ------------        ----------------
                                              znear - zfar          znear - zfar

          0                    0                   -1                    0
    */
    static Mat4 projection(float fov, float aspect, float znear, float zfar)
    {
        float f = 1.0f / tanf(deg_to_rad(fov) * 0.5f);
        Mat4 proj;
        proj[0][0] = f / aspect;
        proj[1][1] = f;
        proj[2][2] = (zfar + znear) / (znear - zfar);
        proj[2][3] = -1.0f;
        proj[3][2] = (2.0f * zfar * znear) / (znear - zfar);
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

Mat4 operator*(float s, const Mat4& a)
{
    Mat4 m;

    for (int i = 0; i < 4; ++i)
    {
        m[i] = a[i] * s;
    }

    return m;
}

// The 3x3 submatrix formed by removing column i and row j from m
Mat3 submatrix(const Mat4& m, int i, int j)
{
    Mat3 s;
    int dest_col = 0;

    for (int src_col = 0; src_col < 4; ++src_col)
    {
        if (src_col == i)
        {
            continue;
        }

        int dest_row = 0;

        for (int src_row = 0; src_row < 4; ++src_row)
        {
            if (src_row == j)
            {
                continue;
            }

            s[dest_col][dest_row] = m[src_col][src_row];
            ++dest_row;
        }

        ++dest_col;
    }

    return s;
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

float det(const Mat4& m)
{
    float minors[4];

    for (int i = 0; i < 4; ++i)
    {
        minors[i] = det(submatrix(m, 0, i));
    }

    return m[0][0] * minors[0] - m[0][1] * minors[1] + m[0][2] * minors[2] - m[0][3] * minors[3];
}

Mat4 transpose(const Mat4& m)
{
    Mat4 tm;

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            tm[i][j] = m[j][i];
        }
    }

    return tm;
}

Mat4 inverse(const Mat4& m)
{
    Mat4 mi;

    Mat4 minors;

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            minors[i][j] = det(submatrix(m, i, j));
        }
    }

    float det = m[0][0] * minors[0][0] - m[0][1] * minors[0][1] + m[0][2] * minors[0][2] - m[0][3] * minors[0][3];

    if (det == 0.0f)
    {
        return mi;
    }

    Mat4 adjoint;

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            float sign = ((i + j) & 1) ? -1.0f : 1.0f;
            adjoint[i][j] = sign * minors[i][j];
        }
    }

    Mat4 adjoint_t = transpose(adjoint);

    mi = (1.0f / det) * adjoint_t;

    return mi;
}

//===================================================================================================================================================
//
// Geometry
//
//===================================================================================================================================================
struct Triangle
{
    Vec3 p[3];
};

struct Mesh
{
    std::vector<Triangle> tris;
};

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

// Given an array of 3 screen space coordinates determine
// winding of the triangle
// -1 = clockwise
//  0 = degenerate
//  1 = counter-clockwise
int classify(const Vec4* abc)
{
    return orient2d(abc[0].xy(), abc[1].xy(), abc[2].xy());
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

    float operator()(const Vec2& p) { return a * p.x + b * p.y + c; }
};

//===================================================================================================================================================
//
// Application
//
//===================================================================================================================================================
class TestVgfw : public Vgfw
{
public:
    bool on_create() override
    {
        proj = Mat4::projection(90.0f, screen_width / (float)screen_height, 0.1f, 10.0f);

        viewport_transform.X.x = screen_width * 0.5f;
        viewport_transform.Y.y = -screen_height * 0.5f;
        viewport_transform.Z.z = 0.5f;
        viewport_transform.P.x = screen_width * 0.5f;
        viewport_transform.P.y = screen_height * 0.5f;
        viewport_transform.P.z = screen_height * 0.5f;

        model = Mat4::identity();
        camera = Mat4::identity();
        camera.P.z = 5.0f;

        // clang-format off
#if 0
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
#else
        Vec3 A( 0.f,  sqrtf(0.5f), 0.f);
        Vec3 B(-1.f, -sqrtf(0.5f), 0.f);
        Vec3 C( 1.f, -sqrtf(0.5f), 0.f);

        Mesh triangle = { { { A, B, C } } };
        meshes.push_back(triangle);
#endif
        // clang-format on

        return true;
    }

    bool on_update(float delta) override
    {
        if (m_keys[VK_F1].pressed)
        {
            wireframe = !wireframe;
        }

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

            if (time > 360.0f)
            {
                time -= 360.0f;
            }

            model = Mat4::rotate_y(time * 60.0f * 1.5f) * Mat4::rotate_z(time * 30.0f * 1.5f) * Mat4::rotate_x(-time * 45.0f * 1.5f);
        }

        view = inverse(camera);

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

        Vec4 window_coords[3];

        for (int i = 0; i < 3; ++i)
        {
            window_coords[i] = viewport_transform * ndc_coords[i];
            window_coords[i].w = clip_coords[i].w;
        }

        if (classify(window_coords) < 0)
        {
            if (wireframe)
            {
                draw_line(window_coords[0].x, window_coords[0].y, window_coords[1].x, window_coords[1].y, c);
                draw_line(window_coords[1].x, window_coords[1].y, window_coords[2].x, window_coords[2].y, c);
                draw_line(window_coords[2].x, window_coords[2].y, window_coords[0].x, window_coords[0].y, c);
            }
            else
            {
                fill_triangle(window_coords, c);
            }
        }
    }

    void fill_triangle(Vec4* screen_coords, uint8_t c)
    {
        Edge e01(screen_coords[0].xy(), screen_coords[1].xy());
        Edge e12(screen_coords[1].xy(), screen_coords[2].xy());
        Edge e20(screen_coords[2].xy(), screen_coords[0].xy());

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

        float area_scale = static_cast<float>(classify(screen_coords));

        for (int y = bounds_min_y; y < bounds_max_y; ++y)
        {
            for (int x = bounds_min_x; x < bounds_max_x; ++x)
            {
                Vec2 p(x, y);

                if (e01(p) * area_scale >= 0.0f && e12(p) * area_scale >= 0.0f && e20(p) * area_scale >= 0.0f)
                {
                    set_pixel(x, y, c);
                }
            }
        }
    }

    std::vector<Mesh> meshes;
    Mat4 camera;
    Mat4 model;
    Mat4 view;
    Mat4 proj;
    Mat4 viewport_transform;
    float time = 0.0f;
    bool anim = false;
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
