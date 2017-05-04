
# C++17 `constexpr` Compile-time Ray Tracer #


![50% centre](./output512x512.png)

## Introduction ##

This is a C++17 ray tracer using `constexpr` function evaluation to
produce the above image at either compile time or at run time, from the same code.
It is based on [Microsoft's TypeScript ray tracer example](https://github.com/Microsoft/TypeScriptSamples/tree/master/raytracer),
translated almost line-for-line into C++.

## Requirements ##

This code requires a recent compiler with good support for the upcoming
C++17 standard. It works with Clang 4.0 and (sort of) with GCC 7.1. MSVC is
currently untested.

---

**WARNING**

While compile-time image generation works with GCC 7, compiler memory usage is **extreme** 
-- tens of gigabytes for even relatively small image sizes.
If you want to try it out with GCC, stick to very small images or prepare
for violent retribution from the OOM-killer.

(I believe this is because GCC [memoizes the result](https://gcc.gnu.org/ml/gcc-patches/2009-11/msg01504.html)
of every single intermediate `constexpr` function call, rather than
because of any sort of memory leak.)

---

With Clang, you'll need to use the `-fconstexpr-steps` parameter to increase
the maximum permitted number of constexpr evaluations in order to generate
compile-time images. The included CMake project sets this to the maximum allowed
value (INT32_MAX), which is sufficient to generate an 800x800 pixel image but
not much larger.

## Files ##

**raytracer.hpp** is the bit which contains all the magic. As mentioned above,
the implementation is that from Microsoft's TypeScript examples set, translated
almost exactly into C++.

**stb_image_write.h** is one of Sean Barratt's excellent [single-header C
libraries](https://github.com/nothings/stb). It's used for writing out PNGs in
`compile_time.cpp` and `run_time.cpp`.

**stb_image_write.c** is the implementation file for the above.

**compile_time.cpp** contains a static description of a scene, which is then
rendered into a `constexpr` `std::array`. The image size can be changed using the
`IMAGE_WIDTH` and `IMAGE_HEIGHT` compiler defines. For larger image sizes, this
 will take a *long* time to compile. The upper image size is limited by (a) the
 amount of RAM on your system with GCC, or (b) the constexpr step limit with Clang,
 or (c) your patience. Outputs a file called `render-ct.png`.
 
 **run_time.cpp** is almost identical to the above, except that the scene data is
contained in run-time data structure,
and the image is rendered into a `std::vector`. Rather than compile-time parameters,
you can change the image size by providing command-line arguments to the generated
program, e.g. `renderer-rt 1024 1024` for a 1024x1024 image. Outputs a file called
`render-rt.png`.

**CMakeLists.txt** contains a CMake project which builds the two
targets listed above, as well as taking care of setting things like compiler flags
for you.

## Performance ##

Generating the above 512x512 image at compile time took around 45 minutes with
Clang 4.0 on my Macbook Pro. For comparison, the same code executing at run time
takes less than half a second on the same machine, or somewhere in the region 
of 6000x faster.

Believe it or not, this is actually decent performance compared to
compile-time raytracers which use template metaprogramming.
