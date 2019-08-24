#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <iostream>
#include <gl/gl.h>
#include "definitions.h"

using namespace std;

class projectile
{
    public:
        projectile();
        projectile(st_pos start,st_pos end,float speed,float damage,float damage_range,int team_fire=-1);

        bool update(void);
        bool draw(void);

        float  m_damage_range,m_damage;
        st_pos m_pos_start,m_pos_end,m_pos_curr,m_speed;
        int    m_team_fire;
        float  m_life_time,m_angle;

    private:



};

#endif // PROJECTILE_H
