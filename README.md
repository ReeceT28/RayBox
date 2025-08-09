# 🌟 RayBox

**RayBox** is a ray optics simulation tool that models various light phenomena, including **reflection**, **refraction**, and **Fresnel equations**. It’s designed for experimentation, visualization, and learning optics. RayBox was developed for the 2025 BPhO Computational Challenge however, I hope to keep on developing and improving it. 

---
## Features

-  Simulates **Snell's Law** of refraction
-  Models **Fresnel equations** for transmission ratios
-  Uses **Sellmeier equations** to model refractive index as a function of wavelength
-  Variety of components such as **lenses**, **mirrors**, **Customised prism shapes**, and more...

---

## Tools Used

- **Programming Language**:  C++
- **Graphics**: SFML
- **Hardware acceleration**: OpenCL and OpenCL-Wrapper by ProjectPhysX
- **GUI**: ImGui and ImGui-SFML
- **Miscellaneous**: Earcut for shape triangulation

---

## Assumptions/ source of errors
There are a few assumptions in this simulation that could cause inaccuracies, however, these may be changed in the future. 
- Light is always **unpolarized**
- Rays **do not** lose intensity as they travel through space.
- Light behaves as rays, which in this simulation are considered to be infinitely thin, and drawn as just lines (1 pixel thick).
- Uses **floating point arithmetic** for the most part so errors could accumulate from this.
- Circular entities, e.g. mirrors, are broken up into many **line segments not calculated exactly**, which can allow errors to accumulate. 

## Screenshots / Demo

This image is with a resolution of 8,000 rays produced when white light hits a prism, it was also taken with a very high line segment count for circular arcs to create a good image otherwise you often get these black gaps between rays when spread out light hits mirrors. When dragging components around in this scene with a preview count of 10 iterations, meaning for example:
1. White light hits the prism, and coloured rays are spawned
2. coloured rays collide with the prism, and new directions are worked out due to refraction, along with any reflected component modelled by Fresnel equations.
3. this process continues, each coloured ray detects where it collides, calculates refracted and reflected directions, and adds these new rays.

With these settings, I do not acquire 60 fps, it would be closer to around 30 fps on my hardware. 

<img src="Screenshots/screenshot_1754003253.jpg">

Correction of long and short-sightedness:

<img src="Screenshots/screenshot_1754012398.jpg">
<img src="Screenshots/screenshot_1754009917.jpg">

Formation of a secondary rainbow:

<img src="Screenshots/screenshot_1753233670.jpg">

Formation of a primary rainbow:

<img src="Screenshots/screenshot_1753233828.jpg">

## Ideas for future improvement:

- Currently, ray data is sent to a kernel, from this ray data we compute a few things for each ray: it's collision point, reflected and refracted direction and transmission coefficients. We read this data from the kernel, from this we use collision and origin points to add rays to a Vertex Array of lines which stores every single ray in the game, we also create new rays for the reflected and refracted components or if white light hits a prism create coloured rays. So if we have our max count of iterations set to 10, it has to write and read data to the kernel 10 times and process all the data each iteration, reading and writing can add a lot of delay and create inefficiencies, the program could be modified to simply handle all 10 iterations and then return all of the data about the rays in one batch however this would require a lot more information to be read from and written to the kernel.
