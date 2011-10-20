//////////////////////////////////////////////////////////////
///                                                        ///
///         HGLCamera.cpp                                  ///
///                                                        ///
///         Nov. 21. 2000                                  ///
///         Dong Soo Han                                   ///  
///                                                        ///  
//////////////////////////////////////////////////////////////

#include "HGLCamera.h"
#include <math.h>

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
HGLCamera::HGLCamera(MouseButtonMode m, GLfloat init_distance, GLfloat init_x_pan, 
                     GLfloat init_y_pan, GLfloat init_alpha, GLfloat init_beta)
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
{
    for(int i=0; i<4; i++)
    {    
	   for(int j=0; j<4; j++)
	   {
            rotateTransformMx[i][j]=0.0;
            panTransformMx[i][j]=0.0;
            upvectorTransformMx[i][j]=0.0;    


            if(i == j)
            {
               former_rotateTransformMx[i][j]=1.0;
               former_panTransformMx[i][j]=1.0;
               former_upvectorTransformMx[i][j]=1.0;  
            }

            else  
            {
               former_rotateTransformMx[i][j]=0.0;
               former_panTransformMx[i][j]=0.0;
               former_upvectorTransformMx[i][j]=0.0;  
            } 

	   }
    }

    distance_betw_cam_n_WOrgin = init_distance;
    Hx_pan = init_x_pan;
    Hy_pan = init_y_pan;
    Halpha = init_alpha;
    Hbeta = init_beta;

    HzoomRate = 0;
    Hx_panRate = 0;
    Hy_panRate = 0;
    HalphaRate = 0;
    HbetaRate = 0;

    HzoomSpeedRate = 0.1f;
    HpanSpeedRate = 0.008f;
    HrotateSpeedRate = 0.015f;

    // default mouse mode is two buttons mouse. 
     if( m == twoButtons )
     {     
    	MouseButtonModeSetting.Zoom_Mouse_Button =  Left_button;
    	MouseButtonModeSetting.Pan_Mouse_Button = Right_button;
    	MouseButtonModeSetting.Rotate_view_Mouse_Button = ( Left_button | Right_button );
     }

     else if( m == threeButtons )
     {   
    	MouseButtonModeSetting.Zoom_Mouse_Button =  Middle_button;
    	MouseButtonModeSetting.Pan_Mouse_Button = Right_button;
    	MouseButtonModeSetting.Rotate_view_Mouse_Button = ( Middle_button | Right_button );  
     }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~*/
HGLCamera::~HGLCamera()
/*~~~~~~~~~~~~~~~~~~~~~~~~*/
{  
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void HGLCamera::ApplyCamera()
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*********************************************************************************************************
*   'alpha_angle' is X axis rotating angle, and 'beta_angle' is Y axis rotating angle.                   *    
*   They are degree units.                                                                               *
*   They are absolute angles from origin axis.                                                           *
*                                                                                                        *
*   'delta_x_pan' & 'delta_y_pan' are relative values.                                                   * 
*   They are related not world coordinate but window coordinate(viewer coordinate).                      *
*   So if 'delta_x_pan' would increase, all of objects in window would be moved to monitor's rightside.  *
*   'delta_y_pan' is too.                                                                                *
*                                                                                                        * 
*   'Distance' invokes zoom effect.                                                                      *  
*   If 'Distance' would increase, zoomout would be invoked. vice versa.                                  *
*   Distance is absolute value.                                                                          * 
*********************************************************************************************************/
{
    static GLfloat X = 0.0f;
	static GLfloat Y = 0.0f;
	static GLfloat Z = 0.0f;
    GLfloat a, b = 0.0f;

  ////////////////////////////////////////////////////////
  // for transforming Object coords to World coords     //
  // This is very very important.                       //
  ////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////////////////////////
  //                                                                                   //
  //    |   |    |                      |     |                      |     |        |  //        
  //    | X |    | cos(b)   0   -sin(b) |     | 1      0       0     |     | Hx_pan |  //                                                     
  //    | Y |  = |   0      1     0     |  X  | 0    cos(a)   sin(a) |  X  | Hy_pan |  //                                                                     
  //    | Z |    | sin(b)   0    cos(b) |     | 0   -sin(a)   cos(a) |     |    0   |  //                                                              
  //    |   |    |                      |     |                      |     |        |  //                                        
  //                                                                                   //                     
  ///////////////////////////////////////////////////////////////////////////////////////
    
    a = DEG2RAD(Halpha);
    b = DEG2RAD(Hbeta); 

    X += Hx_pan*cos(b) + Hy_pan*sin(a)*sin(b);
    Y += Hy_pan*cos(a);
    Z += Hx_pan*sin(b) - Hy_pan*sin(a)*cos(b);

  ////////////////////////////////////////////////////////

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(0.0, 0.0, -distance_betw_cam_n_WOrgin);

    glRotatef(Halpha, 1.0, 0.0, 0.0);
    glRotatef(Hbeta, 0.0, 1.0, 0.0);

    glTranslatef(X, Y, Z);

    Hx_pan = 0.0;
    Hy_pan = 0.0;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
Point3Df HGLCamera::GetUnProjectPoint(GLdouble winX, GLdouble winY, GLdouble winZ)
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
{
    GLdouble objX, objY, objZ;
    GLdouble model[16], proj[16]; 
    GLint view[4];
    Point3Df  objPoint;                   

    objX = objY = objZ = 0.0;

    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glGetIntegerv(GL_VIEWPORT, view);

    gluUnProject(winX, winY, winZ,
                 model, proj, view,
                 &objX, &objY, &objZ );

    objPoint.x =  (GLfloat)objX;
    objPoint.y =  (GLfloat)objY;
    objPoint.z =  (GLfloat)objZ;

    return objPoint;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
Point3Df HGLCamera::GetProjectPoint(GLdouble objX, GLdouble objY, GLdouble objZ)
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
{
    GLdouble winX, winY, winZ;
    GLdouble model[16], proj[16]; 
    GLint view[4];
    Point3Df  winPoit;                   

    winX = winY = winZ = 0.0;

    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glGetIntegerv(GL_VIEWPORT, view);

    gluProject(objX, objY, objZ,
               model, proj, view,
               &winX, &winY, &winZ );  

    winPoit.x =  (GLfloat)winX;
    winPoit.y =  (GLfloat)winY;
    winPoit.z =  (GLfloat)winZ;

    return winPoit;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
GLuint HGLCamera::makeBottomPlate(const GLfloat plate_color[], const GLfloat grid_color[], 
                           GLfloat width, GLfloat length, GLfloat grid_interval_distance, GLfloat y)
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// if grid_interval_distance equals 0, there is no grid.
{
   //////////////////////////////
   //  make a bottom plain
   //////////////////////////////
   GLfloat x = width/2;
   GLfloat z = length/2;

   GLuint listName = glGenLists (1);
   
   glNewList (listName, GL_COMPILE);
	   glPushMatrix();
	   glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, plate_color );
	   glColor3fv(plate_color);

       glTranslatef(0.0, y, 0.0);
       //Polygon offset for bottom plate grid.
	   glEnable(GL_POLYGON_OFFSET_FILL);
       glPolygonOffset(1.0, 1.0);
	   //making simple plate without grid.
	   glBegin(GL_QUADS);
		  glNormal3f(0.0, 1.0, 0.0);
		  glVertex3f(x, 0, z);
		  glVertex3f(x, 0, -z);      
		  glVertex3f(-x, 0, -x);
		  glVertex3f(-x, 0, z);
	   glEnd();
       glDisable(GL_POLYGON_OFFSET_FILL);

	   glDisable(GL_LIGHTING);
       glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	   glColor3fv(grid_color);

	   //making border lines and grid.
	   glBegin(GL_LINES);
			 if( grid_interval_distance != 0 )
			 {
    			 // make border lines
 				 glVertex3f(x, 0.0, z);
				 glVertex3f(-x, 0.0, z);

				 glVertex3f(x, 0.0, -z);
				 glVertex3f(-x, 0.0, -z);

				 glVertex3f(x, 0.0, z);
				 glVertex3f(x, 0.0, -z);

				 glVertex3f(-x, 0.0, z);
				 glVertex3f(-x, 0.0, -z);

				 // make grid  
       			 glVertex3f(-x, 0.0, 0.0);
				 glVertex3f(x, 0.0, 0.0);

				 glVertex3f(0.0, 0.0, z);
				 glVertex3f(0.0, 0.0, -z);

				 GLfloat temp = grid_interval_distance;       

				 for(grid_interval_distance ; grid_interval_distance < x ; grid_interval_distance += temp)
				 {
					glVertex3f(grid_interval_distance, 0.0, z);
					glVertex3f(grid_interval_distance, 0.0, -z);

					glVertex3f(-grid_interval_distance, 0.0, z);
					glVertex3f(-grid_interval_distance, 0.0, -z);
				 } // end of for-loop

				grid_interval_distance = temp;  
     
				for(grid_interval_distance ; grid_interval_distance < z ; grid_interval_distance += temp)
				{
				   glVertex3f(x, 0.0, grid_interval_distance);
				   glVertex3f(-x, 0.0, grid_interval_distance);

				   glVertex3f(x, 0.0, -grid_interval_distance);
				   glVertex3f(-x, 0.0, -grid_interval_distance);
		  		} // end of for-loop
			 }  // end of if
   
	   glEnd(); //end of GL_LINES
       glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	   glEnable(GL_LIGHTING);
	   glPopMatrix();

   glEndList();

   return listName;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void HGLCamera::setMouseButtonMode(MouseButtonMode m)
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
{
	if( m == twoButtons )
	{     
		MouseButtonModeSetting.Zoom_Mouse_Button =  Left_button;
		MouseButtonModeSetting.Pan_Mouse_Button = Right_button;
		MouseButtonModeSetting.Rotate_view_Mouse_Button = ( Left_button | Right_button );
	}
	else if( m == threeButtons )
	{  
		MouseButtonModeSetting.Zoom_Mouse_Button =  Middle_button;
		MouseButtonModeSetting.Pan_Mouse_Button = Right_button;
		MouseButtonModeSetting.Rotate_view_Mouse_Button = ( Middle_button | Right_button );  
	}
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void HGLCamera::mousePress(GLint x, GLint y)
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
{
     HzoomRate = 0;
     Hx_panRate = 0;
     Hy_panRate = 0;
     HalphaRate = 0;
     HbetaRate = 0;

     HmousePressPoint.x = (GLfloat)x;
     HmousePressPoint.y = (GLfloat)y;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void HGLCamera::mouseRelease(GLint x, GLint y)
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
{
     HzoomRate = 0;
     Hx_panRate = 0;
     Hy_panRate = 0;
     HalphaRate = 0;
     HbetaRate = 0;

     HmousePressPoint.x = (GLfloat)x;
     HmousePressPoint.y = (GLfloat)y;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void HGLCamera::mouseMove(GLint x, GLint y, GLint Mouse_button_press_status)
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
{
    if(Mouse_button_press_status == MouseButtonModeSetting.Zoom_Mouse_Button )
    {
       HzoomRate = (GLint)(x - HmousePressPoint.x);
    }
    else if(Mouse_button_press_status == MouseButtonModeSetting.Pan_Mouse_Button )
    {
       Hx_panRate = (GLint)(x - HmousePressPoint.x); 
       Hy_panRate = (GLint)(-y + HmousePressPoint.y); 
    }
    else if(Mouse_button_press_status == MouseButtonModeSetting.Rotate_view_Mouse_Button )
    {
       HalphaRate =  (GLint)(-y + HmousePressPoint.y); 
       HbetaRate = (GLint)(-x + HmousePressPoint.x); 
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void HGLCamera::SetMouseSensitivity(GLfloat zoom, GLfloat pan, GLfloat rot)
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
{
	if(zoom > 1.0f)  zoom = 1.0f;
		else if(zoom < 0.0f) zoom = 0.0f;

	if(pan > 1.0f)  pan = 1.0f;
		else if(pan < 0.0f) pan = 0.0f;

	if(rot > 1.0f)  rot = 1.0f;
		else if(rot < 0.0f) rot = 0.0f;

    HzoomSpeedRate *= zoom;
    HpanSpeedRate *= pan;
    HrotateSpeedRate *= rot;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void HGLCamera::Update(void)
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
{
    if(HzoomRate != 0)    
    {
        distance_betw_cam_n_WOrgin -= HzoomRate*HzoomSpeedRate ;
    }

    Hx_pan = Hx_panRate*HpanSpeedRate;
    Hy_pan = Hy_panRate*HpanSpeedRate;

    Halpha += HalphaRate*HrotateSpeedRate;
    Hbeta += HbetaRate*HrotateSpeedRate;
}
