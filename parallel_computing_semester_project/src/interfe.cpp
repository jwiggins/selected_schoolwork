/*
  interfe.cpp
  $Id: interfe.cpp,v 1.5 2003/05/04 23:18:05 prok Exp $
  
  $Log: interfe.cpp,v $
  Revision 1.5  2003/05/04 23:18:05  prok
  Cleanup day! Several preemptive changes made before testing begins:
  - interfe.cpp got some changes related to outputting view params so that output
  from the renderer more closely matches OpenGL output
  - cachmanager.cpp lrumemcache.* and volren_par.cpp got some code to assist in
  exiting cleanly at the end of the render
  - extra attention was paid to asynchronous I/O voodoo in lrumemcache.*.
  Specifically, I/O now happens in two threads, requests get stuffed into a queue
  before being handled, requests stay in a list until they are received. All told,
  I think this code will actually work with the way it is being called now.(lots
  of calls to FetchBlock() were a vector for ugliness)
  - fixed a tiny bug in pvolume.cpp where a ray's state could change even if it
  didn't actually complete a sample.
  - moved sequential rendering out of interfe.cpp and into volren_seq.cpp.

  Revision 1.4  2003/04/25 21:43:25  prok
  Volume rendering is finally correct. Shading is turned off, because it's broken.
  Two nasty bugs were squashed in transferfunction.h. Data distribution code in
  read_data.cpp was modified to do a random distribution, rather than a modulo
  distribution.

  Revision 1.3  2003/04/21 04:24:15  prok
  improved rendering. works with float and char datasets so far. (should work
  just fine on short data)

  Revision 1.2  2003/04/20 08:14:49  prok

  This is the initial working version. Output is mostly correct...
  I Need to look at more transfer functions and datasets.

  Revision 1.1.1.1  2003/04/19 05:18:21  prok
  This is my parallel ray casting volume renderer. I'm writing it for CS377, but
  if it turns out that this parallelization strategy is fast, I might develop it
  further.

*/

#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>

#include "veclib.h"
#include "quaternion.h"
#include "invertMatrix.h"
#include "ray.h"

#define Z_NEAR 1.0
#define Z_FAR 10000.0

bool mouse_is_down = false;
int oldX, oldY;
int screen_width, screen_height;

Quaternion rotation;
double rotMat[16] = {0.0}, eye_dist=5.0;
float dims[3],lookat[3];

void swap_big_to_little_float(float &val, char *buffer)
{
	union {
		float value;
		char bytes[4];
	} fs;

	// swap the order (little -> big)
	fs.bytes[0] = buffer[3];
	fs.bytes[1] = buffer[2];
	fs.bytes[2] = buffer[1];
	fs.bytes[3] = buffer[0];
	// assign the value
	val = fs.value;
}

void parse_rawiv_header(char *buffer,float *min, float *max)
{
	// read minX
	swap_big_to_little_float(min[0], buffer);
	buffer += 4;
	// read minY
	swap_big_to_little_float(min[1], buffer);
	buffer += 4;
	// read minZ
	swap_big_to_little_float(min[2], buffer);
	buffer += 4;

	// read maxX
	swap_big_to_little_float(max[0], buffer);
	buffer += 4;
	// read maxY
	swap_big_to_little_float(max[1], buffer);
	buffer += 4;
	// read maxZ
	swap_big_to_little_float(max[2], buffer);
}

void do_rotation(double theta, double ax, double ay, double az)
{
  Quaternion q;

  // create the quaternion describing the rotation
  q = Quaternion::RotationQuat(theta, ax, ay, az);

  // rotate the rotation vector
  rotation = q*rotation;

  // alter the rotation matrix
  rotation.RotationMatrix(rotMat);
}

void draw_volume_bounds_wireframe(float xdim, float ydim, float zdim)
{
  float wire_verts[8][3] =
  { {0.0,0.0,0.0},
    {xdim, 0.0, 0.0},
    {0.0, ydim, 0.0},
    {xdim, ydim, 0.0},
    {0.0, 0.0, zdim},
    {xdim, 0.0, zdim},
    {0.0, ydim, zdim},
    {xdim, ydim, zdim}
  };
  int wire_faces[6][4] =
  { {0,2,6,4},
    {1,3,7,5},
    {2,3,7,6},
    {4,5,7,6},
    {0,1,3,2},
    {0,1,5,4}
  };
  
  // draw the wireframe
  for (int i=0; i < 6; i++)
  {
    glBegin(GL_LINE_LOOP);
      for (int j=0; j < 4; j++)
      {
        glVertex3f(wire_verts[wire_faces[i][j]][0],
                   wire_verts[wire_faces[i][j]][1],
                   wire_verts[wire_faces[i][j]][2]);
      }
    glEnd();
  }

	glPushMatrix();
	// draw the axes
	glTranslated(-0.1, -0.1, -0.1);
	glScaled(0.25,0.25,0.25);
	glBegin(GL_LINES);
		glColor3d(1.0,0.0,0.0);
		glVertex3fv(wire_verts[0]);
		glVertex3fv(wire_verts[1]);

		glColor3d(0.0,1.0,0.0);
		glVertex3fv(wire_verts[0]);
		glVertex3fv(wire_verts[2]);

		glColor3d(0.0,0.0,1.0);
		glVertex3fv(wire_verts[0]);
		glVertex3fv(wire_verts[4]);
	glEnd();
	glPopMatrix();
}

void DefineView()
{
  float up[3] = { 0.0, 1.0, 0.0 },
        eye[3] = { 0.0, 0.0, eye_dist};

	// set the lookat vector
	//lookat[0] = dims[0] / 2.0;
	//lookat[1] = dims[1] / 2.0;
	//lookat[2] = dims[2] / 2.0;
	lookat[0] = 0.0;
	lookat[1] = 0.0;
	lookat[2] = 0.0;

  //printf("Eye = (%2.2f, %2.2f, %2.2f)\n",
  //      eye[0], eye[1], eye[2]);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity() ;
  gluLookAt(eye[0], eye[1], eye[2],
      lookat[0], lookat[1], lookat[2],
      up[0], up[1], up[2] );
}

void DefineLights()
{
  GLfloat ambient_light[4] = { 1.75, 1.75, 1.75, 1.0 };

  /* Enable ambient light. */
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_light );

  //glEnable(GL_LIGHTING);
}

void BeginFrame()
{
  DefineView();
  DefineLights();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
} // BeginFrame


void EndFrame()
{
  // buffer management
  glutSwapBuffers();
} // EndFrame

void Display(void)
{
	Vector4 span(dims[0], dims[1], dims[2], 1.0);
  //printf("Display()\n");
  BeginFrame();

	span *= 0.5;

  // translate center of wireframe to origin
  // and apply the rotation
  //glTranslated(span.X(), span.Y(), span.Z());
  glMultMatrixd(rotMat);
  glTranslated(-span.X(), -span.Y(), -span.Z());

  // set the drawing color and draw the wireframe
  glColor4f(1.0, 1.0, 1.0, 1.0);
  draw_volume_bounds_wireframe(dims[0], dims[1], dims[2]);

  EndFrame();
}

void Idle()
{
}

void Keyboard(unsigned char key, int x, int y)
{
  // escape key
  if ( key == 27 )
  {
    exit(0);
  }
	else if (key == '+')
	{
		if (eye_dist > 5.0)
		{
			eye_dist -= 10.0;
			glutPostRedisplay();
		}
		//printf("key +\n");
	}
	else if (key == '-')
	{
		if (eye_dist < 1000.0)
		{
			eye_dist += 10.0;
			glutPostRedisplay();
		}
		//printf("key -\n");
	}
	else if (key == 'v')
	{
		double mv[16], imv[16], tmp[16];
		//Vector4 eye(0.0,0.0,eye_dist,1.0),
		Vector4 eye(0.0,0.0,0.0,1.0),
						//lookatv(lookat[0],lookat[1],lookat[2],1.0),
						lookatv(0.0,0.0,-1.0,1.0),
						up(0.0,1.0,0.0,0.0);
		
		// get the modelview matrix
		glGetDoublev(GL_MODELVIEW_MATRIX, mv);
		// get its inverse
		m4_inverse(imv, mv);

		// transform the various vectors
		eye = eye*imv;
		lookatv = lookatv*imv;
		up = up*imv;

		// write the vectors to a file
		FILE *fp = fopen("parameters.view", "w");
		if (fp != NULL)
		{
			fprintf(fp, "%f\n%f\n%f\n%f\n", eye.X(), eye.Y(), eye.Z(), eye.W());
			fprintf(fp, "%f\n%f\n%f\n%f\n", lookatv.X(), lookatv.Y(), lookatv.Z(),
							lookatv.W());
			fprintf(fp, "%f\n%f\n%f\n%f\n", up.X(), up.Y(), up.Z(), up.W());
			fclose(fp);
		}
		else
		{
			printf("failed to write view parameters file!\n");
		}

		//printf("eye: "); eye.Print();
		//printf("look: "); lookat.Print();
		
		/*printf("current modelview:\n");
		for (int i=0; i<4; i++)
		{
			for (int j=0; j<4; j++)
			{
				printf("%2.3f ", mv[i+j*4]);
			}
			printf("\n");
		}

		printf("inverse of modelview:\n");
		for (int i=0; i<4; i++)
		{
			for (int j=0; j<4; j++)
			{
				printf("%2.3f ", imv[i+j*4]);
			}
			printf("\n");
		}*/
		
		/*m4_multiply(tmp, mv, imv);
		printf("identity?\n");
		for (int i=0; i<4; i++)
		{
			for (int j=0; j<4; j++)
			{
				printf("%2.3f ", tmp[i+j*4]);
			}
			printf("\n");
		}*/
	}
}

void SpecialKeys(int key, int x, int y)
{
//  printf( "%d\n", key );
}

void Mouse(int button, int state, int x, int y)
{
//  printf("Mouse(): x = %d, y = %d\n", x,y);
  if (button == GLUT_LEFT_BUTTON)
  {
    if (state == GLUT_DOWN)
    {
      mouse_is_down = true;
      oldX = x;
      oldY = y;
    }
    else
    {
      mouse_is_down = false;
      oldX = 0;
      oldY = 0;
    }
  }
}


void Motion(int x, int y)
{
  int dX, dY;
  double radius = 100.0, dR, theta;
  Vector4 axis(0.0, 0.0, 1.0, 0.0); // vector pointing out of the screen

  if (mouse_is_down)
  {
    // x displacement
    dX = x - oldX;
    oldX = x;
    // y displacement
    dY = y - oldY;
    oldY = y;

    // relative displacement
    dR = sqrt((double)(dX*dX + dY*dY));
    // the rotation angle
    theta = atan(dR / radius);
    // the axis of rotation
    axis = axis.Cross(Vector4((float)dX, -(float)dY, 0.0, 0.0));
    // normalize the axis (important!)
    axis.Normalize();
    // compute the rotation
    do_rotation(theta, axis.X(), axis.Y(), axis.Z());

    glutPostRedisplay();
  }
}

void Reshape(int width, int height)
{
  float a_ratio = (GLfloat)width / (GLfloat)height;
  
  // set the viewport
  glViewport (0, 0, width, height);
  // set the projection matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective( 60.0,/* field of view in degree */
                  a_ratio, 
                  Z_NEAR, /* Z near */
                  Z_FAR /* Z far */ );

  // refresh the display
  glutPostRedisplay();
}

void MyInitialize( GLfloat aspect_ratio )
{
  /* Setup the perspective view */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity() ;
  gluPerspective( 60.0,/* field of view in degree */
                  aspect_ratio, 
                  Z_NEAR, /* Z near */
                  Z_FAR /* Z far */ ) ;

 
  /* Use depth buffering for visible surface determination. */
  glEnable(GL_DEPTH_TEST);
  
  // compute the rotation matrix
  rotation.RotationMatrix(rotMat);
  
	// set the line width
	glLineWidth(2.5);
  //glutIdleFunc(Idle);
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("usage: %s <rawiv file>\n", argv[0]);
		exit(-1);
	}

	// load the dataset bounding box
	FILE *fp = fopen(argv[1], "r");
	if (fp != NULL)
	{
		char buffer[24];
		float min[3],max[3];
		// read the first 24 bytes of the header
		fread(buffer, sizeof(buffer), 1, fp);
		// parse
		parse_rawiv_header(buffer, min, max);
		// set dim
		for (int i=0; i<3; i++)
			dims[i] = max[i] - min[i];
		// close
		fclose(fp);
	}
	else
	{
		printf("could not open rawiv file %s\n", argv[1]);
	}
	
	
	// init GLUT
  glutInit(&argc, argv);
  
  screen_width  = glutGet( GLUT_SCREEN_WIDTH ) ;
  screen_height = glutGet( GLUT_SCREEN_HEIGHT ) ;
  
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  //glutInitWindowSize( screen_width >> 1, screen_height >> 1 ) ;
  glutInitWindowSize(500,500) ;
  glutInitWindowPosition( screen_width >> 2, screen_height >> 2 ) ;

  glutCreateWindow("Interactive Frontend");

  glutDisplayFunc( Display ) ;
  
  glutKeyboardFunc( Keyboard );
  glutSpecialFunc( SpecialKeys );
  glutMouseFunc( Mouse );
  glutMotionFunc( Motion );
  glutReshapeFunc( Reshape );

  //MyInitialize( screen_width / (GLfloat) screen_height );
  MyInitialize( 1.0 );

  glutMainLoop();

  return 0;
}
