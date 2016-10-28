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

class Controller
{
        int state;
        int prevState;
    public:
        Controller ( )
        {
            state = 0;
            prevState = 0;
        }
        Controller (int s)
        {
            state = s;
            prevState = 0;
        }
        void set_state(int s)
        {
            state = s;
        }
        void set_prevState(int s)
        {
            prevState = s;
        }
        int get_state()
        {
            return state;
        }
        bool pressed(int button)
        {
            return (button & state);
        }
        bool rePressed(int button)
        {
            return (!(button & prevState) && (button & state)) ;
        }
};

Controller xbox;

double PI = 4.0*atan(1.0);

GLFWwindow* window;
double prevx = -1, prevy = -1;
double resx = 1100, resy = 600;

float frameTime = 0.0; 

float joyAxisX1 = 0.0;
float joyAxisY1 = 0.0;
float joyAxisX2 = 0.0;
float joyAxisY2 = 0.0;
float joyAxisL2 = 0.0;
float joyAxisR2 = 0.0;

bool buttonToggleL1         = false;
bool buttonToggleR1         = false;
bool buttonToggleUPArrow    = false;
bool buttonToggleDownArrow  = false;
bool buttonToggleRightArrow = false;
bool buttonToggleLeftArrow  = false;

float disVecX    = 0.0;
float disVecY    = 0.0;
float disVecZ    = 0.0;

int fractalMaxIt    = 10;
int marchMaxIt      = 100;
float marchEpsilon  = 0.01;

// int xboxButtonState = 0;

// CAMERA INITIALIZATION 
vec3 rightVec  = vec3(1.0,  0.0,  0.0);
vec3 upVec     = vec3(0.0,  1.0,  0.0);
vec3 viewVec   = vec3(0.0,  0.0, -1.0);
vec3 posVec    = vec3(0.0,  0.0,  5.0);

float frustumD = resx; //Depth of frustum

int joystickPresent;
int clickedButtons = 0;

enum buttonMaps { FIRST_BUTTON=1, SECOND_BUTTON=2, THIRD_BUTTON=4, FOURTH_BUTTON=8, FIFTH_BUTTON=16, NO_BUTTON=0 };
enum modifierMaps { CTRL=2, SHIFT=1, ALT=4, META=8, NO_MODIFIER=0 };

enum xboxButtonMap { XBOX_A = 1, XBOX_B = 2, XBOX_X = 4, XBOX_Y = 8, XBOX_UP = 16, XBOX_DOWN = 32, XBOX_RIGHT = 64, 
                     XBOX_LEFT = 128, XBOX_L1 = 256, XBOX_R1 = 512, XBOX_OPTION = 1024, XBOX_START = 2048, XBOX_AXIS1 = 4096, XBOX_AXIS2 = 8192};

int drawMandelbrot = 1;

GLuint programID;
GLuint VertexArrayID;
GLuint vertexbuffer;    

static const GLfloat vertex_buffer_data[] =
{
    -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,
};


char *readFile(const char *filename)
{
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

void CompileShader(const char * file_path, GLuint ShaderID)
{
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

    if ( Result == GL_FALSE )
    {
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

    if ( InfoLogLength > 0 )
    {
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

int main()
{
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

    while ( !glfwWindowShouldClose(window)) 
    {
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

void Draw()
{
    frameTime += 0.001;

    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(programID);

    // ====================================================================================================
    // ------------------ TRANSFER DATA TO GPU SHADER -----------------------------------------------------
    // ====================================================================================================
    GLint loc;
    loc = glGetUniformLocation(programID, "resx");
    if (loc != -1) 
        glUniform1f(loc, resx);
    loc = glGetUniformLocation(programID, "resy");
    if (loc != -1) 
        glUniform1f(loc, resy);

    loc = glGetUniformLocation(programID, "frameTime"); 
    if (loc != -1) 
        glUniform1f(loc, frameTime);

    loc = glGetUniformLocation(programID, "frustumD"); 
    if (loc != -1) 
        glUniform1f(loc, frustumD);

    loc = glGetUniformLocation(programID, "viewVec"); 
    if (loc != -1) 
        glUniform3f(loc, viewVec.x, viewVec.y, viewVec.z);
    loc = glGetUniformLocation(programID, "upVec"); 
    if (loc != -1) 
        glUniform3f(loc, upVec.x, upVec.y, upVec.z);
    loc = glGetUniformLocation(programID, "rightVec"); 
    if (loc != -1) 
        glUniform3f(loc, rightVec.x, rightVec.y, rightVec.z);
    loc = glGetUniformLocation(programID, "posVec"); 
    if (loc != -1) 
        glUniform3f(loc, posVec.x, posVec.y, posVec.z);

    loc = glGetUniformLocation(programID, "fractalMaxIt"); 
    if (loc != -1) 
        glUniform1i(loc, fractalMaxIt);
    loc = glGetUniformLocation(programID, "marchMaxIt"); 
    if (loc != -1) 
        glUniform1i(loc, marchMaxIt);
    loc = glGetUniformLocation(programID, "marchEpsilon"); 
    if (loc != -1) 
        glUniform1f(loc, marchEpsilon);

    // ====================================================================================================

    glEnableVertexAttribArray(0);                            // matches shader layout specifier
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);             // vertex data bound to this buffer
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0); // specify layout (vec3, float)
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);                   // Draw the triangles using a triangle strip
    glDisableVertexAttribArray(0);

    // Swap buffers
    glfwSwapBuffers(window);
}
void initGL()
{

    printf("Initializing OpenGL/GLFW\n"); 

    if (!glfwInit())
    {
        printf("Could not initialize\n");
        exit(-1);
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(resx, resy, "Mandelbrot", 0, 0);

    if (!window)
    {
        printf("Could not open glfw window\n");
        glfwTerminate();
        exit(-2);
    }

    glfwMakeContextCurrent(window); 

    // Initialize GLEW
    printf("Initializing GLEW\n"); 
    glewExperimental = true; // Needed for core profile

    if (glewInit() != GLEW_OK)
    {
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

void joystick_callback()
{
    // Configured for XBOX controller

	int axesCount;
    const float *axes = glfwGetJoystickAxes( GLFW_JOYSTICK_1, &axesCount );
     
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
   
    int buttonCount;
    const unsigned char *buttons = glfwGetJoystickButtons( GLFW_JOYSTICK_1, &buttonCount );

    int xboxButtonState = 0;
    xboxButtonState += buttons[0]  ? XBOX_A       : 0;
    xboxButtonState += buttons[1]  ? XBOX_B       : 0;
    xboxButtonState += buttons[2]  ? XBOX_X       : 0;
    xboxButtonState += buttons[3]  ? XBOX_Y       : 0;
    xboxButtonState += buttons[4]  ? XBOX_L1      : 0;
    xboxButtonState += buttons[5]  ? XBOX_R1      : 0;
    xboxButtonState += buttons[6]  ? XBOX_OPTION  : 0;
    xboxButtonState += buttons[7]  ? XBOX_START   : 0;
    xboxButtonState += buttons[8]  ? XBOX_AXIS1   : 0;
    xboxButtonState += buttons[9]  ? XBOX_AXIS2   : 0;
    xboxButtonState += buttons[10] ? XBOX_UP      : 0;
    xboxButtonState += buttons[11] ? XBOX_RIGHT   : 0;
    xboxButtonState += buttons[12] ? XBOX_DOWN    : 0;
    xboxButtonState += buttons[13] ? XBOX_LEFT    : 0;
    xbox.set_prevState(xbox.get_state());
    xbox.set_state(xboxButtonState);

    if ( xbox.rePressed(XBOX_L1) )
    {
        marchEpsilon *= 0.5;
        std::cout << "march epsilon = " << marchEpsilon << std::endl;
    }
    if ( xbox.rePressed(XBOX_R1) )
    {
        marchEpsilon *= 5.0;
        std::cout << "march epsilon = " << marchEpsilon << std::endl;
    }

    if ( xbox.rePressed(XBOX_UP) )
    {
        fractalMaxIt += (1 + 9*xbox.pressed(XBOX_X))*3;
        std::cout << "fractal max it = " << fractalMaxIt  << std::endl;
    }
    if ( xbox.rePressed(XBOX_DOWN) )
    {
        fractalMaxIt -= (1.0 + 9.0*xbox.pressed(XBOX_X))*3.0;
        if (fractalMaxIt < 1)
            fractalMaxIt = 1;
        std::cout << "fractal max it = " << fractalMaxIt  << std::endl;
    }

    if ( xbox.rePressed(XBOX_RIGHT) )
    {
        marchMaxIt += (1.0 + 9.0*xbox.pressed(XBOX_X))*10.0;
        std::cout << "march max it = " << marchMaxIt  << std::endl;
    }
    if ( xbox.rePressed(XBOX_LEFT) )
    {
        marchMaxIt -= (1.0 + 9.0*xbox.pressed(XBOX_X))*10.0;
        if(marchMaxIt < 1)
            marchMaxIt = 1;
        std::cout << "march max it = " << marchMaxIt  << std::endl;
    }

    
    const char *name = glfwGetJoystickName( GLFW_JOYSTICK_1 );

    // Camera controll

	float viewSpeed = 0.03;

	rightVec = normalize(rightVec - viewSpeed*joyAxisX2*viewVec) ;
	upVec    = normalize(upVec    - viewSpeed*joyAxisY2*viewVec) ;
	viewVec  = normalize(viewVec  + viewSpeed*joyAxisY2*upVec + viewSpeed*joyAxisX2*rightVec) ;

	rightVec = normalize(rightVec);

	posVec   = posVec + (0.01*xbox.pressed(XBOX_B) +  0.0001)*(joyAxisR2 - joyAxisL2)*viewVec;

}

void windowsize_callback(GLFWwindow *win, int width, int height)
{

    resx = width;
    resy = height;

    glViewport(0, 0, resx, resy);
}

void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE)
    {
        glfwSetWindowShouldClose(win, GL_TRUE);
    }

    if (key == GLFW_KEY_SPACE && action)
    {

    }
}

void mousebutton_callback( GLFWwindow* win, int button, int action, int mods )
{
    glfwGetCursorPos( win, &prevx, &prevy );

    if (action == 1)
        clickedButtons |= (1 << button);
    else
        clickedButtons &= ~(1 << button);

    if ( clickedButtons & FIRST_BUTTON )
    {
        
    }
    else if ( clickedButtons & SECOND_BUTTON )
    {

    }
    else if ( clickedButtons & THIRD_BUTTON ) 
    {

    }
    else if ( clickedButtons & FOURTH_BUTTON )
    {

    }
    else if ( clickedButtons & FIFTH_BUTTON )
    {

    }
}

void mousepos_callback(GLFWwindow* win, double xpos, double ypos)
{
    if (clickedButtons&FIRST_BUTTON)
    {
    }
    else if (clickedButtons&SECOND_BUTTON)
    {
    }
    else if (clickedButtons&THIRD_BUTTON)
    {

    }
    else if (clickedButtons&FOURTH_BUTTON)
    {

    }
    else if (clickedButtons&FIFTH_BUTTON)
    {

    }
}

void mousewheel_callback(GLFWwindow* win, double xoffset, double yoffset)
{
    frustumD += 100*yoffset;
    if(frustumD < 100)
    {
        frustumD = 100;
    }
    std::cout << "fD = " << frustumD << "\n";
}
