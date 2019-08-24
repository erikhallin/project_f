#ifndef GAME_H
#define GAME_H

#include <gl/gl.h>
#include <vector>
#include <ctime>
#include "definitions.h"
#include "factory.h"
#include "unit.h"
#include "hud.h"
#include "projectile.h"
#include "networkCom.h"
#include "menu.h"

using namespace std;

enum game_states
{
    gs_menu=0,
    gs_in_game
};

enum ret_states_game
{
    ret_game_idle=0,
    ret_game_host,
    ret_game_join,
    ret_game_exit
};

struct col_square
{
    vector<unit*> vec_pUnits;
};

struct st_client_info
{
    st_client_info(SOCKET _soc)
    {
        soc=_soc;
        ping_list_ptr=0;
    }

    SOCKET soc;
    int ping_list[_ping_list_size];
    int ping_list_ptr;

    int get_ping_curr(void)
    {
        return ping_list[ping_list_ptr];
    }
    void add_ping(int _ping)
    {
        ping_list_ptr++;
        if(ping_list_ptr>=_ping_list_size) ping_list_ptr=0;

        ping_list[ping_list_ptr]=_ping;

        return;
    }
};

class game
{
    public:
        game();

        bool init(int screen_size[2],bool* pKeys,int* pMouse_pos,bool* pMouse_button,networkCom* pNetCom);
        int  update(void);
        bool draw(void);

        //network
        bool recv_data(SOCKET soc_sender);
        bool send_start_package_to_client(SOCKET soc_client);
        bool send_start_package_to_server(void);
        bool send_client_denied_package(SOCKET soc_client);
        bool add_server_player(void);
        bool lost_player(int socket);
        bool lost_server(void);

        //TEMP
        bool setup_network_temp(void);

    private:

        //input
        int    m_window_width,m_window_height;
        int*   m_pMouse_pos;
        bool*  m_pKeys;
        bool*  m_pMouse_button;
        bool   m_key_pressed_left_mouse,m_key_pressed_right_mouse,m_mouse_drag_LMB,m_mouse_drag_RMB;
        bool   m_custom_formation_line_in_progress;
        st_pos m_mouse_start_drag_pos,m_mouse_pos_curr;

        //key input
        bool   m_key_spawn_factory_pressed;//temp
        float  m_double_click_timer;

        //camera
        float m_cam_pos[2];

        //objects
        menu               m_Menu;
        hud                m_Hud;
        vector<factory*>   m_vec_pFactories;
        vector<unit*>      m_vec_pUnits;
        col_square         m_arr_col_squares[_world_width/_col_grid_size][_world_height/_col_grid_size];
        string             m_arr_team_squares[_world_width/_col_grid_size][_world_height/_col_grid_size];
        vector<projectile> m_vec_projectiles;
        vector<st_col_id>  m_vec_col_id;
        vector<st_pos_and_val> m_vec_unit_formation_points;

        //misc
        int   m_game_state;
        float m_game_time_sec;
        int   m_unit_type_counter;
        int   m_col_id_counter;
        int   m_human_player_team;
        bool  m_fog_of_war_enabled;
        bool  m_factory_build_order_in_progress;
        float m_unit_formation_gap_size;

        //fog of war
        int   m_tex_fow_mask;

        //network
        networkCom* m_pNetCom;
        vector<st_client_info> m_vec_client_info;
        float m_ping_timer;
        bool ping_clients(void);

        //functions
        bool load_sound(void);
        bool load_texture(void);
};

#endif // GAME_H
