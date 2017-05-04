
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

struct static_scene {
private:
    std::array<any_thing, 3> things_{{
            plane{{ 0.0, 1.0, 0.0 }, 0.0, surfaces::checkerboard},
            sphere{{ 0.0, 1.0, -0.25 }, 1.0, surfaces::shiny},
            sphere{{ -1.0, 0.5, 1.5 }, 0.5, surfaces::shiny}
    }};
    std::array<light, 4> lights_{{
            light{{-2.0, 2.5, 0.0}, {0.49, 0.07, 0.07}},
            light{{1.5, 2.5, 1.5}, {0.07, 0.07, 0.49}},
            light{{1.5, 2.5, -1.5}, {0.07, 0.49, 0.071}},
            light{{0.0, 3.5, 0.0}, {0.21, 0.21, 0.35}}
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
        static constexpr rgba from_color(const color& col)
        {
            constexpr auto clamp = [] (real_t val) { return std::clamp<real_t>(val, 0.0, 1.0); };
            return {uint8_t(clamp(col.r) * 255.0),
                    uint8_t(clamp(col.g) * 255.0),
                    uint8_t(clamp(col.b) * 255.0),
                    255};
        }

        uint8_t r, g, b, a;
    };

    std::array<rgba, Width * Height> pixels_;
};

}

int main()
{
    constexpr auto image = [] {
        ray_tracer r{};
        static_canvas<IMAGE_WIDTH, IMAGE_HEIGHT> c{};
        r.render(static_scene{}, c, c.width, c.height);
        return c;
    }();
    stbi_write_png("render-ct.png", image.width, image.height, 4,
                   image.get_pixels().data(), image.width * image.bpp);
}