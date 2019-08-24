#include "menu.h"
#include <cassert>

menu::menu() : menu_status(menu_MAIN) { v_menu_buttons.reserve(4); }

void menu::init_texture_buttons(void)
{
	// START BUTTON TEXTURE
	v_menu_buttons[0].texture = SOIL_load_OGL_texture
		(
		"./texture/buttons/start.bmp",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

	// START SELCTED BUTTON TEXTURE
	v_menu_buttons[0].textureS = SOIL_load_OGL_texture
		(
		"./texture/buttons/startS.bmp",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

	// MULTIPLAYER BUTTON TEXTURE
	v_menu_buttons[1].texture = SOIL_load_OGL_texture
		(
		"./texture/buttons/multiplayer.bmp",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

	// MULTIPLAYER SELECTED BUTTON TEXTURE
	v_menu_buttons[1].textureS = SOIL_load_OGL_texture
		(
		"./texture/buttons/multiplayerS.bmp",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

	// OPTIONS BUTTON TEXTURE
	v_menu_buttons[2].texture = SOIL_load_OGL_texture
		(
		"./texture/buttons/options.bmp",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

	// OPTIONS SELECTED BUTTON TEXTURE
	v_menu_buttons[2].textureS = SOIL_load_OGL_texture
		(
		"./texture/buttons/optionsS.bmp",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

	// EXIT BUTTON TEXTURE
	v_menu_buttons[3].texture = SOIL_load_OGL_texture
		(
		"./texture/buttons/exit.bmp",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

	// EXIT SELECTED BUTTON TEXTURE
	v_menu_buttons[3].textureS = SOIL_load_OGL_texture
		(
		"./texture/buttons/exitS.bmp",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

    //multiplayer buttons
    v_mp_buttons[0].texture = SOIL_load_OGL_texture
		(
		"./texture/buttons/host.bmp",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);
    v_mp_buttons[0].textureS = SOIL_load_OGL_texture
		(
		"./texture/buttons/hostS.bmp",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);
    v_mp_buttons[1].texture = SOIL_load_OGL_texture
		(
		"./texture/buttons/join.bmp",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);
    v_mp_buttons[1].textureS = SOIL_load_OGL_texture
		(
		"./texture/buttons/joinS.bmp",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);
}

void menu::init(const int game_width, const int game_height)
{

	m_g_width = game_width;
	m_g_height = game_height;

	const int buttons_width = 200;
	const int buttons_height = 40;

	//in the center of the window
	const int buttons_sx = (game_width / 2) - (buttons_width / 2);
	const int buttons_fx = (game_width / 2) + (buttons_width / 2);

	const int buttons_sy = (game_height / 2) - (buttons_height / 2);
	const int buttons_fy = (game_height / 2) + (buttons_height / 2);


	// sx, fx, sy, fy, colour
	//START BUTTON = 0
	v_menu_buttons.push_back(menu_button(buttons_sx, buttons_fx,
                                         buttons_sy - 200, buttons_fy-200,
                                         0,BUTTON_START));
	// MULTIPLAYER BUTTON = 1
	v_menu_buttons.push_back(menu_button(buttons_sx, buttons_fx,
                                         buttons_sy - 100, buttons_fy - 100,
                                         0,BUTTON_MULTIPLAYER));
	// OPTIONS BUTTON = 2
	v_menu_buttons.push_back(menu_button(buttons_sx, buttons_fx,
                                         buttons_sy, buttons_fy,
                                         0,BUTTON_OPTIONS));
	// EXIT BUTTON = 3
	v_menu_buttons.push_back(menu_button(buttons_sx, buttons_fx,
                                         buttons_sy + 100, buttons_fy + 100,
                                         0,BUTTON_EXIT));

	//Multiplayer buttons
	v_mp_buttons.push_back(menu_button(buttons_sx-200, buttons_fx-200,
                                       buttons_sy - 200, buttons_fy-200,
                                       0,BUTTON_HOST));
	v_mp_buttons.push_back(menu_button(buttons_sx-200, buttons_fx-200,
                                       buttons_sy - 100, buttons_fy - 100,
                                       0,BUTTON_JOIN));

	init_texture_buttons();

}

void menu::drawMain() const
{
	glPushMatrix();
	glLineWidth(1);
	glColor3f(0.38, 0.186, 0.81);
	glBegin(GL_QUADS);
	glVertex2f(0, 0);
	glVertex2f(0, m_g_height);
	glVertex2f(m_g_width, m_g_height);
	glVertex2f(m_g_width, 0);
	glEnd();
	glPopMatrix();
}

void menu::draw(void) const
{
	switch(menu_status)
	{
	    case menu_DEACTIVE:
	    {
	        //nothing to draw
	    }break;

	    case menu_MAIN:
	    {
	        //daw main menu
	        drawMain();

	        //draw buttons
            for(unsigned int i=0;i<v_menu_buttons.size();i++)
            {
                v_menu_buttons[i].draw();
            }
	    }break;

	    case menu_MULTIPLAYER:
	    {
	        //TEMP
	        drawMain();

	        //draw buttons
            for(unsigned int i=0;i<v_mp_buttons.size();i++)
            {
                v_mp_buttons[i].draw();
            }

	    }break;

	    case menu_IN_GAME:
	    {
	        //draw nothing
	    }break;
	}

    return;
}

int menu::update(const st_pos& m_mouse_pos_curr, bool mouse_left_button_pressed)
{
	switch(menu_status)
	{
	    case menu_DEACTIVE:
	    {
	        //nothing
	    }break;

	    case menu_MAIN:
	    {
            for (unsigned int i = 0; i < v_menu_buttons.size(); i++)
            {
                if (v_menu_buttons[i].mouse_is_inside_then_color(m_mouse_pos_curr))
                {
                    if (mouse_left_button_pressed)
                    {
                        // in which button the mouse is?
                        if (v_menu_buttons[i].button_id == BUTTON_START)
                        {
                            menu_status=menu_DEACTIVE;
                            return ret_to_game;
                        }
                        else if (v_menu_buttons[i].button_id == BUTTON_MULTIPLAYER)
                        {
                            cout << "Menu: Go to Multiplayer" << endl;
                            menu_status=menu_MULTIPLAYER;
                            return ret_to_multiplayer;
                        }
                        else if (v_menu_buttons[i].button_id == BUTTON_OPTIONS)
                        {
                            cout << "Menu: Go to Options" << endl;
                            return ret_to_options;
                        }
                        else if (v_menu_buttons[i].button_id == BUTTON_EXIT)
                        {
                            cout << "Menu: Exit the game" << endl;
                            return ret_to_exit;
                        }
                    }
                    break;
                }
            }
	    }break;

	    case menu_MULTIPLAYER:
	    {
            for (unsigned int i = 0; i < v_mp_buttons.size(); i++)
            {
                if (v_mp_buttons[i].mouse_is_inside_then_color(m_mouse_pos_curr))
                {
                    if (mouse_left_button_pressed)
                    {
                        // in which button the mouse is?
                        if (v_mp_buttons[i].button_id == BUTTON_HOST)
                        {
                            menu_status = menu_DEACTIVE;
                            cout << "Menu: Host selected" << endl;
                            return ret_to_host;
                        }
                        else if (v_mp_buttons[i].button_id == BUTTON_JOIN)
                        {
                            menu_status = menu_DEACTIVE;
                            cout << "Menu: Join selected" << endl;
                            return ret_to_join;
                        }
                    }
                    break;
                }
            }
	    }break;

	    case menu_IN_GAME:
	    {
	        //nothing
	    }break;
	}

	return ret_idle;
}
