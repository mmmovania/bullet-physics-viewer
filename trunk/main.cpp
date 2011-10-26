#ifdef _WIN32
    #ifndef _CRT_SECURE_NO_WARNINGS
        #define _CRT_SECURE_NO_WARNINGS 
    #endif
    #ifndef _CRT_SECURE_NO_DEPRECATE
        #define _CRT_SECURE_NO_DEPRECATE
    #endif
#endif

#define WIN32_LEAN_AND_MEAN		

#include <windows.h>			
#include <gl/gl.h>				
#include <gl/glu.h>				
#include <mmsystem.h>           
#include <stdio.h>
#include <CommDlg.h>

#include "HGLCamera.h"
#include "BulletRigidbodySim.h"

////// Global Variables
HDC         g_hDC = NULL;		
HGLRC	    g_hRC = NULL;		
HWND        g_hWnd = NULL;		
HINSTANCE	g_hInstance;		
char        g_ClassName[] = "Bullet Physics Viewer";
char        g_WindowName[] = "Bullet Physics Viewer v0.1 - Dongsoo Han";

BOOL   g_isFullscreen = FALSE;          // Fullscreen flag
BOOL   g_active = TRUE;		            // Window Active Flag Set To TRUE By Default
BOOL   g_bEnd = FALSE;

int width = 1280;
int height = 1024;
int bits = 32; 

HGLCamera   hCamera(threeButtons, false, 200.0f, 0.0f, 30.0f, 30.0f, 30.0f);
GLuint btmPlate;

GLfloat green[4] = {0.0f, 0.8f, 0.2f, 1.0f};
GLfloat white[4] = {0.9f, 0.9f, 0.9f, 1.0f};
GLfloat gray[4] = {0.5f, 0.5f, 0.5f, 1.0f};
GLfloat black[4] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat yellow[4]  = { 1.0f, 1.0f, 0.0f, 1.0f};
int gmx, gmy;
float g_DeltaTime = 0.0f;

CBulletRigidbodySim g_BulletSim;
double g_dt = 0.1;
int g_NumFrame = 0;
bool g_bPause = false;
unsigned int g_NumIter = 0;

GLuint g_glLtAxis;

// declare funtions..
void SetupPixelFormat(HDC hDC);
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
GLvoid KillGLWindow(GLvoid);
GLuint GenerateAxis(float fAxisLength);

void RenderFunc();
void Initialize();
void GetFPS();

//==================================================================================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
//==================================================================================================
{
	WNDCLASSEX windowClass;		// window class
	HWND	   hwnd;			// window handle
	MSG		   msg;				// message
	bool	   done;			// flag saying when our app is complete

//	MessageBox(NULL, "LeftButton+Mouse Move : zoom\nRightButton+Mouse Move : pan\nLeftButton+RightButton+Mouse Move : rotate\nESC : Exit\n\nAuthor: Dongsoo Han\nDate:2/20/08\nemail:saggitasaggita@yahoo.com", "How to control camera", MB_OK);

	g_hInstance = hInstance;

	// fill out the window class structure
	windowClass.cbSize			= sizeof(WNDCLASSEX);
	windowClass.style			= CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc		= WndProc;
	windowClass.cbClsExtra		= 0;
	windowClass.cbWndExtra		= 0;
	windowClass.hInstance		= hInstance;
	windowClass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);	// default icon
	windowClass.hCursor			= LoadCursor(NULL, IDC_ARROW);		// default arrow
	windowClass.hbrBackground	= NULL;								// don't need background
	windowClass.lpszMenuName	= NULL;								// no menu
	windowClass.lpszClassName	= g_ClassName;
	windowClass.hIconSm			= LoadIcon(NULL, IDI_WINLOGO);		// windows logo small icon

	// register the windows class
	if (!RegisterClassEx(&windowClass))
		return 0;

	
	////////////////////////////////////////////////////
	// For fullscreen mode
	////////////////////////////////////////////////////


	// if we're in fullscreen mode, set the display up for it
	if (g_isFullscreen)
	{
		// set up the device mode structure
		DEVMODE screenSettings;
		memset(&screenSettings,0,sizeof(screenSettings));

		screenSettings.dmSize       = sizeof(screenSettings);	
		screenSettings.dmPelsWidth  = width;			// screen width
		screenSettings.dmPelsHeight = height;			// screen height
		screenSettings.dmBitsPerPel = bits;				// bits per pixel
		screenSettings.dmFields     = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// attempt to switch to the resolution and bit depth we've selected
		if (ChangeDisplaySettings(&screenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
		  // if we can't get fullscreen, let them choose to quit or try windowed mode
		  if (MessageBox(NULL, "Cannot run in the fullscreen mode at the selected resolution\n"
							   "on your video card. Try windowed mode instead?",
							   "OpenGL Game Programming",
							   MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
		  {
			g_isFullscreen = FALSE;	
		  }
		  else
				{
					return FALSE;
				}
		}
	  }

	DWORD dwExStyle;
	DWORD dwStyle;

  // if we're still in fullscreen mode, set the window style appropriately
	if (g_isFullscreen)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP;						// simple window with no borders or title bar
		//ShowCursor(FALSE);            // hide the cursor for now
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW;
		//dwStyle = WS_POPUP;
		width = 900;
		height = 700;
	}

  // set up the window we're rendering to so that the top left corner is at (0,0)
  // and the bottom right corner is (height,width)
  RECT  windowRect;
  windowRect.left = 0;
  windowRect.right = (LONG) width;
  windowRect.top = 0;
  windowRect.bottom = (LONG) height;

  // change the size of the rect to account for borders, etc. set by the style
  AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

  hwnd = CreateWindowEx(dwExStyle,							  // extended style
						  (LPCTSTR)g_ClassName,				  // class name
						  (LPCTSTR)g_WindowName,              // app name
						  dwStyle |                           // window style
						  WS_CLIPCHILDREN |					  // required for
						  WS_CLIPSIBLINGS,                    // using OpenGL
						  0, 0,                               // x,y coordinate
						  windowRect.right - windowRect.left, // width
						  windowRect.bottom - windowRect.top, // height
						  NULL,                               // handle to parent
						  NULL,                               // handle to menu
						  hInstance,                          // application instance
						  NULL);                              // no extra params


	if (!hwnd)
	{
		KillGLWindow();
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (!(g_hDC = GetDC(hwnd)))		
	{
		KillGLWindow();							
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							
	}

	SetupPixelFormat(g_hDC);		// call our pixel format setup function

	// create rendering context and make it current
	g_hRC = wglCreateContext(g_hDC);
	wglMakeCurrent(g_hDC, g_hRC);

	g_hWnd = hwnd;

	ShowWindow(hwnd, SW_SHOW);			// display the window
	UpdateWindow(hwnd);					// update the window

	done = false;						// intialize the loop condition variable

	Initialize();

	// main message loop
	while (!done)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)		// do we receive a WM_QUIT message?
			{
				done = TRUE;				// if so, time to quit the application
			}
			else
			{
				TranslateMessage(&msg);		// translate and dispatch to event queue
				DispatchMessage(&msg);
			}
		}
		else
		{
			if(g_bEnd)
				done = TRUE;
			
			if(g_active)  
				RenderFunc();

			// display Frame/Second rate..
			GetFPS();

			glFlush();
			SwapBuffers(g_hDC);			// bring backbuffer to foreground
		}

	}

	KillGLWindow();

	return msg.wParam;
}




//======================================================================================
// the Windows Procedure event handler
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
//======================================================================================
{
	static HGLRC hRC;					// rendering context
	static HDC hDC;						// device context
	char string[] = "Hello, world!";	// text to be displayed
	//int width, height;					// window width and height
	int cx, cy;
	GLfloat h;
	int mx, my;
	
	switch(message)
	{
		case WM_CREATE:					// window is being created
			return 0;
			break;

		case WM_ACTIVATE:							// Watch For Window Activate Message
		{
			if (!HIWORD(wParam))					// Check Minimization State
			{
				g_active=TRUE;						// Program Is Active
			}
			else
			{
				g_active=FALSE;						// Program Is No Longer Active
			}

			return 0;								// Return To The Message Loop
		}


		case WM_CLOSE:					// windows is closing
			// send WM_QUIT to message queue
			PostQuitMessage(0);
			return 0;
		

		case WM_KEYDOWN:
			if(wParam == VK_ESCAPE)            
				g_bEnd = TRUE;
			else if ( wParam == VK_SPACE )
			{
				g_bPause = !g_bPause;			
			}				
			else if ( wParam == 79 ) // 'o'
			{
				bool bPause = g_bPause;
				g_bPause = true;

				TCHAR lpstrFile[MAX_PATH]="";  // MAX_PATH = 260

				OPENFILENAME OFN = 
                { 
		            sizeof(OPENFILENAME), 
                    g_hWnd, NULL,
		            "Bullet Physics Files (*.bullet)\0*.bullet\0ALL Files (*.*)\0*.*\0\0",
		            NULL, 0, 1, 
                    lpstrFile, MAX_PATH, 
                    NULL, 0,
		            NULL, "Load Bullet Physics File Dialog", 
                    OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, 0, 
                    0, "bullet", 0, 
                    NULL, NULL 
                };

				if ( GetOpenFileName(&OFN) )
				{
					if ( !g_BulletSim.LoadBulletFile(lpstrFile) )
					{
						MessageBox(g_hWnd, "Failed to load the bullet file", "Error", MB_OK | MB_ICONERROR);
					}
				}
			
				g_bPause = bPause;
			}
			else if ( wParam == 82 ) // 'r'
			{
				g_BulletSim.ReloadBulletFile();
			}
				
			break;


		case WM_SIZE:
			cy = HIWORD(lParam);		// retrieve width and height
			cx = LOWORD(lParam);

			GLfloat w;
			h = 1.0;

			if(cy == 0)
				w = (GLfloat) cx;
			else
				w = (GLfloat) cx / (GLfloat) cy;

			glViewport(0, 0, cx, cy);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glFrustum(-w, w, -h, h, 3.0, 3000.0);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			return 0;
			break;

		case WM_LBUTTONDOWN:
			mx = LOWORD(lParam);
			my = HIWORD(lParam);
			//SetCapture(g_hWnd);
			hCamera.mousePress(mx, my);
			break;

		case WM_LBUTTONUP:
			mx = LOWORD(lParam);
			my = HIWORD(lParam);
			//ReleaseCapture();
			hCamera.mouseRelease(mx, my);
			break;

		case WM_RBUTTONDOWN:
			mx = LOWORD(lParam);
			my = HIWORD(lParam);
			//SetCapture(g_hWnd);
			hCamera.mousePress(mx, my);
			break;

		case WM_RBUTTONUP:
			mx = LOWORD(lParam);
			my = HIWORD(lParam);
			//ReleaseCapture();
			hCamera.mouseRelease(mx, my);
			break;

		case WM_MBUTTONDOWN:
			mx = LOWORD(lParam);
			my = HIWORD(lParam);
			//SetCapture(g_hWnd);
			hCamera.mousePress(mx, my);
			break;

		case WM_MBUTTONUP:
			mx = LOWORD(lParam);
			my = HIWORD(lParam);
			//ReleaseCapture();
			hCamera.mouseRelease(mx, my);
			break;

		case WM_MOUSEWHEEL:
			{
				float zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
				hCamera.mouseWheel(-zDelta*0.13);
			}
			break;

		case WM_MOUSEMOVE:
			mx = LOWORD(lParam);
			my = HIWORD(lParam);

			gmx = mx; gmy = my;

			if( wParam == MK_LBUTTON )
			{
			   hCamera.mouseMove(mx, my, Left_button);
			}
			else if( wParam == MK_RBUTTON )
			{
			   hCamera.mouseMove(mx, my, Right_button);
			}
			else if( wParam == (MK_RBUTTON | MK_LBUTTON) )
			{
			   hCamera.mouseMove(mx, my, (Right_button | Left_button) );
			}
			else if( wParam == MK_MBUTTON )
			{
			   hCamera.mouseMove(mx, my, Middle_button);
			}			

			return 0;
			break;

		default:
			break;
	}

	return (DefWindowProc(hwnd, message, wParam, lParam));
}

//==============================================================
// function to set the pixel format for the device context
void SetupPixelFormat(HDC hDC)
//==============================================================
{
	int nPixelFormat;					// our pixel format index

	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),	// size of structure
		1,								// default version
		PFD_DRAW_TO_WINDOW |			// window drawing support
		PFD_SUPPORT_OPENGL |			// OpenGL support
		PFD_DOUBLEBUFFER,				// double buffering support
		PFD_TYPE_RGBA,					// RGBA color mode
		32,								// 32 bit color mode
		0, 0, 0, 0, 0, 0,				// ignore color bits, non-palettized mode
		0,								// no alpha buffer
		0,								// ignore shift bit
		0,								// no accumulation buffer
		0, 0, 0, 0,						// ignore accumulation bits
		16,								// 16 bit z-buffer size
		0,								// no stencil buffer
		0,								// no auxiliary buffer
		PFD_MAIN_PLANE,					// main drawing plane
		0,								// reserved
		0, 0, 0 };						// layer masks ignored

	nPixelFormat = ChoosePixelFormat(hDC, &pfd);	// choose best matching pixel format

	SetPixelFormat(hDC, nPixelFormat, &pfd);		// set pixel format to device context
}

//===============================
GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
//===============================
{
	glDeleteLists(btmPlate, 1);

	if (g_isFullscreen)									// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL,0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	if (g_hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL,NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(g_hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		g_hRC=NULL;										// Set RC To NULL
	}

	if (g_hDC && !ReleaseDC(g_hWnd,g_hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		g_hDC=NULL;										// Set DC To NULL
	}

	if (g_hWnd && !DestroyWindow(g_hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		g_hWnd=NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass(g_ClassName, g_hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		g_hInstance=NULL;									// Set hInstance To NULL
	}
}

GLuint GenerateAxis(float fAxisLength)
{
	GLuint list = glGenLists(1);

	GLfloat axisL = fAxisLength;
	GLfloat axis_matA[] = {0.8f, 0, 0, 0.6f };
	GLfloat axis_matB[] = {0.0, 0.0, 0.8f, 0.6f };

	glNewList(list, GL_COMPILE);	
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, axis_matA);
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glLineWidth(4);
	//glLineStipple(1, 0x0C0F);
	//glEnable(GL_LINE_STIPPLE);

	GLUquadricObj* quad = gluNewQuadric();

	glBegin(GL_LINES);

	//X Axis
	glPushMatrix();
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(axisL, 0.0, 0.0);
	glTranslatef(axisL, 0.0f, 0.0f);
	//gluCylinder(quad, 1.0, 0.0, 2.0, 5, 5);
	glPopMatrix();

	//Y Axis
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, axisL, 0.0);

	//Z Axis
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, axisL);

	glEnd();

	glDisable(GL_BLEND);	
	glEnable(GL_LIGHTING);

	glLineWidth(1);
	glEndList();

	return list;
}

//===================
void Initialize()
//===================
{
	/*if (glewInit() != GLEW_OK)
	{
		printf("GLEW initialization failed\n");
		return;
	}*/

	GLfloat ambientLight[4] = {0.2f, 0.2f, 0.2f, 1.0f};
	GLfloat diffuseLight[4] = {0.9f, 0.9f, 0.9f, 1.0f};
	GLfloat diffuseLight1[4] = {0.2f, 0.2f, 0.2f, 1.0f};
	GLfloat specularLight[4] = {0.8f, 0.8f, 0.8f, 1.0f};
	GLfloat lightPosition[4] = {30.0f, 30.0f, 30.0f, 0.0f};
	GLfloat lightPosition1[4] = {-30.0f, 30.0f, -30.0f, 0.0f};

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	
	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	//glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);	
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseLight1);
	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition1);
	glEnable(GL_LIGHT1);

	// Setting Camera Moving Sensitivity..
	hCamera.SetMouseSensitivity(1.5f, 0.3f, 0.3f);
	
	// Generating bottom plate...
	btmPlate = hCamera.makeBottomPlate(gray, black, 200.0f, 200.0f, 10.0f, 0.0f);

	// Generate an axis
	g_glLtAxis = GenerateAxis(5.0f);

	// Create a rigidbody simulation
	g_BulletSim.Create();
}

//===================
void RenderFunc()
//===================
{	
	static GLfloat RedSurface[]   = { 1.0f, 0.0f, 0.0f, 1.0f};
	static GLfloat GreenSurface[] = { 0.0f, 1.0f, 0.0f, 1.0f};
	static GLfloat BlueSurface[]  = { 0.0f, 0.0f, 1.0f, 1.0f};
	static GLfloat YellowSurface[]  = { 1.0f, 1.0f, 0.0f, 1.0f};

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// clear screen and depth buffer
	glLoadIdentity();										// reset modelview matrix
	
	//---------------
	//Draw background
	//---------------
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	glBegin(GL_QUADS);
		glColor3f(0.9F, 0.9F, 0.9F); glVertex2f( 1.0F,  1.0F);
		glColor3f(0.9F, 0.9F, 0.9F); glVertex2f(-1.0F,  1.0F);
		glColor3f(0.5F, 0.5F, 0.5F); glVertex2f(-1.0F, -1.0F);
		glColor3f(0.5F, 0.5F, 0.5F); glVertex2f( 1.0F, -1.0F);
	glEnd();
	
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	//----------------
	// Applying camera
	//----------------
	hCamera.ApplyCamera();
	
	glCallList(btmPlate);

	//-------------------------------------------
	// Rotate GL coordinate so that Z axis is up.
	//-------------------------------------------

	/*

			Y                         		Z
			|						  		|
			|						  		|
			|				  ===>	  		|
			|________ X				  		|________ Y
			/						  		/
		   /  GL coordinate			  	   /  Modeler coordinate
		  /							      /
		 Z							     X
	*/

	//glRotatef(-90.0f, 0, 0, 1.0f);
	//glRotatef(-90.0f, 0, 1.0f, 0);
	
	//-------------
	// Draw an axis
	//-------------
	glPushMatrix();
	glCallList(g_glLtAxis);
	glPopMatrix();

	glEnable(GL_LIGHTING);
	
	if ( !g_bPause )
		g_BulletSim.Update();
		
	g_BulletSim.Render(2);
		 
	glDisable(GL_LIGHTING);
}

void GetFPS()
{
	static FLOAT fFPS      = 0.0f;
	static FLOAT fLastTime = 0.0f;
	static DWORD dwFrames  = 0L;
	static bool bFirstTime = true;
	char str[100];

	// Keep track of the time lapse and frame count
	FLOAT fTime = timeGetTime() * 0.001f; // Get current time in seconds
	++dwFrames;

	g_DeltaTime = fTime - fLastTime;
	fLastTime = fTime;

	if ( bFirstTime )
	{
		g_DeltaTime = 0;
		bFirstTime = false;
	}

	// Update the frame rate once per 0.2 second
	if( fTime - fLastTime > 0.2f )
	{
		fFPS      = dwFrames / (fTime - fLastTime);
		dwFrames  = 0L;

		sprintf(str, "Cloth Simulation - Dongsoo Han    FPS : %d  mx : %d my : %d", (int)fFPS, gmx, gmy);
		SetWindowText(g_hWnd, str);
	}
}