#include "game.h"

game::game()
{
    //ctor
}

bool game::init(int screen_size[2],bool* pKeys,int* pMouse_pos,bool* pMouse_button,networkCom* pNetCom)
{
    int seed=time(0);
    //seed=4;//error at start
    cout<<"SEED: "<<seed<<endl;
    srand(seed);

    m_window_width=screen_size[0];
    m_window_height=screen_size[1];
    m_pKeys=pKeys;
    m_pMouse_pos=pMouse_pos;
    m_pMouse_button=pMouse_button;
    m_key_pressed_left_mouse=m_key_pressed_right_mouse=m_mouse_drag_LMB=m_mouse_drag_RMB=false;
    m_key_spawn_factory_pressed=false;
    m_double_click_timer=0;
    m_unit_type_counter=0;
    m_col_id_counter=0;
    m_human_player_team=0;
    m_fog_of_war_enabled=true;
    m_factory_build_order_in_progress=false;
    m_game_time_sec=0;
    m_unit_formation_gap_size=_unit_size*4.0;
    m_custom_formation_line_in_progress=false;
    m_pNetCom=pNetCom;
    m_ping_timer=_ping_send_interval;

    m_cam_pos[0]=0.0;
    m_cam_pos[1]=0.0;

    //init team squares
    int square_x_max=_world_width/_col_grid_size;
    int square_y_max=_world_height/_col_grid_size;
    for(int square_x=0;square_x<square_x_max;square_x++)
    for(int square_y=0;square_y<square_y_max;square_y++)
    {
        m_arr_team_squares[square_x][square_y]=string(_max_teams,'0');
    }

    //load texture
    if(!load_texture())
    {
        cout<<"ERROR: Cold not load textures\n";
        return false;
    }

    //load sound
    if(!load_sound())
    {
        cout<<"ERROR: Cold not load sound\n";
        return false;
    }

    //add unit temp
    for(int i=0;i<1000;i++)
    {
        int pos_x=rand()%_world_width;
        int pos_y=rand()%_world_height;

        //spawn 2 teams
        int team=-1;
        if(pos_x<_world_width*0.4) team=0;
        else if(pos_x>_world_width*0.6) team=1;
        if(team==-1) continue;
        unit_spec specs;
        specs.team=team;

        /*//spawn 2 units
        unit_spec specs;
        specs.team=i;*/

        m_vec_pUnits.push_back( new unit(specs) );
        m_vec_pUnits.back()->init( st_pos(pos_x,pos_y) );

        //place in col square
        int col_square_x=pos_x/_col_grid_size;
        int col_square_y=pos_y/_col_grid_size;
        m_arr_col_squares[col_square_x][col_square_y].vec_pUnits.push_back( m_vec_pUnits.back() );
        m_vec_pUnits.back()->set_curr_col_square_pos( st_pos(col_square_x,col_square_y) );
    }

    /*m_vec_pUnits.push_back( new unit() );
    m_vec_pUnits.back()->init( st_pos(80,50) );
    m_vec_pUnits.push_back( new unit() );
    m_vec_pUnits.back()->init( st_pos(100,60) );
    m_vec_pUnits.push_back( new unit() );
    m_vec_pUnits.back()->init( st_pos(113,45) );
    m_vec_pUnits.push_back( new unit() );
    m_vec_pUnits.back()->init( st_pos(158,27) );*/

    //add factory temp
    m_vec_pFactories.push_back( new factory() );
    m_vec_pFactories.back()->init( st_pos(150,70) );
    unit_spec factory_specs=m_vec_pFactories.back()->get_spec();
    factory_specs.type=m_unit_type_counter;
    m_unit_type_counter++;

    //init HUD
    m_Hud.init(m_window_width,m_window_height,pKeys,pMouse_pos,pMouse_button,
               m_cam_pos,&m_vec_pFactories,&m_vec_pUnits,&m_game_time_sec);

    //init menu
    m_Menu.init(m_window_width, m_window_height);

    m_game_state=gs_menu;

    return true;
}

int game::update(void)
{
    //mouse input
    m_mouse_pos_curr.x=m_pMouse_pos[0]+m_cam_pos[0];
    m_mouse_pos_curr.y=m_pMouse_pos[1]+m_cam_pos[1];

    //ping clients, if server
    if(m_pNetCom->get_status()==net_server)
    {
        if(m_ping_timer>0)
        {
            m_ping_timer-=_time_step*0.001;
            if(m_ping_timer<=0)
            {
                //send ping
                ping_clients();
            }
        }
    }


    switch(m_game_state)
    {
        case gs_menu:
        {
            //update menu
            int ret_val=m_Menu.update(m_mouse_pos_curr, (!m_key_pressed_left_mouse && m_pMouse_button[0]));
            switch(ret_val)
            {
                case ret_idle:
                {
                    ;//do nothing
                }break;

                case ret_to_game:
                {
                    m_game_state=gs_in_game;
                }break;

                case ret_to_multiplayer:
                {
                    /*//set up LAN com
                    int status=0;
                    string ip="127.0.0.1";
                    setup_network_temp(status,ip);*/

                }break;

                case ret_to_host:
                {
                    return ret_game_host;
                }break;

                case ret_to_join:
                {
                    return ret_game_join;
                }break;

                case ret_to_options:
                {
                    ;//do nothing so far
                }break;

                case ret_to_exit:
                {
                    cout<<"Exit the game\n";
                    return ret_game_exit;//exit the game
                }break;

            }

        }break;

        case gs_in_game:
        {
            //time update
            m_game_time_sec+=_time_step*0.001;
            if(m_double_click_timer>0) m_double_click_timer-=_time_step*0.001;

            //square grid size
            int square_x_max=_world_width/_col_grid_size;
            int square_y_max=_world_height/_col_grid_size;

            //update input
            float cam_sens=3.0;
            if(m_pKeys[37])
            {
                m_cam_pos[0]-=cam_sens;
            }
            if(m_pKeys[38])
            {
                m_cam_pos[1]-=cam_sens;
            }
            if(m_pKeys[39])
            {
                m_cam_pos[0]+=cam_sens;
            }
            if(m_pKeys[40])
            {
                m_cam_pos[1]+=cam_sens;
            }

            //change human player team (TEMP)
            {
                if(m_pKeys[48]) m_fog_of_war_enabled=true;
                if(m_pKeys[49]) m_human_player_team=0;
                if(m_pKeys[50]) m_human_player_team=1;
                if(m_pKeys[51]) m_human_player_team=2;
                if(m_pKeys[52]) m_human_player_team=3;
                if(m_pKeys[53]) m_human_player_team=4;
                if(m_pKeys[54]) m_human_player_team=5;
                if(m_pKeys[55]) m_human_player_team=6;
                if(m_pKeys[56]) m_human_player_team=7;
                if(m_pKeys[220]) m_fog_of_war_enabled=false;
            }

            //mouse input
            m_mouse_pos_curr.x=m_pMouse_pos[0]+m_cam_pos[0];
            m_mouse_pos_curr.y=m_pMouse_pos[1]+m_cam_pos[1];

            //spawn factory
            if(m_pKeys[70])//f
            {
                if(!m_key_spawn_factory_pressed)
                {
                    m_key_spawn_factory_pressed=true;

                    //enable factory build order
                    m_factory_build_order_in_progress=true;
                }
            }
            else m_key_spawn_factory_pressed=false;

            //update HUD
            bool hud_interaction=m_Hud.update();

            //cout<<"MB: "<<m_pMouse_button[0]<<"\t"<<m_pMouse_button[1]<<"\t"<<m_pMouse_button[2]<<"\t"<<m_pMouse_button[3]<<"\n";

            //left mouse button
            if(m_pMouse_button[0])
            {
                if(!m_key_pressed_left_mouse)
                {
                    m_key_pressed_left_mouse=true;
                    m_mouse_start_drag_pos=m_mouse_pos_curr;//remember startpos if starting to drag

                    if(!hud_interaction)
                    {
                        if(m_factory_build_order_in_progress)
                        {
                            //spawn a factory
                            bool spawn_factory=true;

                            //test if near world edge
                            st_pos spawn_pos=m_mouse_pos_curr;
                            if( spawn_pos.x<_fact_size*2.0 || spawn_pos.x>_world_width-_fact_size*2.0 ||
                                spawn_pos.y<_fact_size*2.0 || spawn_pos.y>_world_height-_fact_size*2.0 )
                            {
                                spawn_factory=false;
                            }

                            //test if near other factory
                            if(spawn_factory)
                            {
                                //calc requested area, factory size + 1.5 unit size
                                st_pos pos_top_left( spawn_pos.x-_fact_size*2.0-_fact_spawn_gap_min,
                                                     spawn_pos.y-_fact_size*2.0-_fact_spawn_gap_min );
                                st_pos pos_low_right( spawn_pos.x+_fact_size*2.0+_fact_spawn_gap_min,
                                                      spawn_pos.y+_fact_size*2.0+_fact_spawn_gap_min );

                                for(unsigned int fact_i=0;fact_i<m_vec_pFactories.size();fact_i++)
                                {
                                    if( m_vec_pFactories[fact_i]->is_pos_inside_rect(pos_top_left,pos_low_right) )
                                    {
                                        spawn_factory=false;
                                        break;
                                    }
                                }

                                //spawn factory
                                if(spawn_factory)
                                {
                                    m_vec_pFactories.push_back( new factory() );
                                    m_vec_pFactories.back()->init( spawn_pos );
                                    int team=rand()%_max_teams;
                                    unit_spec new_spec;
                                    new_spec.team=team;
                                    new_spec.type=m_unit_type_counter;
                                    m_unit_type_counter++;
                                    m_vec_pFactories.back()->set_spec(new_spec);

                                    m_factory_build_order_in_progress=false;
                                }
                            }
                        }


                        //double click test of LMB
                        if(m_double_click_timer>0)
                        {
                            m_double_click_timer=0.0;//ignore triple click

                            //unit selection test
                            int selected_unit_type=-1;
                            for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
                            {
                                if(selected_unit_type==-1)//not found yet
                                {
                                    if( m_vec_pUnits[unit_i]->is_pos_inside(m_mouse_pos_curr) )
                                    {
                                        //can only select your units
                                        if(_control_all_units || m_vec_pUnits[unit_i]->get_spec().team==m_human_player_team)
                                        {
                                            selected_unit_type=m_vec_pUnits[unit_i]->get_spec().type;
                                            break;
                                        }
                                    }
                                }
                            }
                            //select all unit of same type, if on the screen
                            if(selected_unit_type!=-1)
                            {
                                //calc screen edge pos
                                st_pos screen_top_left(m_cam_pos[0],m_cam_pos[1]);
                                st_pos screen_low_right(m_cam_pos[0]+m_window_width,m_cam_pos[1]+m_window_height);

                                for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
                                {
                                    if(m_vec_pUnits[unit_i]->get_spec().type==selected_unit_type &&
                                      (_control_all_units || m_vec_pUnits[unit_i]->get_spec().team==m_human_player_team) )//not found yet
                                    {
                                        //test if on screen
                                        if(m_vec_pUnits[unit_i]->is_pos_inside_rect(screen_top_left,screen_low_right))
                                        {
                                            m_vec_pUnits[unit_i]->m_selected=true;
                                        }
                                        else m_vec_pUnits[unit_i]->m_selected=false;
                                    }
                                    else m_vec_pUnits[unit_i]->m_selected=false;
                                }
                            }
                        }
                        else//single click
                        {
                            //start double click timer
                            m_double_click_timer=_mouse_double_click_time;

                            //click test
                            bool found_object=false;
                            for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
                            {
                                if(!found_object)
                                {
                                    if( m_vec_pUnits[unit_i]->is_pos_inside(m_mouse_pos_curr) )
                                    {
                                        //can only select your units
                                        if(_control_all_units || m_vec_pUnits[unit_i]->get_spec().team==m_human_player_team)
                                        {
                                            m_vec_pUnits[unit_i]->m_selected=true;
                                            found_object=true;
                                        }
                                    }
                                    else m_vec_pUnits[unit_i]->m_selected=false;
                                }
                                else m_vec_pUnits[unit_i]->m_selected=false;
                            }

                            for(unsigned int fact_i=0;fact_i<m_vec_pFactories.size();fact_i++)
                            {
                                if(!found_object)
                                {
                                    if( m_vec_pFactories[fact_i]->is_pos_inside(m_mouse_pos_curr) )
                                    {
                                        //can only select your factories
                                        if(_control_all_units || m_vec_pFactories[fact_i]->get_spec().team==m_human_player_team)
                                        {
                                            m_vec_pFactories[fact_i]->m_selected=true;
                                            found_object=true;
                                        }
                                    }
                                    else m_vec_pFactories[fact_i]->m_selected=false;
                                }
                                else m_vec_pFactories[fact_i]->m_selected=false;
                            }
                        }
                    }

                }
                else if(!m_mouse_drag_LMB && !hud_interaction)//holding down
                {
                    //test if pos dif is larger than trigger value
                    if( m_mouse_pos_curr.x-_mouse_drag_min_value > m_mouse_start_drag_pos.x ||
                        m_mouse_pos_curr.x+_mouse_drag_min_value < m_mouse_start_drag_pos.x ||
                        m_mouse_pos_curr.y-_mouse_drag_min_value > m_mouse_start_drag_pos.y ||
                        m_mouse_pos_curr.y+_mouse_drag_min_value < m_mouse_start_drag_pos.y )
                    {
                        m_mouse_drag_LMB=true;
                    }
                }
            }
            else//mouse not pressed
            {
                //release button test
                if(m_key_pressed_left_mouse)
                {
                    //LMB was just released

                    //test if mouse was dragged
                    if(m_mouse_drag_LMB)
                    {
                        m_mouse_drag_LMB=false;

                        //selection test
                        st_pos pos_top_left=m_mouse_pos_curr;
                        st_pos pos_low_right=m_mouse_pos_curr;
                        if(m_mouse_start_drag_pos.x<pos_top_left.x)
                         pos_top_left.x=m_mouse_start_drag_pos.x;
                        if(m_mouse_start_drag_pos.x>pos_low_right.x)
                         pos_low_right.x=m_mouse_start_drag_pos.x;
                        if(m_mouse_start_drag_pos.y<pos_top_left.y)
                         pos_top_left.y=m_mouse_start_drag_pos.y;
                        if(m_mouse_start_drag_pos.y>pos_low_right.y)
                         pos_low_right.y=m_mouse_start_drag_pos.y;

                        //select unit
                        for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
                        {
                            if( m_vec_pUnits[unit_i]->is_pos_inside_rect(pos_top_left,pos_low_right) )
                            {
                                //only select your own units
                                if(_control_all_units || m_vec_pUnits[unit_i]->get_spec().team==m_human_player_team)
                                 m_vec_pUnits[unit_i]->m_selected=true;
                            }
                            else m_vec_pUnits[unit_i]->m_selected=false;
                        }

                        //unselect factories
                        for(unsigned int fact_i=0;fact_i<m_vec_pFactories.size();fact_i++)
                        {
                            m_vec_pFactories[fact_i]->m_selected=false;
                        }
                    }
                }

                m_key_pressed_left_mouse=false;
            }

            //right mouse button
            if(m_pMouse_button[1] && !m_pMouse_button[0])
            {
                if(!m_key_pressed_right_mouse)
                {
                    m_key_pressed_right_mouse=true;
                    m_mouse_start_drag_pos=m_mouse_pos_curr;//remember startpos if starting to drag

                    //precision move (old)
                    //test if any units are selected
                    /*for(int unit_i=0;unit_i<(int)m_vec_pUnits.size();unit_i++)
                    {
                        if(m_vec_pUnits[unit_i]->m_selected)
                        {
                            //send move order to that unit
                            m_vec_pUnits[unit_i]->move_to(m_mouse_pos_curr);
                        }
                    }*/
                }
                else if(!m_mouse_drag_RMB)//holding down
                {
                    //test if pos dif is larger than trigger value
                    if( m_mouse_pos_curr.x-_mouse_drag_min_value > m_mouse_start_drag_pos.x ||
                        m_mouse_pos_curr.x+_mouse_drag_min_value < m_mouse_start_drag_pos.x ||
                        m_mouse_pos_curr.y-_mouse_drag_min_value > m_mouse_start_drag_pos.y ||
                        m_mouse_pos_curr.y+_mouse_drag_min_value < m_mouse_start_drag_pos.y )
                    {
                        m_mouse_drag_RMB=true;
                    }
                }

                //test if custom line formation (S key)
                if(m_pKeys[83] && m_mouse_drag_RMB)
                {
                    if(!m_custom_formation_line_in_progress)
                    {
                        m_custom_formation_line_in_progress=true;

                        //init, remember start pos
                        m_vec_unit_formation_points.clear();
                        m_vec_unit_formation_points.push_back( st_pos_and_val(m_mouse_pos_curr,0) );
                    }
                    else//record the current pos?
                    {
                        //have the mouse moved enough to record the next pos
                        if( m_vec_unit_formation_points.back().pos.distance( m_mouse_pos_curr ) > _custom_line_formation_distance )
                        {
                            //test if enough units selected
                            int unit_selected_counter=0;
                            for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
                            {
                                if(m_vec_pUnits[unit_i]->m_selected) unit_selected_counter++;
                            }

                            if( unit_selected_counter>=(int)m_vec_unit_formation_points.size() )
                            {
                                //calc angle perpendicular to the last pos
                                float angle=m_mouse_pos_curr.get_angle_to( m_vec_unit_formation_points.back().pos )-90;

                                //shift the previous pos to the center between that and the current pos
                                st_pos unit_pos( m_vec_unit_formation_points.back().pos.x+0.5*
                                                (m_vec_unit_formation_points.back().pos.x-m_mouse_pos_curr.x),
                                                 m_vec_unit_formation_points.back().pos.y+0.5*
                                                (m_vec_unit_formation_points.back().pos.y-m_mouse_pos_curr.y) );
                                m_vec_unit_formation_points.back()=st_pos_and_val( unit_pos,angle );

                                //save the current pos for angle calculations for the next pos
                                m_vec_unit_formation_points.push_back( st_pos_and_val(m_mouse_pos_curr,0) );
                            }
                        }
                    }
                }

                //scroll test
                if(m_pMouse_button[2] || m_pMouse_button[3])
                {
                    //change unit formation gap size
                    float change_value=1.0;
                    if(m_pMouse_button[2]) m_unit_formation_gap_size+=change_value;
                    else                   m_unit_formation_gap_size-=change_value;
                    if(m_unit_formation_gap_size<_unit_size*2.0) m_unit_formation_gap_size=_unit_size*2.0;

                    //cout<<m_unit_formation_gap_size<<endl;
                }
            }
            else//RMB not clicked
            {
                //release button test
                if(m_key_pressed_right_mouse)
                {
                    //RMB was just released

                    //unit move order with specific rotaion
                    bool attack_move=!m_pKeys[65];//if a not pressed
                    //find general unit pos and rotation
                    st_pos avg_unit_pos;
                    float avg_rot=0;
                    int unit_selected_counter=0;
                    for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
                    {
                        if(m_vec_pUnits[unit_i]->m_selected)
                        {
                            unit_selected_counter++;
                            avg_unit_pos+=m_vec_pUnits[unit_i]->get_curr_pos();
                            avg_rot+=m_vec_pUnits[unit_i]->get_curr_rotation();
                        }
                    }
                    if(unit_selected_counter>0)
                    {
                        avg_unit_pos/=float(unit_selected_counter);
                        avg_rot/=float(unit_selected_counter);
                    }


                    //test if mouse was dragged
                    if(m_mouse_drag_RMB)
                    {
                        m_mouse_drag_RMB=false;

                        //if unit custom line formation is ordered, cancel the move order below
                        if(!m_custom_formation_line_in_progress)
                        {
                            //find selected units and calc new pos and rotation

                            //front line formation, place units along the drawn line
                            st_pos front_direction(m_mouse_pos_curr.x-m_mouse_start_drag_pos.x,m_mouse_pos_curr.y-m_mouse_start_drag_pos.y);
                            float front_length=front_direction.length();
                            front_direction/=front_length;//normalize
                            //float gap_size=_unit_size*4.0;//one unit in between
                            float gap_size=m_unit_formation_gap_size;
                            int units_per_row=int(front_length/gap_size);
                            st_pos back_row_direction(-front_direction.y,front_direction.x);
                            //cout<<"front: "<<front_direction.x<<", "<<front_direction.y<<"\t"<<"back: "<<back_row_direction.x<<", "<<back_row_direction.y<<endl;
                            float rotation_target_final=atan2f(-front_direction.x,front_direction.y)*_Rad2Deg;
                            //reset custom gap size
                            m_unit_formation_gap_size=_unit_size*4.0;

                            //place units
                            int units_on_row_counter=0;
                            int row_counter=0;
                            st_pos unit_pos(m_mouse_start_drag_pos);
                            st_pos front_row_pos_left(unit_pos);
                            st_pos front_row_pos_right(unit_pos);
                            st_pos last_row_pos_left(unit_pos);
                            st_pos last_row_pos_right(unit_pos);
                            last_row_pos_left+=(back_row_direction*gap_size);
                            for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
                            {
                                if(m_vec_pUnits[unit_i]->m_selected)
                                {
                                    //send move order to that unit, force movement
                                    m_vec_pUnits[unit_i]->move_to( unit_pos,true,rotation_target_final );

                                    //shift pos for next unit
                                    units_on_row_counter++;
                                    if(units_on_row_counter>units_per_row)
                                    {
                                        //go to new row
                                        row_counter++;
                                        units_on_row_counter=0;
                                        unit_pos=st_pos(m_mouse_start_drag_pos.x+back_row_direction.x*gap_size*(float)row_counter,
                                                        m_mouse_start_drag_pos.y+back_row_direction.y*gap_size*(float)row_counter);

                                        //remember pos
                                        last_row_pos_left=unit_pos;
                                    }
                                    else//move along the row
                                    {
                                        unit_pos.x+=front_direction.x*gap_size;
                                        unit_pos.y+=front_direction.y*gap_size;
                                    }

                                    //remember pos
                                    if(row_counter==0) front_row_pos_right=unit_pos;
                                }
                            }

                            //calc col_id
                            last_row_pos_right=st_pos( last_row_pos_left+(front_direction*gap_size*(float)row_counter) );
                            m_col_id_counter++;
                            if(m_col_id_counter>1000000) m_col_id_counter=1;//cap
                            m_vec_col_id.push_back( st_col_id( m_col_id_counter,
                                                               front_row_pos_left,
                                                               last_row_pos_left,
                                                               last_row_pos_right,
                                                               front_row_pos_right ) );
                            //inform the units about col_id
                            for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
                            {
                                if(m_vec_pUnits[unit_i]->m_selected)
                                {
                                    m_vec_pUnits[unit_i]->m_col_id=m_col_id_counter;
                                    m_vec_pUnits[unit_i]->m_col_state=active;
                                }
                            }


                            /*//calc general move direction (from the mouse pos where the drag started)
                            st_pos rel_pos(m_mouse_start_drag_pos.x-avg_unit_pos.x,m_mouse_start_drag_pos.y-avg_unit_pos.y);
                            st_pos rel_drag_pos(m_mouse_start_drag_pos.x-m_mouse_pos_curr.x,m_mouse_start_drag_pos.y-m_mouse_pos_curr.y);
                            rel_drag_pos.normalize();
                            float rotation_target=atan2f(rel_drag_pos.y,rel_drag_pos.x)*_Rad2Deg;
                            float rotation_rel=(avg_rot-rotation_target)-180.0;
                            float rot_val[4]={m_mouse_start_drag_pos.x,m_mouse_start_drag_pos.y,
                                              cosf(rotation_rel*_Deg2Rad),sinf(rotation_rel*_Deg2Rad)};
                            //

                            //inform units
                            for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
                            {
                                if(m_vec_pUnits[unit_i]->m_selected)
                                {
                                    //send move order to that unit
                                    m_vec_pUnits[unit_i]->move_to_rel(rel_pos,rotation_target,!attack_move,rot_val);
                                }
                            }*/

                            //cout<<"Unit move, with rotation: "<<rotation_rel<<endl;

                        }

                    }
                    else//no drag, general move without specific rotation
                    {
                        //test if an enemy unit was marked as target
                        bool attack_order=false;
                        unit* pUnit_to_attack=0;
                        for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
                        {
                            if( m_vec_pUnits[unit_i]->is_pos_inside(m_mouse_pos_curr) )
                            {
                                //test if that is an enemy
                                if(m_vec_pUnits[unit_i]->get_spec().team!=m_human_player_team)
                                {
                                    //test if that unit is vivible for the human player, or FOW off
                                    if(m_vec_pUnits[unit_i]->m_unit_seen_by_team[m_human_player_team]=='1' || !m_fog_of_war_enabled)
                                    {
                                        attack_order=true;
                                        pUnit_to_attack=m_vec_pUnits[unit_i];
                                    }
                                }
                            }
                        }
                        if(attack_order)
                        {
                            //all selected units should move towards that units pos
                            for(int unit_i=0;unit_i<(int)m_vec_pUnits.size();unit_i++)
                            {
                                if(m_vec_pUnits[unit_i]->m_selected)
                                {
                                    //send move order to that unit
                                    m_vec_pUnits[unit_i]->move_to(m_mouse_pos_curr,true,9999,pUnit_to_attack);
                                }
                            }
                        }
                        //move order
                        else
                        {
                            //calc general move direction (from the mouse pos where the drag started)
                            st_pos rel_pos(m_mouse_start_drag_pos.x-avg_unit_pos.x,m_mouse_start_drag_pos.y-avg_unit_pos.y);
                            float rotation_target=atan2f(rel_pos.y,rel_pos.x)*_Rad2Deg;
                            float rot_val[4]={0,0,0,0};//no specific rotation
                            //inform units
                            for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
                            {
                                if(m_vec_pUnits[unit_i]->m_selected)
                                {
                                    //send move order to that unit
                                    m_vec_pUnits[unit_i]->move_to_rel(rel_pos,rotation_target,!attack_move,rot_val);
                                }
                            }
                        }
                    }
                }

                m_key_pressed_right_mouse=false;
            }

            //custom unit line formation move done
            if(m_custom_formation_line_in_progress && (!m_pKeys[83] || !m_mouse_drag_RMB) )
            {
                m_custom_formation_line_in_progress=false;

                //if just the S key was released, cancel custom line order
                if(!m_pKeys[83] && m_mouse_drag_RMB)
                {
                    m_vec_unit_formation_points.clear();
                    //will cancel the custom line move order and will go to normal formation order
                }

                //if any points in vector, give order
                if( (int)m_vec_unit_formation_points.size()>1 )
                {
                    bool attack_move=!m_pKeys[65];//if a not pressed
                    int unit_to_move_i=-1;
                    //move first units in unit vector to the selected pos
                    for(unsigned int pos_i=0;pos_i<m_vec_unit_formation_points.size()-1;pos_i++)
                    {
                        //find a selected unit
                        bool unit_found=false;
                        for(unsigned int unit_i=unit_to_move_i+1;unit_i<m_vec_pUnits.size();unit_i++)
                        {
                            if(m_vec_pUnits[unit_i]->m_selected)
                            {
                                unit_found=true;

                                //order unit to move
                                m_vec_pUnits[unit_i]->move_to( m_vec_unit_formation_points[pos_i].pos,
                                                               !attack_move,
                                                               m_vec_unit_formation_points[pos_i].value );
                                unit_to_move_i=unit_i;//start with the next unit for the next pos
                                break;
                            }
                        }
                        if(!unit_found) break;//selected units have been lost, skip the rest of the points
                    }

                    //restore buttons to avoid movement of the rest of the selected units

                }

                m_vec_unit_formation_points.clear();
            }

            //stop unit movement
            if(m_pKeys[32])
            {
                //find selected units
                for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
                {
                    if(m_vec_pUnits[unit_i]->m_selected)
                    {
                        //cancel the movement of that unit
                        m_vec_pUnits[unit_i]->cancel_movement();
                    }
                }
            }

            //set unit attack mode
            if(m_pKeys[81])//q, pursuit mode
            {
                //find selected units
                for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
                {
                    if(m_vec_pUnits[unit_i]->m_selected) m_vec_pUnits[unit_i]->m_attack_mode=am_pursuit;
                }
            }
            if(m_pKeys[87])//w, defence mode
            {
                //find selected units
                for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
                {
                    if(m_vec_pUnits[unit_i]->m_selected) m_vec_pUnits[unit_i]->m_attack_mode=am_defence;
                }
            }
            if(m_pKeys[69])//e, stand mode
            {
                //find selected units
                for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
                {
                    if(m_vec_pUnits[unit_i]->m_selected) m_vec_pUnits[unit_i]->m_attack_mode=am_stand;
                }
            }

            /*//reset units seen string ( done inside units.update() )
            for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
            {
                m_vec_pUnits[unit_i]->m_unit_seen_by_team=string(_max_teams,'0');
            }*/

            //update team squares
            for(int square_x=0;square_x<square_x_max;square_x++)
            for(int square_y=0;square_y<square_y_max;square_y++)
            {
                //reset team flags
                m_arr_team_squares[square_x][square_y]=string(_max_teams,'0');

                //test if square is empty of units
                if( m_arr_col_squares[square_x][square_y].vec_pUnits.empty() )
                {
                    ;//nothing, already reseted to 0
                }
                else//test which teams that are present
                {
                    for(unsigned int unit_i=0;unit_i<m_arr_col_squares[square_x][square_y].vec_pUnits.size();unit_i++)
                    {
                        m_arr_team_squares[square_x][square_y].at(
                         m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i]->get_spec().team )='1';
                    }
                }
            }

            //update projectiles
            for(unsigned int proj_i=0;proj_i<m_vec_projectiles.size();proj_i++)
            {
                if( m_vec_projectiles[proj_i].update() )
                {
                    //damage test, test if any target close (circle test)
                    //cout<<"Projectile reached target\n";

                    //calc square pos
                    int col_square_x=m_vec_projectiles[proj_i].m_pos_end.x/_col_grid_size;
                    int col_square_y=m_vec_projectiles[proj_i].m_pos_end.y/_col_grid_size;
                    //cap grid pos, if unit temp outside world limit
                    if(col_square_x<0) col_square_x=0;
                    else if(col_square_x>_world_width/_col_grid_size-1) col_square_x=_world_width/_col_grid_size-1;
                    if(col_square_y<0) col_square_y=0;
                    else if(col_square_y>_world_height/_col_grid_size-1) col_square_y=_world_height/_col_grid_size-1;

                    //calc square range
                    float range_px=m_vec_projectiles[proj_i].m_damage_range;
                    if(range_px<=0) range_px=1;//1 pix min dist
                    float range_px2=range_px*range_px;
                    int range_squares=range_px/_col_grid_size+1;
                    st_pos center_square(col_square_x,col_square_y);

                    for(int square_x=(int)center_square.x-range_squares;
                        square_x<square_x_max && square_x<=(int)center_square.x+range_squares; square_x++)
                    for(int square_y=(int)center_square.y-range_squares;
                        square_y<square_y_max && square_y<=(int)center_square.y+range_squares; square_y++)
                    {
                        if(square_x<0 || square_y<0 ) continue;//outside world

                        //enemy test, turn off for friendly fire
                        /*enemy_in_square=false;
                        for(int team_i=0;team_i<_max_teams;team_i++)
                        {
                            if(team_i==m_vec_projectiles[proj_i].team_fire) continue;

                            if(m_arr_team_squares[square_x][square_y].at(team_i)!='0')
                            {
                                //cout<<" enemy found\n";
                                enemy_in_square=true;
                                break;
                            }
                        }*/

                        //if(enemy_in_square)//if not, skip this ant test other squares
                         for(unsigned int unit_i1=0;unit_i1<m_arr_col_squares[square_x][square_y].vec_pUnits.size();unit_i1++)
                        {
                            //team test, OFF
                            //if(m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_spec().team!=specs.team)
                            {
                                //other team found, test dist
                                float dist2=m_vec_projectiles[proj_i].m_pos_end.distance2(
                                      m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_curr_pos() );
                                if(range_px2>dist2)
                                {
                                    //cout<<"Damage caused to unit\n";
                                    float damage=m_vec_projectiles[proj_i].m_damage;

                                    //shield test
                                    float unit_shield_angle=m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_spec().shield_angle;
                                    if(unit_shield_angle>0)
                                    {
                                        float projectile_angle=m_vec_projectiles[proj_i].m_angle;
                                        //is the projectile inside the unit, use inverted projectile angle
                                        if(_unit_size*_unit_size>dist2)
                                        {
                                            projectile_angle+=180;
                                            if(projectile_angle>180) projectile_angle-=360;
                                        }
                                        else//calc angle between the unit and the projectile
                                        {
                                            projectile_angle=m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_curr_pos()
                                                              .get_angle_to(m_vec_projectiles[proj_i].m_pos_end );
                                        }

                                        //shield angle test
                                        if( fabs(m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->m_rotation_curr-projectile_angle) < unit_shield_angle )
                                        {
                                            damage*=_unit_shield_damage_reduction_factor;
                                            //cout<<"+shield angle effect\n";
                                        }
                                        //else cout<<"-not shield\n";
                                    }


                                    //within damage range, cause damage to unit
                                    m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->take_damage(damage);
                                }
                            }
                        }
                    }

                    /*//remove projectile
                    m_vec_projectiles.erase( m_vec_projectiles.begin()+proj_i );
                    proj_i--;*/

                    //new remove, copy last element to current and pop back
                    m_vec_projectiles[proj_i]=m_vec_projectiles.back();
                    m_vec_projectiles.pop_back();
                    proj_i--;
                }
            }

            //update factories
            bool spawn_unit=false;
            for(unsigned int fact_i=0;fact_i<m_vec_pFactories.size();fact_i++)
            {
                m_vec_pFactories[fact_i]->update(spawn_unit);

                if(spawn_unit)
                {
                    spawn_unit=false;

                    //spawn new unit
                    //cout<<"Factory spawned new unit\n";
                    st_pos fact_pos=m_vec_pFactories[fact_i]->get_curr_pos();
                    m_vec_pUnits.push_back( new unit( m_vec_pFactories[fact_i]->get_spec() ) );
                    int pos_x=fact_pos.x+rand()%int(_fact_size*2.0)-_fact_size;
                    int pos_y=fact_pos.y+rand()%int(_fact_size*2.0)-_fact_size;
                    if(pos_x==fact_pos.x && pos_y==fact_pos.y)
                    {
                        pos_y+=_fact_size*0.5;
                        pos_x+=2.0;//slightly diagonal
                    }
                    m_vec_pUnits.back()->init( st_pos(pos_x,pos_y) );

                    //place in col square
                    int col_square_x=pos_x/_col_grid_size;
                    int col_square_y=pos_y/_col_grid_size;
                    m_arr_col_squares[col_square_x][col_square_y].vec_pUnits.push_back( m_vec_pUnits.back() );
                    m_vec_pUnits.back()->set_curr_col_square_pos( st_pos(col_square_x,col_square_y) );
                }

                //update line of sight for the factory, can any enemy units be seen
                int factory_team=m_vec_pFactories[fact_i]->get_spec().team;
                st_pos center_square=m_vec_pFactories[fact_i]->get_curr_pos()/_col_grid_size;
                int range_squares_view=int(_fact_line_of_sight/_col_grid_size);
                float view_dist2=_fact_line_of_sight*_fact_line_of_sight;
                for(int square_x=(int)center_square.x-range_squares_view;
                    square_x<square_x_max && square_x<=(int)center_square.x+range_squares_view; square_x++)
                for(int square_y=(int)center_square.y-range_squares_view;
                    square_y<square_y_max && square_y<=(int)center_square.y+range_squares_view; square_y++)
                {
                    if(square_x<0 || square_y<0 ) continue;//outside world

                    bool enemy_in_square=false;
                    for(int team_i=0;team_i<_max_teams;team_i++)
                    {
                        if(team_i==factory_team) continue;

                        if(m_arr_team_squares[square_x][square_y].at(team_i)!='0')
                        {
                            enemy_in_square=true;
                            break;
                        }
                    }

                    if(enemy_in_square)//if not, skip this ant test other squares
                     for(unsigned int unit_i1=0;unit_i1<m_arr_col_squares[square_x][square_y].vec_pUnits.size();unit_i1++)
                    {
                        if(m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_spec().team!=factory_team)
                        {
                            //other team found, test dist
                            float dist2=m_vec_pFactories[fact_i]->get_curr_pos().distance2(
                                  m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_curr_pos() );
                            if(view_dist2>dist2)
                            {
                                //report that unit visible to this team
                                m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->spotted_by_team(factory_team);
                            }
                        }
                    }
                }
            }

            //update units
            for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
            {
                m_vec_pUnits[unit_i]->update();

                //m_vec_pUnits[unit_i]->accept_new_pos();//TEMP

                //death test
                if( m_vec_pUnits[unit_i]->get_spec().hp_curr<=0.0 )
                {
                    //test if other units had special attack order on this unit
                    for(unsigned int unit_i1=0;unit_i1<m_vec_pUnits.size();unit_i1++)
                    {
                        if(m_vec_pUnits[unit_i1]->m_pUnit_enemy_target==m_vec_pUnits[unit_i])
                        {
                            //reset that target
                            m_vec_pUnits[unit_i1]->m_pUnit_enemy_target=0;
                            m_vec_pUnits[unit_i1]->m_forced_move=false;
                            //m_vec_pUnits[unit_i1]->cancel_movement();
                        }
                    }

                    //remove unit from square arr, team square might be wrong, problem?
                    st_pos square_pos=m_vec_pUnits[unit_i]->get_curr_col_square_pos();
                    bool unit_found=false;
                    for(unsigned int square_unit_i=0;
                        square_unit_i<m_arr_col_squares[(int)square_pos.x][(int)square_pos.y].vec_pUnits.size();
                        square_unit_i++)
                    {
                        if( m_arr_col_squares[(int)square_pos.x][(int)square_pos.y].vec_pUnits[square_unit_i]==m_vec_pUnits[unit_i] )
                        {
                            unit_found=true;
                            //found unit in arr
                            m_arr_col_squares[(int)square_pos.x][(int)square_pos.y].vec_pUnits.erase(
                             m_arr_col_squares[(int)square_pos.x][(int)square_pos.y].vec_pUnits.begin()+square_unit_i );

                            break;
                        }
                    }

                    if(!unit_found)
                    {
                        cout<<"ERROR: Unit removal from square array: Could not find unit\n";
                    }

                    //remove unit memory
                    delete m_vec_pUnits[unit_i];

                    //remove unit from unit vec
                    m_vec_pUnits.erase( m_vec_pUnits.begin()+unit_i );

                    continue;//skip rest
                }

                //if have moved, update col square test
                //if(m_vec_pUnits[unit_i]->m_have_moved)
                {
                    m_vec_pUnits[unit_i]->m_have_moved=false;

                    //test square pos
                    st_pos unit_pos=m_vec_pUnits[unit_i]->get_curr_pos();
                    int col_square_x=unit_pos.x/_col_grid_size;
                    int col_square_y=unit_pos.y/_col_grid_size;
                    //cap grid pos, if unit temp outside world limit
                    if(col_square_x<0) col_square_x=0;
                    else if(col_square_x>_world_width/_col_grid_size-1) col_square_x=_world_width/_col_grid_size-1;
                    if(col_square_y<0) col_square_y=0;
                    else if(col_square_y>_world_height/_col_grid_size-1) col_square_y=_world_height/_col_grid_size-1;

                    st_pos old_col_square_pos=m_vec_pUnits[unit_i]->get_curr_col_square_pos();
                    if( old_col_square_pos.x!=col_square_x || old_col_square_pos.y!=col_square_y )
                    {
                        //needs update
                        //cout<<"Unit moved to new col_square\n";

                        //remove from old square pos
                        bool unit_found=false;
                        for(unsigned int square_unit_i=0;
                            square_unit_i<m_arr_col_squares[(int)old_col_square_pos.x][(int)old_col_square_pos.y].vec_pUnits.size();
                            square_unit_i++)
                        {
                            if( m_arr_col_squares[(int)old_col_square_pos.x][(int)old_col_square_pos.y].vec_pUnits[square_unit_i]==m_vec_pUnits[unit_i] )
                            {
                                unit_found=true;

                                //remove from arr vec
                                m_arr_col_squares[(int)old_col_square_pos.x][(int)old_col_square_pos.y].vec_pUnits.erase(
                                 m_arr_col_squares[(int)old_col_square_pos.x][(int)old_col_square_pos.y].vec_pUnits.begin()+square_unit_i );

                                break;
                            }
                        }
                        if(!unit_found)
                        {
                            cout<<"ERROR: Could not find Unit in old col_square pos\n";
                        }
                        //cout<<"3 - "<<col_square_x<<", "<<col_square_y<<" - ";

                        //place unit in new col_square
                        m_arr_col_squares[col_square_x][col_square_y].vec_pUnits.push_back( m_vec_pUnits[unit_i] );
                        m_vec_pUnits[unit_i]->set_curr_col_square_pos( st_pos(col_square_x,col_square_y) );

                        //cout<<" Old square: "<<old_col_square_pos.x<<", "<<old_col_square_pos.y<<endl;
                        //cout<<" New square: "<<col_square_x<<", "<<col_square_y<<endl;
                    }
                }

                //attack view update
                bool attack_done=false;
                unit_spec specs=m_vec_pUnits[unit_i]->get_spec();
                float range_px_attack=specs.attack_range;
                float range_px2_attack=range_px_attack*range_px_attack;
                int range_squares_attack=range_px_attack/_col_grid_size+1;
                float range_px_view=specs.view_range;
                float range_px2_view=range_px_view*range_px_view;
                int range_squares_view=range_px_view/_col_grid_size+1;

                //special attack order active
                if(m_vec_pUnits[unit_i]->m_pUnit_enemy_target!=0)
                {
                    //test if that unit is within attack range
                    float dist2=m_vec_pUnits[unit_i]->get_curr_pos().distance2( m_vec_pUnits[unit_i]->m_pUnit_enemy_target->get_curr_pos() );
                    if(range_px2_attack>dist2)
                    {
                        //rotation test, copy to other square tests
                        st_pos rel_pos(m_vec_pUnits[unit_i]->m_pUnit_enemy_target->get_curr_pos().x-m_vec_pUnits[unit_i]->get_curr_pos().x,
                                       m_vec_pUnits[unit_i]->m_pUnit_enemy_target->get_curr_pos().y-m_vec_pUnits[unit_i]->get_curr_pos().y);
                        float rotation_target=atan2f(rel_pos.y,rel_pos.x)*_Rad2Deg;

                        if( m_vec_pUnits[unit_i]->aim_to_target(rotation_target,true) && m_vec_pUnits[unit_i]->is_ready_to_attack() )//true of rotation ok
                        {
                            //cout<<"ATTACK\n";
                            //in range, attack
                            m_vec_pUnits[unit_i]->attack_action_done();

                            //create projectile
                            m_vec_projectiles.push_back( projectile(m_vec_pUnits[unit_i]->get_curr_pos(),
                                                         m_vec_pUnits[unit_i]->m_pUnit_enemy_target->get_curr_pos(),
                                                         m_vec_pUnits[unit_i]->get_spec().projectile_speed,
                                                         m_vec_pUnits[unit_i]->get_spec().damage,
                                                         m_vec_pUnits[unit_i]->get_spec().damage_range,
                                                         m_vec_pUnits[unit_i]->get_spec().team ) );

                        }

                        attack_done=true;//might not have attacked but will skip attack testing
                        continue;//go to next unit
                    }
                    else//not within attack range
                    {
                        //if this unit can be seen by any allied unit, update the target pos and move there
                        if(m_vec_pUnits[unit_i]->m_pUnit_enemy_target->m_unit_seen_by_team[specs.team])
                        {
                            //update enemy pos
                            m_vec_pUnits[unit_i]->move_to(m_vec_pUnits[unit_i]->m_pUnit_enemy_target->get_curr_pos(),true,9999,
                                                          m_vec_pUnits[unit_i]->m_pUnit_enemy_target);
                        }
                    }
                }

                //enemy to attack test normal
                if( m_vec_pUnits[unit_i]->is_ready_to_attack() )
                {
                    //test if any target close

                    //test center square first
                    st_pos center_square=m_vec_pUnits[unit_i]->get_curr_col_square_pos();
                    bool enemy_in_square=false;
                    bool priority_to_center_square=false;//toggle on/off
                    if(priority_to_center_square && !attack_done)
                    {
                        for(int team_i=0;team_i<_max_teams;team_i++)
                        {
                            if(team_i==specs.team) continue;

                            if(m_arr_team_squares[(int)center_square.x][(int)center_square.y].at(team_i)!='0')
                            {
                                enemy_in_square=true;
                                break;
                            }
                        }

                        if(enemy_in_square)//if not, skip this and test other squares
                         for(unsigned int unit_i1=0;unit_i1<m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits.size();unit_i1++)
                        {
                            if(m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits[unit_i1]->get_spec().team!=specs.team)
                            {
                                //other team found, test dist
                                float dist2=m_vec_pUnits[unit_i]->get_curr_pos().distance2(
                                      m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits[unit_i1]->get_curr_pos() );
                                if(range_px2_attack>dist2)
                                {
                                    //rotation test, copy to other square tests
                                    st_pos rel_pos(m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits[unit_i1]->get_curr_pos().x-m_vec_pUnits[unit_i]->get_curr_pos().x,
                                                   m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits[unit_i1]->get_curr_pos().y-m_vec_pUnits[unit_i]->get_curr_pos().y);
                                    float rotation_target=atan2f(rel_pos.y,rel_pos.x)*_Rad2Deg;

                                    if( m_vec_pUnits[unit_i]->aim_to_target(rotation_target) )//true of rotation ok
                                    {
                                        //cout<<"ATTACK\n";
                                        attack_done=true;
                                        //in range, attack
                                        m_vec_pUnits[unit_i]->attack_action_done();

                                        //give damage
                                        //m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits[unit_i1]->take_damage(specs.damage,0);

                                        //create projectile
                                        m_vec_projectiles.push_back( projectile(m_vec_pUnits[unit_i]->get_curr_pos(),
                                                                     m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits[unit_i1]->get_curr_pos(),
                                                                     m_vec_pUnits[unit_i]->get_spec().projectile_speed,
                                                                     m_vec_pUnits[unit_i]->get_spec().damage,
                                                                     m_vec_pUnits[unit_i]->get_spec().damage_range,
                                                                     m_vec_pUnits[unit_i]->get_spec().team ) );

                                        //report that unit visible to this team
                                        m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits[unit_i1]->spotted_by_team(specs.team);
                                    }

                                    break;//do not find another unit in sight even if aim angle not ok
                                }
                            }
                        }
                    }

                    //test squares in reach
                    if(!attack_done)
                    {
                        //cout<<"RANGE: x "<<(int)center_square.x-range_squares<<" to "<<(int)center_square.x+range_squares<<endl;

                        for(int square_x=(int)center_square.x-range_squares_attack;
                            square_x<square_x_max && square_x<=(int)center_square.x+range_squares_attack; square_x++)
                        for(int square_y=(int)center_square.y-range_squares_attack;
                            square_y<square_y_max && square_y<=(int)center_square.y+range_squares_attack; square_y++)
                        {
                            //cout<<"ATTACK square: "<<square_x<<", "<<square_y<<endl;

                            if(attack_done) break;
                            if(square_x<0 || square_y<0 ) continue;//outside world
                            if(priority_to_center_square && square_x==(int)center_square.x
                                                         && square_y==(int)center_square.y) continue; //skip center, if already tested

                            enemy_in_square=false;
                            for(int team_i=0;team_i<_max_teams;team_i++)
                            {
                                if(team_i==specs.team) continue;

                                if(m_arr_team_squares[square_x][square_y].at(team_i)!='0')
                                {
                                    //cout<<" enemy found\n";
                                    enemy_in_square=true;
                                    break;
                                }
                            }

                            if(enemy_in_square)//if not, skip this ant test other squares
                             for(unsigned int unit_i1=0;unit_i1<m_arr_col_squares[square_x][square_y].vec_pUnits.size();unit_i1++)
                            {
                                if(m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_spec().team!=specs.team)
                                {
                                    //other team found, test dist
                                    float dist2=m_vec_pUnits[unit_i]->get_curr_pos().distance2(
                                          m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_curr_pos() );
                                    if(range_px2_attack>dist2)
                                    {
                                        //rotation test, copy to other square tests
                                        st_pos rel_pos(m_arr_col_squares[(int)square_x][(int)square_y].vec_pUnits[unit_i1]->get_curr_pos().x-m_vec_pUnits[unit_i]->get_curr_pos().x,
                                                       m_arr_col_squares[(int)square_x][(int)square_y].vec_pUnits[unit_i1]->get_curr_pos().y-m_vec_pUnits[unit_i]->get_curr_pos().y);
                                        float rotation_target=atan2f(rel_pos.y,rel_pos.x)*_Rad2Deg;

                                        if( m_vec_pUnits[unit_i]->aim_to_target(rotation_target) )//true of rotation ok
                                        {
                                            //cout<<"ATTACK in other square "<<specs.team<<endl;;
                                            attack_done=true;
                                            //in range, attack
                                            m_vec_pUnits[unit_i]->attack_action_done();

                                            //give damage
                                            //m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->take_damage(specs.damage,0);

                                            //create projectile
                                            m_vec_projectiles.push_back( projectile( m_vec_pUnits[unit_i]->get_curr_pos(),
                                                                         m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_curr_pos(),
                                                                         m_vec_pUnits[unit_i]->get_spec().projectile_speed,
                                                                         m_vec_pUnits[unit_i]->get_spec().damage,
                                                                         m_vec_pUnits[unit_i]->get_spec().damage_range,
                                                                         m_vec_pUnits[unit_i]->get_spec().team ) );

                                            //report that unit visible to this team
                                            m_arr_col_squares[(int)square_x][(int)square_y].vec_pUnits[unit_i1]->spotted_by_team(specs.team);
                                        }

                                        break;//do not find another unit in sight even if aim angle not ok
                                    }
                                    //else cout<<"attack distance fail\n";
                                }
                            }
                            //else continue;
                        }
                    }
                }

                //spot enemy test, if not attacking
                bool go_through_all_units_in_spotting=true;
                if(!attack_done || go_through_all_units_in_spotting)
                {
                    //is there any enemies within view range
                    bool enemy_spotted=false;

                    //test center square first
                    st_pos center_square=m_vec_pUnits[unit_i]->get_curr_col_square_pos();
                    bool enemy_in_square=false;
                    bool priority_to_center_square=false;//toggle on/off
                    if(priority_to_center_square)
                    {
                        for(int team_i=0;team_i<_max_teams;team_i++)
                        {
                            if(team_i==specs.team) continue;

                            if(m_arr_team_squares[(int)center_square.x][(int)center_square.y].at(team_i)!='0')
                            {
                                enemy_in_square=true;
                                break;
                            }
                        }

                        if(enemy_in_square)//if not, skip this and test other squares
                         for(unsigned int unit_i1=0;unit_i1<m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits.size();unit_i1++)
                        {
                            if(m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits[unit_i1]->get_spec().team!=specs.team)
                            {
                                //other team found, test dist
                                float dist2=m_vec_pUnits[unit_i]->get_curr_pos().distance2(
                                      m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits[unit_i1]->get_curr_pos() );
                                if(range_px2_view>dist2)
                                {
                                    enemy_spotted=true;

                                    //report that unit visible to this team
                                    m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits[unit_i1]->spotted_by_team(specs.team);

                                    //if that enemy is within fire range, dont move
                                    if(range_px2_attack>dist2 && !go_through_all_units_in_spotting) break;//current unit is just reloading

                                    //tell unit to move towards that unit (will recalc angle every frame), if unit not in stand mode
                                    if( (m_vec_pUnits[unit_i]->m_attack_mode==am_pursuit || m_vec_pUnits[unit_i]->m_attack_mode==am_defence) &&
                                         m_vec_pUnits[unit_i]->m_pUnit_enemy_target==0)
                                     m_vec_pUnits[unit_i]->move_to( m_arr_col_squares[(int)center_square.x][(int)center_square.y].vec_pUnits[unit_i1]->get_curr_pos() );

                                    if(!go_through_all_units_in_spotting)
                                     break;//do not find another unit in sight
                                }
                            }
                        }
                    }

                    //test squares in reach
                    if(!enemy_spotted || go_through_all_units_in_spotting)
                    {
                        //cout<<"RANGE: x "<<(int)center_square.x-range_squares<<" to "<<(int)center_square.x+range_squares<<endl;

                        for(int square_x=(int)center_square.x-range_squares_view;
                            square_x<square_x_max && square_x<=(int)center_square.x+range_squares_view; square_x++)
                        for(int square_y=(int)center_square.y-range_squares_view;
                            square_y<square_y_max && square_y<=(int)center_square.y+range_squares_view; square_y++)
                        {
                            //cout<<"ATTACK square: "<<square_x<<", "<<square_y<<endl;

                            if(enemy_spotted) break;
                            if(square_x<0 || square_y<0 ) continue;//outside world
                            if(priority_to_center_square && square_x==(int)center_square.x
                                                         && square_y==(int)center_square.y) continue; //skip center, if already tested

                            enemy_in_square=false;
                            for(int team_i=0;team_i<_max_teams;team_i++)
                            {
                                if(team_i==specs.team) continue;

                                if(m_arr_team_squares[square_x][square_y].at(team_i)!='0')
                                {
                                    //cout<<" enemy found\n";
                                    enemy_in_square=true;
                                    break;
                                }
                            }

                            if(enemy_in_square)//if not, skip this ant test other squares
                             for(unsigned int unit_i1=0;unit_i1<m_arr_col_squares[square_x][square_y].vec_pUnits.size();unit_i1++)
                            {
                                if(m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_spec().team!=specs.team)
                                {
                                    //other team found, test dist
                                    float dist2=m_vec_pUnits[unit_i]->get_curr_pos().distance2(
                                          m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_curr_pos() );
                                    if(range_px2_view>dist2)
                                    {
                                        enemy_spotted=true;

                                        //report that unit visible to this team
                                        m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->spotted_by_team(specs.team);

                                        //if that enemy is within fire range, dont move
                                        if(range_px2_attack>dist2 && !go_through_all_units_in_spotting) break;//current unit is just reloading

                                        //tell unit to move towards that unit (will recalc angle every frame), if unit not in stand mode
                                        if( (m_vec_pUnits[unit_i]->m_attack_mode==am_pursuit || m_vec_pUnits[unit_i]->m_attack_mode==am_defence) &&
                                             m_vec_pUnits[unit_i]->m_pUnit_enemy_target==0)
                                         m_vec_pUnits[unit_i]->move_to( m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_curr_pos() );

                                        if(!go_through_all_units_in_spotting)
                                         break;
                                    }
                                    //else cout<<"attack distance fail\n";
                                }
                            }
                            //else continue;
                        }
                    }
                }
            }

            //unit collision test OLD
            /*for(int unit_i1=0;unit_i1<(int)m_vec_pUnits.size();unit_i1++)
            {
                st_pos unit_new_pos=m_vec_pUnits[unit_i1]->get_new_pos();
                bool collision_done=false;
                for(int unit_i2=0;unit_i2<(int)m_vec_pUnits.size();unit_i2++)
                {
                    if(unit_i1==unit_i2) continue;

                    //unit-unit test
                    if( m_vec_pUnits[unit_i2]->is_pos_inside(unit_new_pos,true) )
                    {
                        collision_done=true;
                        break;
                    }
                }

                //unit-factory test
                for(int fact_i=0;fact_i<(int)m_vec_pFactories.size();fact_i++)
                {
                    if( m_vec_pFactories[fact_i]->is_pos_inside(unit_new_pos) )
                    {
                        collision_done=true;
                        break;
                    }
                }

                if(!collision_done) m_vec_pUnits[unit_i1]->accept_new_pos();
            }*/

            //unit square collision test
            int col_test_counter=0;
            for(int square_x=0;square_x<square_x_max;square_x++)
            for(int square_y=0;square_y<square_y_max;square_y++)
            {
                for(unsigned int unit_i1=0;unit_i1<m_arr_col_squares[square_x][square_y].vec_pUnits.size();unit_i1++)
                {
                    st_pos unit_new_pos=m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_new_pos();
                    st_pos unit_curr_pos=m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->get_curr_pos();

                    //unit-unit test
                    st_pos pos_of_col_object;
                    unit* pCollided_unit=m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1];//itself, temp
                    bool collision_done=false;
                    for(unsigned int unit_i2=0;unit_i2<m_arr_col_squares[square_x][square_y].vec_pUnits.size();unit_i2++)
                    {
                        if(unit_i1==unit_i2) continue;

                        col_test_counter++;

                        if( m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i2]->is_pos_inside(unit_new_pos,2.0,true) )
                        {
                            collision_done=true;
                            pos_of_col_object=m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i2]->get_curr_pos();
                            pCollided_unit=m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i2];
                            break;
                        }
                    }

                    //unit-factory test
                    st_pos pos_of_col_factory;
                    bool collision_factory_done=false;
                    for(unsigned int fact_i=0;fact_i<m_vec_pFactories.size();fact_i++)
                    {
                        col_test_counter++;

                        //new pos inside factory, only cancel movement
                        if( m_vec_pFactories[fact_i]->is_pos_inside(unit_new_pos) )
                        {
                            collision_factory_done=true;
                            pos_of_col_factory=m_vec_pFactories[fact_i]->get_curr_pos();
                            //break later
                        }

                        //old pos inside factory, push unit
                        if(collision_factory_done)
                         if( m_vec_pFactories[fact_i]->is_pos_inside(unit_curr_pos) )
                        {
                            //push unit
                            st_pos rel_pos( unit_curr_pos.x-pos_of_col_factory.x,unit_curr_pos.y-pos_of_col_factory.y );
                            float push_factor_factory=1.0;
                            float push_min_limit=3;
                            if( fabs(rel_pos.x)<push_min_limit && fabs(rel_pos.y)<push_min_limit) rel_pos.x=push_min_limit;//force right push
                            m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->push_unit( st_pos( rel_pos.x*push_factor_factory,
                                                                                                          rel_pos.y*push_factor_factory ),
                                                                                                  m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->is_moving() );
                            break;
                        }
                    }

                    //near border test
                    if(!collision_done)
                    {
                        //calc rel square pos
                        float rel_x=unit_new_pos.x-int(unit_new_pos.x/_col_grid_size)*_col_grid_size;
                        float rel_y=unit_new_pos.y-int(unit_new_pos.y/_col_grid_size)*_col_grid_size;

                        //left side
                        if(square_x>0 && rel_x<=_unit_size*2.0 && !collision_done)
                        {
                            for(unsigned int unit_i2=0;unit_i2<m_arr_col_squares[square_x-1][square_y].vec_pUnits.size();unit_i2++)
                            {
                                col_test_counter++;

                                //unit-unit test
                                if( m_arr_col_squares[square_x-1][square_y].vec_pUnits[unit_i2]->is_pos_inside(unit_new_pos,2.0,true) )
                                {
                                    collision_done=true;
                                    pos_of_col_object=m_arr_col_squares[square_x-1][square_y].vec_pUnits[unit_i2]->get_curr_pos();
                                    pCollided_unit=m_arr_col_squares[square_x-1][square_y].vec_pUnits[unit_i2];
                                    break;
                                }
                            }
                        }

                        //right side
                        if(square_x<square_x_max-1 && rel_x>=_col_grid_size-_unit_size*2.0 && !collision_done)
                        {
                            for(unsigned int unit_i2=0;unit_i2<m_arr_col_squares[square_x+1][square_y].vec_pUnits.size();unit_i2++)
                            {
                                col_test_counter++;

                                //unit-unit test
                                if( m_arr_col_squares[square_x+1][square_y].vec_pUnits[unit_i2]->is_pos_inside(unit_new_pos,2.0,true) )
                                {
                                    collision_done=true;
                                    pos_of_col_object=m_arr_col_squares[square_x+1][square_y].vec_pUnits[unit_i2]->get_curr_pos();
                                    pCollided_unit=m_arr_col_squares[square_x+1][square_y].vec_pUnits[unit_i2];
                                    break;
                                }
                            }
                        }

                        //top side
                        if(square_y>0 && rel_y<=_unit_size*2.0 && !collision_done)
                        {
                            for(unsigned int unit_i2=0;unit_i2<m_arr_col_squares[square_x][square_y-1].vec_pUnits.size();unit_i2++)
                            {
                                col_test_counter++;

                                //unit-unit test
                                if( m_arr_col_squares[square_x][square_y-1].vec_pUnits[unit_i2]->is_pos_inside(unit_new_pos,2.0,true) )
                                {
                                    collision_done=true;
                                    pos_of_col_object=m_arr_col_squares[square_x][square_y-1].vec_pUnits[unit_i2]->get_curr_pos();
                                    pCollided_unit=m_arr_col_squares[square_x][square_y-1].vec_pUnits[unit_i2];
                                    break;
                                }
                            }
                        }

                        //low side
                        if(square_y<square_y_max-1 && rel_y>=_col_grid_size-_unit_size*2.0 && !collision_done)
                        {
                            for(unsigned int unit_i2=0;unit_i2<m_arr_col_squares[square_x][square_y+1].vec_pUnits.size();unit_i2++)
                            {
                                col_test_counter++;

                                //unit-unit test
                                if( m_arr_col_squares[square_x][square_y+1].vec_pUnits[unit_i2]->is_pos_inside(unit_new_pos,2.0,true) )
                                {
                                    collision_done=true;
                                    pos_of_col_object=m_arr_col_squares[square_x][square_y+1].vec_pUnits[unit_i2]->get_curr_pos();
                                    pCollided_unit=m_arr_col_squares[square_x][square_y+1].vec_pUnits[unit_i2];
                                    break;
                                }
                            }
                        }

                        //low left side
                        if(square_y<square_y_max-1 && rel_y>=_col_grid_size-_unit_size*2.0 && !collision_done &&
                           square_x>0 && rel_x<=_unit_size*2.0)
                        {
                            for(unsigned int unit_i2=0;unit_i2<m_arr_col_squares[square_x-1][square_y+1].vec_pUnits.size();unit_i2++)
                            {
                                col_test_counter++;

                                //unit-unit test
                                if( m_arr_col_squares[square_x-1][square_y+1].vec_pUnits[unit_i2]->is_pos_inside(unit_new_pos,2.0,true) )
                                {
                                    collision_done=true;
                                    pos_of_col_object=m_arr_col_squares[square_x-1][square_y+1].vec_pUnits[unit_i2]->get_curr_pos();
                                    pCollided_unit=m_arr_col_squares[square_x-1][square_y+1].vec_pUnits[unit_i2];
                                    break;
                                }
                            }
                        }

                        //low right side
                        if(square_y<square_y_max-1 && rel_y>=_col_grid_size-_unit_size*2.0 && !collision_done &&
                           square_x<square_x_max-1 && rel_x>=_col_grid_size-_unit_size*2.0)
                        {
                            for(unsigned int unit_i2=0;unit_i2<m_arr_col_squares[square_x+1][square_y+1].vec_pUnits.size();unit_i2++)
                            {
                                col_test_counter++;

                                //unit-unit test
                                if( m_arr_col_squares[square_x+1][square_y+1].vec_pUnits[unit_i2]->is_pos_inside(unit_new_pos,2.0,true) )
                                {
                                    collision_done=true;
                                    pos_of_col_object=m_arr_col_squares[square_x+1][square_y+1].vec_pUnits[unit_i2]->get_curr_pos();
                                    pCollided_unit=m_arr_col_squares[square_x+1][square_y+1].vec_pUnits[unit_i2];
                                    break;
                                }
                            }
                        }

                        //top left side
                        if(square_y>0 && rel_y<=_unit_size*2.0 && !collision_done &&
                           square_x>0 && rel_x<=_unit_size*2.0)
                        {
                            for(unsigned int unit_i2=0;unit_i2<m_arr_col_squares[square_x-1][square_y-1].vec_pUnits.size();unit_i2++)
                            {
                                col_test_counter++;

                                //unit-unit test
                                if( m_arr_col_squares[square_x-1][square_y-1].vec_pUnits[unit_i2]->is_pos_inside(unit_new_pos,2.0,true) )
                                {
                                    collision_done=true;
                                    pos_of_col_object=m_arr_col_squares[square_x-1][square_y-1].vec_pUnits[unit_i2]->get_curr_pos();
                                    pCollided_unit=m_arr_col_squares[square_x-1][square_y-1].vec_pUnits[unit_i2];
                                    break;
                                }
                            }
                        }

                        //top right side
                        if(square_y>0 && rel_y<=_unit_size*2.0 && !collision_done &&
                           square_x<square_x_max-1 && rel_x>=_col_grid_size-_unit_size*2.0)
                        {
                            for(unsigned int unit_i2=0;unit_i2<m_arr_col_squares[square_x+1][square_y-1].vec_pUnits.size();unit_i2++)
                            {
                                col_test_counter++;

                                //unit-unit test
                                if( m_arr_col_squares[square_x+1][square_y-1].vec_pUnits[unit_i2]->is_pos_inside(unit_new_pos,2.0,true) )
                                {
                                    collision_done=true;
                                    pos_of_col_object=m_arr_col_squares[square_x+1][square_y-1].vec_pUnits[unit_i2]->get_curr_pos();
                                    pCollided_unit=m_arr_col_squares[square_x+1][square_y-1].vec_pUnits[unit_i2];
                                    break;
                                }
                            }
                        }

                    }

                    //test if collision can be ignored (unit-unit)
                    if(collision_done && !collision_factory_done)
                    {
                        if(m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->m_col_id==pCollided_unit->m_col_id)
                        {
                            //same id
                            if(m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->m_col_state==active ||
                               pCollided_unit->m_col_state==active )
                            {
                                //one is active (the other should be active or passive)
                                //noone can be off (none)
                                if(m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->m_col_state!=none &&
                                   pCollided_unit->m_col_state!=none)
                                {
                                    //test if any of the units are inside the formation square
                                    //find col_id
                                    int col_id_ind=-1;
                                    for(unsigned int col_i=0;col_i<m_vec_col_id.size();col_i++)
                                    {
                                        if(m_vec_col_id[col_i].id==pCollided_unit->m_col_id)
                                        {
                                            col_id_ind=col_i;
                                            break;
                                        }
                                    }
                                    if(col_id_ind!=-1)
                                    {
                                        if( m_vec_col_id[col_id_ind].is_pos_inside_square(unit_new_pos) ||
                                            m_vec_col_id[col_id_ind].is_pos_inside_square(pCollided_unit->get_curr_pos() ) )
                                        {
                                            //at least one unit inside the formation square
                                            collision_done=false;//collision avoided
                                        }
                                    }
                                    else//bad
                                    {
                                        cout<<"ERROR: Could not find col_id in vector\n";
                                    }
                                }
                            }
                        }
                    }

                    if(!collision_done && !collision_factory_done)
                     m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->accept_new_pos();
                    else if(collision_done)//trigger unit push
                    {
                        //calc push direction
                        st_pos rel_pos( unit_curr_pos.x-pos_of_col_object.x,unit_curr_pos.y-pos_of_col_object.y );
                        //push unit
                        float push_min_limit=3;
                        if( fabs(rel_pos.x)<push_min_limit && fabs(rel_pos.y)<push_min_limit) rel_pos.x=push_min_limit;//force right push
                        //m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->push_unit(rel_pos);
                        //push other unit, stronger if other unit is moving
                        float push_factor=1.0;
                        //if( m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->is_moving() ) push_factor=1.0;
                        if(!m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->is_moving() &&
                           pCollided_unit->is_moving() ) ;//no push if idle detects moveing inside
                        else if( m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->is_moving() &&
                                 pCollided_unit->is_moving()) //both are pushing, rand to push otherwise front element will be favourd
                        {
                            if(rand()%2==0)//push collided
                            {
                                pCollided_unit->push_unit( st_pos( -rel_pos.x*push_factor,-rel_pos.y*push_factor ),
                                                           m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->is_moving() );
                            }
                            else//push current unit
                            {
                                m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->push_unit( st_pos( rel_pos.x*push_factor,rel_pos.y*push_factor ),
                                                                                                      pCollided_unit->is_moving() );
                            }
                        }
                        else//the idle is pushed
                         pCollided_unit->push_unit( st_pos( -rel_pos.x*push_factor,-rel_pos.y*push_factor ),
                                                    m_arr_col_squares[square_x][square_y].vec_pUnits[unit_i1]->is_moving() );
                        //cout<<push_factor<<endl;
                    }
                }
            }
            //cout<<col_test_counter<<endl;

            //update col_id
            for(unsigned int col_i=0;col_i<m_vec_col_id.size();col_i++)
            {
                //see if any active units using the current col_id
                bool col_id_active=false;
                for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
                {
                    if( m_vec_pUnits[unit_i]->m_col_id==m_vec_col_id[col_i].id )
                    {
                        //same id, test if active
                        if(m_vec_pUnits[unit_i]->m_col_state==active)
                        {
                            col_id_active=true;
                            break;
                        }
                    }
                }
                if(!col_id_active)
                {
                    //no units are using this col_id

                    //make pasive units to none state
                    for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
                    {
                        if( m_vec_pUnits[unit_i]->m_col_id==m_vec_col_id[col_i].id )
                        {
                            m_vec_pUnits[unit_i]->m_col_state=none;
                        }
                    }

                    //remove col_id from vector
                    m_vec_col_id[col_i]=m_vec_col_id.back();
                    m_vec_col_id.pop_back();
                    col_i--;
                }
            }

        }break;
    }

    //reset mouse scroll
    m_pMouse_button[2]=false;
    m_pMouse_button[3]=false;

    return ret_game_idle;
}

bool game::draw(void)
{
    switch(m_game_state)
    {
        case gs_menu:
        {
            m_Menu.draw();
        }break;

        case gs_in_game:
        {
            //move to cam pos
            glPushMatrix();
            glTranslatef(-m_cam_pos[0],-m_cam_pos[1],0.0);

            //draw background
            glColor3f(0.1,0.2,0.1);
            glBegin(GL_QUADS);
            glVertex2f(0,0);
            glVertex2f(0,_world_height);
            glVertex2f(_world_width,_world_height);
            glVertex2f(_world_width,0);
            glEnd();

            //draw world edge
            glLineWidth(2);
            glColor3f(1,1,1);
            glBegin(GL_LINE_STRIP);
            glVertex2f(0,0);
            glVertex2f(0,_world_height);
            glVertex2f(_world_width,_world_height);
            glVertex2f(_world_width,0);
            glVertex2f(0,0);
            glEnd();

            //draw col_squares
            glLineWidth(1);
            glColor3f(1,1,1);
            glBegin(GL_LINES);
            for(int x=0;x<_world_width;x+=_col_grid_size)
            {
                glVertex2f(x,0);
                glVertex2f(x,_world_height);
            }
            glEnd();
            glBegin(GL_LINES);
            for(int y=0;y<_world_height;y+=_col_grid_size)
            {
                glVertex2f(0,y);
                glVertex2f(_world_width,y);
            }
            glEnd();

            //draw factories
            for(unsigned int fact_i=0;fact_i<m_vec_pFactories.size();fact_i++)
            {
                m_vec_pFactories[fact_i]->draw();
            }

            //draw units
            //glBegin(GL_QUADS);//global draw
            for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
            {
                //test if inside screen borders
                st_pos unit_pos=m_vec_pUnits[unit_i]->get_curr_pos();

                if( unit_pos.x>m_cam_pos[0]+m_window_width+_unit_size || unit_pos.x<m_cam_pos[0]-_unit_size ||
                    unit_pos.y>m_cam_pos[1]+m_window_height+_unit_size || unit_pos.y<m_cam_pos[1]-_unit_size )
                {
                    ;//outside screen, do not draw
                }
                else
                {
                    //test if this unit is seen by your team, or belongs to your team
                    if(!m_fog_of_war_enabled ||
                       m_vec_pUnits[unit_i]->m_unit_seen_by_team[m_human_player_team]=='1' ||
                       m_vec_pUnits[unit_i]->get_spec().team==m_human_player_team )
                    {
                        m_vec_pUnits[unit_i]->draw();
                    }
                }
            }
            //glEnd();

            //draw projectiles (test if outside screen?)
            glPointSize(2);
            glColor3f(1,1,1);
            glBegin(GL_POINTS);//global draw
            for(unsigned int proj_i=0;proj_i<m_vec_projectiles.size();proj_i++)
            {
                m_vec_projectiles[proj_i].draw();
            }
            glEnd();

            //draw mask to the stencil buffer for areas within vision
            for(int fow_layer=0;fow_layer<5;fow_layer++)
            {
                float layer_dist=1;
                float layer_col[4]={0,0,0,0};
                switch(fow_layer)
                {
                    case 0:
                    {
                        layer_dist=0.4;
                        layer_col[0]=0.15;
                        layer_col[1]=0.15;
                        layer_col[2]=0.15;
                        layer_col[3]=0.20;
                    }break;

                    case 1:
                    {
                        layer_dist=0.24;
                        layer_col[0]=0.12;
                        layer_col[1]=0.12;
                        layer_col[2]=0.12;
                        layer_col[3]=0.3;
                    }break;

                    case 2:
                    {
                        layer_dist=0.16;
                        layer_col[0]=0.1;
                        layer_col[1]=0.1;
                        layer_col[2]=0.1;
                        layer_col[3]=0.4;
                    }break;

                    case 3:
                    {
                        layer_dist=0.12;
                        layer_col[0]=0.08;
                        layer_col[1]=0.08;
                        layer_col[2]=0.08;
                        layer_col[3]=0.50;
                    }break;

                    case 4:
                    {
                        layer_dist=0.1;
                        layer_col[0]=0.05;
                        layer_col[1]=0.05;
                        layer_col[2]=0.05;
                        layer_col[3]=0.60;
                    }break;


                }

                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D,m_tex_fow_mask);
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
                glClear(GL_STENCIL_BUFFER_BIT);
                glEnable(GL_STENCIL_TEST);
                glStencilFunc(GL_ALWAYS, 0,0);
                glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
                glEnable(GL_ALPHA_TEST);
                glAlphaFunc(GL_GREATER, layer_dist);
                //draw vision area to stencil buffer
                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
                glColor4f(1,1,1,1);
                //unit FOV
                for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
                {
                    if(m_vec_pUnits[unit_i]->get_spec().team==m_human_player_team)
                    {
                        glBegin(GL_QUADS);
                        glTexCoord2f(0.0,0.0);
                        glVertex2f(m_vec_pUnits[unit_i]->get_curr_pos().x-m_vec_pUnits[unit_i]->get_spec().view_range,
                                   m_vec_pUnits[unit_i]->get_curr_pos().y-m_vec_pUnits[unit_i]->get_spec().view_range);
                        glTexCoord2f(0.0,1.0);
                        glVertex2f(m_vec_pUnits[unit_i]->get_curr_pos().x-m_vec_pUnits[unit_i]->get_spec().view_range,
                                   m_vec_pUnits[unit_i]->get_curr_pos().y+m_vec_pUnits[unit_i]->get_spec().view_range);
                        glTexCoord2f(1.0,1.0);
                        glVertex2f(m_vec_pUnits[unit_i]->get_curr_pos().x+m_vec_pUnits[unit_i]->get_spec().view_range,
                                   m_vec_pUnits[unit_i]->get_curr_pos().y+m_vec_pUnits[unit_i]->get_spec().view_range);
                        glTexCoord2f(1.0,0.0);
                        glVertex2f(m_vec_pUnits[unit_i]->get_curr_pos().x+m_vec_pUnits[unit_i]->get_spec().view_range,
                                   m_vec_pUnits[unit_i]->get_curr_pos().y-m_vec_pUnits[unit_i]->get_spec().view_range);
                        glEnd();
                    }
                }
                //factory FOV
                for(unsigned int fact_i=0;fact_i<m_vec_pFactories.size();fact_i++)
                {
                    if(m_vec_pFactories[fact_i]->get_spec().team==m_human_player_team)
                    {
                        glBegin(GL_QUADS);
                        glTexCoord2f(0.0,0.0);
                        glVertex2f(m_vec_pFactories[fact_i]->get_curr_pos().x-_fact_line_of_sight,
                                   m_vec_pFactories[fact_i]->get_curr_pos().y-_fact_line_of_sight);
                        glTexCoord2f(0.0,1.0);
                        glVertex2f(m_vec_pFactories[fact_i]->get_curr_pos().x-_fact_line_of_sight,
                                   m_vec_pFactories[fact_i]->get_curr_pos().y+_fact_line_of_sight);
                        glTexCoord2f(1.0,1.0);
                        glVertex2f(m_vec_pFactories[fact_i]->get_curr_pos().x+_fact_line_of_sight,
                                   m_vec_pFactories[fact_i]->get_curr_pos().y+_fact_line_of_sight);
                        glTexCoord2f(1.0,0.0);
                        glVertex2f(m_vec_pFactories[fact_i]->get_curr_pos().x+_fact_line_of_sight,
                                   m_vec_pFactories[fact_i]->get_curr_pos().y-_fact_line_of_sight);
                        glEnd();
                    }
                }
                glDisable(GL_ALPHA_TEST);
                glDisable(GL_TEXTURE_2D);
                //draw fog of war
                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                glStencilFunc(GL_EQUAL, 0, -1);
                glColor4fv(layer_col);
                glBegin(GL_QUADS);
                glTexCoord2f(0.0,0.0);
                glVertex2f(0,0);
                glTexCoord2f(0.0,1.0);
                glVertex2f(0,_world_height);
                glTexCoord2f(1.0,1.0);
                glVertex2f(_world_width,_world_height);
                glTexCoord2f(1.0,0.0);
                glVertex2f(_world_width,0);
                glEnd();
                glDisable(GL_STENCIL_TEST);
                glDisable(GL_BLEND);
            }

            /*//temp
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            //glBlendFunc(GL_ONE, GL_ONE);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_tex_fow_mask);
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_ALWAYS, 0,0);
            glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
            glEnable(GL_ALPHA_TEST);
            glAlphaFunc(GL_GREATER, 0.5);
            //draw vision area to stencil buffer
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
            glColor4f(1,1,1,1);
            glBegin(GL_QUADS);
            glTexCoord2f(0.0,0.0);
            glVertex2f(0,0);
            glTexCoord2f(0.0,1.0);
            glVertex2f(0,m_window_height);
            glTexCoord2f(1.0,1.0);
            glVertex2f(m_window_width,m_window_height);
            glTexCoord2f(1.0,0.0);
            glVertex2f(m_window_width,0);
            glEnd();
            glDisable(GL_ALPHA_TEST);
            glDisable(GL_TEXTURE_2D);
            //draw fog of war
            glEnable(GL_BLEND);
            //glBlendFunc(GL_ONE, GL_ONE);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            //glBlendFuncSeparate(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE, GL_ZERO);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
            glStencilFunc(GL_LESS, 0, -1);
            glColor3f(0.1,0.1,0.1);
            glBegin(GL_QUADS);
            glTexCoord2f(0.0,0.0);
            glVertex2f(0,0);
            glTexCoord2f(0.0,1.0);
            glVertex2f(0,m_window_height);
            glTexCoord2f(1.0,1.0);
            glVertex2f(m_window_width,m_window_height);
            glTexCoord2f(1.0,0.0);
            glVertex2f(m_window_width,0);
            glEnd();
            glDisable(GL_STENCIL_TEST);
            glDisable(GL_BLEND);*/
            //temp

            /*//temp2 depth
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LESS);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            //glBlendFunc(GL_ONE, GL_ONE);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_tex_fow_mask);
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glTranslatef(0,0,1);
            glColor3f(1,1,1);
            glBegin(GL_QUADS);
            glTexCoord2f(0.0,0.0);
            glVertex2f(0,0);
            glTexCoord2f(0.0,1.0);
            glVertex2f(0,200);
            glTexCoord2f(1.0,1.0);
            glVertex2f(200,200);
            glTexCoord2f(1.0,0.0);
            glVertex2f(200,0);
            glEnd();
            glDisable(GL_TEXTURE_2D);
            //x
            glTranslatef(0,0,-1);
            glEnable(GL_DEPTH_TEST);
            glColor3f(0.1,0.1,0.1);
            glBegin(GL_QUADS);
            glTexCoord2f(0.0,0.0);
            glVertex2f(0,0);
            glTexCoord2f(0.0,1.0);
            glVertex2f(0,m_window_height);
            glTexCoord2f(1.0,1.0);
            glVertex2f(m_window_width,m_window_height);
            glTexCoord2f(1.0,0.0);
            glVertex2f(m_window_width,0);
            glEnd();
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            //temp2*/

            //draw selection square
            if(m_mouse_drag_LMB)
            {
                glLineWidth(1);
                glColor3f(0.7,0.7,0.7);
                glBegin(GL_LINE_STRIP);
                glVertex2f(m_mouse_start_drag_pos.x,m_mouse_start_drag_pos.y);
                glVertex2f(m_mouse_pos_curr.x,m_mouse_start_drag_pos.y);
                glVertex2f(m_mouse_pos_curr.x,m_mouse_pos_curr.y);
                glVertex2f(m_mouse_start_drag_pos.x,m_mouse_pos_curr.y);
                glVertex2f(m_mouse_start_drag_pos.x,m_mouse_start_drag_pos.y);
                glEnd();
            }

            /*//draw move+rotation arrow
            if(m_mouse_drag_RMB)
            {
                glLineWidth(1);
                glColor3f(0.7,0.7,0.7);
                glBegin(GL_LINES);
                glVertex2f(m_mouse_start_drag_pos.x,m_mouse_start_drag_pos.y);
                glVertex2f(m_mouse_pos_curr.x,m_mouse_pos_curr.y);
                glEnd();
            }*/

            glPopMatrix();

            /*//TEMP
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_tex_fow_mask);
            glColor4f(1,1,1,1);
            //glColor3f(1,1,1);
            glBegin(GL_QUADS);
            glTexCoord2f(0.0,0.0);
            glVertex2f(0,0);
            glTexCoord2f(0.0,1.0);
            glVertex2f(0,m_window_height);
            glTexCoord2f(1.0,1.0);
            glVertex2f(m_window_width,m_window_height);
            glTexCoord2f(1.0,0.0);
            glVertex2f(m_window_width,0);
            glEnd();
            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);
            //TEMP*/

            //draw unit formation prediction
            if(m_mouse_drag_RMB && !m_custom_formation_line_in_progress)
            {
                st_pos front_direction(m_mouse_pos_curr.x-m_mouse_start_drag_pos.x,m_mouse_pos_curr.y-m_mouse_start_drag_pos.y);
                float front_length=front_direction.length();
                front_direction/=front_length;//normalize
                //float gap_size=_unit_size*4.0;//one unit in between
                float gap_size=m_unit_formation_gap_size;
                int units_per_row=int(front_length/gap_size);
                st_pos back_row_direction(-front_direction.y,front_direction.x);
                int units_on_row_counter=0;
                float rotation_target_final=atan2f(-front_direction.x,front_direction.y)*_Rad2Deg;

                //count number of selected units
                int unit_counter=0;
                for(unsigned int unit_i=0;unit_i<m_vec_pUnits.size();unit_i++)
                 if(m_vec_pUnits[unit_i]->m_selected) unit_counter++;

                glPushMatrix();
                glColor3f(0.5,0.5,0.5);
                glTranslatef(m_mouse_start_drag_pos.x-m_cam_pos[0],m_mouse_start_drag_pos.y-m_cam_pos[1],0);
                glRotatef(rotation_target_final+90,0,0,1);
                st_pos draw_pos(0,0);
                glBegin(GL_QUADS);
                for(int unit_i=0;unit_i<unit_counter;unit_i++)
                {
                    glVertex2f(draw_pos.x-_unit_size,draw_pos.y-_unit_size);
                    glVertex2f(draw_pos.x-_unit_size,draw_pos.y+_unit_size);
                    glVertex2f(draw_pos.x+_unit_size,draw_pos.y+_unit_size);
                    glVertex2f(draw_pos.x+_unit_size,draw_pos.y-_unit_size);

                    units_on_row_counter++;
                    if(units_on_row_counter>units_per_row)
                    {
                        units_on_row_counter=0;
                        draw_pos.x=0;
                        draw_pos.y+=gap_size;
                    }
                    else draw_pos.x+=gap_size;
                }
                glEnd();
                glPopMatrix();
            }

            //draw custom unit line formation
            if(m_custom_formation_line_in_progress)
            {
                glColor3f(0.5,0.5,0.5);
                for(unsigned int pos_i=0;pos_i<m_vec_unit_formation_points.size()-1;pos_i++)
                {
                    glPushMatrix();
                    glTranslatef(m_vec_unit_formation_points[pos_i].pos.x-m_cam_pos[0],
                                 m_vec_unit_formation_points[pos_i].pos.y-m_cam_pos[1],0.0);
                    glRotatef(m_vec_unit_formation_points[pos_i].value,0,0,1);
                    glBegin(GL_QUADS);
                    glVertex2f(-_unit_size,-_unit_size);
                    glVertex2f(-_unit_size,+_unit_size);
                    glVertex2f(+_unit_size,+_unit_size);
                    glVertex2f(+_unit_size,-_unit_size);
                    glEnd();
                    glPopMatrix();
                }
            }

            //draw factory build
            if(m_factory_build_order_in_progress)
            {
                glColor3f(0.5,0.5,0.5);
                glBegin(GL_QUADS);
                glVertex2f(m_pMouse_pos[0]-_fact_size,m_pMouse_pos[1]-_fact_size);
                glVertex2f(m_pMouse_pos[0]-_fact_size,m_pMouse_pos[1]+_fact_size);
                glVertex2f(m_pMouse_pos[0]+_fact_size,m_pMouse_pos[1]+_fact_size);
                glVertex2f(m_pMouse_pos[0]+_fact_size,m_pMouse_pos[1]-_fact_size);
                glEnd();
            }

            //draw HUD
            m_Hud.draw();

        }break;
    }

    return true;
}


//network

bool game::recv_data(SOCKET soc_sender)
{

    return true;
}

bool game::send_start_package_to_client(SOCKET soc_client)
{
    //add client info
    m_vec_client_info.push_back( st_client_info(soc_client) );

    return true;
}

bool game::send_start_package_to_server(void)
{

    return true;
}

bool game::send_client_denied_package(SOCKET soc_client)
{

    return true;
}

bool game::add_server_player(void)
{

    return true;
}

bool game::lost_player(int socket)
{
    //remove from vec_client_info

    return true;
}

bool game::lost_server(void)
{

    return true;
}


//private

bool game::setup_network_temp(void)
{
    cout<<"Enter Host IP: ";
    string line;
    getline(cin,line);
    string host_ip=line;

    cout<<"Connecting to "<<host_ip<<endl;
    m_pNetCom->connect_to_server(host_ip,5001);

    return true;
}

bool game::ping_clients(void)
{
    if(m_pNetCom->get_status()!=net_server) return false;

    for(unsigned int client_i=0;client_i<m_vec_client_info.size();client_i++)
    {
        //send ping request packet

    }

    return true;
}

bool game::load_sound(void)
{
    cout<<"Loading Sounds\n";


    return true;
}

bool game::load_texture(void)
{
    cout<<"Loading Textures\n";

    m_tex_fow_mask=SOIL_load_OGL_texture
	(
		"fow_mask.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);

	if(m_tex_fow_mask==0)
	{
	    return false;
	}

    return true;
}
