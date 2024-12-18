# RasterTek-DX11 Tutorials (Unified)

These tutorials are built upon the original [**RasterTek DX11 Tutorials**](https://www.rastertek.com/tutdx11win10.html) and [**DX11-Examples**](https://github.com/matt77hias/RasterTek). Key enhancements include:

- Creating a **CMake configuration** to generate Visual Studio solutions for **VS2022**, **VS2019**, and other versions.
- Combining all tutorials into a **single program**, selectable through **command-line arguments**.  

---

## How to Build and Run

Follow these steps to build and execute the program:

1. Open the **Visual Studio Developer Command Prompt** to set all Visual Studio paths correctly.  
2. Run the following command to configure the project:  
   ```bash
   cmake . -B build
3. Open Visual Studio 2022 and load the generated project.
4. Set the active project to RasterTek, then build and run the program.
Note: Solution files will be generated in the build directory.

---
## Learnings / Best Known Methods (BKMs)
Discovered DirectX App Templates: [**DirectX-VS-Templates**](https://github.com/walbourn/directx-vs-templates).