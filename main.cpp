#define _WIN32_WINNT 0x0500 //Needed for GetConsoleWindow()
#include <iostream>
//#include <windows.h>
#include <windowsx.h>
#include <winsock2.h>
#include <gl/gl.h>
#include <fcntl.h> //for console
#include <stdio.h>
#include <time.h>
#include "definitions.h"
#include "game.h"
#include "networkCom.h"

#define WM_WSAASYNC (WM_USER +5) //for Async windows messages

using namespace std;

int  g_window_width=800;
int  g_window_height=600;
bool g_keys[256];
int  g_mouse_pos[2];
bool g_mouse_but[4];

game*       pGame;
networkCom* pNetCom;

HWND g_main_hwnd;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);


int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    string command_line=lpCmdLine;
    bool debug_mode=true;
    if(command_line=="debug") debug_mode=true;

    WNDCLASSEX wcex;
    //HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;

    //register window class
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "Project F";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    if(!debug_mode)
    {
        //Detect screen resolution
        RECT desktop;
        // Get a handle to the desktop window
        const HWND hDesktop = GetDesktopWindow();
        // Get the size of screen to the variable desktop
        GetWindowRect(hDesktop, &desktop);
        // The top left corner will have coordinates (0,0)
        // and the bottom right corner will have coordinates
        // (horizontal, vertical)
        g_window_width = desktop.right;
        g_window_height = desktop.bottom;
    }

    //if debug mode start console
    if(debug_mode)
    {
        //Open a console window
        AllocConsole();
        //Connect console output
        HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
        int hCrt          = _open_osfhandle((long) handle_out, _O_TEXT);
        FILE* hf_out      = _fdopen(hCrt, "w");
        setvbuf(hf_out, NULL, _IONBF, 1);
        *stdout = *hf_out;
        //Connect console input
        HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
        hCrt             = _open_osfhandle((long) handle_in, _O_TEXT);
        FILE* hf_in      = _fdopen(hCrt, "r");
        setvbuf(hf_in, NULL, _IONBF, 128);
        *stdin = *hf_in;
        //Set console title
        SetConsoleTitle("Debug Console");
        HWND hwnd_console=GetConsoleWindow();
        MoveWindow(hwnd_console,g_window_width,0,680,510,TRUE);

        cout<<"Software started\n";
        cout<<"Version: "<<_version<<endl;
    }
    /*else
    {
        ShowCursor(FALSE);//hide cursor
    }*/

    //create main window
    g_main_hwnd = CreateWindowEx(0,
                          "Project F",
                          "Project F",
                          WS_VISIBLE | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          g_window_width,
                          g_window_height,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(g_main_hwnd, nCmdShow);

    //enable OpenGL for the window
    EnableOpenGL(g_main_hwnd, &hDC, &hRC);

    //main loop
    for(int i=0;i<256;i++) g_keys[i]=false; //reset keys
    g_mouse_pos[0]=0;
    g_mouse_pos[1]=0;
    g_mouse_but[0]=false;
    g_mouse_but[1]=false;
    g_mouse_but[2]=false;
    g_mouse_but[3]=false;

    clock_t time_now=clock();
    clock_t time_last=time_now;
    clock_t time_step=_time_step;

    int screen_size[2]={g_window_width,g_window_height};
    pGame=new game();
    pNetCom=new networkCom();
    pNetCom->block_trap();
    if( !pGame->init(screen_size,g_keys,g_mouse_pos,g_mouse_but,pNetCom) )
    {
        cout<<"ERROR: Could not initialize the game\n";
        return 1;
    }

    while (!bQuit)
    {
        //check for messages
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            //handle or dispatch messages
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            bool update_screen=false;
            //quit test
            if(g_keys[VK_ESCAPE]) bQuit=TRUE;

            time_now=clock();
            //game update
            while( time_last+time_step <= time_now )
            {
                time_last+=time_step;

                switch( pGame->update() )//static time update
                {
                    case ret_game_idle:
                    {

                    }break;

                    case ret_game_host:
                    {
                        cout<<"Network: Now Server\n";
                        pNetCom->init("server");
                        pNetCom->set_port_and_bind(5001);
                        WSAAsyncSelect( pNetCom->get_server_socket() , g_main_hwnd, WM_WSAASYNC, FD_READ | FD_WRITE | FD_ACCEPT | FD_CLOSE);
                        pNetCom->start_to_listen(10);
                        //pGame->set_check_for_broadcast_flag(true);
                        //pGame->add_server_player();
                    }break;

                    case ret_game_join:
                    {
                        cout<<"Network: Now Client\n";
                        pNetCom->init("client");
                        WSAAsyncSelect( pNetCom->get_server_socket() , g_main_hwnd, WM_WSAASYNC, FD_WRITE | FD_CONNECT | FD_READ | FD_CLOSE);
                        //broadcast to get server ip
                        if( !pNetCom->broadcast_my_ip() ) cout<<"ERROR: Network: Broadcast problem\n";
                        //pGame->set_check_for_broadcast_reply_flag(true);

                        //TEMP
                        pGame->setup_network_temp();
                        //TEMP

                    }break;

                    case ret_game_exit:
                    {
                        bQuit=true;
                    }break;
                }

                update_screen=true;
            }

            //draw, if anything updated
            if(update_screen)
            {
                update_screen=false;

                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
                glLoadIdentity();
                pGame->draw();
                SwapBuffers(hDC);
            }
        }
    }

    //remove the game and network
    pNetCom->clean_up();
    delete pNetCom;
    delete pGame;

    //shutdown OpenGL
    DisableOpenGL(g_main_hwnd, hDC, hRC);

    //destroy the window explicitly
    DestroyWindow(g_main_hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
        {
            PostQuitMessage(0);
        }
        break;

        case WM_DESTROY:
        {
            return 0;
        }
        break;

        case WM_MOUSEMOVE:
        {
             g_mouse_pos[0]=GET_X_LPARAM(lParam);
             g_mouse_pos[1]=GET_Y_LPARAM(lParam);//-22 pixel shift to get same coord as drawing
        }
        break;

        case WM_LBUTTONDOWN:
        {
             g_mouse_but[0]=true;
        }
        break;

        case WM_LBUTTONUP:
        {
             g_mouse_but[0]=false;
        }
        break;

        case WM_RBUTTONDOWN:
        {
             g_mouse_but[1]=true;
             cout<<"Screen Pos: "<<g_mouse_pos[0]<<", "<<g_mouse_pos[1]<<"\tRel: ";
             cout<<(float)g_mouse_pos[0]/(float)g_window_width<<", "<<(float)g_mouse_pos[1]/(float)g_window_height<<endl;
        }
        break;

        case WM_RBUTTONUP:
        {
             g_mouse_but[1]=false;
        }
        break;

        case WM_MOUSEWHEEL:
        {
            //cout<<short(HIWORD(wParam))<<endl;
            //cout<<GET_WHEEL_DELTA_WPARAM(wParam)<<endl;
            //if(HIWORD(wParam)>5000) {g_mouse_but[2]=true;}
            //if(HIWORD(wParam)>100&&HIWORD(wParam)<5000) {g_mouse_but[3]=true;}

            if(short(HIWORD(wParam))<0) g_mouse_but[2]=true;
            else                        g_mouse_but[3]=true;
        }
        break;

		case WM_KEYDOWN:
		{
			g_keys[wParam]=true;

			if(g_keys[17] && wParam!=17) cout<<"Button pressed: "<<wParam<<endl;
		}
		break;

		case WM_KEYUP:
		{
			g_keys[wParam]=false;
		}
		break;

		case WM_WSAASYNC://network message
        {
            // what word?
            switch(WSAGETSELECTEVENT(lParam))
            {
                case FD_READ: //incomming data from SOCKET wParam
                {
                    cout<<"FD_READ\n";
                    pGame->recv_data(wParam);
                }break;

                case FD_WRITE: //only used if sending large files
                {
                    cout<<"FD_WRITE\n";
                }break;

                case FD_ACCEPT: //client wants to join
                {
                    cout<<"FD_ACCEPT\n";
                    if(pNetCom->add_client(wParam))
                    {
                        cout<<"Network: New Client connected, Socket: "<<int(wParam)<<endl;
                    }
                    else cout<<"Network: Bad Client tried to connect\n";
                    //test if server allows new players to join
                    if(!pNetCom->get_accept_new_clients_flag())
                    {
                        cout<<"Network: Server does not accept new clients in this state\n";
                        pGame->send_client_denied_package(wParam);
                    }

                    return(0);
                }break;

                case FD_CONNECT: //Client is now connected to server
                {
                    cout<<"FD_CONNECT\n";

                    //Test Connection
                    if(pNetCom->test_connection())
                    {
                        cout<<"You are now connected to the server, Socket: "<<int(pNetCom->get_server_socket())<<endl;
                        //send start package to server with name
                        pGame->send_start_package_to_server();
                    }
                    else//not connected
                    {
                        cout<<"Could not connect to server\n";
                        break;
                    }
                } break;

                case FD_CLOSE: // lost client
                {
                    cout<<"FD_CLOSE\n";
                    if(pNetCom->get_status()==net_server)
                    {
                        if(pNetCom->remove_client(wParam))
                        {
                            cout<<"Client Removed\n";
                            pGame->lost_player(wParam);
                        }
                    }
                    else//client
                    {
                        cout<<"Lost connection to Server\n";
                        pNetCom->lost_connection();
                        pGame->lost_server();
                    }
                }break;

            }
        }

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);

    //set 2D mode
    glClearColor(0.0,0.0,0.0,0.0);  //Set the cleared screen colour to black
    glViewport(0,0,g_window_width,g_window_height);   //This sets up the viewport so that the coordinates (0, 0) are at the top left of the window

    //Set up the orthographic projection so that coordinates (0, 0) are in the top left
    //and the minimum and maximum depth is -10 and 10. To enable depth just put in
    //glEnable(GL_DEPTH_TEST)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,g_window_width,g_window_height,0,-1,1);

    //Back to the modelview so we can draw stuff
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //Enable antialiasing
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearStencil(0);
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

