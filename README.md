Performance CPU Raytracer

The long running explanation for why raytracing has not been done is because of
the raw processing power needed to compute it. This is true. Raytracing is an
expensive operation. Raytracing requires the computation of every single pixel
on the screen. This can be done concurrently, which is why it is so often done
on the GPU using specialized RTX graphics cards and the likes.

Benchmarking my computer:

Using intuition, we would think that a 2.4GHz computer would have about 2.4
billion cycles per core. The benchmark tells a slightly different story. Without
load, it appears like we get about 2.5 billion cycles per CPU thread. Meaning
about 20 billion cycles in general (with floating point operations). So what
does this mean?

2256x1504

Trying to render in native resolution at 60fps means we have about 98 cycles per
frame asssuming perfect utilization. Is this possible? We'd have to be very
careful with memory placement in order to maximize time in cache. Maybe then.
Maybe if a ray hits, assume that a ray nearby will hit? Easy edge detection?

Check for 4 pixels in a lattice. If all 4 pixels hit an object, assuem the pixel
in the middle also hits the object.

Maximum savings with this are roughly 2x. 
Minimum savings are ?
Predicted savings (assuming many continuous triangles are 1.25x)