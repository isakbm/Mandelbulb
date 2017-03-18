#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string.h>

#include "controller.h"
#include "vec.h"

Controller xbox;

double PI = 4.0*atan(1.0);

// WINDOW


GLFWwindow* window;
double prevx = -1, prevy = -1;
double resx = 1193, resy = 720;

// "COUNTING" RENDERED FRAMES
float frameTime = 0.0; 

// CAMERA INITIALIZATION 
vec3 rightVec  = vec3(1.0,  0.0,  0.0);
vec3 upVec     = vec3(0.0,  1.0,  0.0);
vec3 viewVec   = vec3(0.0,  0.0, -1.0);
vec3 posVec    = vec3(0.0,  0.0,  5.0);
float frustumD = resx; //Depth of frustum

// RELATED TO RAY-MARCHED FRACTAL
int fractalMaxIt    = 10;
int marchMaxIt      = 40;
float marchEpsilon  = 0.01;
float lightAdjust = 1.0;
float logCoeffDE = 0.5;
float powerParam = 6.0;
float maxFractalDist = 3.0;

// MISC GLOBALS
int shaderProgramToggle = 0;
int clickedButtons = 0;

enum buttonMaps { FIRST_BUTTON=1, SECOND_BUTTON=2, THIRD_BUTTON=4, FOURTH_BUTTON=8, FIFTH_BUTTON=16, NO_BUTTON=0 };
enum modifierMaps { CTRL=2, SHIFT=1, ALT=4, META=8, NO_MODIFIER=0 };

GLuint programID;
GLuint programID2;
GLuint VertexArrayID;
GLuint vertexbuffer; 
GLuint colorbuffer; 
 
static const GLfloat vertex_buffer_data[] =
{
    -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,
};

const GLfloat color_buffer_data[] = 
{
      1.0f,  0.0f,  0.0f  , 
      0.0f,  1.0f,  0.0f  , 
      0.0f,  0.0f,  1.0f  , 
      1.0f,  0.0f,  0.0f  , 
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

//======================================================================================================================================================
// Testing drawing pixel by pixel
//======================================================================================================================================================

//======================================================================================================================================================
//======================================================================================================================================================

// =============== TO DO FINISH IMPLEMENTING FRACTAL RAY TRACING ON CPU SIDE 
float DE(vec3 ray)
{
// return 2;
    vec3 z = ray;
    float r = 0.0;  // r =  |f(0;c) | = | z | ,                     where  f(n;c) = f(n-1;c)^2 + c    and of course  f(0;c) = 0
    float dr = 1.0; // dr = |f'(0;c)| = 2 | prevz | prevdr + 1 ,    computes to  f'(n;c) = 2 f(n-1;c)f'(n-1;c) + 1
    
    int escapeTime = 0;


    for (int ii = 0; ii < fractalMaxIt ; ii++) { 
        r = length(z);
        if (r > maxFractalDist)
        {
            escapeTime = ii;
            break;
        }
        
        // convert to polar coordinates
        float theta = acos(z.z/r);
        float phi = atan2(z.y,z.x);
        float rpow = pow(r,powerParam - 1.0);
        dr = rpow*powerParam*dr + 1.0; //dr =  pow( r, powerParam - 1.0)*powerParam*dr + 1.0;

        
        // scale and rotate the point
        float zr =  rpow*r; //pow( r, powerParam);
        theta = theta*powerParam;
        phi = phi*powerParam;

        
        // convert back to cartesian coordinates
        z = zr*vec3(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
        z = z + ray;
        
    }
    return logCoeffDE*log(r)*r/dr;
    // return 0.0;
}
// ===================



int main()
{
    initGL();

    // vertex buffers are bound to a vertex array object (VAO)
    glGenVertexArrays(1, &VertexArrayID);  //?these are strictly not necessary but improve performance somewhat? 
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "vertex_shader.vs", "fragment_shader.fs"  );
    // programID2 = programID;
    programID2 = LoadShaders( "vertex_shader.vs", "fragment_shader2.fs" );
    // programID3 = LoadShaders( "passthroughVertex.vs", "fragment_shader2.fs" );

    // vertex data are bound to vertex buffer objects (VBO)
    glGenBuffers(1, &vertexbuffer);

    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer); //we bind the first vertexbuffer and assign it vertex data
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);  // copy to gpu

    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer); //we bind the second vertexbuffer and assign it color data
    glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data), color_buffer_data, GL_STATIC_DRAW);  // copy to gpu

    while ( !glfwWindowShouldClose(window)) 
    {
        // Display FPS in window title
     // calculate time between frames
        static double t1 = glfwGetTime(); // store previous time
        double t2 = glfwGetTime();
        double dt = t2-t1;
        t1 = t2;
        
        // change window title to show exponentially averaged fps, should probably move somewhere else...
        double alpha = 0.02;
        static double dt_smooth = 1.0/60;
        dt_smooth = dt*alpha + dt_smooth*(1.0 - alpha);
        char str[256];
        sprintf(str, "Mandelbulb | FPS = %f", 1.0/dt_smooth);
        glfwSetWindowTitle(window, str);


       

    	int joystickPresent = glfwJoystickPresent( GLFW_JOYSTICK_1 );
    	// std::cout << "Joystick status: " << present << std::endl;   
		if (joystickPresent)
        {
			joystick_callback(); // Updates state of controller

           
            float factor = 0.75; // marchEpsilon control
            if ( xbox.rePressed(XBOX_L1) )
            {
                marchEpsilon *= factor;
                std::cout << "march epsilon = " << marchEpsilon << std::endl;
            }
            if ( xbox.rePressed(XBOX_R1) )
            {
                marchEpsilon *= 1.0/factor;
                std::cout << "march epsilon = " << marchEpsilon << std::endl;
            }

            if ( xbox.rePressed(XBOX_UP) ) // fractalMaxIt control
            {
                if ( xbox.pressed(XBOX_B) )
                {
                    fractalMaxIt += (1 + 9*xbox.pressed(XBOX_X))*3;
                    std::cout << "fractal max it = " << fractalMaxIt  << std::endl;
                }
                else
                {   
                    lightAdjust += 0.1;
                }
            }
            if ( xbox.rePressed(XBOX_DOWN) )
            {
                if ( xbox.pressed(XBOX_B) )
                {
                    fractalMaxIt -= (1.0 + 9.0*xbox.pressed(XBOX_X))*3.0;
                    if (fractalMaxIt < 1)
                        fractalMaxIt = 1;
                    std::cout << "fractal max it = " << fractalMaxIt  << std::endl;
                }
                else
                {
                    lightAdjust -= 0.1;
                }
            }

            if ( xbox.rePressed(XBOX_RIGHT) ) // marchMaxIt control
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

            if (xbox.pressed(XBOX_AXIS2) && xbox.rePressed(XBOX_AXIS1))
            {
                logCoeffDE += 0.01;
                std::cout << "logCoeffDE = " << logCoeffDE  << std::endl;

            }
            if (xbox.pressed(XBOX_AXIS1) && xbox.rePressed(XBOX_AXIS2))
            {
                logCoeffDE -= 0.01;
                std::cout << "logCoeffDE = " << logCoeffDE  << std::endl;

            }
            if ( xbox.rePressed(XBOX_A) ) // for testing purposes
            {
                std::cout << xbox.trigger_R << std::endl;    
            }



            // Camera controll
            float viewSpeed = 0.03;
            vec2 joyAxis2 = vec2(xbox.stick_R_x, xbox.stick_R_y);
            rightVec = normalize( rightVec - viewSpeed*joyAxis2.x*viewVec ) ;
            upVec    = normalize( upVec    + viewSpeed*joyAxis2.y*viewVec ) ;
            viewVec  = normalize( viewVec  - viewSpeed*joyAxis2.y*upVec + viewSpeed*joyAxis2.x*rightVec ) ;
            rightVec = normalize( rightVec );
            posVec   = posVec + 2.0*marchEpsilon*( 2.0*xbox.pressed(XBOX_B) +  1.0 )*(xbox.trigger_R - xbox.trigger_L)*viewVec; //( 0.01*xbox.pressed(XBOX_B) +  0.0001 )*(xbox.trigger_R - xbox.trigger_L)*viewVec;
        }

        Draw();
        glfwPollEvents();
    }

    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &colorbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(programID); 
    glDeleteProgram(programID2); 

    glfwTerminate();

    return 0;
}


void Draw()
{
    frameTime += 0.001;

    glClear(GL_COLOR_BUFFER_BIT);

    shaderProgramToggle = xbox.rePressed(XBOX_Y) ? (!shaderProgramToggle) : shaderProgramToggle;
    if ( shaderProgramToggle == 0 )
    {
        glUseProgram(programID);
    }
    else 
    {
        glUseProgram(programID2);

    }

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

    loc = glGetUniformLocation(programID, "logCoeffDE"); 
    if (loc != -1) 
        glUniform1f(loc, logCoeffDE);

    loc = glGetUniformLocation(programID, "lightAdjust"); 
    if (loc != -1) 
        glUniform1f(loc, lightAdjust);
    loc = glGetUniformLocation(programID, "powerParam"); 
    if (loc != -1) 
        glUniform1f(loc, powerParam);
    loc = glGetUniformLocation(programID, "maxFractalDist"); 
    if (loc != -1) 
        glUniform1f(loc, maxFractalDist);
    // ====================================================================================================

    int vb_location = 0;
    glEnableVertexAttribArray(vb_location);                                     // argument matches shader layout location 
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);                                // vertex data bound to this buffer
    glVertexAttribPointer(vb_location, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);     // specify layout (vec3, float)
    
    int cb_location = 1;
    glEnableVertexAttribArray(cb_location);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glVertexAttribPointer(cb_location, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);


    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);                       // Draw the triangles using a triangle strip
    
    glDisableVertexAttribArray(0);
    // glDisableVertexAttribArray(1);



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

    window = glfwCreateWindow(resx, resy, "Mandelbulb", 0, 0);

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
	int axesCount, buttonCount;

    const char *name = glfwGetJoystickName( GLFW_JOYSTICK_1 );
    const float         *axes    = glfwGetJoystickAxes   ( GLFW_JOYSTICK_1, &axesCount   );
    const unsigned char *buttons = glfwGetJoystickButtons( GLFW_JOYSTICK_1, &buttonCount );

    xbox.set_deadZone(0.30);
    xbox.set_sticks(axes);
    xbox.set_triggers(axes);
    xbox.set_buttons(buttons);
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