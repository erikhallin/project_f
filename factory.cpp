#include "factory.h"

factory::factory() : m_produce_unit(true), m_spawn_timer(3.0), m_spawn_time(3.0)
{
    //ctor
}

bool factory::init(const st_pos& pos)
{
	m_pos = pos;
	m_size = _fact_size;
	m_selected = false;

	return true;
}

bool factory::update(bool &spawn_unit)
{
	//update time
	if (m_produce_unit)
	{
		m_spawn_timer -= _time_step*0.001;
		if (m_spawn_timer<0.0)
		{
			m_spawn_timer = m_spawn_time;

			spawn_unit = true;
		}
	}

	return true;
}

bool factory::draw(void)
{
	if (m_selected) glColor3f(1.0, 1.0, 0.7);
	else glColor3f(0.8, 0.8, 0.0);

	glBegin(GL_QUADS);
	glVertex2f(m_pos.x - m_size, m_pos.y - m_size);
	glVertex2f(m_pos.x - m_size, m_pos.y + m_size);
	glVertex2f(m_pos.x + m_size, m_pos.y + m_size);
	glVertex2f(m_pos.x + m_size, m_pos.y - m_size);
	glEnd();

	return true;
}

bool factory::is_pos_inside(const st_pos& pos) const
{
	if (m_pos.x + m_size + _unit_size > pos.x && m_pos.x - m_size - _unit_size < pos.x &&
		m_pos.y + m_size + _unit_size > pos.y && m_pos.y - m_size - _unit_size < pos.y)
	{
		return true;
	}

	return false;
}

bool factory::is_pos_inside_rect(const st_pos& pos_top_left,const st_pos& pos_low_right) const
{
	if (pos_top_left.x < m_pos.x && pos_low_right.x > m_pos.x &&
		pos_top_left.y < m_pos.y && pos_low_right.y > m_pos.y)
	{
		return true;
	}

	return false;
}

st_pos factory::get_curr_pos(void)
{
	return m_pos;
}

bool factory::set_spec(const unit_spec& spec)
{
	m_spec = spec;

	return true;
}

unit_spec factory::get_spec(void)
{
	return m_spec;
}


bool factory::modify_spawn_time(float number)
{
    m_spawn_time=m_spawn_time+number;
    return true;
}

bool factory::modify_speed_unit(float number)
{
    m_spec.speed_move=m_spec.speed_move+number;

}

bool factory::modify_attack_strength(float number)

{

    m_spec.damage=m_spec.damage+number;

}

bool factory::modify_unit_health(float number)
{

    m_spec.hp_max=m_spec.hp_max+number;

}

bool factory::modify_unit_starting_health(float number)
 {

     m_spec.hp_curr=m_spec.hp_curr+number;


 }

bool factory::modify_range(float number)
 {
     m_spec.attack_range+=number;
 }

bool factory::modify_attack_speed(float number)
{


}


bool factory::modify_splash_damage(float number)
{

    m_spec.damage_range+=number;

}
bool factory::modify_turning_speed(float number)
{


    m_spec.speed_rotate+=number;
}

bool factory::modify_projectile_speed(float number)
{

    m_spec.projectile_speed+=number;
}

