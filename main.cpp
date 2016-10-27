#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

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

float norm(vec3 vec) {
	return sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}

vec3 normalize(vec3 vec) {
	return vec/norm(vec);
}


double PI = 4.0*atan(1.0);

GLFWwindow* window;
double prevx = -1, prevy = -1;
double resx = 1100,resy = 600;

// "camera": specify x and y coordinate of lower left pixel
double dy = 2.0;                    // height
double dx = dy*resx/float(resy);    // scaled width
double minx = -2.5;                 // x from -2.0 to 0.5
double miny = -1.0;                 // y from -1.0 to 1.0

float frameTime = 0.0; 

float lightX = 400.0;
float lightY = 0.0;
float lightZ = 250.0;

float joyAxisX1 = 0.0;
float joyAxisY1 = 0.0;
float joyAxisX2 = 0.0;
float joyAxisY2 = 0.0;
float joyAxisL2 = 0.0;
float joyAxisR2 = 0.0;

int joyButtonA = 0;
int joyButtonB = 0;
int joyButtonX = 0;
int joyButtonY = 0;

bool buttonToggleL1 = false;
bool buttonToggleR1 = false;
bool buttonToggleUPArrow = false;
bool buttonToggleDownArrow = false;
bool buttonToggleRightArrow = false;
bool buttonToggleLeftArrow = false;



float panButtonB = 0.0;
float disVecX    = 0.0;
float disVecY    = 0.0;
float disVecZ    = 0.0;

float axisPanX1 = 0.0;
float axisPanY1 = 0.0;
float axisPanX2 = 0.0;
float axisPanY2 = 0.0;
float axisPanL2 = 0.0;
float axisPanR2 = 0.0;

int fractalMaxIt = 10;
int marchMaxIt = 100;
float marchEpsilon = 0.01;

float camTheta = 0.0;
float camPhi = 0.0;

// CAMERA INITIALIZATION 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vec3 rightVec  = vec3(1.0,  0.0,  0.0);
vec3 upVec     = vec3(0.0,  1.0,  0.0);
vec3 viewVec   = vec3(0.0,  0.0, -1.0);

vec3 posVec    = vec3(0.0,  0.0, 5);

float frustumD = resx; //Depth of frustum
int lightToggle = 1; 

int joystickPresent;
int clickedButtons = 0;
enum buttonMaps { FIRST_BUTTON=1, SECOND_BUTTON=2, THIRD_BUTTON=4, FOURTH_BUTTON=8, FIFTH_BUTTON=16, NO_BUTTON=0 };
enum modifierMaps { CTRL=2, SHIFT=1, ALT=4, META=8, NO_MODIFIER=0 };

int drawMandelbrot = 1;

GLuint programID;
GLuint VertexArrayID;
GLuint vertexbuffer;    

static const GLfloat vertex_buffer_data[] = {
    -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,
};


char *readFile(const char *filename) {
    FILE *f = fopen(filename, "rb");

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    printf("Reading %s, Filesize = %d\n", filename, int(fsize));

    char *string = (char*)malloc(fsize + 1);
    fread(string, fsize, 1, f);
    string[fsize] = '\0';
    fclose(f);

    return string;
}

void CompileShader(const char * file_path, GLuint ShaderID) {
    GLint Result = GL_FALSE;
    int InfoLogLength;

    char *ShaderCode   = readFile(file_path);

    // Compile Shader
    printf("Compiling shader : %s\n", file_path);
    glShaderSource(ShaderID, 1, (const char**)&ShaderCode , NULL);
    glCompileShader(ShaderID);

    // Check Shader
    glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);

    if ( Result == GL_FALSE ){
        char ShaderErrorMessage[InfoLogLength];
        glGetShaderInfoLog(ShaderID, InfoLogLength, NULL, ShaderErrorMessage);
        printf("%s", ShaderErrorMessage);
    }
}

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){
    printf("Creating shaders\n");
    GLuint VertexShaderID   = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    CompileShader(vertex_file_path, VertexShaderID);
    CompileShader(fragment_file_path, FragmentShaderID);


    printf("Create and linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    GLint Result = GL_FALSE;
    int InfoLogLength;

    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);

    if ( InfoLogLength > 0 ){
        GLchar ProgramErrorMessage[InfoLogLength+1];
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    fflush(stdout);

    return ProgramID;
}


void initGL();
void Draw();

void joystick_callback();
void windowsize_callback(GLFWwindow *win, int width, int height);
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void mousebutton_callback(GLFWwindow* win, int button, int action, int mods);
void mousepos_callback(GLFWwindow* win, double xpos, double ypos);
void mousewheel_callback(GLFWwindow* win, double xoffset, double yoffset);

int main() {
    initGL();

    // vertex buffers are bound to a vertex array object (VAO)
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "vertex_shader.vs", "fragment_shader.fs" );

    // vertex data are bound to vertex buffer objects (VBO)
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);  // copy to gpu

    while ( !glfwWindowShouldClose(window)) {
    	
    	joystickPresent = glfwJoystickPresent( GLFW_JOYSTICK_1 );
    	// std::cout << "Joystick status: " << present << std::endl;   
		if (joystickPresent)
			joystick_callback();


        Draw();
        glfwPollEvents();
    }

    glDeleteBuffers(1, &vertexbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(programID);

    glfwTerminate();

    return 0;
}

void Draw() {
    // float testTime = glfwGetTime();

    frameTime += 0.001;
    // floatFrameCount = floatFrameCount + 0.1; //To measure how long we've been running
    // std::cout<< frameTime << '\n';

    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(programID);

    // transfer over uniform data (might have changed)
    GLint loc;
    loc = glGetUniformLocation(programID, "resx");
    if (loc != -1) 
        glUniform1f(loc, resx);
    loc = glGetUniformLocation(programID, "resy");
    if (loc != -1) 
        glUniform1f(loc, resy);
    loc = glGetUniformLocation(programID, "minx");
    if (loc != -1) 
        glUniform1f(loc, minx);
    loc = glGetUniformLocation(programID, "miny");
    if (loc != -1) 
        glUniform1f(loc, miny);
    loc = glGetUniformLocation(programID, "dx");
    if (loc != -1) 
        glUniform1f(loc, dx);
    loc = glGetUniformLocation(programID, "dy");
    if (loc != -1) 
        glUniform1f(loc, dy);
    loc = glGetUniformLocation(programID, "drawMandelbrot");
    if (loc != -1) 
        glUniform1i(loc, drawMandelbrot);

    loc = glGetUniformLocation(programID, "frameTime"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1f(loc, frameTime);

    loc = glGetUniformLocation(programID, "lightX"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1f(loc, lightX);

    loc = glGetUniformLocation(programID, "lightY"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1f(loc, lightY);

    loc = glGetUniformLocation(programID, "lightZ"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1f(loc, lightZ);

    loc = glGetUniformLocation(programID, "camTheta"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1f(loc, camTheta);

    loc = glGetUniformLocation(programID, "camPhi"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1f(loc, camPhi);

    loc = glGetUniformLocation(programID, "frustumD"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1f(loc, frustumD);

    // Passing on JoyStick input values
    loc = glGetUniformLocation(programID, "joyAxisX1"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1f(loc, joyAxisX1);    
    loc = glGetUniformLocation(programID, "joyAxisY1"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1f(loc, joyAxisY1);
    loc = glGetUniformLocation(programID, "joyAxisX2"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1f(loc, joyAxisX2);
    loc = glGetUniformLocation(programID, "joyAxisY2"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1f(loc, joyAxisY2);    
    loc = glGetUniformLocation(programID, "joyAxisL2"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1f(loc, joyAxisL2);
    loc = glGetUniformLocation(programID, "joyAxisR2"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1f(loc, joyAxisR2);

    loc = glGetUniformLocation(programID, "axisPanX1"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1f(loc, axisPanX1);
    loc = glGetUniformLocation(programID, "axisPanY1"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1f(loc, axisPanY1);
    loc = glGetUniformLocation(programID, "axisPanX2"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1f(loc, axisPanX2);
    loc = glGetUniformLocation(programID, "axisPanY2"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1f(loc, axisPanY2);
    loc = glGetUniformLocation(programID, "axisPanL2"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1f(loc, axisPanL2);
    loc = glGetUniformLocation(programID, "axisPanR2"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1f(loc, axisPanR2);

 	loc = glGetUniformLocation(programID, "joyButtonB"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1i(loc, joyButtonB);

 	loc = glGetUniformLocation(programID, "panButtonB"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1f(loc, panButtonB);


    loc = glGetUniformLocation(programID, "lightToggle"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1i(loc, lightToggle);

    loc = glGetUniformLocation(programID, "viewVec"); // Let's send over the frame time
    if (loc != -1) 
        glUniform3f(loc, viewVec.x, viewVec.y, viewVec.z);
    loc = glGetUniformLocation(programID, "upVec"); // Let's send over the frame time
    if (loc != -1) 
        glUniform3f(loc, upVec.x, upVec.y, upVec.z);
    loc = glGetUniformLocation(programID, "rightVec"); // Let's send over the frame time
    if (loc != -1) 
        glUniform3f(loc, rightVec.x, rightVec.y, rightVec.z);
    loc = glGetUniformLocation(programID, "posVec"); // Let's send over the frame time
    if (loc != -1) 
        glUniform3f(loc, posVec.x, posVec.y, posVec.z);

    loc = glGetUniformLocation(programID, "fractalMaxIt"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1i(loc, fractalMaxIt);
    loc = glGetUniformLocation(programID, "marchMaxIt"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1i(loc, marchMaxIt);
    loc = glGetUniformLocation(programID, "marchEpsilon"); // Let's send over the frame time
    if (loc != -1) 
        glUniform1f(loc, marchEpsilon);

    glEnableVertexAttribArray(0);                            // matches shader layout specifier
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);             // vertex data bound to this buffer
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0); // specify layout (vec3, float)
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);                   // Draw the triangles using a triangle strip
    glDisableVertexAttribArray(0);

    // Swap buffers
    glfwSwapBuffers(window);
}
void initGL() {

    printf("Initializing OpenGL/GLFW\n"); 
    if (!glfwInit()) {
        printf("Could not initialize\n");
        exit(-1);
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(resx, resy, "Mandelbrot", 0, 0);
    if (!window) {
        printf("Could not open glfw window\n");
        glfwTerminate();
        exit(-2);
    }
    glfwMakeContextCurrent(window); 

    // Initialize GLEW
    printf("Initializing GLEW\n"); 
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        exit(-3);
    }
    

    glfwSetWindowSizeCallback(window,windowsize_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mousebutton_callback);
    glfwSetScrollCallback(window, mousewheel_callback);
    glfwSetCursorPosCallback(window, mousepos_callback);

    glfwSwapInterval(1);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
}

void joystick_callback() {
    // Configured for XBOX controller
	int axesCount;
    const float *axes = glfwGetJoystickAxes( GLFW_JOYSTICK_1, &axesCount );

    // lightX = 0.5*resx*axes[0];
    // lightY = 0.5*resy*axes[1];
    lightZ = 0.5*resx*axes[4] - 0.5*resx*axes[5];

    
    
    float deadZone = 0.30;
  
    if ( axes[0] > deadZone ) // joyAxisX1
    	joyAxisX1 = (axes[0] - deadZone)/(1.0 - deadZone);
    else if ( axes[0] < -deadZone )
    	joyAxisX1 = (axes[0] + deadZone)/(1.0 - deadZone);
    else
    	joyAxisX1 = 0.0;
   
    if ( axes[1] > deadZone ) // joyAxisY1
    	joyAxisY1 = (axes[1] - deadZone)/(1.0 - deadZone);
    else if ( axes[1] < -deadZone )
    	joyAxisY1 = (axes[1] + deadZone)/(1.0 - deadZone);
    else
    	joyAxisY1 = 0.0;
   
    if ( axes[2] > deadZone ) // joyAxisX2
    	 joyAxisX2 = (axes[2] - deadZone)/(1.0 - deadZone);
    else if ( axes[2] < -deadZone )
    	 joyAxisX2 = (axes[2] + deadZone)/(1.0 - deadZone);
    else
    	 joyAxisX2 = 0.0;
    
    if ( axes[3] > deadZone ) // joyAxisY2
    	joyAxisY2 = (axes[3] - deadZone)/(1.0 - deadZone);
    else if ( axes[3] < -deadZone )
    	joyAxisY2 = (axes[3] + deadZone)/(1.0 - deadZone);
    else
    	joyAxisY2 = 0.0;
	
	if ( axes[4] > (deadZone - 1.0) ) // joyAxisL2
    	joyAxisL2 = (axes[4] - 0.5*deadZone)/(1.0 - 0.5*deadZone);
    else
    	joyAxisL2 = -1.0;
	
	if ( axes[5] > (deadZone - 1.0) ) // joyAxisR2
    	joyAxisR2 = (axes[5] - 0.5*deadZone)/(1.0 - 0.5*deadZone);
    else
    	joyAxisR2 = -1.0;

    axisPanX1 += joyAxisX1;
    axisPanY1 += joyAxisY1;
    axisPanX2 += joyAxisX2;
    axisPanY2 += joyAxisY2;
    axisPanL2 += 1 + joyAxisL2;
    axisPanR2 += 1 + joyAxisR2;

    // std::cout << axisPanX2 << " : " <<  axisPanY2 << std::endl;

    // std::cout << axisPanL2 << std::endl;
    // std::cout << "~time = " << frameTime << std::endl;
    // std::cout << "deadZone = " << deadZone << std::endl;
    // std::cout << "AxisX1 = " << joyAxisX1 << std::endl;
    // std::cout << "AxisY1 = " << joyAxisY1 << std::endl;
    // std::cout << "AxisX2 = " << joyAxisX2 << std::endl;
    // std::cout << "AxisY2 = " << joyAxisY2 << std::endl;
    // std::cout << "AxisL2 = " << joyAxisL2 << std::endl;
    // std::cout << "AxisR2 = " << joyAxisR2 << std::endl;
    //std::cout << count << std::endl;

    
    // std::cout << "Left Stick X Axis: " << axes[0] << std::endl; // tested with PS4 controller connected via micro USB cable
    // std::cout << "Left Stick Y Axis: " << axes[1] << std::endl; // tested with PS4 controller connected via micro USB cable
    // std::cout << "Right Stick X Axis: " << axes[2] << std::endl; // tested with PS4 controller connected via micro USB cable
    // std::cout << "Right Stick Y Axis: " << axes[3] << std::endl; // tested with PS4 controller connected via micro USB cable
    // std::cout << "Left Trigger/L2: " << axes[4] << std::endl; // tested with PS4 controller connected via micro USB cable
    // std::cout << "Right Trigger/R2: " << axes[5] << std::endl; // tested with PS4 controller connected via micro USB cable
    
     
    int buttonCount;
    const unsigned char *buttons = glfwGetJoystickButtons( GLFW_JOYSTICK_1, &buttonCount );
    if ( GLFW_PRESS == buttons[0] )
    {
        // std::cout << "A Pressed" << std::endl;
        joyButtonA = 1;
    }
    if ( GLFW_RELEASE == buttons[0] )
    {
        // std::cout << "A Pressed" << std::endl;
        joyButtonA = 0;
    }
    if ( GLFW_PRESS == buttons[1] )
    {
        // std::cout << "B Pressed" << std::endl;
        joyButtonB = 1;
    }
    if ( GLFW_RELEASE == buttons[1] )
    {
        // std::cout << "B Released" << std::endl;
        joyButtonB = 0;
    }
    if ( GLFW_PRESS == buttons[2] )
    {
        joyButtonX = 1;
        // std::cout << "X Pressed" << std::endl;
    }
    if ( GLFW_RELEASE == buttons[2] )
    {
        joyButtonX = 0;
        // std::cout << "X Pressed" << std::endl;
    }
    if ( GLFW_PRESS == buttons[3] )
    {
        joyButtonY = 1;
        // std::cout << "Y Pressed" << std::endl;
    }
    if ( GLFW_RELEASE == buttons[3] )
    {
        joyButtonY = 0;
        // std::cout << "Y Pressed" << std::endl;
    }
    if ( GLFW_PRESS == buttons[4] ) // L1 - press
    {
        if (buttonToggleL1 == false)
        {
            buttonToggleL1 = true;
            marchEpsilon *= 0.5;
            std::cout << "march epsilon = " << marchEpsilon << std::endl;
        }
        
    }
    if ( GLFW_RELEASE == buttons[4] ) // L1 - release
    {
        buttonToggleL1 = false;
    }  
    if ( GLFW_PRESS == buttons[5] ) // R1 - press
    {
        if ( buttonToggleR1 == false )
        {
            buttonToggleR1 = true;
            marchEpsilon *= 5.0;
            std::cout << "march epsilon = " << marchEpsilon << std::endl;
        }
       
    }   
    if ( GLFW_RELEASE == buttons[5] ) // R1 - release
    {
        buttonToggleR1 = false;
    }  
    if ( GLFW_PRESS == buttons[6] ) // option 
    {
        
    }
    if ( GLFW_PRESS == buttons[7] ) // start
    {

    }
    if ( GLFW_PRESS == buttons[8] )
    {
        // std::cout << "axis1 press" << std::endl;
    }
    if ( GLFW_PRESS == buttons[9] )
    {
        // std::cout << "axis2 press" << std::endl;
    }

    if ( GLFW_PRESS == buttons[10] ) // UP ARROW - press
    {
        if (buttonToggleUPArrow == false)
        {
            buttonToggleUPArrow = true;
            fractalMaxIt += (1 + 9*joyButtonX)*3;
            std::cout << "fractal max it = " << fractalMaxIt  << std::endl;
        }
    }
    if ( GLFW_RELEASE == buttons[10] ) // UP ARROW - release
    {
        buttonToggleUPArrow = false;
    }

    if ( GLFW_PRESS == buttons[11] ) // RIGHT ARROW - press
    {
        // std::cout << "right arrow" << std::endl;
        if (buttonToggleRightArrow == false)
        {
            buttonToggleRightArrow = true;
            marchMaxIt += (1.0 + 9.0*joyButtonX)*10.0;
            std::cout << "march max it = " << marchMaxIt  << std::endl;
        }
    }
    if ( GLFW_RELEASE == buttons[11] ) // RIGHT ARROW - release
    {
        buttonToggleRightArrow = false;
    }
    if ( GLFW_PRESS == buttons[12] ) // DOWN ARROW - press
    {
        // std::cout << "down arrow" << std::endl;
        if ( buttonToggleDownArrow == false )
        {
            buttonToggleDownArrow = true;
            fractalMaxIt -= (1.0 + 9.0*joyButtonX)*3.0;
            if (fractalMaxIt < 1)
                fractalMaxIt = 1;
            std::cout << "fractal max it = " << fractalMaxIt  << std::endl;
        }
    }
    if ( GLFW_RELEASE == buttons[12] ) // DOWN ARROW - release
    {
        buttonToggleDownArrow = false;
    }

    if ( GLFW_PRESS == buttons[13] ) // LEFT ARROW - press
    {
        // std::cout << "left arrow" << std::endl;
        if ( buttonToggleLeftArrow == false) 
        {
            buttonToggleLeftArrow = true;
            marchMaxIt -= (1.0 + 9.0*joyButtonX)*10.0;
            if(marchMaxIt < 1)
                marchMaxIt = 1;
            std::cout << "march max it = " << marchMaxIt  << std::endl;
        }
    }
    if ( GLFW_RELEASE == buttons[13] ) // LEFT ARROW - release
    {
        buttonToggleLeftArrow = false;
    }
    else if ( GLFW_RELEASE == buttons[0] )
    {
        // std::cout << "Released" << std::endl;
    }
    
    const char *name = glfwGetJoystickName( GLFW_JOYSTICK_1 );
    //std::cout << name << std::endl;

    // Camera controll

	float viewSpeed = 0.03;

	rightVec = normalize(rightVec - viewSpeed*joyAxisX2*viewVec) ;
	upVec    = normalize(upVec    - viewSpeed*joyAxisY2*viewVec) ;
	viewVec  = normalize(viewVec  + viewSpeed*joyAxisY2*upVec + viewSpeed*joyAxisX2*rightVec) ;

	rightVec = normalize(rightVec);
	// upVec    = normalize(upVec);
	// viewVec  = normalize(viewVec);

	// std::cout << "norm = " << norm(viewVec) << std::endl;
	posVec   = posVec + (0.01*joyButtonB +  0.0001)*(joyAxisR2 - joyAxisL2)*viewVec;

    // disVecX +=  10*(joyButtonB - joyButtonX)*viewVecX;
    // disVecY +=  10*(joyButtonB - joyButtonX)*viewVecY;
    // disVecZ +=  10*(joyButtonB - joyButtonX)*viewVecZ;

}

void windowsize_callback(GLFWwindow *win, int width, int height) {
    double distance_per_pixel = dy/resy; // assuming cam_height/resy == cam_width/resx

    resx = width;
    resy = height;
    dx = distance_per_pixel*resx;
    dy = distance_per_pixel*resy;

    glViewport(0, 0, resx, resy);
}

void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(win, GL_TRUE);
    }

    if (key == GLFW_KEY_SPACE && action) {
        drawMandelbrot = 1 - drawMandelbrot;
        lightToggle *= -1;
        std::cout << "lightToggle = " << lightToggle << "\n";
    }
}

void mousebutton_callback(GLFWwindow* win, int button, int action, int mods) {
    glfwGetCursorPos(win,&prevx,&prevy);

    if (action == 1)
        clickedButtons |= (1 << button);
    else
        clickedButtons &= ~(1 << button);

    if (clickedButtons&FIRST_BUTTON) {
        
    } else if (clickedButtons&SECOND_BUTTON) {

    } else if (clickedButtons&THIRD_BUTTON) {

    } else if (clickedButtons&FOURTH_BUTTON) {

    } else if (clickedButtons&FIFTH_BUTTON) {

    }
}

void mousepos_callback(GLFWwindow* win, double xpos, double ypos) {
    if (clickedButtons&FIRST_BUTTON) {

        //Frame motion 

        // minx -= (xpos-prevx)*dx/resx;
        // miny += (ypos-prevy)*dy/resy;

        // prevx = xpos;
        // prevy = ypos;


        // LIGHT SOURCE MOTION 

        lightX = +(xpos-0.5*resx);
        lightY = -(ypos-0.5*resy);

         std::cout<<lightX << ", " << lightY << '\n';


    } else if (clickedButtons&SECOND_BUTTON) {
        lightZ = 6*(ypos-0.5*resy);
        std::cout<<lightZ << '\n';

    } else if (clickedButtons&THIRD_BUTTON) {

    } else if (clickedButtons&FOURTH_BUTTON) {

    } else if (clickedButtons&FIFTH_BUTTON) {

    }
}

void mousewheel_callback(GLFWwindow* win, double xoffset, double yoffset) {
    // double zoomFactor = pow(0.95,yoffset);

    // minx = minx + dx*(1.0 - zoomFactor)/2.0;
    // miny = miny + dy*(1.0 - zoomFactor)/2.0;
    // dx *= zoomFactor;
    // dy *= zoomFactor;

    frustumD += 100*yoffset;
    if(frustumD < 100){
        frustumD = 100;
    }
    std::cout << "fD = " << frustumD << "\n";

}
