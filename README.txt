Mandelbulb explorer
----------------------------------------------------------------------------------------------------

Hello, I'm relatively new to making projects and pushing them to github, so bare with me :)

This project explores the use of ray-marching to render a 3D fractal, specifically one known as the Mandelbulb. It is a generalization of the standard 2D fractal, the Mandelbrot set. The equation that encodes the Mandelbrot set is perhaps familiar to you

z = z^2 + c 

where z and c are complex numbers. The input, c, is fixed and then the equation is iterated starting from z = 0. The result is a sequence of complex numbers where the first one is c, the second one is c + c^2, and so on. If c is picked to represent a pixel on the screen

c = x + i y. 

then the transformation effectively displaces the original pixel coordinates (x,y), and eventually we might "escape". By escaping we mean that the distance to the origin has become greater than 2

|z| > 2    we bail.  

It can be shown the transformations will continue to increase the distance to the origin if (a proof is given at the end)

|z| > 2.

On the other hand, if the radius remains less than or equal to 2 after an infinite number of iterations, we say that the pixel corresponding to the initial value c is a member of the Mandelbrot set.

Since we are unable to iterate to infinity we are always seeing an approximation of the Mandelbrot set, but the convergence is quite fast and a good approximation is easily gained within the limits of machine precision.

Generalizing to 3D
----------------------------------------------------------------------------------------------------
Let us now generalize to higher dimensions. Consider the position vector associated with c

v = (x, y)  

A step in the iteration (1) can be interpreted as adding v rotated and scaled to v itself.
Indeed, using the Euler formula makes this clear

c = length(v) exp(i phi)

c^2 = length(v)^2 exp(2 i phi)

where phi = atan(y,x)

hence

c^2 + c = v + length(v) Rotate(v,phi).

We see that squaring the complex number c is equivalent to doubling the length of v and rotating it by an angle phi. This viewpoint of the iterative procedure is begging for a naive generalization to higher dimensions by simply adding another angle.

We then pick a point 

p = (x,y,z)

and we let the iterative step be

p = p + length(p)*Rotate(p,theta,phi)

where

phi = atan(y,x),
theta = atan(sqrt(x^2 + y^2), z).

Generalizing to higher power
----------------------------------------------------------------------------------------------------
We can generalize even more by considering

z = z^pow + c.

It is clear that if we set pow = 2 we recover what we considered above, but why not consider other powers. It is simple to deduce what the effect is in the 2D case for complex numbers. The result is that we should take the length to the power of pow, and we should rotate by phi, (pow-1) number of times. This is easy to generalize.

The name Mandebulb is usually associated with the fractal that you get if you have pow ~ 6 to 8.

----------------------------------------------------------------------------------------------------

Claim:

z = z^2 + c 

diverges if at any moment

|z| > 2.

Proof:

There are other ways to prove this, but I'll present a version that is ignorant to the fact that it might be 2 that is the "bailout" value. 

Consider a step in the sequence

..., z, z^2 + c, ...

It is the ratio

	 |z^2 + c|
R =  ---------
	    |z|

that decides convergence. If R > 1 we should bail since the sequence won't converge, this statement implies that we should bail if

|z^2 + c| > |z|

as you can convince yourself by drawing circles and thinking of complex numbers as 2D vectors, it is clear that the minimum of the LHS is

min(|z^2 + c|) = abs(|z|^2 - |c|)

then it follows immediately that

abs(|z|^2 - |c|) > |z|

is the bailout condition. 

Let us consider the two cases that resolve the abs function, first consider

 |z|^2 > |c|

in which case the bailout condition reads

|z|^2 - |c| > |z|

solving the corresponding quadratic equation

|z|^2 - |z| - |c| > 0

gives that the bailout condition is 

|z| > 1/2 ( 1 + sqrt(1 + 4 |c|) ).

The minimum value for |z| that satisfies the bailout condition corresponds to what we call the bailout value. So it follows that the bailout value is

|z| = 1/2( 1 + sqrt(1 + 4 |c|)). 


but the minimum value of |z| is also |c| since |z| > |c|
it follows 

Lastly we consider the case 

 |z|^2 < |c|

in which case the bailout condition reads

|c| - |z|^2  > |z|

solving the corresponding quadratic equation

-|z|^2 - |z| + |c| > 0

gives that the bailout condition is 

|z| > 1/2 ( -1 + sqrt(1 + 4 |c|) ).

The minimum value for |z| that satisfies the bailout condition corresponds to what we call the bailout value. So it follows that the bailout value is

|z| = 1/2( -1 + sqrt(1 + 4 |c|)). 

We want a number not a function depending on |c|. So we simply take the maximum of what we have found so far, and clearly the bailout value (let's denote the bailout value |zb|) we found above for |z|^2 < |c| is less than the one we found previously for |z|^2 > |c| namely

1/2( 1 + sqrt(1 + 4 |c|))

since |c| > 0, we see that 

1/2( 1 + sqrt(1 + 4 |c|)) > 1

We see that the ultimate bailout value is given by the value of |c| for which the bailout value is |c| hence

|zb| = 1/2(1 + sqrt(1 + 4|zb|))

which solves to give

|zb| = 2.

