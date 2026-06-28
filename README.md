 # Mandelbrot Set Renderer (CPU)

A C++ implementation of the Mandelbrot Set featuring multithreaded rendering, interactive exploration, and smooth user controls. This project focuses on efficient CPU-based fractal generation and serves as the foundation for a future GPU/OpenGL implementation.

## Features

- Multithreaded Mandelbrot rendering
- Interactive zoom (mouse wheel and drag)
- Pan across the complex plane
- Reset zoom functionality
- Real-time rendering statistics
- Smooth continuous coloring
- Adjustable iteration count
- Responsive UI built with SFML

## Technologies Used

- C++
- SFML
- STL Threads
- Complex Numbers (`std::complex`)
- Modern C++17

## Controls

| Action | Control |
|--------|---------|
| Zoom In/Out | Mouse Wheel |
| Pan | Click and Drag |
| Reset View | Reset Button |

## Project Structure

```
application.cpp      Window management and UI
mandelbrot.cpp       Fractal generation
main.cpp             Entry point
```

## Performance

The renderer utilizes multiple CPU threads to compute the Mandelbrot set in parallel, significantly reducing render time compared to a single-threaded implementation.

## Future Improvements

- GPU acceleration using OpenGL shaders
- CUDA implementation
- Julia Set support
- Image export
- Custom color palettes
- Performance benchmarking

OpenGL-Integration Branch contains the GPU integration...
