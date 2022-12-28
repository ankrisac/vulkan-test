# vulkan-test
Experiments in Vulkan


## Build instructions

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