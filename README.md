# Description
Exploration of game engine design, focusing primarily on simple and no/minimal dependency C17. Graphics API is Vulkan. This is not going to be a general-purpose engine... I have a game in mind I wish to create, and am only implementing features in pursuit of that game.
## Build and Run
Ensure Vulkan support, and has GLFW (only dependency) as a system library.
```bash
./build.sh
```
Probably only working on Linux, haven't tested on any systems besides my desktop and laptop both running Linux. As well, none of my custom assets are currently packaged with the repo. Running as it is will result in a default cube mesh being loaded for all entities.
```bash
cd bin
./ekwos -w [window-width] -h [window-height] -f [max fps]
```
Need to run the game from the build folder, as the compiled shaders need to be in the same folder as the executable.

One liner for building and running
```bash
./build.sh && (cd bin && ./ekwos)
```

## Demo
This will probably not be updated frequently, please check the later "Fully Complete" or build and run yourself to see a full demo
[![Demo](https://img.youtube.com/vi/aZmN974jDbM/maxresdefault.jpg)](https://www.youtube.com/watch?v=aZmN974jDbM)

## Short Term TODOs
- [x] Entity Pool
    - [ ] More elegant solution for checking if an entity is invalid
        - [ ] Pool free list is stored directly in buffer, meaning that the first 64 bits of the freed pool slot contains a pointer to the next free node... can't store any checks for if entity is invalid there
- [x] CPU->GPU Uploader
    - [x] Basics
    - [ ] More sophisticated synchronization
        - [ ] Semaphore?
        - [ ] Fence?
    - [ ] Move this to its own thread
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
    - [ ] Accuracy
        - [ ] Target frame time stored as f64 milliseconds (since we want 16.66.. ms for 60fps), but calculating elapsed time as u64 milliseconds
    - [ ] Add CLI option to set this
- [ ] Pull camera position updating out of input processing
    - [ ] In future, this will make it so we can use the same logic for updating the player as every other entity

## Long Term TODOs
- [ ] Audio
- [ ] Look into nice Vulkan 1.3 features
    - [ ] Dynamic rendering
    - [ ] Timeline semaphore

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
