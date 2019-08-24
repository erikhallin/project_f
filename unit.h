#ifndef UNIT_H
#define UNIT_H

#include <iostream>
#include <gl/gl.h>
#include "definitions.h"

using namespace std;

class unit
{
    public:
        unit();
        unit(unit_spec specs);

        bool   m_selected,m_awake,m_have_moved,m_forced_move,m_attack_moving,m_is_idle;
        int    m_attack_mode,m_col_id,m_col_state;
        string m_unit_seen_by_team;
        unit*  m_pUnit_enemy_target;
        float  m_rotation_curr;

        bool init(st_pos pos);
        bool update(void);
        bool draw(void);
        bool is_pos_inside(st_pos pos,float size_factor=1.0,bool test_new_pos=false);
        bool is_pos_inside_rect(st_pos pos_top_left,st_pos pos_low_right);
        bool move_to(st_pos end_pos,bool is_forced=false,float final_rotation=9999,unit* pEnemy_target=0);
        bool move_to_rel(st_pos rel_pos_shift,float move_direction,bool forced_move,float rot_val[4]);
        bool push_unit(st_pos push_dir,bool pushing_cancels_movement);
        st_pos get_curr_pos(void);
        st_pos get_new_pos(void);
        st_pos get_curr_col_square_pos(void);
        float get_curr_rotation(void);
        bool set_curr_col_square_pos(st_pos new_pos);
        bool accept_new_pos(void);
        bool is_moving(void);
        unit_spec get_spec(void);
        bool is_ready_to_attack(void);
        bool take_damage(float damage,float time_delay=0.0);
        bool attack_action_done(void);
        bool aim_to_target(float rotation_target,bool special_attack_order_overide=false);
        bool cancel_movement(void);
        bool spotted_by_team(int team);


    private:

        st_pos m_pos,m_pos_new,m_col_square_pos,m_pos_attack_move_target,m_pos_defence;
        float  m_size,m_rotation_target,m_rotation_target_final;
        float  m_attack_cooldown;
        float  m_rotation_check_timer,m_defence_stay_timer;
        float  m_team_spot_timers[_max_teams];

        //unit spec
        unit_spec m_spec;

        st_pos m_target_pos,m_target_direction;

};

#endif // UNIT_H
