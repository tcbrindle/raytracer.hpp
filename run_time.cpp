
#include "raytracer.hpp"

#include <memory>
#include <vector>

#include "stb_image_write.h"

using namespace rt;

namespace {

struct dynamic_scene {
    dynamic_scene()
            : cam_{vec3{ 3.0, 2.0, 4.0 }, vec3{ -1.0, 0.5, 0.0 }}
    {
        things_.push_back(plane{vec3{ 0.0, 1.0, 0.0 }, 0.0, surfaces::checkerboard});
        things_.push_back(sphere{vec3{ 0.0, 1.0, -0.25 }, 1.0, surfaces::shiny});
        things_.push_back(sphere{vec3{ -1.0, 0.5, 1.5 }, 0.5, surfaces::shiny});

        lights_.push_back(light{ {-2.0, 2.5, 0.0}, {0.49, 0.07, 0.07}});
        lights_.push_back(light{ {1.5, 2.5, 1.5}, {0.07, 0.07, 0.49} });
        lights_.push_back(light{ {1.5, 2.5, -1.5}, {0.07, 0.49, 0.071} });
        lights_.push_back(light{ {0.0, 3.5, 0.0}, {0.21, 0.21, 0.35} });
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