
**Mandelbulb Explorer** 
=========================


**This project explores the use of ray-marching to render a 3D fractal, specifically one known as the Mandelbulb. It is a generalization of the standard 2D fractal, the Mandelbrot set. In the video below you see the rendering in action. It is rendered in real time on a GTX960. The flight is controlled by an xbox controller.**

[![Whoo](http://i.imgur.com/Uro1Atp.png)](https://youtu.be/TRwiLgBemGQ)

-----------------------

## The Mandelbrot Map

The equation that encodes the Mandelbrot set is perhaps familiar to you, essentially the entire set is somehow hiding  withint the mapping

` z ->  z*z + c `

where `z` and `c` are complex number.

## Rendering the Mandelbrot Set

When one repeadely applies the Mandelbrot map to a given compex number. That is, you take the output and put it back into the mapping, you get a sequence that you could continue indefinitely.

The Mandelbrot set is defined as the set of complex numbers whose Mandelbrot map sequences do not escape to infinity (diverge). For instance, the point `c = 2.0` is not in the Mandelbrot set since aplying the map serveral times produces the sequence:

```0.0,
2.0,
6.0,
38.0,
1446.0,
2090918.0,
4371938082726.0,
1.9113842599189892e+25,
3.653389789066062e+50,
1.3347256950852164e+1010,
...
```

But hang on, even though it is quite intuitive that this will continue to grow indefinitely, it is not that easy to see in general. At every step in the sequence it is possible to perform a test (checking whether `|z| >= 2.0`) which if it is true implies that the seuqnces is going to diverge. A sketh of a proof is reserved for later.

In general it is harder to say whether something is truly in the Mandelbrot set or not, but one becomes more certain the further along one iterates the sequence as illustrated:

![Iterations](http://i.imgur.com/CMHn3r7.png)

The sets have been rendered by associating each pixel coordinate `(x,y)` with a different choice for the constant `c = x + iy`. White pixels are those that are definitely not in the set, whilst black pixels could be in the set.

Generalizing to Three Dimensions
===========================================


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


We can generalize even more by considering

`z = z^pow + c`.

It is clear that if we set `pow = 2` we recover what we considered above, but why not consider other powers. It is simple to deduce what the effect is in the 2D case for complex numbers. The result is that we should take the length to the power of pow, and we should rotate by `φ`, (`pow-1`) number of times. This is easy to generalize.

The name _Mandebulb_ is usually associated with the fractal that you get if you have `pow ~ 6` to `8`.

Rendering 
=================================

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
