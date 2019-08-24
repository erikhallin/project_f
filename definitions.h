#ifndef DEF_H
#define DEF_H

#include <math.h>
#include <stdlib.h>

#define _world_width 1000
#define _world_height 1000
#define _col_grid_size 100

const float _version=0.2;
const float _time_step=10;
const float _pi=3.14159265359;
const float _Rad2Deg=57.2957795;
const float _Deg2Rad=0.0174532925;

const int   _mouse_drag_min_value=3;
const float _mouse_double_click_time=0.3;

const int   _max_teams=8;
const float _team_color[10][3]={ {1.0,0.0,0.0},
                                 {0.0,1.0,0.0},
                                 {0.0,0.0,1.0},
                                 {1.0,1.0,0.0},
                                 {1.0,0.0,1.0},
                                 {0.0,1.0,1.0},
                                 {0.5,0.5,0.5},
                                 {1.0,0.5,0.0},
                                 {0.0,1.0,0.5},
                                 {0.5,0.0,1.0} };

//unit values
const float _unit_rotation_speed=2.0;
const float _unit_move_speed=0.5;
const float _unit_size=5.0;
const float _unit_push_speed=0.01;
const float _unit_rotation_check_delay=1.0;
const float _unit_defence_stay_delay=1.0;
const float _unit_spot_delay=5.0;
const float _unit_shield_damage_reduction_factor=0.5;

//factory values
const float _fact_size=20.0;
const float _fact_spawn_gap_min=30.0;
const float _fact_line_of_sight=400;

const float _projectile_speed_default=100.0;
const bool  _control_all_units=false;
const float _custom_line_formation_distance=_unit_size*4;

//network
const float _ping_send_interval=1.0;
const int   _ping_list_size=10;

enum attack_modes
{
    am_none=0,
    am_stand,   //shoots enemies but does not move
    am_defence, //will follow an enemy within view a certain distance
    am_pursuit  //will follow enemies within view
};

enum en_states
{
    none=0,
    passive,
    active
};

struct st_pos
{
    st_pos()
    {
        x=y=0.0;
    }

    st_pos(const st_pos& _input)
    {
        x=_input.x;
        y=_input.y;
    }

    st_pos(const float _x,const float _y)
    {
        x=_x;
        y=_y;
    }

    float x,y;

    st_pos operator=(const st_pos pos2)
    {
        x=pos2.x;
        y=pos2.y;

        return *this;
    }

    bool operator!=(const st_pos pos2)
    {
        return (x!=pos2.x || x!=pos2.x);
    }

    bool operator==(const st_pos pos2)
    {
        return (x==pos2.x && x==pos2.x);
    }

    st_pos operator+(const st_pos pos2)
    {
        st_pos pos_sum;
        pos_sum.x=x+pos2.x;
        pos_sum.y=y+pos2.y;

        return pos_sum;
    }

    st_pos operator/(const float scale)
    {
        st_pos pos_quotient;
        pos_quotient.x=x/scale;
        pos_quotient.y=y/scale;

        return pos_quotient;
    }

    st_pos operator*(const float scale)
    {
        st_pos pos_prod;
        pos_prod.x=x*scale;
        pos_prod.y=y*scale;

        return pos_prod;
    }

    st_pos operator+=(const st_pos pos2)
    {
        x+=pos2.x;
        y+=pos2.y;

        return *this;
    }

    st_pos operator/=(const float scale)
    {
        x/=scale;
        y/=scale;

        return *this;
    }

    float length()
    {
        return sqrt( x*x+y*y );
    }

    float length2()
    {
        return x*x+y*y;
    }

    float distance(const st_pos pos2)
    {
        return sqrt( (x-pos2.x)*(x-pos2.x)+(y-pos2.y)*(y-pos2.y) );
    }

    float distance2(const st_pos pos2)
    {
        return (x-pos2.x)*(x-pos2.x)+(y-pos2.y)*(y-pos2.y);
    }

    st_pos normalize()
    {
        float length=sqrt( x*x+y*y );
        if(length==0) return st_pos(1,0);
        x/=length;
        y/=length;
        return *this;
    }

    st_pos get_rel_pos_to(st_pos _pos)
    {
        return st_pos( x-_pos.x,y-_pos.y );
    }

    float get_angle_to(st_pos _pos)
    {
        st_pos rel_pos=this->get_rel_pos_to(_pos);
        //rel_pos.normalize();

        return atan2f(rel_pos.y,rel_pos.x)*_Rad2Deg;
    }

    /*float get_angle_to_m90(st_pos _pos)
    {
        st_pos rel_pos=this->get_rel_pos_to(_pos);
        rel_pos.normalize();

        return atan2f(-rel_pos.x,rel_pos.y)*_Rad2Deg;
    }*/
};

struct st_pos_and_val
{
    st_pos_and_val()
    {
        value=0;
    }

    st_pos_and_val(st_pos _pos,float _value)
    {
        pos=_pos;
        value=_value;
    }

    st_pos pos;
    float value;
};

struct st_col_id
{
    st_col_id()
    {

    }

    st_col_id(int _id,st_pos _p1,st_pos _p2,st_pos _p3,st_pos _p4)
    {
        id=_id;
        p1=_p1;
        p2=_p2;
        p3=_p3;
        p4=_p4;
    }

    st_col_id operator=(st_col_id _input)
    {
        id=_input.id;
        p1=_input.p1;
        p2=_input.p2;
        p3=_input.p3;
        p4=_input.p4;

        return *this;
    }

    bool is_pos_inside_square(st_pos test_pos)
    {
        //triangle 1 test
        bool b1, b2, b3;

        b1=(test_pos.x-p2.x)*(p1.y-p2.y)-(p1.x-p2.x)*(test_pos.y-p2.y) < 0.0f;
        b2=(test_pos.x-p3.x)*(p2.y-p3.y)-(p2.x-p3.x)*(test_pos.y-p3.y) < 0.0f;
        b3=(test_pos.x-p1.x)*(p3.y-p1.y)-(p3.x-p1.x)*(test_pos.y-p1.y) < 0.0f;

        bool t1=((b1 == b2) && (b2 == b3));

        //triangle 2 test
        b1=(test_pos.x-p4.x)*(p1.y-p4.y)-(p1.x-p4.x)*(test_pos.y-p4.y) < 0.0f;
        b2=(test_pos.x-p3.x)*(p4.y-p3.y)-(p4.x-p3.x)*(test_pos.y-p3.y) < 0.0f;
        b3=(test_pos.x-p1.x)*(p3.y-p1.y)-(p3.x-p1.x)*(test_pos.y-p1.y) < 0.0f;

        bool t2=((b1 == b2) && (b2 == b3));

        return (t1 || t2);
    }

    int id;
    st_pos p1,p2,p3,p4;
};

struct unit_spec
{
    unit_spec()
    {
        type=0;
        team=0;
        damage=10;
        damage_range=0;
        projectile_speed=_projectile_speed_default;
        speed_move=_unit_move_speed;
        speed_rotate=_unit_rotation_speed;
        hp_curr=hp_max=100;
        armour=1;
        attack_range=50;
        attack_speed=0.5;
        aim_angle_tolerance=5;
        view_range=100;
        defence_range=40;
        shield_angle=90;
    }

    unit_spec operator=(const unit_spec _input)
    {
        type=_input.type;
        team=_input.team;
        damage=_input.damage;
        damage_range=_input.damage_range;
        projectile_speed=_input.projectile_speed;
        speed_move=_input.speed_move;
        speed_rotate=_input.speed_rotate;
        hp_curr=_input.hp_curr;
        hp_max=_input.hp_max;
        armour=_input.armour;
        attack_range=_input.attack_range;
        attack_speed=_input.attack_speed;
        aim_angle_tolerance=_input.aim_angle_tolerance;
        view_range=_input.view_range;
        defence_range=_input.defence_range;
        shield_angle=_input.shield_angle;

        return *this;
    }

    int   type;//same type = same specs
    int   team;
    float damage;
    float damage_range;
    float projectile_speed;
    float attack_range;
    float attack_speed;
    float speed_move;
    float speed_rotate;
    float hp_max;
    float hp_curr;
    float armour;
    float aim_angle_tolerance;
    float view_range;
    float defence_range;
    float shield_angle;
};


#endif // DEF_H
