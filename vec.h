// vec.h

struct vec2 {
    double x, y; 
 
    vec2(double x = 0.0, double y = 0.0) : x(x), y(y) {}
};
 

struct vec3 {
	double x, y, z;	
 
	vec3(double x = 0.0, double y = 0.0, double z = 0.0) : x(x), y(y), z(z) {}
};
 
vec3 operator+(vec3 lhs, vec3 rhs) {
	return vec3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

vec3 operator-(vec3 lhs, vec3 rhs) {
	return vec3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

vec3 operator*(vec3 lhs, double rhs) {
	return vec3(lhs.x*rhs, lhs.y*rhs, lhs.z*rhs);
}

vec3 operator*(double lhs, vec3 rhs) {
	return vec3(lhs*rhs.x, lhs*rhs.y, lhs*rhs.z);
}
 
vec3 operator/(vec3 lhs, double rhs) {
	return vec3(lhs.x/rhs, lhs.y/rhs, lhs.z/rhs);
}

float length(vec3 vec) {
	return sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}

vec3 normalize(vec3 vec) {
	return vec/length(vec);
}
