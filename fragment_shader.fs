#version 330 core

// Ouput data
out vec3 color;

// constants
float pi = 3.1415926535897932384626433832795;

// window variables
uniform float minx; // x-coordinate lower left corner of camera
uniform float miny; // y-cooridnate lower left corner of camera
uniform float dx;   // width camera
uniform float dy;   // height camera
uniform float resx; // width of window in pixels
uniform float resy; // height of window in pixels

uniform float frameTime; // time running ~ frames

uniform float frustumD;

// camera variables
uniform vec3 posVec;
uniform vec3 viewVec;
uniform vec3 upVec;
uniform vec3 rightVec;

// application control variables 
uniform int fractalMaxIt;
uniform int marchMaxIt;
uniform float marchEpsilon;

// Pixel coordinates
float Xc = gl_FragCoord.x - minx - 0.5*resx; // Centralized FOV x coordinate
float Yc = gl_FragCoord.y - miny - 0.5*resy; // Centralized FOV y coordinate

// Ray for tracing
vec3 rayDir = normalize(frustumD*viewVec + Xc*rightVec + Yc*upVec);

// Distance function

// float sdSphere( vec3 p, float r) // signed distance function for a sphere of radius r located at origin,
// {
// 	return length(p) - r;
// }

#define POWER_PARAM  4.0
#define MAX_FRACTAL_DIST 1.2

#define MAX_MARCH 10.0

#define MAX_ITER_LIGHT 100
#define EPSILON_LIGHT 0.01

// vec3 getNormal(vec3 p, float d)
// {
// 	vec3 x = vec3(1,0,0);
// 	vec3 y = vec3(0,1,0);
// 	vec3 z = vec3(0,0,1);

// 	return vec3(0,0,0);//normalize(vec3(masterDist(p + d*x), masterDist(p + d*y), masterDist(p + d*z)) - masterDist(p)*vec3(1,1,1));
// }

void main() {

	vec3 fogColor = vec3(0.5,0.5,0.5);
	fogColor = vec3(0.0,0.0,0.0);
	color = fogColor;

	float t = 0.0;
	float d = 0.0;
	float blend;
	float blockFactor = 1.0;

	vec3 lightPos = vec3(-2000.0, 0.0, 0.0);
	vec3 lightVector;

	vec3 ray, lightRay;

	float maxFractalDist = 3.0 + pow(2.0,1.0/(POWER_PARAM-1.0));


	// Creat loop over 2 x 2 collection of rays and average for anti-aliasing
		for (int i = 0; i < marchMaxIt; i++)
		{
			ray = posVec + t*rayDir;
					
			// return 2;
			vec3 z = ray;
			float r = 0.0;  // r =  |f(0;c) | = | z | ,    					where  f(n;c) = f(n-1;c)^2 + c    and of course  f(0;c) = 0
			float dr = 1.0; // dr = |f'(0;c)| = 2 | prevz | prevdr + 1 ,    computes to  f'(n;c) = 2 f(n-1;c)f'(n-1;c) + 1
			
			int escapeTime = 0;
			float shade = 0.0;

			for (int ii = 0; ii < fractalMaxIt ; ii++) { 
				r = length(z);
				if (r > maxFractalDist)
				{
					escapeTime = ii;
					break;
				}
				
				// convert to polar coordinates
				float theta = acos(z.z/r);
				float phi = atan(z.y,z.x);
				float rpow = pow(r,POWER_PARAM - 1.0);
				dr = rpow*POWER_PARAM*dr + 1.0;	//dr =  pow( r, POWER_PARAM - 1.0)*POWER_PARAM*dr + 1.0;

				
				// scale and rotate the point
				float zr =  rpow*r; //pow( r, POWER_PARAM);
				theta = theta*POWER_PARAM;
				phi = phi*POWER_PARAM;

				
				// convert back to cartesian coordinates
				z = zr*vec3(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
				z += ray;
				
			}

			d = 0.5*log(r)*r/dr;

			if (d < marchEpsilon)   // Criteria for being close enough to the fractal
			{
				// blend = exp(5.0*(t/MAX_MARCH - 1.0)); // Blending for fogging out
				// blend = exp(5.0*(t/MAX_MARCH - 1.0)); // Blending for fogging out
			
				// lightVector = normalize(ray - lightPos); 
				// float shade = -dot(getNormal(ray,marchEp), lightVector);

				// shade = 1.0 - 1.5*escapeTime/float(fractalMaxIt);
				shade = 1.0 - i/float(marchMaxIt);
				if (shade < 0.0) shade = 0.0;
				shade = exp(5.0*(shade - 1.0));
				
				color = shade*vec3(0.8,0.9,0.9) + (1.0 - shade)*fogColor ;//+ blend*fogColor;// blockFactor*shade*(1.0 - blend)*vec3(1.0,0.0,0.0) + blend*fogColor;

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



