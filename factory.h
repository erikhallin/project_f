#ifndef FACTORY_H
#define FACTORY_H
#include <windows.h> // necessary for me in visual -Dav

#include <gl/gl.h>
#include "definitions.h"


class factory
{
    public:
        factory();

        bool m_selected;

        bool init(const st_pos& pos);
        bool update(bool &spawn_unit);
        bool draw(void) ;
        bool is_pos_inside(const st_pos& pos) const;
        bool is_pos_inside_rect(const st_pos& pos_top_left, const st_pos& pos_low_right) const;
        st_pos get_curr_pos(void);
        bool set_spec(const unit_spec& spec);
        unit_spec get_spec(void);

        bool modify_speed_unit(float number);
        bool modify_spawn_time(float number);
        bool modify_attack_strength(float number);
        bool modify_unit_health(float number);
        bool modify_unit_starting_health(float number);
        bool modify_range(float number);
        bool modify_attack_speed(float number);
        bool modify_splash_damage(float number);
        bool modify_turning_speed(float number);
        bool modify_projectile_speed(float number);


    private:

        st_pos m_pos;
        bool   m_produce_unit;
        float  m_size;
        float  m_spawn_timer, m_spawn_time;
        unit_spec m_spec;
};

#endif // FACTORY_H
