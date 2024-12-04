# General
- [ ] Improve build times / reconsider dependency strategy

# Graphics/Rendering

## General
- [ ] Simple Mesh renderer
- [ ] Advanced real-time lighting
- [ ] Would-be-nice: raytracing (preferrably hardware accelerated)

## Terra Shading Language (TSL)

- [ ] Do type checking at compile time
- [ ] Compile to proper Vulkan GLSL shader (end goal)

## UI Rendering

- [x] Get simple UI rendering using ImGui
- [ ] Refactor HTML-to-DOM parsing to use the TSL scanner

## Vulkan

- [ ] Move all definitions to Render::imp
- [ ] Move implementation definitions to vk_imgui files_
- [ ] Asynchronous mesh uploading
- [ ] Runtime compilation to SPIR-V
