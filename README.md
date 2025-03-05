# low-light-engine
Welcome to the *Low Light Game Engine*! This is a hobby project developed by [me](link.com). I post regular dev/study logs on my [blog](link.com), where I go into detail about implementing new features in the engine.  


> ⚠ **Stability Notice:**  
> This is an **experimental** hobby project. It is **not yet ready** for serious game development and is **very feature-incomplete**. Expect frequent changes and breaking updates. Due to licensing and other factors many files and folders have been excluded from this repo. Including (but not limited to) assets for the example programs, fmod library, old/deprecated features  

## Overview
I am building my engine on top of the [G3D Innovation Engine](https://casual-effects.com/g3d/www/index.html). G3D is a commercial-grade 3D rendering engine that I will be using for mainly for it's rendering capabilities. 

I will be building all of my systems around this rendering framework and plan to eventually change a lot about the renderer as well.


## Key Features
- **Custom Game Loop:** A flexible game loop (supporting both fixed and variable time steps).
- **Advanced Rendering:** Utilize G3D’s real-time rendering, deferred shading, and even ray tracing features.

*(More features coming soon!)*  


## Compilation Workarounds & Modifications
 While integrating G3D, I had to make a few modifications:

- **TBB (Threading Building Blocks) Compatibility:**  
  The bundled version of TBB does not fully meet modern C++ standards. As a workaround, I add the `-permissive` flag to ignore make the compile error a warning.

- **BoxIntersectionIterator Fix:**  
  An incorrect increment of the `this` pointer was causing an error.
  ```cpp
  BoxIntersectionIterator operator++(int) {
      BoxIntersectionIterator old = *this;
      ++(*this);  // Correctly increment the object, not the pointer
      return old;
  }
  ```

- **FMOD Exclusion:**  
  To avoid potential licensing issues with FMOD (which requires attribution or a separate commercial license), FMOD is excluded from this build. The build system defines the `G3D_NO_FMOD` macro.

---

## External Libraries & Licensing

This engine is built on the G3D Innovation Engine (licensed under the OSI-approved **2-Clause BSD** license) along with my own contributions (licensed under the **MIT License**). Note that several third-party libraries are included and are governed by their own licenses:
- **assimp** – BSD-style  
- **civetweb** – BSD-style  
- **zip** – BSD-style  
- **zlib** – BSD-style  
- **glfw** – BSD-style  
- **glew** – BSD-style  
- **tbb** – Apache open source  
- **embree** – Apache open source  
- **enet** – MIT open source  
- **nfd** – zlib open source  
- **FreeImage** – FreeImage Public License v1.0  
- **ffMPEG** – GNU LGPL v2.1  
- **fmod** – *Excluded from repository* (FMOD Sound System requires proper licensing and attribution)  
- **OptiX** – NVIDIA DesignWorks License


## Contributing

This project is primarily a personal learning endeavor, but I welcome feedback and contributions.  
Ways to contribute:
- Report bugs or issues via GitHub
- Suggest features or optimizations
- Submit pull requests with improvements or new documentation


## Acknowledgments

- **G3D Innovation Engine:**  
  Developed by Morgan McGuire, Michael Mara, and Zander Majercik.
  

## License

- **G3D Base:** Licensed under the 2-Clause BSD License.
- **Custom Engine Code:** Licensed under the MIT License.
- **Third-Party Components:** Use of additional libraries is subject to their respective licenses.
