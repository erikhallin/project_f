#ifndef HUD_H
#define HUD_H

#include <SOIL/soil.h>
#include <iostream>
#include <windows.h>
#include <gl/gl.h>
#include <vector>
#include <ctime>

#include "definitions.h"
#include "factory.h"
#include "unit.h"
#include "display.h"

using namespace std;


enum kind_of_button_action {INCREASE_SPAWN_TIME=0,DECREASE_SPAWN_TIME, UNIT_SPEED_INCREASE, UNIT_SPEED_DECREASE,
                            DAMAGE_INCREASE, DAMAGE_DECREASE, HEALTH_INCREASE, HEALTH_DECREASE, RANGE_INCREASE,
                            RANGE_DECREASE, SPLASH_INCREASE, SPLASH_DECREASE,SPEED_ROTATION_INCREASE,
                            SPEED_ROTATION_DECREASE, PROJECTILE_SPEED_INCREASE, PROJECTILE_SPEED_DECREASE};


enum hud_statues
{
    hs_off=0,
    hs_for_unit,
    hs_for_factory
};

struct st_hud_button
{
	float value;
	st_pos upper_left;
	st_pos upper_right;
	st_pos lower_left;
	st_pos lower_right;
	float color1,color2,color3;
	kind_of_button_action local_button;
	display m_display;
};

class hud
{
    public:

        hud();

        int  m_hud_draw_status;

        bool init(int m_window_width,int m_window_height,bool* pKeys,int* pMouse_pos,bool* pMouse_button,float* pCam_pos,
                  vector<factory*>* pVec_pFactories,vector<unit*>* pVec_pUnits,float* pGame_time);
        bool update(void);
        bool draw();

    private:

        int*   m_pMouse_pos;
        bool*  m_pKeys;
        bool*  m_pMouse_button;
        bool   m_key_trigger_mouse_button[2];
        float* m_pCam_pos;
        float* m_pGame_time;

        int texture_font[3];

        vector<st_hud_button> v_st_hud_button;
        vector<display> written_things_to_print;

        vector<factory*>* m_pVec_pFactories;
        vector<unit*>*    m_pVec_pUnits;

        bool loadfont(void);
        bool is_mouse_inside_button(const st_pos& m_mouse_pos_curr, st_hud_button& button);
};

#endif // HUD_H
