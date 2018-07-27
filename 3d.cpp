#include "vgfw.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"

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

Vec2 operator*(const Vec2& a, float s)
{
    return Vec2(a.x * s, a.y * s);
}

Vec2 operator+(const Vec2& a, const Vec2& b)
{
    return Vec2(a.x + b.x, a.y + b.y);
}

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

// Hadamard product
Vec3 operator*(const Vec3& a, const Vec3& b)
{
    return Vec3(a.x * b.x, a.y * b.y, a.z * b.z);
}

Vec3 operator*(const Vec3& a, float s)
{
    return Vec3(a.x * s, a.y * s, a.z * s);
}

Vec3 operator/(const Vec3& a, float s)
{
    float denom = 1.0f / s;
    return a * denom;
}

Vec3 operator+(const Vec3& a, const Vec3& b)
{
    return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vec3 lerp(const Vec3& a, const Vec3& b, float t)
{
    return Vec3(a.x * (1.0f - t) + b.x * t, a.y * (1.0f - t) + b.y * t, a.z * (1.0f - t) + b.z * t);
}

float dot(const Vec3& a, const Vec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

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

    static Mat4 translate(const Vec3& t)
    {
        Mat4 m = identity();
        m.P = Vec4(t, 1.0f);
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

// Given an array of 3 screen space coordinates return twice the area
// of the triangle they form.
float triangle_area_2(const Vec4* abc)
{
    Vec2 ab = abc[1].xy() - abc[0].xy();
    Vec2 ac = abc[2].xy() - abc[0].xy();
    float det = ab.x * ac.y - ac.x * ab.y;
    return det;
}

// Given an array of 3 screen space coordinates determine
// winding of the triangle:
// -1 = clockwise
//  0 = degenerate
//  1 = counter-clockwise
int classify(const Vec4* abc)
{
    float area_2 = triangle_area_2(abc);
    return area_2 > 0.0f ? 1 : (area_2 < 0.0f ? -1 : 0);
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
// Graphical elements
//
//===================================================================================================================================================
struct Texture
{
    int width;
    int height;
    uint8_t* texels = nullptr;

    static std::unique_ptr<Texture> load(const char* filename)
    {
        int comp;
        std::unique_ptr<Texture> tex = std::make_unique<Texture>();
        tex->texels = stbi_load(filename, &tex->width, &tex->height, &comp, 3);

        if (!tex->texels)
        {
            tex->width = 1;
            tex->height = 1;
            tex->texels = static_cast<uint8_t*>(STBI_MALLOC(3));
            tex->texels[0] = 1.0f;
            tex->texels[1] = 0.0f;
            tex->texels[2] = 1.0f;
        }

        return tex;
    }

    ~Texture()
    {
        if (texels)
        {
            stbi_image_free(texels);
        }
    }

    Vec3 lookup(int x, int y)
    {
        float r = texels[((x + y * width) * 3) + 0] / 255.0f;
        float g = texels[((x + y * width) * 3) + 1] / 255.0f;
        float b = texels[((x + y * width) * 3) + 2] / 255.0f;
        return Vec3(r, g, b);
    }

    Vec3 sample(const Vec2& uv)
    {
        float w;
        Vec2 s;
        s.x = modff(uv.x, &w);
        s.y = modff(uv.y, &w);
        int x = static_cast<int>(s.x * (width - 1) + 0.5f) & (width - 1);
        int y = static_cast<int>(s.y * (height - 1) + 0.5f) & (height - 1);
        return lookup(x, y);
    }

    Vec3 sample_box(const Vec2& uv)
    {
        float w;
        Vec2 s;
        s.x = modff(uv.x, &w);
        s.y = modff(uv.y, &w);

        float tx = s.x * (width - 1);
        float ty = s.y * (height - 1);

        int min_x = static_cast<int>(floorf(tx)) & (width - 1);
        int min_y = static_cast<int>(floorf(ty)) & (width - 1);
        int max_x = static_cast<int>(ceilf(tx)) & (width - 1);
        int max_y = static_cast<int>(ceilf(ty)) & (width - 1);

        Vec3 samples[4];
        samples[0] = lookup(min_x, min_y);
        samples[1] = lookup(max_x, min_y);
        samples[2] = lookup(min_x, max_y);
        samples[3] = lookup(max_x, max_y);

        float wx = tx - floorf(tx);
        float wy = ty - floorf(ty);
        return lerp(lerp(samples[0], samples[1], wx), lerp(samples[2], samples[3], wx), wy);
    }
};

class TextureCatalog
{
public:
    Texture* get(const char* filename)
    {
        std::string canonical = filename;
        std::transform(canonical.begin(), canonical.end(), canonical.begin(), ::tolower);
        Collection::iterator it = m_textures.lower_bound(canonical);

        if (it == m_textures.end() || it->first != canonical)
        {
            it = m_textures.insert(it, std::move(std::make_pair(canonical, Texture::load(filename))));
        }

        return it->second.get();
    }

private:
    typedef std::map<std::string, std::unique_ptr<Texture>> Collection;
    Collection m_textures;
};

struct Vertex
{
    Vec3 p;
    Vec3 n;
    Vec2 uv;
};

struct Mesh
{
    std::vector<Vertex> vertex_buffer;
    std::vector<size_t> index_buffer;
};

class MeshCatalog
{
public:
    MeshCatalog()
    {
        Mesh cube;

        // clang-format off
        cube.vertex_buffer = {
            { { -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } }, // 0
            { {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
            { {  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
            { { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },

            { {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, // 4
            { {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
            { {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } },
            { {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },

            { {  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } }, // 8
            { { -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f } },
            { { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } },
            { {  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f } },

            { { -0.5f,  0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, // 12
            { { -0.5f,  0.5f,  0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
            { { -0.5f, -0.5f,  0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } },
            { { -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },

            { { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } }, // 16
            { {  0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
            { {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
            { { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },

            { { -0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } }, // 20
            { {  0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } },
            { {  0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } },
            { { -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } }
        };
        // clang-format on

        cube.index_buffer = { 0,  3,  1,  1,  3,  2,  4,  7,  5,  5,  7,  6,  8,  11, 9,  9,  11, 10,
                              12, 15, 13, 13, 15, 14, 16, 19, 17, 17, 19, 18, 20, 23, 21, 21, 23, 22 };

        m_meshes["_cube"] = std::move(cube);
    }

    Mesh* get(const char* filename)
    {
        std::string canonical = filename;
        std::transform(canonical.begin(), canonical.end(), canonical.begin(), ::tolower);
        Collection::iterator it = m_meshes.lower_bound(canonical);

        if (it == m_meshes.end() || it->first != canonical)
        {
            it = m_meshes.insert(it, std::move(std::make_pair(canonical, load(filename))));
        }

        return &it->second;
    }

private:
    Mesh load(const char* filename)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename);

        Mesh mesh;

        if (ret)
        {
            for (const auto& shape : shapes)
            {
                for (const auto& index : shape.mesh.indices)
                {
                    Vertex v;
                    v.p.x = attrib.vertices[3 * index.vertex_index + 0];
                    v.p.y = attrib.vertices[3 * index.vertex_index + 1];
                    v.p.z = attrib.vertices[3 * index.vertex_index + 2];
                    v.n.x = attrib.normals[3 * index.normal_index + 0];
                    v.n.y = attrib.normals[3 * index.normal_index + 1];
                    v.n.z = attrib.normals[3 * index.normal_index + 2];
                    v.uv.x = attrib.texcoords[2 * index.texcoord_index + 0];
                    v.uv.y = 1.0f - attrib.texcoords[2 * index.texcoord_index + 1];
                    mesh.index_buffer.push_back(mesh.vertex_buffer.size());
                    mesh.vertex_buffer.push_back(v);
                }
            }
        }

        return std::move(mesh);
    }

    typedef std::map<std::string, Mesh> Collection;
    Collection m_meshes;
};

struct MeshRef
{
    Mat4 transform;
    Mesh* mesh;
    Texture* texture;
    bool visible;
};

//===================================================================================================================================================
//
// Application
//
//===================================================================================================================================================
class TestVgfw : public Vgfw
{
public:
    uint8_t pack_color(float r, float g, float b)
    {
        int red_bits = (r == 1.0f) ? 7 : static_cast<int>(r * 8);
        int green_bits = (g == 1.0f) ? 7 : static_cast<int>(g * 8);
        int blue_bits = (b == 1.0f) ? 3 : static_cast<int>(b * 4);
        return (red_bits << 5) | (green_bits << 2) | blue_bits;
    }

    uint8_t pack_color(const Vec3& color) { return pack_color(color.x, color.y, color.z); }

    Vec3 unpack_color(uint8_t c)
    {
        int red_bits = (c >> 5) & 7;
        int green_bits = (c >> 2) & 7;
        int blue_bits = c & 3;
        return Vec3(red_bits / 7.0f, green_bits / 7.0f, blue_bits / 3.0f);
    }

    void bind_texture(Texture* tex) { texture = tex; }

    bool on_create() override
    {
        // 8-bit truecolor palette
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

        // Create depth buffer
        depth_buffer = std::make_unique<float[]>(screen_width * screen_height);

        // Initialize matrices
        proj = Mat4::projection(90.0f, screen_width / (float)screen_height, 0.1f, 10.0f);

        viewport_transform.X.x = screen_width * 0.5f;
        viewport_transform.Y.y = -screen_height * 0.5f;
        viewport_transform.Z.z = 0.5f;
        viewport_transform.P.x = screen_width * 0.5f;
        viewport_transform.P.y = screen_height * 0.5f;
        viewport_transform.P.z = 0.5f;

        camera = Mat4::identity();
        camera.P.z = 5.0f;

        // Create 3D scene
        MeshRef dragon;
        dragon.mesh = mesh_catalog.get("models/dragon/dragon_model.obj");
        dragon.texture = texture_catalog.get("models/dragon/DefaultMaterial_basecolor.png");
        dragon.transform = Mat4::identity();
        dragon.visible = true;
        scene.push_back(dragon);

        MeshRef cube;
        cube.mesh = mesh_catalog.get("_cube");
        cube.texture = texture_catalog.get("textures/checker_board.png");
        cube.transform = Mat4::identity();
        cube.visible = true;
        scene.push_back(cube);

        for (int i = 0; i < 8; ++i)
        {
            MeshRef rick;
            rick.mesh = mesh_catalog.get("_cube");
            rick.texture = texture_catalog.get("textures/rick.png");
            rick.visible = false;
            scene.push_back(rick);
        }

        return true;
    }

    void on_destroy() override {}

    bool on_update(float delta) override
    {
        if (m_keys[VK_F1].pressed)
        {
            wireframe = !wireframe;
        }

        if (m_keys[VK_F2].pressed)
        {
            filter_textures = !filter_textures;
        }

        if (m_keys[L' '].pressed)
        {
            anim = !anim;
        }

        if (m_keys[L'R'].pressed)
        {
            for (MeshRef& mesh_ref : scene)
            {
                mesh_ref.visible = !mesh_ref.visible;
            }
        }

        if (anim)
        {
            time += delta;

            if (time > 360.0f)
            {
                time -= 360.0f;
            }
        }

        scene[0].transform = Mat4::rotate_y(time * 60.0f * 1.5f) * Mat4::rotate_z(time * 30.0f * 1.5f) * Mat4::rotate_x(-time * 45.0f * 1.5f);
        scene[0].transform.P.x = 1.5f * sinf(time);
        scene[0].transform.P.z = 1.5f * cosf(time);

        scene[1].transform = inverse(scene[0].transform);

        for (size_t i = 2; i < scene.size(); ++i)
        {
            scene[i].transform = Mat4::rotate_x(cosf(time * 0.25f) * 360.0f) * Mat4::rotate_y(sinf(time * 0.25f) * 360.0f) * Mat4::rotate_y((i -2) * 45.0f) *
                                 Mat4::translate(Vec3(2.0f + sinf(time), 0.0f, 0.0f)) * Mat4::rotate_y(sinf(time) * sinf(time) * 360.0f) * Mat4::rotate_z(cosf(time) * cosf(time) * 360.0f);
        }

        view = inverse(camera);

        draw_scene();
        return true;
    }

    void draw_scene()
    {
        clear_screen(pack_color(0.5f, 0.5f, 0.5f));

        for (int i = 0; i < screen_width * screen_height; ++i)
        {
            depth_buffer[i] = 1.0f;
        }

        for (const MeshRef& mesh_ref : scene)
        {
            if (mesh_ref.visible)
            {
                Mat4 mvp = proj * view * mesh_ref.transform;
                bind_texture(mesh_ref.texture);

                for (size_t i = 0; i < mesh_ref.mesh->index_buffer.size(); i += 3)
                {
                    size_t i0 = mesh_ref.mesh->index_buffer[i];
                    size_t i1 = mesh_ref.mesh->index_buffer[i + 1];
                    size_t i2 = mesh_ref.mesh->index_buffer[i + 2];
                    draw_triangle(mvp, mesh_ref.transform, mesh_ref.mesh->vertex_buffer[i0], mesh_ref.mesh->vertex_buffer[i1],
                                  mesh_ref.mesh->vertex_buffer[i2]);
                }
            }
        }

        bind_texture(nullptr);
    }

    void draw_triangle(const Mat4& mvp, const Mat4& model, const Vertex& v0, const Vertex& v1, const Vertex& v2)
    {
        Vec4 clip_coords[3];

        clip_coords[0] = mvp * Vec4(v0.p, 1.0f);
        clip_coords[1] = mvp * Vec4(v1.p, 1.0f);
        clip_coords[2] = mvp * Vec4(v2.p, 1.0f);

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
                draw_line(window_coords[0].x, window_coords[0].y, window_coords[1].x, window_coords[1].y, pack_color(1.0f, 1.0f, 1.0f));
                draw_line(window_coords[1].x, window_coords[1].y, window_coords[2].x, window_coords[2].y, pack_color(1.0f, 1.0f, 1.0f));
                draw_line(window_coords[2].x, window_coords[2].y, window_coords[0].x, window_coords[0].y, pack_color(1.0f, 1.0f, 1.0f));
            }
            else
            {
                fill_triangle(window_coords, model, v0, v1, v2);
            }
        }
    }

    void fill_triangle(Vec4* screen_coords, const Mat4& model, const Vertex& v0, const Vertex& v1, const Vertex& v2)
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

        float ooz[3] = { 1.0f / screen_coords[0].w, 1.0f / screen_coords[1].w, 1.0f / screen_coords[2].w };
        float doz[3] = { screen_coords[0].z * ooz[0], screen_coords[1].z * ooz[1], screen_coords[2].z * ooz[2] };
        Vec3 noz[3] = { v0.n * ooz[0], v1.n * ooz[1], v2.n * ooz[2] };
        Vec2 uvoz[3] = { v0.uv * ooz[0], v1.uv * ooz[1], v2.uv * ooz[2] };

        float denom = 1.0f / triangle_area_2(screen_coords);

        for (int y = bounds_min_y; y < bounds_max_y; ++y)
        {
            for (int x = bounds_min_x; x < bounds_max_x; ++x)
            {
                Vec2 p(x, y);

                float w01 = e01(p);
                float w12 = e12(p);
                float w20 = e20(p);

                if (w01 * area_scale >= 0.0f && w12 * area_scale >= 0.0f && w20 * area_scale >= 0.0f)
                {
                    float w0 = w12 * denom;
                    float w1 = w20 * denom;
                    float w2 = w01 * denom;

                    float z = 1.0f / (ooz[0] * w0 + ooz[1] * w1 + ooz[2] * w2);
                    float d = (doz[0] * w0 + doz[1] * w1 + doz[2] * w2) * z;

                    if (d <= depth_buffer[x + (y * screen_width)])
                    {
                        depth_buffer[x + (y * screen_width)] = d;
                        Vec3 normal = rotate(model, (noz[0] * w0 + noz[1] * w1 + noz[2] * w2) * z);
                        float ndotl = dot(normal, Vec3(0.732f, 0.732f, 0.732f));
                        ndotl = ndotl < 0.0f ? 0.0f : (ndotl > 1.0f ? 1.0f : ndotl);
                        Vec3 light_color = Vec3(0.5f, 0.5f, 0.5f) * ndotl + Vec3(0.5f, 0.5f, 0.5f);
                        Vec3 texture_color(1.0f);

                        if (texture)
                        {
                            Vec2 uv = (uvoz[0] * w0 + uvoz[1] * w1 + uvoz[2] * w2) * z;
                            texture_color = filter_textures ? texture->sample_box(uv) : texture->sample(uv);
                        }

                        float alpha = 1.0f;
                        Vec3 existing_color = unpack_color(get_pixel(x, y));
                        Vec3 color = existing_color * (1.0f - alpha) + (texture_color * light_color) * alpha;
                        set_pixel(x, y, pack_color(color));
                    }
                }
            }
        }
    }

    TextureCatalog texture_catalog;
    MeshCatalog mesh_catalog;
    std::vector<MeshRef> scene;
    Mat4 camera;
    Mat4 view;
    Mat4 proj;
    Mat4 viewport_transform;
    float time = 0.0f;
    bool anim = true;
    bool wireframe = false;
    bool filter_textures = true;
    std::unique_ptr<float[]> depth_buffer;
    Texture* texture = nullptr;
};

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    TestVgfw test_app;

    if (!test_app.initialize(L"Vgfw 3D Renderer", 1024, 768, 1))
    {
        exit(EXIT_FAILURE);
    }

    test_app.run();

    return EXIT_SUCCESS;
}
