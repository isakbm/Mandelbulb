// Controller.h

enum xboxButtonMap { XBOX_A = 1, XBOX_B = 2, XBOX_X = 4, XBOX_Y = 8, XBOX_UP = 16, XBOX_DOWN = 32, XBOX_RIGHT = 64, 
                     XBOX_LEFT = 128, XBOX_L1 = 256, XBOX_R1 = 512, XBOX_OPTION = 1024, XBOX_START = 2048, XBOX_AXIS1 = 4096, XBOX_AXIS2 = 8192};

class Controller
{
    public:

        int buttons, prevButtons;
        float trigger_L, trigger_R;
        float dZ;  // deadZone
        float stick_L_x, stick_L_y, stick_R_x, stick_R_y;

        Controller ( );
        void set_deadZone (float deadZone);
        void set_sticks(const float * a);
        void set_triggers(const float * a);
        void set_buttons(const unsigned char * b);
        bool pressed (int button);
        bool rePressed (int button);
};