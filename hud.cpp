#include "hud.h"


hud::hud()
{
	//ctor
}

bool hud::init(int m_window_width,int m_window_height,bool* pKeys,int* pMouse_pos,bool* pMouse_button,float* pCam_pos,
               vector<factory*> * pVec_pFactories,vector<unit*>* pVec_pUnits,float* pGame_time)
{
    m_hud_draw_status=hs_off;
    m_pKeys=pKeys;
    m_pMouse_pos=pMouse_pos;
    m_pMouse_button=pMouse_button;
    m_key_trigger_mouse_button[0]=m_key_trigger_mouse_button[1]=false;
    m_pCam_pos=pCam_pos;
    m_pVec_pFactories=pVec_pFactories;
    m_pVec_pUnits=pVec_pUnits;
    m_pGame_time=pGame_time;

    //load font textures
    if( !loadfont() )
    {
        cout<<"ERROR: Could not load textures for the HUD\n";
        return false;
    }

    //Spawning time decrease
    float x_1_1=m_window_width*0.03;
    float y_1_1=m_window_height*0.77;

	st_hud_button spawning_time_decrease;
	spawning_time_decrease.value=-0.3;
    spawning_time_decrease.color1=0.7;
    spawning_time_decrease.color2=0.7;
    spawning_time_decrease.color3=0.7;
    spawning_time_decrease.upper_left.x=x_1_1;
    spawning_time_decrease.upper_left.y=y_1_1;
    spawning_time_decrease.upper_right.x=x_1_1+40;
    spawning_time_decrease.upper_right.y=y_1_1;
    spawning_time_decrease.lower_right.x=x_1_1+40;
    spawning_time_decrease.lower_right.y=y_1_1+15;
    spawning_time_decrease.lower_left.x=x_1_1;
    spawning_time_decrease.lower_left.y=y_1_1+15;
    spawning_time_decrease.local_button=DECREASE_SPAWN_TIME;
    //spawning time increase
    st_hud_button spawning_time_increase;
    spawning_time_increase.value=0.3;
    spawning_time_increase.color1=0.3;
    spawning_time_increase.color2=0.3;
    spawning_time_increase.color3=0.3;
    spawning_time_increase.upper_left.x=x_1_1+41;
    spawning_time_increase.upper_left.y=y_1_1;
    spawning_time_increase.upper_right.x=x_1_1+81;
    spawning_time_increase.upper_right.y=y_1_1;
    spawning_time_increase.lower_right.x=x_1_1+81;
    spawning_time_increase.lower_right.y=y_1_1+15;
    spawning_time_increase.lower_left.x=x_1_1+41;
    spawning_time_increase.lower_left.y=y_1_1+15;
    spawning_time_increase.local_button=INCREASE_SPAWN_TIME;
    spawning_time_increase.m_display=display(x_1_1+82,y_1_1,80,15,10,texture_font,"Spawn time");
    written_things_to_print.push_back(spawning_time_increase.m_display);

    //unit speed_decrease
    float x_1_2=m_window_width*0.03+200;
    float y_1_2=m_window_height*0.77;

    st_hud_button unit_speed_decrease;
	unit_speed_decrease.value=-0.3;
    unit_speed_decrease.color1=0.7;
    unit_speed_decrease.color2=0.7;
    unit_speed_decrease.color3=0.7;
    unit_speed_decrease.upper_left.x=x_1_2;
    unit_speed_decrease.upper_left.y= y_1_2;
    unit_speed_decrease.upper_right.x=x_1_2+40;
    unit_speed_decrease.upper_right.y= y_1_2;
    unit_speed_decrease.lower_right.x=x_1_2+40;
    unit_speed_decrease.lower_right.y= y_1_2+15;
    unit_speed_decrease.lower_left.x=x_1_2;
    unit_speed_decrease.lower_left.y= y_1_2+15;
    unit_speed_decrease.local_button=UNIT_SPEED_DECREASE;
    //spawning time increase
    st_hud_button unit_speed_increase;
    unit_speed_increase.value=0.3;
    unit_speed_increase.color1=0.3;
    unit_speed_increase.color2=0.3;
    unit_speed_increase.color3=0.3;
    unit_speed_increase.upper_left.x=x_1_2+41;
    unit_speed_increase.upper_left.y=y_1_2;
    unit_speed_increase.upper_right.x=x_1_2+81;
    unit_speed_increase.upper_right.y=y_1_2;
    unit_speed_increase.lower_right.x=x_1_2+81;
    unit_speed_increase.lower_right.y=y_1_2+15;
    unit_speed_increase.lower_left.x=x_1_2+41;
    unit_speed_increase.lower_left.y=y_1_2+15;
    unit_speed_increase.local_button=UNIT_SPEED_INCREASE;
    unit_speed_increase.m_display=display(x_1_2+82,y_1_2,80,15,10,texture_font,"unit speed");
    written_things_to_print.push_back(unit_speed_increase.m_display);

    //Damage value button
    float x_1_3=m_window_width*0.03+400;
    float y_1_3=m_window_height*0.77;

    st_hud_button damage_decrease;
	damage_decrease.value=-0.3;
    damage_decrease.color1=0.7;
    damage_decrease.color2=0.7;
    damage_decrease.color3=0.7;
    damage_decrease.upper_left.x=x_1_3;
    damage_decrease.upper_left.y= y_1_3;
    damage_decrease.upper_right.x=x_1_3+40;
    damage_decrease.upper_right.y= y_1_3;
    damage_decrease.lower_right.x=x_1_3+40;
    damage_decrease.lower_right.y= y_1_3+15;
    damage_decrease.lower_left.x=x_1_3;
    damage_decrease.lower_left.y= y_1_3+15;
    damage_decrease.local_button=DAMAGE_DECREASE;

    st_hud_button damage_increase;
    damage_increase.value=0.3;
    damage_increase.color1=0.3;
    damage_increase.color2=0.3;
    damage_increase.color3=0.3;
    damage_increase.upper_left.x=x_1_3+41;
    damage_increase.upper_left.y=y_1_3;
    damage_increase.upper_right.x=x_1_3+81;
    damage_increase.upper_right.y=y_1_3;
    damage_increase.lower_right.x=x_1_3+81;
    damage_increase.lower_right.y=y_1_3+15;
    damage_increase.lower_left.x=x_1_3+41;
    damage_increase.lower_left.y=y_1_3+15;
    damage_increase.local_button=DAMAGE_INCREASE;
    damage_increase.m_display=display(x_1_3+82,y_1_2,80,15,10,texture_font,"Damage");
    written_things_to_print.push_back(damage_increase.m_display);

    //Health

    float x_1_4=m_window_width*0.03+600;
    float y_1_4=m_window_height*0.77;

    st_hud_button health_decrease;
	health_decrease.value=-10;
    health_decrease.color1=0.7;
    health_decrease.color2=0.7;
    health_decrease.color3=0.7;
    health_decrease.upper_left.x=x_1_4;
    health_decrease.upper_left.y= y_1_4;
    health_decrease.upper_right.x=x_1_4+40;
    health_decrease.upper_right.y= y_1_4;
    health_decrease.lower_right.x=x_1_4+40;
    health_decrease.lower_right.y= y_1_4+15;
    health_decrease.lower_left.x=x_1_4;
    health_decrease.lower_left.y= y_1_4+15;
    health_decrease.local_button=HEALTH_DECREASE;

    st_hud_button health_increase;
    health_increase.value=1000;
    health_increase.color1=0.3;
    health_increase.color2=0.3;
    health_increase.color3=0.3;
    health_increase.upper_left.x=x_1_4+41;
    health_increase.upper_left.y=y_1_4;
    health_increase.upper_right.x=x_1_4+81;
    health_increase.upper_right.y=y_1_4;
    health_increase.lower_right.x=x_1_4+81;
    health_increase.lower_right.y=y_1_4+15;
    health_increase.lower_left.x=x_1_4+41;
    health_increase.lower_left.y=y_1_4+15;
    health_increase.local_button=HEALTH_INCREASE;
    health_increase.m_display=display(x_1_4+82,y_1_4,80,15,10,texture_font,"Health");
    written_things_to_print.push_back(health_increase.m_display);

    //range

    float x_2_3=m_window_width*0.03+400;
    float y_2_3=m_window_height*0.77+20;

    st_hud_button range_decrease;
	range_decrease.value=-0.3;
    range_decrease.color1=0.7;
    range_decrease.color2=0.7;
    range_decrease.color3=0.7;
    range_decrease.upper_left.x=x_2_3;
    range_decrease.upper_left.y= y_2_3;
    range_decrease.upper_right.x=x_2_3+40;
    range_decrease.upper_right.y= y_2_3;
    range_decrease.lower_right.x=x_2_3+40;
    range_decrease.lower_right.y= y_2_3+15;
    range_decrease.lower_left.x=x_2_3;
    range_decrease.lower_left.y= y_2_3+15;
    range_decrease.local_button=RANGE_DECREASE;

    st_hud_button range_increase;
    range_increase.value=3;
    range_increase.color1=0.3;
    range_increase.color2=0.3;
    range_increase.color3=0.3;
    range_increase.upper_left.x=x_2_3+41;
    range_increase.upper_left.y=y_2_3;
    range_increase.upper_right.x=x_2_3+81;
    range_increase.upper_right.y=y_2_3;
    range_increase.lower_right.x=x_2_3+81;
    range_increase.lower_right.y=y_2_3+15;
    range_increase.lower_left.x=x_2_3+41;
    range_increase.lower_left.y=y_2_3+15;
    range_increase.local_button=RANGE_INCREASE;
    range_increase.m_display=display(x_2_3+82,y_2_3,80,15,10,texture_font,"range");
    written_things_to_print.push_back(range_increase.m_display);

    float x_3_3=m_window_width*0.03+400;
    float y_3_3=m_window_height*0.77+40;

    st_hud_button splash_decrease;
	splash_decrease.value=-0.3;
    splash_decrease.color1=0.7;
    splash_decrease.color2=0.7;
    splash_decrease.color3=0.7;
    splash_decrease.upper_left.x=x_3_3;
    splash_decrease.upper_left.y= y_3_3;
    splash_decrease.upper_right.x=x_3_3+40;
    splash_decrease.upper_right.y= y_3_3;
    splash_decrease.lower_right.x=x_3_3+40;
    splash_decrease.lower_right.y= y_3_3+15;
    splash_decrease.lower_left.x=x_3_3;
    splash_decrease.lower_left.y= y_3_3+15;
    splash_decrease.local_button=SPLASH_DECREASE;

    st_hud_button splash_increase;
    splash_increase.value=30;
    splash_increase.color1=0.3;
    splash_increase.color2=0.3;
    splash_increase.color3=0.3;
    splash_increase.upper_left.x=x_3_3+41;
    splash_increase.upper_left.y=y_3_3;
    splash_increase.upper_right.x=x_3_3+81;
    splash_increase.upper_right.y=y_3_3;
    splash_increase.lower_right.x=x_3_3+81;
    splash_increase.lower_right.y=y_3_3+15;
    splash_increase.lower_left.x=x_3_3+41;
    splash_increase.lower_left.y=y_3_3+15;
    splash_increase.local_button=SPLASH_INCREASE;
    splash_increase.m_display=display(x_3_3+82,y_3_3,80,15,10,texture_font,"splash");
    written_things_to_print.push_back(splash_increase.m_display);



    float x_2_2=m_window_width*0.03+200;
    float y_2_2=m_window_height*0.77+20;

    st_hud_button rotation_decrease;
	rotation_decrease.value=-0.3;
    rotation_decrease.color1=0.7;
    rotation_decrease.color2=0.7;
    rotation_decrease.color3=0.7;
    rotation_decrease.upper_left.x=x_2_2;
    rotation_decrease.upper_left.y= y_2_2;
    rotation_decrease.upper_right.x=x_2_2+40;
    rotation_decrease.upper_right.y= y_2_2;
    rotation_decrease.lower_right.x=x_2_2+40;
    rotation_decrease.lower_right.y= y_2_2+15;
    rotation_decrease.lower_left.x=x_2_2;
    rotation_decrease.lower_left.y= y_2_2+15;
    rotation_decrease.local_button=SPEED_ROTATION_DECREASE;
    //spawning time increase
    st_hud_button rotation_increase;
    rotation_increase.value=0.3;
    rotation_increase.color1=0.3;
    rotation_increase.color2=0.3;
    rotation_increase.color3=0.3;
    rotation_increase.upper_left.x=x_2_2+41;
    rotation_increase.upper_left.y=y_2_2;
    rotation_increase.upper_right.x=x_2_2+81;
    rotation_increase.upper_right.y=y_2_2;
    rotation_increase.lower_right.x=x_2_2+81;
    rotation_increase.lower_right.y=y_2_2+15;
    rotation_increase.lower_left.x=x_2_2+41;
    rotation_increase.lower_left.y=y_2_2+15;
    rotation_increase.local_button=SPEED_ROTATION_INCREASE;
    rotation_increase.m_display=display(x_2_2+82,y_2_2,80,15,10,texture_font,"rotation");
    written_things_to_print.push_back(rotation_increase.m_display);

    float x_4_3=m_window_width*0.03+400;
    float y_4_3=m_window_height*0.77+60;

    st_hud_button projectile_speed_decrease;
	projectile_speed_decrease.value=-0.3;
    projectile_speed_decrease.color1=0.7;
    projectile_speed_decrease.color2=0.7;
    projectile_speed_decrease.color3=0.7;
    projectile_speed_decrease.upper_left.x=x_4_3;
    projectile_speed_decrease.upper_left.y= y_4_3;
    projectile_speed_decrease.upper_right.x=x_4_3+40;
    projectile_speed_decrease.upper_right.y= y_4_3;
    projectile_speed_decrease.lower_right.x=x_4_3+40;
    projectile_speed_decrease.lower_right.y= y_4_3+15;
    projectile_speed_decrease.lower_left.x=x_4_3;
    projectile_speed_decrease.lower_left.y= y_4_3+15;
    projectile_speed_decrease.local_button=PROJECTILE_SPEED_DECREASE;

    st_hud_button projectile_speed_increase;
    projectile_speed_increase.value=30;
    projectile_speed_increase.color1=0.3;
    projectile_speed_increase.color2=0.3;
    projectile_speed_increase.color3=0.3;
    projectile_speed_increase.upper_left.x=x_4_3+41;
    projectile_speed_increase.upper_left.y=y_4_3;
    projectile_speed_increase.upper_right.x=x_4_3+81;
    projectile_speed_increase.upper_right.y=y_4_3;
    projectile_speed_increase.lower_right.x=x_4_3+81;
    projectile_speed_increase.lower_right.y=y_4_3+15;
    projectile_speed_increase.lower_left.x=x_4_3+41;
    projectile_speed_increase.lower_left.y=y_4_3+15;
    projectile_speed_increase.local_button=PROJECTILE_SPEED_INCREASE;
    projectile_speed_increase.m_display=display(x_4_3+82,y_4_3,80,15,10,texture_font,"projectile");
    written_things_to_print.push_back(projectile_speed_increase.m_display);


    //putting them in ORDER SO FAR IS FUNDAMENTAL
	v_st_hud_button.push_back(spawning_time_decrease);
    v_st_hud_button.push_back(spawning_time_increase);
    v_st_hud_button.push_back(unit_speed_decrease);
    v_st_hud_button.push_back(unit_speed_increase);
    v_st_hud_button.push_back(damage_decrease);
    v_st_hud_button.push_back(damage_increase);
    v_st_hud_button.push_back(health_decrease);
    v_st_hud_button.push_back(health_increase);
    v_st_hud_button.push_back(range_decrease);
    v_st_hud_button.push_back(range_increase);
    v_st_hud_button.push_back(splash_decrease);
    v_st_hud_button.push_back(splash_increase);
    v_st_hud_button.push_back(rotation_decrease);
    v_st_hud_button.push_back(rotation_increase);
    v_st_hud_button.push_back(projectile_speed_decrease);
    v_st_hud_button.push_back(projectile_speed_increase);



	return true;
}

bool hud::update(void)
{
	bool hud_interaction=false;

	//test if the HUD was clicked on
	if(m_pMouse_button[0])
	{
	    if(!m_key_trigger_mouse_button[0])
	    {
	        m_key_trigger_mouse_button[0]=true;
	        //LMB clicked this cycle
	        //cout<<"HUD: LMB was clicked\n";

            //calc where the mouse is
            st_pos mouse_pos(m_pCam_pos[0]+m_pMouse_pos[0],m_pCam_pos[1]+m_pMouse_pos[1]);

            //factory button test
            for (unsigned int i = 0; i < v_st_hud_button.size(); i++)
            {
                if(is_mouse_inside_button(mouse_pos, v_st_hud_button[i])) //is_mouse_inside_button(m_mouse_start_drag_pos, v_st_hud_button[i])
                {
                    //cout << "clicked inside button" << endl;
                    //cout << v_st_hud_button[i].local_button << endl;
                    //cout << v_st_hud_button[i].value << endl;
                    for(unsigned int fact_i = 0; fact_i<m_pVec_pFactories->size(); fact_i++)
                    {
                        if((*m_pVec_pFactories)[fact_i]->m_selected)
                        {
                            switch(v_st_hud_button[i].local_button)
                            {
                                case  DECREASE_SPAWN_TIME:
                                {
                                    (*m_pVec_pFactories)[fact_i]->modify_spawn_time(v_st_hud_button[i].value);
                                    cout << "decrease spawn time"<< endl;
                                    break;
                                }
                                case INCREASE_SPAWN_TIME:
                                {
                                    (*m_pVec_pFactories)[fact_i]->modify_spawn_time(v_st_hud_button[i].value);
                                    cout << "increase spawn time" << endl;
                                    break;
                                }
                                case UNIT_SPEED_INCREASE:
                                {
                                    (*m_pVec_pFactories)[fact_i]->modify_speed_unit(v_st_hud_button[i].value);
                                    cout << "increase speed" << endl;
                                    break;
                                }
                                case UNIT_SPEED_DECREASE:
                                {
                                    (*m_pVec_pFactories)[fact_i]->modify_speed_unit(v_st_hud_button[i].value);
                                    cout << "decrease speed" << endl;
                                    break;
                                }
                                case DAMAGE_DECREASE:
                                {
                                    (*m_pVec_pFactories)[fact_i]->modify_attack_strength(v_st_hud_button[i].value);
                                    cout << "decrease damage" << endl;
                                    break;
                                }
                                case DAMAGE_INCREASE:
                                {
                                    (*m_pVec_pFactories)[fact_i]->modify_attack_strength(v_st_hud_button[i].value);
                                    cout << "increase damage" << endl;
                                    break;
                                }
                                case HEALTH_DECREASE:
                                {
                                    (*m_pVec_pFactories)[fact_i]->modify_unit_health(v_st_hud_button[i].value);
                                    (*m_pVec_pFactories)[fact_i]->modify_unit_starting_health(v_st_hud_button[i].value);
                                    cout << "decrease health" << endl;
                                    break;
                                }
                                case HEALTH_INCREASE:
                                {
                                    (*m_pVec_pFactories)[fact_i]->modify_unit_health(v_st_hud_button[i].value);
                                    (*m_pVec_pFactories)[fact_i]->modify_unit_starting_health(v_st_hud_button[i].value);
                                    cout << "increase health" << endl;
                                    break;
                                }
                                case RANGE_INCREASE:
                                {
                                    (*m_pVec_pFactories)[fact_i]->modify_range(v_st_hud_button[i].value);
                                    cout << "increase range" << endl;
                                    break;
                                }
                                case RANGE_DECREASE:
                                {
                                    (*m_pVec_pFactories)[fact_i]->modify_range(v_st_hud_button[i].value);
                                    cout << "decreased range" << endl;
                                    break;
                                }
                                case SPLASH_INCREASE:
                                {
                                    (*m_pVec_pFactories)[fact_i]->modify_splash_damage(v_st_hud_button[i].value);
                                    cout << "increase splash damage" << endl;
                                    break;
                                }
                                case SPLASH_DECREASE:
                                {
                                    (*m_pVec_pFactories)[fact_i]->modify_splash_damage(v_st_hud_button[i].value);
                                    cout << "decreased splash damage" << endl;
                                    break;
                                }
                                case SPEED_ROTATION_INCREASE:
                                {
                                    (*m_pVec_pFactories)[fact_i]->modify_turning_speed(v_st_hud_button[i].value);
                                    cout << "increase rotation" << endl;
                                    break;
                                }
                                case SPEED_ROTATION_DECREASE:
                                {
                                    (*m_pVec_pFactories)[fact_i]->modify_turning_speed(v_st_hud_button[i].value);
                                    cout << "decrease rotation" << endl;
                                    break;
                                }
                                case PROJECTILE_SPEED_INCREASE:
                                {
                                    (*m_pVec_pFactories)[fact_i]->modify_projectile_speed(v_st_hud_button[i].value);
                                    cout << "increase projectile speed" << endl;
                                    break;
                                }
                                case PROJECTILE_SPEED_DECREASE:
                                {
                                    (*m_pVec_pFactories)[fact_i]->modify_projectile_speed(v_st_hud_button[i].value);
                                    cout << "decrease projectile speed" << endl;
                                    break;
                                }
                            }
                        }
                    }
                }
            }


	    }
	}
	else m_key_trigger_mouse_button[0]=false;//reset value

	//test if any factories are selected
    bool factory_selected=false;
    for(unsigned int fact_i = 0; fact_i<m_pVec_pFactories->size(); fact_i++)
    {
        if( (*m_pVec_pFactories)[fact_i]->m_selected )
        {
            m_hud_draw_status=hs_for_factory;
            factory_selected=true;
            break;
        }
    }
    if(!factory_selected) m_hud_draw_status=hs_off;

	//test if any units are selected
	if(m_hud_draw_status!=hs_for_factory)
	{
	    bool unit_selected=false;
	    for(unsigned int unit_i = 0; unit_i<m_pVec_pUnits->size(); unit_i++)
	    {
	        if( (*m_pVec_pUnits)[unit_i]->m_selected )
	        {
	            m_hud_draw_status=hs_for_unit;
	            unit_selected=true;
	            break;
	        }
	    }
	    if(!unit_selected) m_hud_draw_status=hs_off;
	}

	//test if there was any hud interaction that will block other selection
	if(m_pMouse_button[0] || m_pMouse_button[0])
	{
	    //was mouse inside the hud area
	    if(m_pMouse_pos[1]>470 && m_hud_draw_status!=hs_off)
	    {
	        hud_interaction=true;
	    }
	}

	return hud_interaction;
}

bool hud::draw()
{
	switch(m_hud_draw_status)
	{
	    case hs_off:
	    {
	        ;//draw nothing
	    }break;

	    case hs_for_unit:
	    {
	        //glPushMatrix();
            glLineWidth(2);
            glColor3f(1, 1, 1);
            glBegin(GL_QUADS);
            glVertex2f(0, 470);
            glVertex2f(1024, 470);
            glVertex2f(1024, 800);
            glVertex2f(0, 800);
            glEnd();
            //glPopMatrix();
	    }break;

	    case hs_for_factory:
	    {
	        //glPushMatrix();
            glLineWidth(2);
            glColor3f(1, 0, 0);
            glBegin(GL_QUADS);
            glVertex2f(0, 470);
            glVertex2f(1024, 470);
            glVertex2f(1024, 800);
            glVertex2f(0, 800);
            glEnd();
            //glPopMatrix();

            //DRAW TIME BAR

            for(unsigned int i = 0; i < v_st_hud_button.size(); i++)
            {
                //glPushMatrix();
                glLineWidth(2);
                glColor3f(v_st_hud_button[i].color1, v_st_hud_button[i].color2, v_st_hud_button[i].color3);
                glBegin(GL_QUADS);
                glVertex2f(v_st_hud_button[i].upper_left.x, v_st_hud_button[i].upper_left.y);
                glVertex2f(v_st_hud_button[i].upper_right.x, v_st_hud_button[i].upper_right.y);
                glVertex2f(v_st_hud_button[i].lower_right.x, v_st_hud_button[i].lower_right.y);
                glVertex2f(v_st_hud_button[i].lower_left.x, v_st_hud_button[i].lower_left.y);
                glEnd();
                //glPopMatrix();
            }
            for(unsigned int i=0; i< written_things_to_print.size();i++)
            {
                written_things_to_print[i].draw_display();
            }
	    }break;
	}

	return true;
}

bool hud::is_mouse_inside_button(const st_pos& m_mouse_pos_curr, st_hud_button& button)
{
    if(m_mouse_pos_curr.x > button.upper_left.x && m_mouse_pos_curr.x < button.upper_right.x &&
       m_mouse_pos_curr.y >button.upper_left.y &&  m_mouse_pos_curr.y <button.lower_left.y)
    {
        return true;
    }

    return false;
}

bool hud::loadfont(void)
{
    texture_font[1] = SOIL_load_OGL_texture
	(
		"default_dark.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);

	texture_font[0] = SOIL_load_OGL_texture
	(
		"default_light.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);

    texture_font[2] = SOIL_load_OGL_texture
	(
		"default_mask.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);

    if(texture_font[0]==0 ||texture_font[1]==0||texture_font[2]==0)
    {
        cout << "ERROR: Could not find the file"<< endl;
        return false;
    }

    return true;
}
