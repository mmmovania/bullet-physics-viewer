//////////////////////////////////////////////////////////////
///                                                        ///
///         HGLCamera.h                                    ///
///                                                        ///
///         Nov. 21. 2000                                  ///
///         Dong Soo Han                                   ///  
///                                                        ///  
//////////////////////////////////////////////////////////////


#ifndef __HGLCAMERA_H__
#define __HGLCAMERA_H__

#if defined(_WIN32)
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#ifndef PI
#define PI   (3.14159265359f)
#endif

#ifndef DEG2RAD
#define DEG2RAD(x)      (PI/180.0f)*(x)
#endif

#ifndef RAD2DEG
#define RAD2DEG(x)      (180.0f/PI)*(x)
#endif


#define Left_button     1
#define Middle_button   2
#define Right_button    4

struct  Point3Df
	{ 
       GLfloat x; 
       GLfloat y; 
       GLfloat z;
	};

struct  Point2Df
	{ 
       GLfloat x; 
       GLfloat y; 
    };

enum MouseButtonMode { twoButtons = 2, threeButtons = 3 };

class  HGLCamera
{
public:
    HGLCamera(MouseButtonMode m = twoButtons, bool bContinuous = false, GLfloat init_distance = 20.0, GLfloat init_x_pan = 0.0, 
            GLfloat init_y_pan = 0.0, GLfloat init_alpha = 0.0, GLfloat init_beta = 0.0);          
    virtual ~HGLCamera();

    void ApplyCamera(); 
 
    GLuint makeBottomPlate(const GLfloat plate_color[], const GLfloat grid_color[], 
                        GLfloat width = 25.0, GLfloat length = 25.0, 
                        GLfloat grid_interval_distance = 5.0, GLfloat y = 0.0);

    Point3Df GetUnProjectPoint(GLdouble winX, GLdouble winY, GLdouble winZ);
    Point3Df GetProjectPoint(GLdouble objX, GLdouble objY, GLdouble objZ);

    void setMouseButtonMode(MouseButtonMode m);

	void mousePress(GLint x, GLint y);
	void mouseRelease(GLint x, GLint y);
	void mouseMove(GLint x, GLint y, GLint Mouse_button_press_status);
	void Update(void);

	void SetMouseSensitivity(GLfloat zoom, GLfloat pan, GLfloat rot); 
	void SetContinouse(bool bContinuous) { m_bContinuous = bContinuous; }

private:
	bool m_bContinuous;

	GLfloat rotateTransformMx[4][4];
	GLfloat panTransformMx[4][4];
	GLfloat upvectorTransformMx[4][4];
	GLfloat former_rotateTransformMx[4][4];
	GLfloat former_panTransformMx[4][4];
	GLfloat former_upvectorTransformMx[4][4];

	GLfloat  distance_betw_cam_n_WOrgin;  // a distance between camera(viewer) and world orgin
	GLfloat  Hx_pan;   // panning distance along X axis whitch is based on User carmera coordinate
	GLfloat  Hy_pan;   // panning distance along Y axis whitch is based on User carmera coordinate
	GLfloat  Halpha;   // angle around X axis
	GLfloat  Hbeta;    // angle around Y axis
	Point2Df HmousePressPoint;  // 2D point of mouse pressing

	GLint old_x;
	GLint old_y;
		 
	GLint    HzoomRate;
	GLint    Hx_panRate; 
	GLint    Hy_panRate; 
	GLint    HalphaRate;
	GLint    HbetaRate;

	GLfloat  HzoomSpeedRate;
	GLfloat  HpanSpeedRate;
	GLfloat  HrotateSpeedRate;

	struct{
			GLint Zoom_Mouse_Button;
			GLint Pan_Mouse_Button;
			GLint Rotate_view_Mouse_Button;
		}  MouseButtonModeSetting; 
};


#endif /* __HGLCAMERA_H__ */
