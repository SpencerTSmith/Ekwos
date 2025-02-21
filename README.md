# Description
Exploration of game engine design, focusing primarily on simple and no/minimal dependency C17. Graphics API is Vulkan. This is not going to be a general-purpose engine... I have a game in mind I wish to create, and am only implementing features in pursuit of that game.
## Build and Run
Ensure Vulkan support, and has GLFW (only dependency) as a system library.
```bash
cmake -B build && cmake --build build
```
Probably only working on Linux, haven't tested on any systems besides my desktop and laptop both running Linux. As well, none of my custom assets are currently packaged with the repo. Running as it is will result in a default cube mesh being loaded for all entities.
```bash
cd build
./ekwos -w [window-width] -h [window-height] -f [max fps]
```
Need to run the game from the build folder, as the compiled shaders need to be in the same folder as the executable.

## Short Term TODOs
- [x] CPU->GPU Uploader
    - [x] Basics
    - [ ] More sophisticated synchronization
        - [ ] Semaphore?
        - [ ] Fence?
- [x] GPU Memory Allocator
    - [x] Basics
    - [ ] Frame Bump Allocator
    - [ ] Nicer interface
- [x] Asset System
    - [x] Basics
        - [x] Meshes
            - [x] Custom obj loader
                - [ ] Only load unique vertices
            - [ ] Look into writing gltf loader or using this [library](https://github.com/jkuhlmann/cgltf/tree/master)
                - [ ] STB-like, wouldn't mind using it
        - [ ] Textures
            - [ ] STB-image for now
            - [ ] glb files can pack an entire model, textures and all into a single file, see [library](https://github.com/jkuhlmann/cgltf/tree/master)
    - [x] Reference Counting
        - [x] Basics
        - [ ] Hashing to check if already loaded
- [x] FPS Limiter
    - [x] Basics
    - [ ] Accuracy, for some reason not completely accurate (when target set to 60fps, get 62.5 why?)
    - [ ] Add CLI option to set this
- [ ] Pull camera position updating out of input processing
    - [ ] In future, this will make it so we can use the same logic for updating the player as every other entity

## Long Term TODOs
- [ ] Audio
- [ ] Transition off Cmake
    - [ ] Not planning on having a ton of dependencies, can make this simpler?

## Fully Complete
- [x] Vulkan Context Setup
    - [x] Basic 3D rendering
    - [x] Simple pipeline initialization
    - [x] Automated shader recompliation integrated with build system
- [x] Custom Allocators
    - [x] Bump/Arena
    - [x] Pool
- [x] Linear Algebra Library
    - [x] Custom Tailored for Vulkan NDC
    - [x] Consistent right handed system with z growing negative away from viewer
    - [x] Efficient autovectorization (Checked with GodBolt)
- [x] Camera
    - [x] Mouse Look
- [x] Custom Logging
    - [x] Vulkan Validations Layers
- [x] Thread Context
    - [x] Thread local scratch bump allocator
- [x] Arg Parsing
    - [x] Change render resolution, fps max from CLI
