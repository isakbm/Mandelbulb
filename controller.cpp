//controller.cpp

#include <iostream>
#include "controller.h"

Controller::Controller ( )
{
    buttons     = 0;
    prevButtons = 0;

    trigger_L = -1.0;
    trigger_R = -1.0;
    
    stick_L_x = 0.0;
    stick_L_y = 0.0;
    stick_R_x = 0.0;
    stick_R_y = 0.0;

    dZ = 0.30;
}
void Controller::set_deadZone (float deadZone)
{
    dZ = deadZone;
}

void Controller::set_sticks(const float * a)  
{
    stick_L_x = ( a[0] > dZ ) ? (a[0] - dZ)/(1.0 - dZ) : std::min( (a[0] + dZ)/(1.0 - dZ) , 0.0);   // These computations handle the dead-zone problem 
    stick_L_y = ( a[1] > dZ ) ? (a[1] - dZ)/(1.0 - dZ) : std::min( (a[1] + dZ)/(1.0 - dZ) , 0.0);
    stick_R_x = ( a[2] > dZ ) ? (a[2] - dZ)/(1.0 - dZ) : std::min( (a[2] + dZ)/(1.0 - dZ) , 0.0);
    stick_R_y = ( a[3] > dZ ) ? (a[3] - dZ)/(1.0 - dZ) : std::min( (a[3] + dZ)/(1.0 - dZ) , 0.0);
}
void Controller::set_triggers(const float * a)
{ 
    trigger_L =  ( a[4] > (dZ - 1.0) ) ? (a[4] - 0.5*dZ)/(1.0 - 0.5*dZ) : -1.0;  // These computations handle the dead-zone problem 
    trigger_R =  ( a[5] > (dZ - 1.0) ) ? (a[5] - 0.5*dZ)/(1.0 - 0.5*dZ) : -1.0;
} 
void Controller::set_buttons(const unsigned char * b)
{
    prevButtons = buttons;
    buttons = 0;
    buttons += b[0]  ? XBOX_A       : 0;
    buttons += b[1]  ? XBOX_B       : 0;
    buttons += b[2]  ? XBOX_X       : 0;
    buttons += b[3]  ? XBOX_Y       : 0;
    buttons += b[4]  ? XBOX_L1      : 0;
    buttons += b[5]  ? XBOX_R1      : 0;
    buttons += b[6]  ? XBOX_OPTION  : 0;
    buttons += b[7]  ? XBOX_START   : 0;
    buttons += b[8]  ? XBOX_AXIS1   : 0;
    buttons += b[9]  ? XBOX_AXIS2   : 0;
    buttons += b[10] ? XBOX_UP      : 0;
    buttons += b[11] ? XBOX_RIGHT   : 0;
    buttons += b[12] ? XBOX_DOWN    : 0;
    buttons += b[13] ? XBOX_LEFT    : 0;
}
bool Controller::pressed (int button)
{
    return (button & buttons);
}
bool Controller::rePressed (int button)
{
    return (!(button & prevButtons) && (button & buttons)) ;
}
