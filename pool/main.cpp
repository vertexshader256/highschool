#include <windows.h>		
#include <stdio.h>
#include <math.h>
#include <fstream.h>
#include <iostream.h>		
#include <gl\gl.h>			
#include <gl\glu.h>
#include <gl\glaux.h>
#include <time.h>
#include "walls.h"
#include "timer.h"
#include "output.h"
#include "texture.h"
#include "mouse.h"
#include "balls.h"
#include "holes.h"
#include "fmod.h"
//#include "glext.h"

HDC			hDC=NULL;		
HGLRC		hRC=NULL;		
HWND		hWnd=NULL;		
HINSTANCE	hInstance;		

bool 	done=FALSE;								// Bool Variable To Exit Loop
bool	keys[256];			
bool	active=TRUE;		
bool	fullscreen=TRUE;

const MAX_WALLS = 100;

FSOUND_SAMPLE *sound1 = 0;
GLuint wallsDL;
GLuint sphereDL;

wallclass *wall = 0;
ballclass *ball = 0;
holeclass *hole = 0;
int count, ballcount, holecount;
void Load(char *loadfile)
{
	ifstream fin;
	fin.open(loadfile, ios::nocreate);
	if (fin.good())
	{
	fin >> count;
	wall = new wallclass[count];
	fin >> ballcount;
	ball = new ballclass[ballcount];
	fin >> holecount;
	hole = new holeclass[holecount];


	for (int i = 0; i < count; i++)
	{
		fin >> wall[i].linex;
		fin >> wall[i].line2x;
		fin >> wall[i].liney;
		fin >> wall[i].line2y;
		fin >> wall[i].longx;
		fin >> wall[i].shortx;
		fin >> wall[i].longy;
		fin >> wall[i].shorty;
		fin >> wall[i].slope;
		fin >> wall[i].yintercept;
		fin >> wall[i].red;
		fin >> wall[i].green;
		fin >> wall[i].blue;
	}
	for (i = 0; i < ballcount; i++)
	{
		fin >> ball[i].X;
		fin >> ball[i].Y;
	}
	for (i = 0; i < holecount; i++)
	{
		fin >> hole[i].X;
		fin >> hole[i].Y;
	}

	}
	fin.close();
}


//wallclass wall[100];
textureclass texture[2];
outputclass output;
mouseclass mouse;
timerclass timer2;
particleclass particle[10];

GLUquadricObj *quadratic;

static charplace = -1;
bool loading;

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM lParam);	

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		
{
	if (height==0)										
	{
		height=1;										
	}

	glViewport(0,0,width,height);						

	glMatrixMode(GL_PROJECTION);						
	glLoadIdentity();									

	// Calculate The Aspect Ratio Of The Window
	//gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);
	gluOrtho2D(0, 640, 0, 480);
	glMatrixMode(GL_MODELVIEW);							
	glLoadIdentity();									
}

void CaptureScreen()
{
BITMAPFILEHEADER bf;
BITMAPINFOHEADER bi;
unsigned char *image	= (unsigned char*)malloc(sizeof(unsigned char)*640*480*3);
FILE *file				= fopen("screenshot.bmp", "wb");
if( image!=NULL )
{
if( file!=NULL )
{
glReadPixels( 0, 0, 640, 480, GL_BGR_EXT, GL_UNSIGNED_BYTE, image );
memset( &bf, 0, sizeof( bf ) );
memset( &bi, 0, sizeof( bi ) );
bf.bfType			= 'MB';
bf.bfSize			= sizeof(bf)+sizeof(bi)+640*480*3;
bf.bfOffBits		= sizeof(bf)+sizeof(bi);
bi.biSize			= sizeof(bi);
bi.biWidth			= 640;
bi.biHeight			= 480;
bi.biPlanes			= 1;
bi.biBitCount		= 24;
bi.biSizeImage		= 640*480*3;
fwrite( &bf, sizeof(bf), 1, file );
fwrite( &bi, sizeof(bi), 1, file );
fwrite( image, sizeof(unsigned char), 480*640*3, file );
fclose( file );
}
free( image );
}
}

void InitGL(GLvoid)										
{
	gluOrtho2D(0, 640, 0, 480);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_BLEND);	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH); glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_POLYGON_SMOOTH); glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
	glEnable(GL_LINE_SMOOTH);  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glShadeModel(GL_FLAT);
	//glDisable(GL_DITHER);
	glLineWidth(1.4f);
	glClearColor(0.0f, 0.7f, 0.0f, 0.5f);				
	glPointSize(6.0f);
	//ball.XVector = 0.5f;
	//ball.YVector = 0.5f;
	glAlphaFunc(GL_NOTEQUAL,0.0f);
	srand( time(NULL) );

	GLfloat mat_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat mat_shininess[] = {120.0f};
	GLfloat light_position[] = {1.0f, 1.0f, 5.0f, 0.0f};
	GLfloat white_light[]  = {1.0f, 1.0f, 1.0f, 1.0f};
//	GLfloat lmodel_ambient[] = {0.0f, 0.0f, 1.0f, 1.0f};
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);

	//glEnable(GL_LIGHT1);

	texture[0].LoadTGA(1,"Data/Font.tga");
	texture[0].BuildFont();
	texture[1].LoadTGA(2,"Data/Ground.tga");

quadratic=gluNewQuadric();
gluQuadricNormals(quadratic, GLU_SMOOTH);
//gluQuadricTexture(quadratic, GL_TRUE);

	FSOUND_Init(44100, 32, 0);

	Load("Maps/brian.dlf");

	wallsDL = glGenLists(1);
	glNewList(wallsDL,GL_COMPILE);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINES);
	for (int i = 0; i < count; i++)
	{
	wall[i].DrawLine();
	}
	glEnd();
	glEndList();

	sphereDL = glGenLists(1);
	glNewList(sphereDL,GL_COMPILE);
	gluSphere(quadratic,6.0f,10,10);
	glEndList();


	sound1 = FSOUND_Sample_Load(FSOUND_UNMANAGED, "Data/dang.wav", FSOUND_NORMAL, 0);
}

void DrawGLScene(GLvoid)									
{
	timer2.begintime = timeGetTime();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // | GL_DEPTH_BUFFER_BIT);

	//glBegin(GL_LINES);
	//for (int i = 0; i < count; i++)
	//{
	//wall[i].DrawLine();
	//}
	//glEnd();
	glCallList(wallsDL);

	for (int f = 0; f < ballcount; f++)
	{
	if (ball[f].Alive == false || ball[f].moving == false)
		continue;
	int temp = ceil(ball[f].speed*timer2.scalar);
	for (int i = 0; i < temp; i++)
	{
		if (ball[f].CollisionDetection(&wall[0], timer2.scalar, &ball[0], f, &particle[0], sound1, count, ballcount) == false)
			ball[f].Move(timer2.scalar, temp);
	}

	if (f != 0) //so cue ball wont go in hole
	{
		for (int i = 0; i < holecount; i++)
		{
		ball[f].Hole(hole[i].X, hole[i].Y,timer2.scalar);
		}
	}
	}


	mouse.DrawMouse();

	for (f = 0; f < holecount; f++)
	{
	hole[f].DrawHole();
	}

	//glEnable(GL_TEXTURE_2D);
	output.SaveButton(mouse.mouse_x, mouse.mouse_y, 580, 630, 62, 78);
	glEnable(GL_ALPHA_TEST);
	texture[0].glPrint(0.8f, 580, 63, 0, "%2i", timer2.ShowFps());
	glDisable(GL_ALPHA_TEST);
	//glDisable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);

	for (f = 0; f < ballcount; f++)
	{
	if (ball[f].Alive == false)
		continue;
	ball[f].DrawBlur(timer2.scalar);
	}
	//a check to see if all balls have stopped
	if (ball[0].Moving(&ball[0], ballcount))
	{
		//if so, aim with cue ball (ball[0])
		ball[0].Aim(mouse.mouse_x, mouse.mouse_y, &ball[0], ballcount, mouse.lbutton, &wall[0], count, keys[VK_LEFT], keys[VK_RIGHT], timer2.scalar);
	}

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 640, 0, 480, 6, 0);


	//gluPerspective(45.0f,(GLfloat)640/(GLfloat)480,0.1f,100.0f);
	glBindTexture(GL_TEXTURE_2D, 2);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_SMOOTH);
	//glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
			float light_position1[] = {mouse.mouse_x, mouse.mouse_y, -15.0f, 1.0f};
	glLightfv(GL_LIGHT0, GL_POSITION, light_position1);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);
glCullFace(GL_FRONT_AND_BACK);
	glEnable(GL_CULL_FACE);

	
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT,GL_DIFFUSE);

	for (f = 0; f < ballcount; f++)
	{
	if (ball[f].Alive == false)
		continue;
	glPushMatrix();
	//if (f == 0)
		glDisable(GL_TEXTURE_2D);
	ball[f].DrawBall(sphereDL);
	glPopMatrix();
	//if (f == 0)
	//	glEnable(GL_TEXTURE_2D);
	}
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	glShadeModel(GL_FLAT);
	glDisable(GL_CULL_FACE);
	glDisable(GL_POLYGON_SMOOTH);
	//glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHT0);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	for (f = 0; f < 10; f++)
	{
		particle[f].DrawParticle(timer2.scalar);
	}
	if (mouse.lbutton)
		mouse.lbutton=false;
}

GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
{
	if (fullscreen)										// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL,0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL,NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL",hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;									// Set hInstance To NULL
	}
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/
 
BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;			// Set Left Value To 0
	WindowRect.right=(long)width;		// Set Right Value To Requested Width
	WindowRect.top=(long)0;				// Set Top Value To 0
	WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height

	fullscreen=fullscreenflag;			// Set The Global Fullscreen Flag

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}
	
	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullscreen=FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle=WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle=WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
								"OpenGL",							// Class Name
								title,								// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0,								// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	
	if (!(hDC=GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC=wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!wglMakeCurrent(hDC,hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen
	InitGL();

	return TRUE;									// Success
}

LRESULT CALLBACK WndProc(	HWND	hWnd,			// Handle For This Window
							UINT	uMsg,			// Message For This Window
							WPARAM	wParam,			// Additional Message Information
							LPARAM	lParam)			// Additional Message Information
{
	switch (uMsg)									// Check For Windows Messages
	{
		case WM_ACTIVATE:							// Watch For Window Activate Message
		{
			if (!HIWORD(wParam))					// Check Minimization State
			{
				active=TRUE;						// Program Is Active
			}
			else
			{
				active=FALSE;						// Program Is No Longer Active
			}

			return 0;								// Return To The Message Loop
		}

		case WM_SYSCOMMAND:							// Intercept System Commands
		{
			switch (wParam)							// Check System Calls
			{
				case SC_SCREENSAVE:					// Screensaver Trying To Start?
				case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
				return 0;							// Prevent From Happening
			}
			break;									// Exit
		}

		case WM_CLOSE:								// Did We Receive A Close Message?
		{
			PostQuitMessage(0);						// Send A Quit Message
			return 0;								// Jump Back
		}

		case WM_KEYDOWN:							// Is A Key Being Held Down?
		{
			keys[wParam] = TRUE;					// If So, Mark It As TRUE
			return 0;								// Jump Back
		}

		case WM_KEYUP:								// Has A Key Been Released?
		{
			keys[wParam] = FALSE;					// If So, Mark It As FALSE
			return 0;								// Jump Back
		}

		case WM_SIZE:								// Resize The OpenGL Window
		{
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));  // LoWord=Width, HiWord=Height
			return 0;								// Jump Back
		}

		case WM_CHAR:
		{
		//	if (saving == true && keys[VK_RETURN] == false && keys[VK_BACK] == false)
		//	{
		//	if (charplace<9)
		//		charplace++;
		//	output.savefile[charplace] = (char)wParam;
		//	strcpy (output.tempfile, output.savefile);
		//	strncat (output.tempfile, ".dlf", 4);
		//	}
		//	else if (loading == true && keys[VK_RETURN] == false && keys[VK_BACK] == false)
		//	{
		//	if (charplace<9)
		//		charplace++;
		//	output.loadfile[charplace] = (char)wParam;
		//	strcpy (output.tempfile, output.loadfile);
		//	strncat (output.tempfile, ".dlf", 4);

			// *file = *inputtext;
		//	}
			break;
		}

	//	case WM_LBUTTONDOWN:
	//	{
	//	break;
	//	}

		case WM_LBUTTONUP:
		{
		mouse.lbutton = true;
		if (mouse.mouse_x > 580 && mouse.mouse_x < 630 && mouse.mouse_y > 62 && mouse.mouse_y < 78)
			done = true;
		break;
		}

	//	case WM_RBUTTONDOWN:
	//	{
	//	break;
	//	}

	//	case WM_RBUTTONUP:
	//	{
	//	break;
	//	}




		case WM_MOUSEMOVE:
		{
	    mouse.mouse_x = LOWORD(lParam);          
		mouse.mouse_y = HIWORD(lParam);
		mouse.mouse_y = 480 - mouse.mouse_y;
		//if (mouse.mouse_y > 479)
		//	SetCursorPos(mouse.mouse_x, 1);
		break;
		}



	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

int WINAPI WinMain(	HINSTANCE	hInstance,			// Instance
					HINSTANCE	hPrevInstance,		// Previous Instance
					LPSTR		lpCmdLine,			// Command Line Parameters
					int			nCmdShow)			// Window Show State
{
	MSG		msg;									// Windows Message Structure

	// Ask The User Which Screen Mode They Prefer
	if (MessageBox(NULL,"Would You Like To Run In Fullscreen Mode?", "Start FullScreen?",MB_YESNO|MB_ICONQUESTION)==IDNO)
	{
		fullscreen=FALSE;							// Windowed Mode
	}

	// Create Our OpenGL Window
	if (!CreateGLWindow("NeHe's OpenGL Framework",640,480,16,fullscreen))
	{
		return 0;									// Quit If Window Was Not Created
	}

	while(!done)									// Loop That Runs While done=FALSE
	{
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message==WM_QUIT)				// Have We Received A Quit Message?
			{
				done=TRUE;							// If So done=TRUE
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else										// If There Are No Messages
		{
			// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if (active)								// Program Active?
			{
				if (keys[VK_ESCAPE])				// Was ESC Pressed?
				{
					done=TRUE;						// ESC Signalled A Quit
				}
				else								// Not Time To Quit, Update Screen
				{
					DrawGLScene();					// Draw The Scene
					SwapBuffers(hDC);				// Swap Buffers (Double Buffering)
					timer2.scalar = (timeGetTime() - timer2.begintime)/1000.0f;
				}
			}
				
			if (keys[VK_SPACE])
			{
				mouse.lbutton = true;
			}

			if (keys[VK_RETURN])
			{
				if (loading == true)
				{
				strcpy (output.tempfile,"Maps/");
				strncat (output.tempfile, output.loadfile, strlen(output.loadfile));
				strncat (output.tempfile, ".dlf", 4);

				loading = false;
				charplace = -1;
				//for (int i = 0; i < 30; i++)
				//	output.loadfile[i] = ' ';
				strcpy (output.tempfile, ".dlf");
				strcpy (output.loadfile, "");
				}
				keys[VK_RETURN] = false;
			}

			if (keys[VK_UP]) {
				CaptureScreen();
				keys[VK_UP] = false;
			}

			if (keys[VK_F1])						// Is F1 Being Pressed?
			{
				keys[VK_F1]=FALSE;					// If So Make Key FALSE
				KillGLWindow();						// Kill Our Current Window
				fullscreen=!fullscreen;				// Toggle Fullscreen / Windowed Mode
				// Recreate Our OpenGL Window
				if (!CreateGLWindow("NeHe's OpenGL Framework",640 ,480,16,fullscreen))
				{
					return 0;						// Quit If Window Was Not Created
				}
			}
		}
	}
	delete [count]wall;
	delete [ballcount]ball;
	delete [holecount]hole;
	for (int i = 0; i < 1; i++)
		glDeleteTextures(1, texture[i].texture);
	texture[0].KillFont();
	glDeleteLists(wallsDL, 1);
	glDeleteLists(sphereDL, 1);
	//FSOUND_Sample_Free(sound2);
	FSOUND_Sample_Free(sound1);
	FSOUND_Close();
	KillGLWindow();									// Kill The Window
	return (msg.wParam);							// Exit The Program
}