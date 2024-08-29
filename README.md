# DirectX11 Level Renderer

A custom 3D level renderer built using DirectX11, capable of important and rendering complex game levels created in Blender. This started as an assignment intended to enhance graphics API skills.
## Table of Contents

- [DirectX11 Level Renderer](#directx11-level-renderer)
- [Features](#features)
- [Technical Highlights](#technical-highlights)
- [Future Enhancements](#future-enhancements)
- [Building](#building)
- [Controls](#controls)
- [Credits and Acknowledgements](#credits-and-acknowledgements)

## Features

- Efficient file I/O for parsing exported level information and binary model data.
- Robust 3D fly-through camera system for seamless level exploration.
- Flexible rendering pipeline supporting multiple models with correct positioning, orientation, and scaling.
- Advanced lighting techniques, including directional lighting with ambient terms and specular highlights.
- Object-oriented renderer architecture, encapsulating loading and drawing code within a Model class.
- Hot-swapping system for loading multiple levels without application restart.
- Split-screen camera views and support for multiple preset cameras.
- Integrated audio features including background music and sound effects.

## Technical Highlights
- Proficient use of DirectX11 API and 3D graphics programming techniques.
- Implementation of 3D mathematics and transformation matrices for accurate rendering.
- Real-time rendering techniques and optimization for smooth performance.
- Integration with 3D modeling software (Blender) for seamless asset pipeline.
- Effective version control practices using Git.

## Future Enhancements
- Implement Skyboxes and Surface Reflections: Add support for skyboxes and surface reflections using cubemap textures.
- Support Additional 3D Model Formats: Extend compatibility to include formats like FBX and COLLADA.
- Enable Frustum Culling: Optimize rendering by implementing frustum culling.
- Add Shadows and Lighting Effects: Support shadows and advanced lighting effects, including point and spotlight sources.
- Use Post-Processing Effects: Implement post-processing screen effects like bloom and tone mapping.
- Upgrade to PBR Shaders: Enhance material realism by supporting physically-based rendering (PBR) shaders.
- Support Normal Mapping: Improve surface detail with normal mapping techniques.
- Implement Occlusion Culling: Optimize performance using hardware occlusion queries.
- Switch to Raytracing or Pathtracing: Upgrade the renderer from rasterization to raytracing or pathtracing for more realistic visuals.
- Support for Transparent Materials: Enable rendering of transparent materials in scenes.

## Building
To build the project, use the following CMake command in the `DirectX11` folder:
```bash
cmake ./S ./ .B ./build
```
## Controls

**General Controls:**

- **ESC**: Closes Application
- **C**: Switch Camera Presets
- **F1**: Open File Dialog Box  
  *(Select a "GameLevel" text file found inside the Level Renderer folder)*

**Keyboard Controls:**

- **W**: Move Forward
- **S**: Move Backward
- **A**: Move Left
- **D**: Move Right
- **Space**: Move Up
- **Left Shift**: Move Down

**Mouse Controls:**

- **Move Mouse Left/Right**: Rotate Camera Yaw
- **Move Mouse Up/Down**: Rotate Camera Pitch

**Controller Controls:**

- **Left Stick (Y Axis)**: Move Forward/Backward
- **Left Stick (X Axis)**: Move Left/Right
- **Right Stick (Y Axis)**: Rotate Camera Pitch
- **Right Stick (X Axis)**: Rotate Camera Yaw
- **Right Trigger**: Move Up
- **Left Trigger**: Move Down

For more detailed controls and customization options, please refer to [README.txt](DirectX11/README.txt).

## Credits and Acknowledgements

This project was developed with the help of resources provided by:

- **Lari Norri** - Instructor and Department Chair of Game Development at Full Sail University.
- **Full Sail University** - For providing foundational materials and guidance.
- **Gateware API** - Utilized in this project. Please refer to the accompanying license files for more information.

Special thanks to the developers of the Gateware API, which was instrumental in the development of this project.

For more details about the licensing of Gateware and other included software, see the `LICENSE.md` file.
