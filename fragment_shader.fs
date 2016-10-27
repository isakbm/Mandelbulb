#version 330 core

// Ouput data
out vec3 color;

// varying variables
uniform float minx; // x-coordinate lower left corner of camera
uniform float miny; // y-cooridnate lower left corner of camera
uniform float dx;   // width camera
uniform float dy;   // height camera
uniform float resx; // width of window in pixels
uniform float resy; // height of window in pixels

uniform float frameTime; // time running ~ frames

uniform float frustumD;
uniform int lightToggle;

uniform float joyAxisX1; // Joystick input 
uniform float joyAxisY1;
uniform float joyAxisX2;
uniform float joyAxisY2;
uniform float joyAxisL2;
uniform float joyAxisR2;

uniform float axisPanX1;
uniform float axisPanY1;
uniform float axisPanX2;
uniform float axisPanY2;
uniform float axisPanL2;
uniform float axisPanR2;


uniform vec3 posVec;
uniform vec3 viewVec;
uniform vec3 upVec;
uniform vec3 rightVec;

uniform int joyButtonB;
uniform float panButtonB;

uniform int fractalMaxIt;
uniform int marchMaxIt;

uniform float marchEpsilon;

uniform int drawMandelbrot = 1;

// Camera orientation
float pi = 3.1415926535897932384626433832795;

// Pixel coordinates
float Xc = gl_FragCoord.x - minx - 0.5*resx; // Centralized FOV x coordinate
float Yc = gl_FragCoord.y - miny - 0.5*resy; // Centralized FOV y coordinate

// Ray
vec3 rayDir = normalize(frustumD*viewVec + Xc*rightVec + Yc*upVec);//normalize(frustumD*vec3(0,0,-1) + Xc*vec3(1,0,0) + Yc*vec3(0,1,0));
vec3 raySourcePos = posVec;

// Fov

float FOVx = 2*atan(0.5*resx/frustumD);
float FOVy = 2*atan(0.5*resy/frustumD);

// Distance function

float sdSphere( vec3 p, float r) // signed distance function for a sphere of radius r located at origin,
{
	return length(p) - r;
}

float sdTorus( vec3 p, vec2 t )
{
	vec2 q = vec2(length(p.xz)-t.x,p.y);
  	return length(q)-t.y;
}

float masterDist( vec3 p )
{
	vec2 q = vec2(length(p.xz) - 1000, p.y);
	return min(min(length(q) - 100, length(p) - 500), min(length(p-vec3(-600,600,0)) - 200,length(p-vec3(-1000,600,0)) - 50 ));
}


#define POWER_PARAM  16.0
#define MAX_FRACTAL_DIST 1.2

#define MAX_MARCH 10.0

#define MAX_ITER_LIGHT 100
#define EPSILON_LIGHT 0.01

// float DE(vec3 pos)
// {
// 	// return 2;
// 	vec3 z = pos;
// 	float dr = 3.5;
// 	float r = 0.0;

// 	float maxFractalDist = pow(2.0,1.0/(POWER_PARAM-1.0));
	
// 	for (int i = 0; i < fractalMaxIt ; i++) {
// 		r = length(z);
// 		if (r > maxFractalDist){
// 			break;
// 		}
		
// 		// convert to polar coordinates
// 		float theta = acos(z.z/r);
// 		float phi = atan(z.y,z.x);
// 		dr =  pow( r, POWER_PARAM - 1.0)*POWER_PARAM*dr + 1.0;
		
// 		// scale and rotate the point
// 		float zr = pow( r, POWER_PARAM);
// 		theta = theta*POWER_PARAM;
// 		phi = phi*POWER_PARAM;
		
// 		// convert back to cartesian coordinates
// 		z = zr*vec3(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
// 		z+=pos;
// 	}

// 	return 0.5*log(r)*r/dr;//min(0.5*log(r)*r/dr, length(pos) - 1);
// }





vec3 getNormal(vec3 p, float d)
{
	vec3 x = vec3(1,0,0);
	vec3 y = vec3(0,1,0);
	vec3 z = vec3(0,0,1);

	return normalize(vec3(masterDist(p + d*x), masterDist(p + d*y), masterDist(p + d*z)) - masterDist(p)*vec3(1,1,1));
}

void main() {

	vec3 fogColor = vec3(0.5,0.5,0.5);
	color = fogColor;

	float t = 0.0;
	float d = 0.0;
	float blend;
	float blockFactor = 1.0;

	vec3 lightPos = vec3(-2000.0, 0.0, 0.0);
	vec3 lightVector;

	vec3 ray, lightRay;

	float maxFractalDist = 1.0 + pow(2.0,1.0/(POWER_PARAM-1.0));

	// Creat loop over 2 x 2 collection of rays and average for anti-aliasing
		for (int i = 0; i < marchMaxIt; i++)
		{
			ray = raySourcePos + t*rayDir;
					
			// return 2;
			vec3 z = ray;
			float r = 0.0;  // r =  |f(0;c) | = | z | ,    					where  f(n;c) = f(n-1;c)^2 + c    and of course  f(0;c) = 0
			float dr = 1.0; // dr = |f'(0;c)| = 2 | prevz | prevdr + 1 ,    computes to  f'(n;c) = 2 f(n-1;c)f'(n-1;c) + 1
			

			float shade = 0.0;
			for (int ii = 0; ii < fractalMaxIt ; ii++) {
				r = length(z);
				if (r > maxFractalDist)
				{
					break;
				}

				//Experimental bail condition 
				// if (r > pow(2.0,1.0/(POWER_PARAM-1.0)))  // 2^(1/(p-1))   for p = 2 you get 2 as you should
				// {
				// 	break;
				// }
				
				// convert to polar coordinates
				float theta = acos(z.z/r);
				float phi = atan(z.y,z.x);
				dr =  pow( r, POWER_PARAM - 1.0)*POWER_PARAM*dr + 1.0;
				
				// scale and rotate the point
				float zr = pow( r, POWER_PARAM);
				theta = theta*POWER_PARAM;
				phi = phi*POWER_PARAM;

				
				// convert back to cartesian coordinates
				z = zr*vec3(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
				z += ray;
				
				// optional shade
				shade = 1.0 - (ii*1.0/(abs(log(marchEpsilon))));
				if (shade < 0) shade = 0.0;


			}

			d = 0.5*log(r)*r/dr;

			if (d < marchEpsilon) //marchEpsilon)
			{
				blend = exp(5.0*(t/MAX_MARCH - 1.0)); // Blending for fogging out
				blend = 0.0;
				// lightVector = normalize(ray - lightPos); 
				// float shade = -dot(getNormal(ray,marchEp), lightVector);

				// if (shade > 0.0) // Only do raymarching for lightRay when we are sure that the surface is facing the light
				// {
				// 	float tt = 0.0;
				// 	float dd;
				// 	for (int ii = 0; ii < MAX_ITER_LIGHT; ii++)
				// 	{
				// 		lightRay = lightPos + tt*lightVector;
				// 		dd = masterDist(lightRay);
				// 		if (dd < EPSILON_LIGHT)
				// 		{
				// 				if( abs(length(ray - lightRay)) > 1)
				// 				{
				// 					blockFactor = 0.0;
				// 				}
				// 		}
				// 		else if (tt > MAX_MARCH)
				// 		{
				// 			break;
				// 		}

				// 		tt += dd;
				// 	}
				// }
				
				color = shade*(1.0 - blend)*vec3(0.8,0.9,0.9) ;//+ blend*fogColor;// blockFactor*shade*(1.0 - blend)*vec3(1.0,0.0,0.0) + blend*fogColor;

				break;
			}
			else if (t > MAX_MARCH)
			{
				color = fogColor;
				break;
			}

			t += d;
		}
	

}



