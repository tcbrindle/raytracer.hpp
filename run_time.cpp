
#include "raytracer.hpp"

#include <memory>
#include <vector>

#include "stb_image_write.h"

using namespace rt;

namespace {

struct dynamic_scene {
    dynamic_scene()
            : cam_{vec3{ 3.0_r, 2.0_r, 4.0_r }, vec3{ -1.0_r, 0.5_r, 0.0_r }}
    {
        things_.push_back(plane{vec3{ 0.0_r, 1.0_r, 0.0_r }, 0.0_r, surfaces::checkerboard});
        things_.push_back(sphere{vec3{ 0.0_r, 1.0_r, -0.25_r }, 1.0_r, surfaces::shiny});
        things_.push_back(sphere{vec3{ -1.0_r, 0.5_r, 1.5_r }, 0.5_r, surfaces::shiny});

        lights_.push_back(light{ {-2.0_r, 2.5_r, 0.0_r}, {0.49_r, 0.07_r, 0.07_r}});
        lights_.push_back(light{ {1.5_r, 2.5_r, 1.5_r}, {0.07_r, 0.07_r, 0.49_r} });
        lights_.push_back(light{ {1.5_r, 2.5_r, -1.5_r}, {0.07_r, 0.49_r, 0.071_r} });
        lights_.push_back(light{ {0.0_r, 3.5_r, 0.0_r}, {0.21_r, 0.21_r, 0.35_r} });
    }

    const auto& get_things() const { return things_; }

    const auto& get_lights() const { return lights_; }

    const auto& get_camera() const { return cam_; }

private:
    std::vector<any_thing> things_;
    std::vector<light> lights_;
    camera cam_;
};

struct dynamic_canvas {

    int width;
    int height;
    static constexpr int bpp = 4;

    dynamic_canvas(int width, int height)
            : width{width},
              height{height},
              pixels_(width * height)
    {}

    void set_pixel(int x, int y, color col)
    {
        pixels_[x + width * y] = rgba::from_color(col);
    }

    const auto& get_pixels() const { return pixels_; }

private:
    struct rgba {
        static rgba from_color(const color& col) {
            constexpr auto clamp = [] (real_t val) { return std::clamp<real_t>(val, 0.0, 1.0); };
            return {uint8_t(std::floor(clamp(col.r) * 255.0)),
                    uint8_t(std::floor(clamp(col.g) * 255.0)),
                    uint8_t(std::floor(clamp(col.b) * 255.0)),
                    255 };
        }

        uint8_t r, g, b, a;
    };

    std::vector<rgba> pixels_;
};

}

int main(int argc, char** argv)
{
    int width = 512;
    int height = 512;

    if (argc > 2) {
        width = atoi(argv[1]);
        height = atoi(argv[2]);
    }

    const auto image = [&] {
        ray_tracer r{};
        dynamic_canvas canvas{width, height};
        r.render(dynamic_scene{}, canvas, width, height);
        return canvas;
    }();
    stbi_write_png("render-rt.png", image.width, image.height, 4,
                   image.get_pixels().data(), image.width * image.bpp);
}