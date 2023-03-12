# YoghurtGL

A C++ rendering library and an almost-game-engine, built with OpenGL. Supports PBR shading and uses the ECS model.

## Examples

In the **examples** directory there are a couple of examples of how the library is used in C++. Note that the resource files such as textures and 3D models are not in the github repo currently.

## Building

### - Requirements

The project is built with **cmake** 3.2 and requires [**Assimp**](https://github.com/assimp/assimp) to be installed. (all dependencies except Assimp are included as submodules).
I use **make** as a build system, but any other should also work. 

### - Build process

The simplest way to build the library if you have **g++** and **make** is to run:
```
./configure.bat # for Windows or ./configure.sh for Unix-like systems
cd build/
make <target>
```
Target can be YoghurtGL or the name of any example in the **examples** directory.

## Images
Some renders to demonstrate the library's capabilities

### Dragon Model
Credit to **ricocilliers** on sketchfab for the model

<a href="https://drive.google.com/uc?export=view&id=1gTfoDRnjRbrKdWHwY5qHUAdUvYiD2Z5A"><img src="https://drive.google.com/uc?export=view&id=1gTfoDRnjRbrKdWHwY5qHUAdUvYiD2Z5A" style="width: 800px; max-width: 100%; height: auto" title="Click to enlarge picture" /></a>


### Path tracer
Image produced in about 30-40 seconds on a GTX 1060

<a href="https://drive.google.com/uc?export=view&id=1Nxj9I452-T5ziNONsY4iTGvq9hLVFz7l"><img src="https://drive.google.com/uc?export=view&id=1Nxj9I452-T5ziNONsY4iTGvq9hLVFz7l" style="width: 800px; max-width: 100%; height: auto" title="Click to enlarge picture" /></a>

### Grass System
Instanced bezier-curve grass renderer like in Ghost of Tsushuma

<a href="https://drive.google.com/uc?export=view&id=1bpw34GJFpkKzEFdXmH5SXQhMn_ghzdkT"><img src="https://drive.google.com/uc?export=view&id=1bpw34GJFpkKzEFdXmH5SXQhMn_ghzdkT" style="width: 800px; max-width: 100%; height: auto" title="Click to enlarge picture" /></a>

