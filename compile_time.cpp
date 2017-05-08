
#include "raytracer.hpp"

#include <array>

#include "stb_image_write.h"

#ifndef IMAGE_WIDTH
#define IMAGE_WIDTH 32
#endif

#ifndef IMAGE_HEIGHT
#define IMAGE_HEIGHT 32
#endif

using namespace rt;

namespace {

// Constexpr std::array, since MSVC doesn't have this yet
template <typename T, size_t N>
struct carray {

    constexpr T* begin() { return arr_; }
    constexpr T* end() { return arr_ + N; }
    constexpr const T* begin() const { return arr_; }
    constexpr const T* end() const { return arr_ + N; }

    constexpr T& operator[](size_t i) { return arr_[i]; }
    constexpr const T& operator[](size_t i) const { return arr_[i]; }

    constexpr const T* data() const { return arr_; }

    T arr_[N];
};

struct static_scene {
private:
    carray<any_thing, 3> things_{{
            plane{{ 0.0_r, 1.0_r, 0.0_r }, 0.0_r, surfaces::checkerboard},
            sphere{{ 0.0_r, 1.0_r, -0.25_r }, 1.0_r, surfaces::shiny},
            sphere{{ -1.0_r, 0.5_r, 1.5_r }, 0.5_r, surfaces::shiny}
    }};
    carray<light, 4> lights_{{
            light{{-2.0_r, 2.5_r, 0.0_r}, {0.49_r, 0.07_r, 0.07_r}},
            light{{1.5_r, 2.5_r, 1.5_r}, {0.07_r, 0.07_r, 0.49_r}},
            light{{1.5_r, 2.5_r, -1.5_r}, {0.07_r, 0.49_r, 0.071_r}},
            light{{0.0_r, 3.5_r, 0.0_r}, {0.21_r, 0.21_r, 0.35_r}}
    }};
    camera cam_{vec3{ 3.0, 2.0, 4.0 }, vec3{ -1.0, 0.5, 0.0 }};

public:
    constexpr const auto& get_things() const { return things_; }

    constexpr const auto& get_lights() const { return lights_; }

    constexpr const auto& get_camera() const { return cam_; }
};

template <int Width, int Height>
struct static_canvas {

    static constexpr int width = Width;
    static constexpr int height = Height;
    static constexpr int bpp = 4;

    constexpr void set_pixel(int x, int y, color col)
    {
        pixels_[x + width * y] = rgba::from_color(col);
    }

    constexpr const auto& get_pixels() const { return pixels_; }

private:
    struct rgba {
        static constexpr auto clamp(real_t val) {
            return std::clamp<real_t>(val, 0.0, 1.0);
        }

        static constexpr rgba from_color(const color& col)
        {
            return {uint8_t(clamp(col.r) * 255.0),
                    uint8_t(clamp(col.g) * 255.0),
                    uint8_t(clamp(col.b) * 255.0),
                    255};
        }

        uint8_t r, g, b, a;
    };

    carray<rgba, Width * Height> pixels_;
};

constexpr auto get_image()
{
    ray_tracer r{};
    static_canvas<IMAGE_WIDTH, IMAGE_HEIGHT> c{};
    const static_scene s{};
    r.render(s, c, c.width, c.height);
    return c;
}

}

int main()
{
    constexpr auto image = get_image();
    stbi_write_png("render-ct.png", image.width, image.height, 4,
                   image.get_pixels().data(), image.width * image.bpp);
}