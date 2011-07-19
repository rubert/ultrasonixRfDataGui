#include "../nehewidget.h"
#include <cmath>

class NeHeChapter2 : public NeHeWidget
{
public:
	NeHeChapter2( QWidget *parent=0, char *name=0 ) : NeHeWidget( 0, parent, name )
	{
	}
	
protected:
	void initializeGL()
	{
		glShadeModel(GL_SMOOTH);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearDepth(1.0f);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	}
	
	void resizeGL( int width, int height )
	{
  	height = height?height:1;

		glViewport( 0, 0, (GLint)width, (GLint)height );

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
	
	void paintGL()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();   //reset the view so its centered on the position (0,0,0)


		const float PI = 3.14;
		const float MIN_ANGLE = -PI/6;
		const float MAX_ANGLE = PI/6;
		const float SECTOR_RAD = PI/3;

	glTranslatef(0.0f,0.5f,-5.0f);					// Move Into The Screen 4.0


		glBegin(GL_QUAD_STRIP);
		int x1 = 0;  //origin of sector array
		int y1 = 2;

		float radius2 = 4.0;
		float radius1 = 1.0;

		float t = 333.0;
		//Need to loop through A-lines and specify end then start for vertexes
		for(float angle = 3*PI/2 + MIN_ANGLE; angle <= (3*PI/2 + MAX_ANGLE + SECTOR_RAD/t); angle += SECTOR_RAD/t)
		{
		float vnorm = 1.0 - ((angle - PI - MIN_ANGLE)/SECTOR_RAD );
		//glTexCoord2f(vnorm, 1.0);
		glVertex2f(x1+ cos(angle)*radius1, y1 +sin(angle)*radius1);   //beginning of A-line in image 
		//glTexCoord2f(vnorm, 0.0);  //end of A-line in image
		glVertex2f(x1 + cos(angle)*radius2, y1 + sin(angle)*radius2);
		}
		glEnd();
	
	}
};
