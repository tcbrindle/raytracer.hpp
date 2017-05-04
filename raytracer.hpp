
/*
 * Compile-time ray tracer example
 * Based on raytracer.ts from Microsoft TypeScript examples
 * https://github.com/Microsoft/TypeScriptSamples/tree/master/raytracer
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <variant>

namespace rt {

using real_t = float;

// Constexpr maths functions.
namespace cmath {

// libstdc++ provides some constexpr math functions as an extension, so
// use them if we can.
#ifdef __GLIBCXX__
#define HAVE_CONSTEXPR_STD_MATH
#endif

// Compile-time square root using Newton-Raphson, adapted from
// https://gist.github.com/alexshtf/eb5128b3e3e143187794
constexpr real_t sqrt(real_t val)
{
#ifdef HAVE_CONSTEXPR_STD_MATH
    return std::sqrt(val);
#else
    real_t curr = val;
    real_t prev = 0;

    while (curr != prev) {
        prev = curr;
        curr = 0.5 * (curr + val/curr);
    }

    return curr;
#endif
}

constexpr real_t floor(real_t val)
{
#ifdef HAVE_CONSTEXPR_STD_MATH
    return std::floor(val);
#else
    // This is wrong for anything outside the range of intmax_t
    return static_cast<intmax_t>(val >= 0.0 ? val : val - 1.0);
#endif
}

constexpr real_t pow(real_t base, int iexp)
{
#ifdef HAVE_CONSTEXPR_STD_MATH
    return std::pow(base, iexp);
#else
    while (iexp-- > 0) {
        base *= base;
    }
    return base;
#endif
}

} // end namespace cmath

struct vec3 {
    real_t x;
    real_t y;
    real_t z;
};

constexpr vec3 operator*(real_t k, const vec3& v)
{
    return {k * v.x, k * v.y, k * v.z};
}

constexpr vec3 operator-(const vec3& v1, const vec3& v2)
{
    return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

constexpr vec3 operator+(const vec3& v1, const vec3& v2)
{
    return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
}

constexpr real_t dot(const vec3& v1, const vec3& v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

constexpr real_t mag(const vec3& v)
{
    return cmath::sqrt(dot(v, v));
}

constexpr vec3 norm(const vec3& v)
{
    return (real_t{1.0} / mag(v)) * v;
}

constexpr vec3 cross(const vec3& v1, const vec3& v2)
{
    return { v1.y * v2.z - v1.z * v2.y,
             v1.z * v2.x - v1.x * v2.z,
             v1.x * v2.y - v1.y * v2.x };
}

struct color {
    real_t r;
    real_t g;
    real_t b;

    static constexpr color white() { return { 1.0, 1.0, 1.0 }; }
    static constexpr color grey() { return { 0.5, 0.5, 0.5 }; }
    static constexpr color black() { return {}; };
    static constexpr color background() { return black(); }
    static constexpr color default_color() { return black(); }
};

constexpr color scale(real_t k, const color& v)
{
    return { k * v.r, k * v.g, k * v.b };
}

constexpr color operator+(const color& v1, const color& v2)
{
    return { v1.r + v2.r, v1.g + v2.g, v1.b + v2.b };
}

constexpr color operator*(const color& v1, const color& v2)
{
    return {v1.r * v2.r, v1.g * v2.g, v1.b * v2.b};
}

struct camera {
    vec3 pos;
    vec3 forward;
    vec3 right;
    vec3 up;

    constexpr camera(const vec3& pos, const vec3& look_at)
            : pos{pos},
              forward{norm(look_at - pos)},
              right{1.5 * norm(cross(forward, {0.0, -1.0, 0.0}))},
              up{1.5 * norm(cross(forward, right))}
    {}
};

struct ray {
    vec3 start;
    vec3 dir;
};

struct light {
    vec3 pos;
    color col;
};

struct surface {
    using diffuse_func_t = color (*)(const vec3&);
    using specular_func_t = color (*)(const vec3&);
    using reflect_func_t = real_t (*)(const vec3&);

    diffuse_func_t diffuse = nullptr;
    specular_func_t specular = nullptr;
    reflect_func_t reflect = nullptr;
    int roughness = 0;
};

struct any_thing;

struct intersection {
    const any_thing* thing_;
    ray ray_;
    real_t dist;
};

struct sphere {
    vec3 centre;
    real_t radius2;
    surface surface_;

public:
    constexpr sphere(const vec3& centre, real_t radius, const surface& surface_)
            : centre{centre},
              radius2{radius * radius},
              surface_{surface_}
    {}

    constexpr
    std::optional<intersection> intersect(const any_thing* pself, const ray& ray_) const
    {
        const vec3 eo = centre - ray_.start;
        const auto v = dot(eo, ray_.dir);
        real_t dist = 0;

        if (v >= 0) {
            auto disc = radius2 - (dot(eo, eo) - v * v);
            if (disc >= 0) {
                dist = v - cmath::sqrt(disc);
            }
        }
        if (dist == 0.0) {
            return std::nullopt;
        } else {
            return intersection{pself, ray_, dist};
        }
    }

    constexpr vec3 get_normal(const vec3& pos) const
    {
        return norm(pos - centre);
    }

    constexpr const surface& get_surface() const
    {
        return surface_;
    }
};

struct plane {
    vec3 norm;
    real_t offset;
    surface surface_;

    constexpr
    std::optional<intersection> intersect(const any_thing* pself, const ray& ray_) const
    {
        const auto denom = dot(norm, ray_.dir);
        if (denom > 0) {
            return std::nullopt;
        } else {
            real_t dist = (dot(norm, ray_.start) + offset) / (-denom);
            return intersection{ pself, ray_, dist };
        }
    }

    constexpr vec3 get_normal(const vec3&) const
    {
        return norm;
    }

    constexpr const surface& get_surface() const
    {
        return surface_;
    }
};

struct any_thing {
private:
    // Workaround for no capturing constexpr lambdas in Clang 4.0
    struct intersect_visitor {
        const any_thing* pself;
        const ray& ray_;

        template <typename Thing>
        constexpr decltype(auto) operator()(const Thing& thing) const
        {
            return thing.intersect(pself, ray_);
        }
    };

    struct normal_visitor {
        const vec3& pos;

        template <typename Thing>
        constexpr decltype(auto) operator()(const Thing& thing) const
        {
            return thing.get_normal(pos);
        }
    };

public:
    template <typename T>
    constexpr any_thing(T&& t) : item_(std::forward<T>(t)) {}

    constexpr std::optional<intersection> intersect(const ray& ray_) const
    {
        return std::visit(intersect_visitor{this, ray_}, item_);
    }

    constexpr vec3 get_normal(const vec3& pos) const
    {
        return std::visit(normal_visitor{pos}, item_);
    }

    constexpr const surface& get_surface() const
    {
        return std::visit([](const auto& thing_) -> decltype(auto) {
            return thing_.get_surface();
        }, item_);
    }

private:
    std::variant<sphere, plane> item_;
};

namespace surfaces {

inline constexpr surface shiny{
        [](const vec3&) { return color::white(); },
        [](const vec3&) { return color::grey(); },
        [](const vec3&) { return real_t{0.7}; },
        250
};

inline constexpr surface checkerboard{
        [](const vec3& pos) {
            if (int(cmath::floor(pos.z) + cmath::floor(pos.x)) % 2 != 0) {
                return color::white();
            } else {
                return color::black();
            }
        },
        [](const vec3&) { return color::white(); },
        [](const vec3& pos) -> real_t {
            if (int(cmath::floor(pos.z) + cmath::floor(pos.x)) % 2 != 0) {
                return 0.1;
            } else {
                return 0.7;
            }
        },
        150
};

} // end namespace surfaces

class ray_tracer {
private:
    int max_depth = 5;

    template <typename Scene>
    constexpr std::optional<intersection> get_intersections(const ray& ray_, const Scene& scene_) const
    {
        auto closest = std::numeric_limits<real_t>::max();
        // Workaround lack of constexpr copy/move assignment and operator->()
        // in libstdc++ std::optional w/ GCC 7.1.
        intersection closest_inter{};

        for (const auto& t : scene_.get_things()) {
            auto inter = t.intersect(ray_);
            if (inter && (*inter).dist < closest) {
                closest = (*inter).dist;
                closest_inter = *inter;
            }
        }

        if (closest == std::numeric_limits<real_t>::max()) {
            return std::nullopt;
        }
        return closest_inter;
    }

    template <typename Scene>
    constexpr std::optional<real_t> test_ray(const ray& ray_, const Scene& scene_) const
    {
        if (const auto isect = get_intersections(ray_, scene_); isect) {
            return isect->dist;
        }
        return std::nullopt;
    }

    template <typename Scene>
    constexpr color trace_ray(const ray& ray_, const Scene& scene_, int depth) const
    {
        if (const auto isect = get_intersections(ray_, scene_); isect) {
            return shade(*isect, scene_, depth);
        }
        return color::background();
    }

    template <typename Scene>
    constexpr color shade(const intersection& isect, const Scene& scene, int depth) const
    {
        const vec3& d = isect.ray_.dir;
        const vec3 pos = (isect.dist * d) + isect.ray_.start;
        const vec3 normal = isect.thing_->get_normal(pos);
        const vec3 reflect_dir = d - (2 * (dot(normal, d) * normal));
        const color natural_color = color::background() + get_natural_color(*isect.thing_, pos, normal, reflect_dir, scene);
        const color reflected_color = depth >= max_depth ? color::grey() : get_reflection_color(*isect.thing_, pos, reflect_dir, scene, depth);
        return natural_color + reflected_color;
    }

    template <typename Scene>
    constexpr color get_reflection_color(const any_thing& thing_, const vec3& pos,
                                         const vec3& rd, const Scene& scene, int depth) const
    {
        return scale(thing_.get_surface().reflect(pos), trace_ray({pos, rd }, scene, depth + 1));
    }

    template <typename Scene>
    constexpr color add_light(const any_thing& thing, const vec3& pos, const vec3& normal,
                              const vec3& rd, const Scene& scene, const color& col,
                              const light& light_) const
    {
        const vec3 ldis = light_.pos - pos;
        const vec3 livec = norm(ldis);
        const auto near_isect = test_ray({pos, livec}, scene);
        const bool is_in_shadow = near_isect ? *near_isect < mag(ldis) : false;
        if (is_in_shadow) {
            return col;
        }
        const auto illum = dot(livec, normal);
        const auto lcolor = (illum > 0) ? scale(illum, light_.col) : color::default_color();
        const auto specular = dot(livec, norm(rd));
        const auto& surf = thing.get_surface();
        const auto scolor = (specular > 0) ? scale(cmath::pow(specular, surf.roughness), light_.col)
                                           : color::default_color();
        return col + (surf.diffuse(pos) * lcolor) + (surf.specular(pos) * scolor);
    }

    template <typename Scene>
    constexpr color get_natural_color(const any_thing& thing, const vec3& pos,
                                      const vec3& norm_, const vec3& rd, const Scene& scene) const
    {
        color col = color::default_color();
        for (const auto& light : scene.get_lights()) {
            col = add_light(thing, pos, norm_, rd, scene, col, light);
        }
        return col;
    }

    constexpr vec3 get_point(int width, int height, int x, int y, const camera& cam) const
    {
        const auto recenterX =  (x - (width / 2.0)) / 2.0 / width;
        const auto recenterY = -(y - (height / 2.0)) / 2.0 / height;
        return norm(cam.forward + ((recenterX * cam.right) + (recenterY * cam.up)));
    }

public:
    template <typename Scene, typename Canvas>
    constexpr void render(const Scene& scene, Canvas& canvas, int width, int height) const
    {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                const auto point = get_point(width, height, x, y, scene.get_camera());
                const auto color = trace_ray({ scene.get_camera().pos, point }, scene, 0);
                canvas.set_pixel(x, y, color);
            }
        }
    }
};

} // end namespace
