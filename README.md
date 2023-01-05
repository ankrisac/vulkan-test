# vulkan-test
Experiments in Vulkan

Currently working to build up abstractions (loosely based of wgpu-rs/WebGPU)

## Build Instructions

Requirements: 
- Any C++20 compliant compiler
- [Meson](https://mesonbuild.com/) (>=1.0.0)
- [Vulkan SDK](https://vulkan.lunarg.com/)

```bash
meson setup builddir

cd builddir 
meson compile
.\main.exe
```