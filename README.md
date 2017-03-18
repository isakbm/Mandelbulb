
**Mandelbulb Explorer** 
=========================
--------------------------

**This project explores the use of ray-marching to render a 3D fractal, specifically one known as the Mandelbulb. It is a generalization of the standard 2D fractal, the Mandelbrot set.**

The equation that encodes the Mandelbrot set is perhaps familiar to you

` z ->  z*z + C `

where `z` and `c` are complex numbers. The input, `c`, is fixed and the equation is iterated starting from `z = 0`. The result is a sequence of complex numbers where the first one is `c`, the second one is `c + c*c`, and so on. If `c` is picked to represent a pixel on the screen, that is 

`c = x + iy`

then the transformation effectively displaces the original pixel coordinates ```(x,y)```, and eventually we might "escape". By escaping we mean that the distance to the origin has become greater than 2 (`|z| > 2`) then we bail.  

It can be shown the transformations will continue to increase the distance to the origin if (a proof is given at the end)

`|z| > 2`.

On the other hand, if the radius remains less than or equal to 2 after an infinite number of iterations, we say that the pixel corresponding to the initial value c is a member of the Mandelbrot set.

Since we are unable to iterate to infinity we are always seeing an approximation of the Mandelbrot set, but the convergence is quite fast and a good approximation is easily gained within the limits of machine precision.

Generalizing to Three Dimensions
===========================================
------------------------------------------

Let us now generalize to higher dimensions. Consider the position vector associated with c

`v = (x, y),`
`|v| = sqrt(x^2 + y^2)`

A step in the iteration (1) can be interpreted as adding v rotated and scaled to v itself.
Indeed, using the Euler formula makes this clear

`c = |v| exp(i φ)`

`c^2 = |v|^2 exp(2i φ)`

where `φ = atan(x,y)`

hence

`c^2 + c = |v| Rotate(v, φ) + v`

We see that squaring the complex number `c` is equivalent to doubling the length of `v` and rotating it by an angle `φ`. This viewpoint of the iterative procedure is begging for a naive generalization to higher dimensions by simply adding another angle.

We then pick a point 

`p = (x,y,z)`

and we let the iterative step be

`p = p + |p| Rotate(p, θ, φ)`

where

`φ = atan(x,y)`,
`θ = acos(z/|p|)`.

Generalizing to Higher Powers 
======================================
----------------------------------

We can generalize even more by considering

`z = z^pow + c`.

It is clear that if we set `pow = 2` we recover what we considered above, but why not consider other powers. It is simple to deduce what the effect is in the 2D case for complex numbers. The result is that we should take the length to the power of pow, and we should rotate by `φ`, (`pow-1`) number of times. This is easy to generalize.

The name _Mandebulb_ is usually associated with the fractal that you get if you have `pow ~ 6` to `8`.

Rendering 
=================================
---------------------------------

There is a tremendously powerful technique called _distance estimation_, which allows efficient rendering of the three dimensional fractals by use of regular _ray-marching techniques_. Essentially **John C. Hart**, **Daniel J. Sanding** and **Louis H. Kauffman** were the first to use this approach. Distance estimation says that for any position `p = (x,y,z)` you can compute a real number `d(p)`, which gives the radius of a sphere which centered at `p` contains a set of points (its volume) such that none of the points are in the fractal set. This number `d(p)` can then be used as the marching step parameter, and recomputed at every step until `d(p) < ε` at which point we say that we are close enough to the fractal and we render that point in the standard ray-tracing kind of way. A very useful resource on this technique can be found [**here**](http://blog.hvidtfeldts.net/index.php/2011/09/distance-estimated-3d-fractals-iv-the-holy-grail/)

We would like to shed some more light on how `d(p)` is computed...

`d(p) = 0.5 log(r) r / dr`

where 

`r = |f(n;c)|`

and

`dr = |f'(n;c)|`

For the Mandelbrot one has 

`f(0;c) = 0`
`f(n;c) = f(n-1;c)^2 + c`

this follows from `f(1; c) = c^2 + c`. We can compute the derivative and find

`f'(n;c) = 2 f(n-1;c)f'(n-1;c) + 1`

now it's a simple matter to compute this for a given number of iterations, and the result is that we get an estimate for a bounding volume within which the fractal is guaranteed not to be. Of course we have not motivated the expression

`d(p) = ... `

that you see above, but that can be found in the literature.


Proof of Bailout Condition for Mandelbrot
================================
--------------------------------

**Claim**:

`z = z^2 + c` 

diverges _if_ at **any** moment

`|z| > 2`.

**Proof**:

There are other ways to prove this, but I'll present a version that is ignorant to the fact that it might be 2 that is the _bailout_ value. 

Consider a step in the sequence

`..., z, z^2 + c, ...`

It is the ratio

`R = |z^2 + c| / |z|`

that decides convergence. If `R > 1` we should bail since the sequence won't converge, this statement implies that we should bail if

`|z^2 + c| > |z|`

as you can convince yourself by drawing circles and thinking of complex numbers as 2D vectors, it is clear that the minimum of the LHS is

`min(|z^2 + c|) = abs(|z|^2 - |c|)`

then it follows immediately that

`abs(|z|^2 - |c|) > |z|`

is the bailout condition. 

Let us consider the two cases that resolve the abs function

 `|z|^2 >= |c|`   and    `|z|^2 < |c|`.

We should only be concerned with the case

 `|z|^2 >= |c|`

due to the nature of divergence --> increase in |z|.
The bailout condition then reads

`|z|^2 - |c| > |z|`

solving the corresponding quadratic equation

`|z|^2 - |z| - |c| > 0`

gives that a local bailout condition (local meaning `|c|` dependent)

`|z| > 1/2 ( 1 + sqrt(1 + 4 |c|) ).`

For any |c| greater than this local bailout value we have guaranteed divergence since the first step of the Mandelbrot iteration sets `|z| = |c|`. The value of `|c|` for which `|c|` is equal to the local bailout value is then the maximum bailout value and hence a globally valid bailout value, it is given by solving

`|c| = 1/2 ( 1 + sqrt(1 + 4 |c|) )`

which gives that `|c| = 2` and hence that is the global bailout value.

-----------------------------------------------------------------------------------------------------------