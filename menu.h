#ifndef MENU_H
#define MENU_H

#include <SOIL/soil.h>
#include <gl/gl.h>
#include <vector>
#include <iostream>
#include "definitions.h"

using namespace std;

enum BUTTONS
{
    NO_BUTTON=-1,
    BUTTON_START,
    BUTTON_MULTIPLAYER,
    BUTTON_OPTIONS,
    BUTTON_EXIT,
    BUTTON_HOST,
    BUTTON_JOIN
};

enum menu_STATUS
{
    menu_DEACTIVE = 0,
    menu_MAIN,
    menu_MULTIPLAYER,
    menu_IN_GAME
};

struct menu_button
{
	float sx;//start x - left
	float fx;//final x - right

	float sy;//start y - top
	float fy;//final y - down

	float colour;

	GLuint texture;
	GLuint textureS;
	bool selected;

	int button_id;

	menu_button(const float _sx, const float _fx, const float _sy, const float _fy, const float _colour,int _button_id):
		sx(_sx), fx(_fx), sy(_sy), fy(_fy), selected(false), button_id(_button_id)
	{}

	void draw(void) const
	{
		glEnable(GL_TEXTURE_2D);
		if (selected) { glBindTexture(GL_TEXTURE_2D, textureS); }
		else { glBindTexture(GL_TEXTURE_2D, texture); }
		glPushMatrix();
		glColor3f(1, 1, 1);
		glBegin(GL_POLYGON);

		glTexCoord2f(0, 1);
		glVertex2f(sx, sy);// left,top
		glTexCoord2f(0, 0);
		glVertex2f(sx, fy); // left, down
		glTexCoord2f(1, 0);
		glVertex2f(fx, fy); // right, down
		glTexCoord2f(1, 1);
		glVertex2f(fx, sy);// right,top
		glEnd();
		glPopMatrix();
		glDisable(GL_TEXTURE_2D);

	}
	bool mouse_is_inside_then_color(const st_pos& m_mouse_pos_curr)
	{
		if ((m_mouse_pos_curr.x >= sx && m_mouse_pos_curr.x <= fx) && (m_mouse_pos_curr.y >= sy && m_mouse_pos_curr.y <= fy))
		{
			selected = true;;
			return true;
		}

		selected = false;;
		return false;
	}

};

enum ret_states
{
    ret_idle=0,
    ret_to_game,
    ret_to_multiplayer,
    ret_to_options,
    ret_to_exit,
    ret_to_join,
    ret_to_host
};

class menu
{
    public:
        menu_STATUS menu_status;

                    menu();
        void		init(const int game_width, const int game_height);
        void		draw(void) const;
        int 		update(const st_pos& m_mouse_pos_curr, bool mouse_left_button_pressed);


    private:
        void				drawMain(void) const;
        void				init_texture_buttons(void);
        vector<menu_button> v_menu_buttons;
        vector<menu_button> v_mp_buttons;
        int					m_g_height;
        int					m_g_width;

};

#endif
