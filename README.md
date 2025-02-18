# Short Term TODOs
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

# Long Term TODOs
- [ ] Audio
- [ ] Transition off Cmake
    - [ ] Not planning on having a ton of dependencies, can make this simpler?
