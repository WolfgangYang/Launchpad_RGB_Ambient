#define UNICODE
#define _UNICODE
#include <windows.h>
#include <mmsystem.h>
#include <commctrl.h>
#include <pdh.h>
#include <math.h>
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"pdh.lib")

#define T_FRAME 1
#define T_STATS 2
#define C_PORT 100
#define C_REFRESH 101
#define C_CONNECT 102
#define C_RAINBOW 110
#define C_BREATHE 111
#define C_WAVE 112
#define C_STARS 113
#define C_SOLID 114
#define C_CPU 120
#define C_GPU 121
#define C_RAM 122
#define C_TEMP 123
#define C_BRIGHT 130
#define C_SPEED 131
#define C_STATUS 140

static HMIDIOUT midi=NULL; static HWND mainw,ports,statusw,bright,speed; static int running=0,mode=0; static BOOL cpuOn=0,gpuOn=0,ramOn=0,tempOn=0; static double phase=0,cpu=0; static PDH_HQUERY q=NULL; static PDH_HCOUNTER cc=NULL; static MEMORYSTATUSEX mem={sizeof(mem)};

static UINT lid(int x,int y){return 11+(7-y)*10+x;}
static BYTE c63(double x){if(x<0)x=0;if(x>63)x=63;return (BYTE)(x+.5);}
static BOOL sysex(BYTE*d,DWORD n){if(!midi)return 0;MIDIHDR h={0};h.lpData=(LPSTR)d;h.dwBufferLength=n;if(midiOutPrepareHeader(midi,&h,sizeof h))return 0;MMRESULT r=midiOutLongMsg(midi,&h,sizeof h);DWORD t=GetTickCount();while(!(h.dwFlags&MHDR_DONE)&&GetTickCount()-t<500)Sleep(1);midiOutUnprepareHeader(midi,&h,sizeof h);return r==MMSYSERR_NOERROR;}
static void rgb(int x,int y,BYTE r,BYTE g,BYTE b){BYTE m[12]={0xF0,0,0x20,0x29,2,0x18,0x0B,0,0,0,0,0xF7};m[7]=(BYTE)lid(x,y);m[8]=r;m[9]=g;m[10]=b;sysex(m,12);}
static void off(void){if(!midi)return;for(int y=0;y<8;y++)for(int x=0;x<8;x++)rgb(x,y,0,0,0);}
static void closem(void){if(midi){off();midiOutReset(midi);midiOutClose(midi);midi=NULL;}}
static void stat(LPCWSTR s){SetWindowTextW(statusw,s);}
static void refresh(void){SendMessageW(ports,CB_RESETCONTENT,0,0);UINT n=midiOutGetNumDevs();int p=-1;for(UINT i=0;i<n;i++){MIDIOUTCAPSW c;if(midiOutGetDevCapsW(i,&c,sizeof c))continue;int z=(int)SendMessageW(ports,CB_ADDSTRING,0,(LPARAM)c.szPname);SendMessageW(ports,CB_SETITEMDATA,z,i);if(wcsstr(c.szPname,L"Launchpad MK2"))p=z;}if(SendMessageW(ports,CB_GETCOUNT,0,0)>0)SendMessageW(ports,CB_SETCURSEL,p>=0?p:0,0);}
static BOOL connectm(void){closem();int z=(int)SendMessageW(ports,CB_GETCURSEL,0,0);if(z<0)return 0;UINT id=(UINT)SendMessageW(ports,CB_GETITEMDATA,z,0);if(midiOutOpen(&midi,id,0,0,CALLBACK_NULL)){midi=NULL;stat(L"MIDI 打开失败");return 0;}stat(L"Launchpad 已连接");return 1;}
static void hsv(double h,double s,double v,double*r,double*g,double*b){h=fmod(h,1);if(h<0)h+=1;double i=floor(h*6),f=h*6-i,p=v*(1-s),q=v*(1-f*s),t=v*(1-(1-f)*s);switch((int)i%6){case 0:*r=v;*g=t;*b=p;break;case 1:*r=q;*g=v;*b=p;break;case 2:*r=p;*g=v;*b=t;break;case 3:*r=p;*g=q;*b=v;break;case 4:*r=t;*g=p;*b=v;break;default:*r=v;*g=p;*b=q;}}
static void frame(void){if(!midi||!running)return;double br=SendMessageW(bright,TBM_GETPOS,0,0)/100.,sp=SendMessageW(speed,TBM_GETPOS,0,0)/20.;phase+=.035*(.2+sp);for(int y=0;y<8;y++)for(int x=0;x<8;x++){double r=0,g=0,b=0;if(mode==0)hsv(x/8.+y/16.+phase*.08,.9,br,&r,&g,&b);else if(mode==1){double v=(.5+.5*sin(phase*2))*br;hsv(phase*.02,.75,v,&r,&g,&b);}else if(mode==2){double d=sqrt((x-3.5)*(x-3.5)+(y-3.5)*(y-3.5)),v=(.5+.5*sin(d*2-phase*5))*br;r=v;g=.45*v;b=.1*v;}else if(mode==3){double z=sin(x*12.9898+y*78.233+floor(phase*1.5)*37.7),v=((z-floor(z))*.85+.15)*br;BOOL on=((x*17+y*31+(int)phase)%19)==0;r=on?v:.02*v;g=on?.25*v:.02*v;b=on?v:.02*v;}else{r=.5*br;g=.05*br;b=.12*br;}if(cpuOn&&y==0){double v=cpu/100.;if(x<(int)(v*8)) {r=.08;g=1-v;b=.05;}else{r*=.15;g*=.15;b*=.15;}}if(ramOn&&y==7){double v=1-(double)mem.ullAvailPhys/mem.ullTotalPhys;if(x<(int)(v*8)){r=.1;g=1-v;b=.05;}}if(tempOn&&x==7&&y==7){r=1;g=.1;b=0;}rgb(x,y,c63(r*63),c63(g*63),c63(b*63));}}
static void stats(void){if(q&&cc){PDH_FMT_COUNTERVALUE v;if(PdhCollectQueryData(q)==ERROR_SUCCESS&&PdhGetFormattedCounterValue(cc,PDH_FMT_DOUBLE,0,&v)==ERROR_SUCCESS)cpu=v.doubleValue;}GlobalMemoryStatusEx(&mem);}
static void button(HWND h,int id,int x,int y,int w,int he,LPCWSTR s){CreateWindowW(L"BUTTON",s,WS_CHILD|WS_VISIBLE,x,y,w,he,h,(HMENU)(INT_PTR)id,GetModuleHandle(NULL),0);}
static LRESULT CALLBACK W(HWND h,UINT m,WPARAM w,LPARAM l){switch(m){case WM_CREATE:
 mainw=h;CreateWindowW(L"STATIC",L"Launchpad RGB Ambient",WS_CHILD|WS_VISIBLE,18,12,300,28,h,0,GetModuleHandle(NULL),0);CreateWindowW(L"STATIC",L"MIDI",WS_CHILD|WS_VISIBLE,18,48,40,22,h,0,GetModuleHandle(NULL),0);
 ports=CreateWindowW(L"COMBOBOX",0,WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST,60,45,340,200,h,(HMENU)C_PORT,GetModuleHandle(NULL),0);button(h,C_REFRESH,18,78,95,28,L"刷新");button(h,C_CONNECT,120,78,95,28,L"连接");
 button(h,C_RAINBOW,18,118,72,28,L"彩虹");button(h,C_BREATHE,96,118,72,28,L"呼吸");button(h,C_WAVE,174,118,72,28,L"波纹");button(h,C_STARS,252,118,72,28,L"星空");button(h,C_SOLID,330,118,72,28,L"纯色");
 CreateWindowW(L"STATIC",L"状态指示",WS_CHILD|WS_VISIBLE,18,158,70,22,h,0,GetModuleHandle(NULL),0);button(h,C_CPU,92,153,60,28,L"CPU");button(h,C_GPU,158,153,60,28,L"GPU");button(h,C_RAM,224,153,60,28,L"RAM");button(h,C_TEMP,290,153,60,28,L"温度");
 CreateWindowW(L"STATIC",L"亮度",WS_CHILD|WS_VISIBLE,18,198,45,22,h,0,GetModuleHandle(NULL),0);bright=CreateWindowW(TRACKBAR_CLASSW,0,WS_CHILD|WS_VISIBLE,60,193,340,30,h,(HMENU)C_BRIGHT,GetModuleHandle(NULL),0);SendMessageW(bright,TBM_SETRANGE,1,MAKELONG(5,100));SendMessageW(bright,TBM_SETPOS,1,70);
 CreateWindowW(L"STATIC",L"速度",WS_CHILD|WS_VISIBLE,18,238,45,22,h,0,GetModuleHandle(NULL),0);speed=CreateWindowW(TRACKBAR_CLASSW,0,WS_CHILD|WS_VISIBLE,60,233,340,30,h,(HMENU)C_SPEED,GetModuleHandle(NULL),0);SendMessageW(speed,TBM_SETRANGE,1,MAKELONG(1,50));SendMessageW(speed,TBM_SETPOS,1,20);
 statusw=CreateWindowW(L"STATIC",L"未连接",WS_CHILD|WS_VISIBLE,18,280,380,25,h,(HMENU)C_STATUS,GetModuleHandle(NULL),0);refresh();PdhOpenQueryW(0,0,&q);PdhAddEnglishCounterW(q,L"\\Processor(_Total)\\% Processor Time",0,&cc);PdhCollectQueryData(q);SetTimer(h,T_STATS,1000,0);SetTimer(h,T_FRAME,80,0);break;
case WM_COMMAND:switch(LOWORD(w)){case C_REFRESH:refresh();break;case C_CONNECT:connectm();break;case C_RAINBOW:mode=0;running=1;break;case C_BREATHE:mode=1;running=1;break;case C_WAVE:mode=2;running=1;break;case C_STARS:mode=3;running=1;break;case C_SOLID:mode=4;running=1;break;case C_CPU:cpuOn=!cpuOn;break;case C_GPU:gpuOn=!gpuOn;break;case C_RAM:ramOn=!ramOn;break;case C_TEMP:tempOn=!tempOn;break;}break;
case WM_TIMER:if(w==T_FRAME)frame();else if(w==T_STATS)stats();break;case WM_DESTROY:KillTimer(h,T_FRAME);KillTimer(h,T_STATS);closem();if(q)PdhCloseQuery(q);PostQuitMessage(0);break;}return DefWindowProcW(h,m,w,l);}
int WINAPI wWinMain(HINSTANCE hi,HINSTANCE pi,LPWSTR cmd,int show){INITCOMMONCONTROLSEX ic={sizeof ic,ICC_BAR_CLASSES};InitCommonControlsEx(&ic);WNDCLASSW c={0};c.lpfnWndProc=W;c.hInstance=hi;c.lpszClassName=L"LPAmbient";c.hCursor=LoadCursor(0,IDC_ARROW);c.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);RegisterClassW(&c);HWND h=CreateWindowW(L"LPAmbient",L"Launchpad RGB Ambient",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,CW_USEDEFAULT,CW_USEDEFAULT,440,350,0,0,hi,0);ShowWindow(h,show);MSG msg;while(GetMessageW(&msg,0,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}return 0;}
