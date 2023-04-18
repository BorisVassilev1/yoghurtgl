# YoghurtGL

A C++ rendering library and an almost-game-engine, built with OpenGL. Supports PBR shading and uses the ECS model.

## Examples

In the **examples** directory there are a couple of examples of how the library is used in C++. Note that the resource files such as textures and 3D models are not in the github repo currently.

## Building

### - Requirements

The project is built with **cmake** 3.2 (all dependencies are included as submodules).
I use **make** as a build system, but any other should also work. 

The library does not use resources other than those in github, but the examples require models and textures that are uploaded in this [google drive](https://drive.google.com/drive/folders/1NYudbCy1uhkO4FWUEhxi7uQqQz1VXAd2?usp=share_link)

### - Build process

The simplest way to build the library **make** is to run:
```
./configure.bat # for Windows or ./configure.sh for Unix-like systems
cmake --build ./build --target=<target>
```
Target can be YoghurtGL or the name of any example in the **examples** directory.

Any CMake configuration should work. Here is a list of things that work/don't work
 - on Windows MinGW-gcc cannot compile assimp and thus cannot build the project
 - on Windows MinGW-clang works
 - on Linux gcc and clang work

## Images
Some renders to demonstrate the library's capabilities

### Dragon Model
Credit to **ricocilliers** on sketchfab for the model

<a href="https://drive.google.com/uc?export=view&id=1gTfoDRnjRbrKdWHwY5qHUAdUvYiD2Z5A"><img src="https://drive.google.com/uc?export=view&id=1gTfoDRnjRbrKdWHwY5qHUAdUvYiD2Z5A" style="width: 800px; max-width: 100%; height: auto" title="Click to enlarge picture" /></a>


### Path tracer
Image Produced in about 3 minutes (418 samples/pixel)

<a href="https://drive.google.com/uc?export=view&id=1ADTU-RrXpkhYPIFIpeL6FyOn0rVvInly"><img src="https://drive.google.com/uc?export=view&id=1ADTU-RrXpkhYPIFIpeL6FyOn0rVvInly" style="width: 800px; max-width: 100%; height: auto" title="Click to enlarge picture" /></a>

### Grass System
Instanced bezier-curve grass renderer like in Ghost of Tsushuma

<a href="https://drive.google.com/uc?export=view&id=1bpw34GJFpkKzEFdXmH5SXQhMn_ghzdkT"><img src="https://drive.google.com/uc?export=view&id=1bpw34GJFpkKzEFdXmH5SXQhMn_ghzdkT" style="width: 800px; max-width: 100%; height: auto" title="Click to enlarge picture" /></a>

