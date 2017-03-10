#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <time.h>
#include <stdlib.h>
#include <map>
#include <cstdlib>
//#include <FTGL/ftgl.h>
#define BITS 8
#include <glad/glad.h>
#include <ao/ao.h>
#include <mpg123.h>
#include <GLFW/glfw3.h>
#include <string> 
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
float rectangle_rotation = 0;
time_t start, end;
double elapsed;
float x_change = 0; //For the camera pan
float y_change = 0; //For the camera pan
float zoom_camera = 1;
int fl=1;
using namespace std;
int width = 1200;
int height = 600;
struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;

	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;
};
int h2=1;
struct COLOR {
	float r;
	float g;
	float b;
};
typedef struct COLOR COLOR;
typedef struct VAO VAO;
typedef struct bricks{
	int index;
	float x;
	float y;
	int color;
	int type;
	int live;
	VAO* b;
}bricks;
int counter=0;
typedef struct game{
	int noofbricks;
	float speed;
	bricks brick[50];
	int score;
	float increase;
	float time;
	int numlife;
	int start;
	int pause;
	int stage;
}game;
VAO* background3;
float timeb=0;
VAO* eyec,*eyec2;
struct ball {
	string name;
	COLOR color;
	float x,y;
	VAO* object;
	int status;
	float height,width;
	float angle; //Current Angle (Actual rotated angle of the object)
	int inAir;
	float radius;
	int fixed;
	int isRotating;
	int direction; //0 for clockwise and 1 for anticlockwise for animation
	float remAngle; //the remaining angle to finish animation
	int isMoving;
	float dx;
	float dy;
	float weight;
};
typedef struct ball ball;
map <string, ball> cannon;
struct figures{
	VAO*obj;
	int x1;
	int y1;
	int num;
};
typedef struct figures figures;
map<int,figures> heart;
game g;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

VAO* circle,* semicircle,*ellipse,*bucket,*bucket2,*ellipse3,*ellipse4;
GLuint programID;
typedef struct DISTANCE{
	float x;
	float y;
}DISTANCE;
bool collision(float x1,float y1,float x2,float y2,float h1,float h2,float w1,float w2){
	if(abs(x2-x1)<=(w1+w2)/2 && abs(y2-y1)<=(h1+h2)/2)
		return true;
	else
		return false;
}
float cannonh=0;
float cannonangle=0;
DISTANCE d1={2,-3.3};
DISTANCE d2={-2,-3.3};
double mouse_pos_x, mouse_pos_y;
double new_mouse_pos_x, new_mouse_pos_y;
int mouse_clicked=0;
/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	//    exit(EXIT_SUCCESS);
}

void mousescroll(GLFWwindow* window, double xoffset, double yoffset)
{
	if (yoffset==-1) { 
		zoom_camera /= 1.1; //make it bigger than current size
	}
	else if(yoffset==1){
		zoom_camera *= 1.1; //make it bigger than current size
	}
	if (zoom_camera<=1) {
		zoom_camera = 1;
	}
	if (zoom_camera>=4) {
		zoom_camera=4;
	}
	if(x_change-4.0f/zoom_camera<-4)
		x_change=-4+4.0f/zoom_camera;
	else if(x_change+4.0f/zoom_camera>4)
		x_change=4-4.0f/zoom_camera;
	if(y_change-4.0f/zoom_camera<-4)
		y_change=-4+4.0f/zoom_camera;
	else if(y_change+5.0f/zoom_camera>5)
		y_change=5-5.0f/zoom_camera;
	//     cout<<x_change<<' '<<y_change<<'\n';
	Matrices.projection = glm::ortho((float)(-4.0f/zoom_camera+x_change), (float)(4.0f/zoom_camera+x_change), (float)(-4.0f/zoom_camera+y_change), (float)(5.0f/zoom_camera+y_change), 0.1f, 500.0f);
}

//Ensure the panning does not go out of the map
void check_pan(){
	if(x_change-4.0f/zoom_camera<-4)
		x_change=-4+4.0f/zoom_camera;
	else if(x_change+4.0f/zoom_camera>4)
		x_change=4-4.0f/zoom_camera;
	if(y_change-4.0f/zoom_camera<-4)
		y_change=-4+4.0f/zoom_camera;
	else if(y_change+5.0f/zoom_camera>5)
		y_change=5-5.0f/zoom_camera;
	Matrices.projection = glm::ortho((float)(-4.0f/zoom_camera+x_change), (float)(4.0f/zoom_camera+x_change), (float)(-4.0f/zoom_camera+y_change), (float)(5.0f/zoom_camera+y_change), 0.1f, 500.0f);

}

/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			1,                  // attribute 1. Color
			3,                  // size (r,g,b)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}
VAO* f1(){

}
VAO* createCircle (string name, float weight, COLOR color, float x, float y, float r, int NoOfParts, string component, int fill)
{
	int parts = NoOfParts;
	float radius = r;
	GLfloat vertex_buffer_data[parts*9];
	GLfloat color_buffer_data[parts*9];
	int i,j;
	float angle=(2*M_PI/parts);
	float current_angle = 0;
	for(i=0;i<parts;i++){
		for(j=0;j<3;j++){
			color_buffer_data[i*9+j*3]=color.r;
			color_buffer_data[i*9+j*3+1]=color.g;
			color_buffer_data[i*9+j*3+2]=color.b;
		}
		vertex_buffer_data[i*9]=0;
		vertex_buffer_data[i*9+1]=0;
		vertex_buffer_data[i*9+2]=0;
		vertex_buffer_data[i*9+3]=radius*cos(current_angle);
		vertex_buffer_data[i*9+4]=radius*sin(current_angle);
		vertex_buffer_data[i*9+5]=0;
		vertex_buffer_data[i*9+6]=radius*cos(current_angle+angle);
		vertex_buffer_data[i*9+7]=radius*sin(current_angle+angle);
		vertex_buffer_data[i*9+8]=0;
		current_angle+=angle;
	}

	if(fill==1)
		return create3DObject(GL_TRIANGLES, (parts*9)/3, vertex_buffer_data, color_buffer_data, GL_FILL);
	else
		return create3DObject(GL_TRIANGLES, (parts*9)/3, vertex_buffer_data, color_buffer_data, GL_LINE);

}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}


/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
int ballcount=0;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.
	// cout<<"action :"<<action<<'\n';
	//cout<<"key :"<<key<<'\n';
	//cout<<"mods :"<<mods<<'\n';
	char y;
	string str2;
	COLOR gold={255/255.0,215/255.0,0};
	ball b;
	VAO* cir;
	float r;
	if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_UP:
				mousescroll(window,0,+1);
				check_pan();
				break;
			case GLFW_KEY_DOWN:
				mousescroll(window,0,-1);
				check_pan();
				break;
			case GLFW_KEY_C:
				rectangle_rot_status = !rectangle_rot_status;
				break;
			case GLFW_KEY_P:
				triangle_rot_status = !triangle_rot_status;
				break;
			case GLFW_KEY_F:
				cannonh-=0.2;
				break;
			case GLFW_KEY_R:
				if(g.pause==0)
					g.pause=1;
				else
					g.pause=0;
				break;
			case GLFW_KEY_S:
				cannonh+=0.2;
				break;
			case GLFW_KEY_A:
				if(cannonangle<=45){
					cannonangle+=5;

				}
				break;
			case GLFW_KEY_SPACE:
				r=glfwGetTime();
				if(r-timeb<1)
					break;
				timeb=r;
				y=ballcount+'0';
				timeb=glfwGetTime();
				str2= "ball";
				str2=str2+y;
				ballcount++;
				b.object=createCircle("cloud1ac1",10000,gold,-250,110,0.07,1150,"background",1);
				b.name=str2;
				b.angle=cannonangle;
				b.x=-3.7+0.2*cos((cannonangle*M_PI)/180)-0.2;
				b.y=cannonh+0.2*sin((cannonangle*M_PI)/180);
				b.dx=0.05;
				b.isMoving=1;
				b.dy=0.05;
				cannon[str2]=b;
				//       cout<<str2<<'\n';
				break;
			case GLFW_KEY_D:
				if(cannonangle>=-45)
					cannonangle-=5;
				break;
			case GLFW_KEY_Y:
				if(g.start==0){
					g.start=1;
					g.numlife=5;
					g.score=0;
				}
				break;
			case GLFW_KEY_X:
				g.start=0;
				break;
			case GLFW_KEY_M:
				if(g.speed>=0.2)
					break;
				g.speed*=2;
				(g.increase)=g.increase+1;
				h2++;
				if(g.speed<=0.005)
					g.speed=0.01;
				//	g.increase=1;
				h2=1;
				break;
			case GLFW_KEY_N:
				if(g.speed<=0.002)
					break;
				g.speed/=2;
				if((g.increase)>1)
					g.increase=1;
				h2=1;
				break;
			case 263:
				//	cout<<"asas\n";
				if(mods==2){
					d1.x-=0.1;
				}
				else if(mods==4)
				{
					d2.x-=0.1;
				}
				else{

					x_change-=0.1;
					//  	cout<<x_change<<'\n';
					check_pan();

				}
				//  cout<<d1.x<<'\n';
				break;
			case 262:
				if(mods==2)
					d1.x+=0.1;
				else if(mods==4)
					d2.x+=0.1;
				else
				{x_change+=0.1;
					check_pan();
				}
				break;
			default:
				break;
		}
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;
			default:
				break;
		}
	}
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
			quit(window);
			break;
		default:
			break;
	}
}
void mouse_release(GLFWwindow* window, int button){ 
	if(mouse_clicked==1){
		float r=glfwGetTime();
		if(r-timeb<1)
			return;
		timeb=r;
		char y;
		string str2;
		COLOR gold={255/255.0,215/255.0,0};
		ball b;
		VAO* cir;
		y=ballcount+'0';
		str2= "ball";
		str2=str2+y;
		ballcount++;
		b.object=createCircle("cloud1ac1",10000,gold,-250,110,0.07,1150,"background",1);
		b.name=str2;
		b.angle=cannonangle;
		b.x=-3.7+0.2*cos((cannonangle*M_PI)/180)-0.2;
		b.y=cannonh+0.2*sin((cannonangle*M_PI)/180);
		b.dx=0.05;
		b.isMoving=1;
		b.dy=0.05;
		cannon[str2]=b;
	}
	mouse_clicked=0;
}
/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_PRESS){
				glfwGetCursorPos(window, &new_mouse_pos_x, &new_mouse_pos_y);      
				//	cout<<new_mouse_pos_x<<' '<<new_mouse_pos_y<<'\n';  
				//cout<<(new_mouse_pos_x-600)*4/600<<' '<<-(new_mouse_pos_y-300)*4/300<<'\n';
				//cout<<d2.x<<' '<<d2.y<<'\n';
				if((new_mouse_pos_x-600)*4/600>=d1.x && (new_mouse_pos_x-600)*4/600<=d1.x+0.6 &&-(new_mouse_pos_y-330)*4/300 >=d1.y-1 && -(new_mouse_pos_y-300)*4/300<=d1.y+1){
					mouse_clicked=2;
				}
				else if((new_mouse_pos_x-600)*4/600>=d2.x-0.2 && (new_mouse_pos_x-600)*4/600<=d2.x+0.5 &&-(new_mouse_pos_y-330)*4/300 >=d2.y-1 && -(new_mouse_pos_y-300)*4/300<=d2.y+1){
					mouse_clicked=3;
				}
				else if((new_mouse_pos_x-600)*4/600>=-3.9-0.35 && (new_mouse_pos_x-600)*4/600<=-3.9+0.3 &&-(new_mouse_pos_y-330)*4/300 >=cannonh-1 && -(new_mouse_pos_y-300)*4/300<=cannonh+1)
					mouse_clicked=4;
				else
					mouse_clicked=1;
				cout<<mouse_clicked<<'\n';
			}
			if (action == GLFW_RELEASE) {
				//cout<<"ssa"<<'\n';
				mouse_release(window,button);
			}
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_RELEASE) {
				rectangle_rot_dir *= -1;
			}
			break;
		default:
			break;
	}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	   is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	// Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 5.0f, 0.1f, 500.0f);
}

VAO *triangle, *rectangle,*circle2,*rectangle2,*background,*ellipse2;

// Creates the triangle object used in this sample code
VAO* createheart ()
{
	/* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

	/* Define vertex array as used in glBegin (GL_TRIANGLES) */
	const GLfloat vertex_buffer_data [] = {
		-0.1, 0.1,0, // vertex 0
		0.1, 0.1,0, // vertex 1
		0,0,0, // vertex 2
	};

	const GLfloat color_buffer_data [] = {
		1,0,0, // color 0
		1,0,0, // color 1
		1,0,0, // color 2
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	return create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createTriangle ()
{
	/* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

	/* Define vertex array as used in glBegin (GL_TRIANGLES) */
	const GLfloat vertex_buffer_data [] = {
		0, 1,0, // vertex 0
		-1,-1,0, // vertex 1
		1,-1,0, // vertex 2
	};

	const GLfloat color_buffer_data [] = {
		1,0,0, // color 0
		0,1,0, // color 1
		0,0,1, // color 2
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}
VAO* createtreetop(int val){
	const GLfloat vertex_buffer_data [] = {
		0, 2.5,0, // vertex 0
		-1+val,0,0, // vertex 1
		1-val,0,0, // vertex 2
	};

	const GLfloat color_buffer_data [] = {
		0.35,0.5843,0.3412, // color 0
		0.35,0.5843,0.3412, // color 1
		0.35,0.5843,0.3412, // color 2
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	return create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);

}
VAO* create2bg ()
{
	/* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

	/* Define vertex array as used in glBegin (GL_TRIANGLES) */
	const GLfloat vertex_buffer_data [] = {
		-0.25 , 0.5,0, // vertex 0
		-0.25,0,0, // vertex 1
		0.25,0,0, // vertex 2

		0.25,0,0,
		0.25,0.5,0,
		-0.25,0.5,0,

		-0.25,0.5,0,
		-0.25,1.0,0,
		0.25,1.0,0,

		0.25,1.0,0,
		0.25,0.5,0,
		-0.25,0.5,0,
	};

	const GLfloat color_buffer_data [] = {
		0.1176,0.2627,0.298, // color 0
		0.1176,0.2627,0.298, // color 1
		0.1176,0.2627,0.298, // color 2

		0.1176,0.2627,0.298, // color 0
		0.1176,0.2627,0.298, // color 1
		0.1176,0.2627,0.298, // color 2

		0.6078,0.3098,0.0157,
		0.6078,0.3098,0.0157,
		0.6078,0.3098,0.0157,

		0.6078,0.3098,0.0157,
		0.6078,0.3098,0.0157,
		0.6078,0.3098,0.0157,
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	return create3DObject(GL_TRIANGLES, 12, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createBackground3 ()
{
	// GL3 accepts only Triangles. Quads are not supported
	const GLfloat vertex_buffer_data [] = {
		-1*width/2,height/2,0, // vertex 1
		-1*width/2,-height/2,0, // vertex 2
		width/2,-height/2,0, // vertex 3

		width/2,-height/2,0, // vertex 3
		width/2,height/2,0, // vertex 4
		-width/2,height/2,0  // vertex 1
	};

	const GLfloat color_buffer_data [] = {
		0.2196,0.85,1, // color 1
		0.2196,0.85,1, // color 2
		0.2196,0.85,1, // color 3

		0.2196,0.85,1, // color 3
		0.2196,0.85,1, // color 4
		0.2196,0.85,1,  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	background3 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createBackground ()
{
	// GL3 accepts only Triangles. Quads are not supported
	const GLfloat vertex_buffer_data [] = {
		-1*width/2,height/2,0, // vertex 1
		-1*width/2,-height/2,0, // vertex 2
		width/2,-height/2,0, // vertex 3

		width/2,-height/2,0, // vertex 3
		width/2,height/2,0, // vertex 4
		-width/2,height/2,0  // vertex 1
	};

	const GLfloat color_buffer_data [] = {
		1,1,1, // color 1
		1,1,1, // color 2
		1,1,1, // color 3

		1,1,1, // color 3
		1,1,1, // color 4
		1,1,1  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	background = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
// Creates the rectangle object used in this sample code
VAO* createBeach(int depth,COLOR c){
	const GLfloat vertex_buffer_data [] = {
		-5,-1+depth,0, // vertex 1
		-5,1,0, // vertex 2
		5, 1,0, // vertex 3

		5, 1,0, // vertex 3
		5, -1+depth,0, // vertex 4
		-5,-1+depth,0  // vertex 1
	};

	const GLfloat color_buffer_data [] = {
		//0.0,0.7725,0.8078, // color 1
		c.r,c.g,c.b,
		c.r,c.g,c.b,
		c.r,c.g,c.b,

		c.r,c.g,c.b,
		c.r,c.g,c.b,
		c.r,c.g,c.b,
		// color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

}
VAO* createStem ()
{
	// GL3 accepts only Triangles. Quads are not supported
	const GLfloat vertex_buffer_data [] = {
		-0.6,-2.05,0, // vertex 1
		0.1,-2.05,0, // vertex 2
		0.1, 0.05,0, // vertex 3

		0.1, 0.05,0, // vertex 3
		-0.6, 0.05,0, // vertex 4
		-0.6,-2.05,0  // vertex 1
	};

	const GLfloat color_buffer_data [] = {
		0.5098,0.4078,0.302, // color 1
		0.5098,0.4078,0.302, // color 2
		0.5098,0.4078,0.302, // color 3

		0.5098,0.4078,0.302, // color 3
		0.5098,0.4078,0.302, // color 4
		0.5098,0.4078,0.302  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
VAO* createtank ()
{
	// GL3 accepts only Triangles. Quads are not supported
	const GLfloat vertex_buffer_data [] = {
		-1,-1.5,0, // vertex 1
		0.1,-1.5,0, // vertex 2
		0.1, 0.05,0, // vertex 3

		0.1, 0.05,0, // vertex 3
		-1, 0.05,0, // vertex 4
		-1,-1.5,0  // vertex 1
	};

	const GLfloat color_buffer_data [] = {
		0.1412,0.5137,0.1137, // color 1
		0.1412,0.5137,0.1137, // color 2
		0.1412,0.5137,0.1137, // color 3

		0.1412,0.5137,0.1137, // color 3
		0.1412,0.5137,0.1137, // color 4
		0.1412,0.5137,0.1137  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createRectangle ()
{
	// GL3 accepts only Triangles. Quads are not supported
	const GLfloat vertex_buffer_data [] = {
		-0.1,-0.05,0, // vertex 1
		0.1,-0.05,0, // vertex 2
		0.1, 0.05,0, // vertex 3

		0.1, 0.05,0, // vertex 3
		-0.1, 0.05,0, // vertex 4
		-0.1,-0.05,0  // vertex 1
	};

	const GLfloat color_buffer_data [] = {
		0.1216,0.2863,0.4902, // color 1
		0.1216,0.2863,0.4902, // color 2
		0.1216,0.2863,0.4902, // color 3

		0.1216,0.2863,0.4902, // color 3
		0.1216,0.2863,0.4902, // color 4
		0.1216,0.2863,0.4902  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	rectangle2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
VAO* block[3];
VAO* createBlock (COLOR c1)
{
	// GL3 accepts only Triangles. Quads are not supported
	const GLfloat vertex_buffer_data [] = {
		-0.05,-0.15,0, // vertex 1
		0.05,-0.15,0, // vertex 2
		0.05, 0.15,0, // vertex 3

		0.05, 0.15,0, // vertex 3
		-0.05, 0.15,0, // vertex 4
		-0.05,-0.15,0  // vertex 1
	};

	const GLfloat color_buffer_data [] = {
		c1.r,c1.g,c1.b, // color 1
		c1.r,c1.g,c1.b, // color 2
		c1.r,c1.g,c1.b, // color 3

		c1.r,c1.g,c1.b, // color 3
		c1.r,c1.g,c1.b, // color 4
		c1.r,c1.g,c1.b  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
VAO* mirror[4];
void createBucket (COLOR c1,COLOR c2,int num)
{
	// GL3 accepts only Triangles. Quads are not supported
	const GLfloat vertex_buffer_data [] = {
		-0.3,-0.5,0, // vertex 1
		0.3,-0.5,0, // vertex 2
		0.3, 0.5,0, // vertex 3

		0.3, 0.5,0, // vertex 3
		-0.3, 0.5,0, // vertex 4
		-0.3,-0.5,0  // vertex 1
	};

	const GLfloat color_buffer_data [] = {
		c1.r,c1.g,c1.b, // color 3
		c1.r,c1.g,c1.b, // color 4
		c2.r,c2.g,c2.b,  // color 1

		c2.r,c2.g,c2.b, // color 1
		c2.r,c2.g,c2.b, // color 2
		c1.r,c1.g,c1.b // color 3

	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	if(num==1)
		bucket = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
	else
		bucket2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void translate(float v1,float v2,float v3,VAO* s){
	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;  // MVP = Projection * View * Model

	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateRectangle2 = glm::translate (glm::vec3(v1, v2, v3));        // glTranslatef
	glm::mat4 rotateRectangle2 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= (translateRectangle2 * rotateRectangle2);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(s);

}

void translateandrotate(float v1,float v2,float v3,float sangle,VAO* s){
	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;  // MVP = Projection * View * Model

	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateRectangle2 = glm::translate (glm::vec3(v1, v2, v3));        // glTranslatef
	glm::mat4 rotateRectangle2 = glm::rotate((float)((sangle*M_PI)/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= (translateRectangle2 * rotateRectangle2);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(s);

}
VAO* createMirror2 (COLOR c1)
{
	// GL3 accepts only Triangles. Quads are not supported
	const GLfloat vertex_buffer_data [] = {
		1,2,0, // vertex 1
		0,2,0, // vertex 2
		0,2.2,0,
		0,2.2,0,
		1,2.2,0,
		1,2,0
	};

	const GLfloat color_buffer_data [] = {
		c1.r,c1.g,c1.b, // color 3
		c1.r,c1.g,c1.b, // color 4
		c1.r,c1.g,c1.b,
		c1.r,c1.g,c1.b, // color 3
		c1.r,c1.g,c1.b, // color 4
		c1.r,c1.g,c1.b

	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

}
VAO* createMirror (COLOR c1)
{
	// GL3 accepts only Triangles. Quads are not supported
	const GLfloat vertex_buffer_data [] = {
		1,2,0, // vertex 1
		0,2,0, // vertex 2
		0,2.05,0,
		0,2.05,0,
		1,2.05,0,
		1,2,0
	};

	const GLfloat color_buffer_data [] = {
		c1.r,c1.g,c1.b, // color 3
		c1.r,c1.g,c1.b, // color 4
		c1.r,c1.g,c1.b,
		c1.r,c1.g,c1.b, // color 3
		c1.r,c1.g,c1.b, // color 4
		c1.r,c1.g,c1.b

	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

}
VAO* createLine (COLOR c1)
{
	// GL3 accepts only Triangles. Quads are not supported
	const GLfloat vertex_buffer_data [] = {
		-4,-2,0, // vertex 1
		4,-2,0, // vertex 2
		4,-2.02,0,
		4,-2.02,0,
		-4,-2.02,0,
		-4,-2,0
	};

	const GLfloat color_buffer_data [] = {
		c1.r,c1.g,c1.b, // color 3
		c1.r,c1.g,c1.b, // color 4
		c1.r,c1.g,c1.b,
		c1.r,c1.g,c1.b, // color 3
		c1.r,c1.g,c1.b, // color 4
		c1.r,c1.g,c1.b

	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

}
const float DEG2RAD = 3.14159/180.0;
void createRectangle2 ()
{
	// GL3 accepts only Triangles. Quads are not supported
	const GLfloat vertex_buffer_data [] = {
		-0.0,-0.18,0, // vertex 1
		0.18,-0.18,0, // vertex 2
		0.18, 0.18,0, // vertex 3

		0.18, 0.18,0, // vertex 3
		-0.0, 0.18,0, // vertex 4
		-0.0,-0.18,0  // vertex 1
	};

	const GLfloat color_buffer_data [] = {
		0.1216,0.2863,0.4902, // color 1
		0.1216,0.2863,0.4902, // color 2
		0.1216,0.2863,0.4902, // color 3

		0.1216,0.2863,0.4902, // color 3
		0.1216,0.2863,0.4902, // color 4
		0.1216,0.2863,0.4902  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
float camera_rotation_angle = 90;

float triangle_rotation = 0;
VAO* lher,*lher2;
void createEllipse(float xradius, float yradius,COLOR color,int num)
{
	GLfloat vertex_buffer_data[360*9];
	GLfloat color_buffer_data[360*9];
	for(int i=0;i<360;i++){
		for(int j=0;j<3;j++){
			color_buffer_data[i*9+j*3]=color.r;
			color_buffer_data[i*9+j*3+1]=color.g;
			color_buffer_data[i*9+j*3+2]=color.b;
		}       float degInRad = i*DEG2RAD;
		vertex_buffer_data[i*9]=0;
		vertex_buffer_data[i*9+1]=0;
		vertex_buffer_data[i*9+2]=0;
		vertex_buffer_data[i*9+3]=xradius/2;
		vertex_buffer_data[i*9+4]=0;
		vertex_buffer_data[i*9+5]=0;
		vertex_buffer_data[i*9+6]=cos(degInRad)*xradius;
		vertex_buffer_data[i*9+7]=sin(degInRad)*yradius;
		vertex_buffer_data[i*9+8]=0;


	}
	if(num==1)
		ellipse = create3DObject(GL_TRIANGLES, 360*3, vertex_buffer_data, color_buffer_data, GL_LINE);
	else if(num==2)
		ellipse2 = create3DObject(GL_TRIANGLES, 360*3, vertex_buffer_data, color_buffer_data, GL_LINE);
	else if(num==3)
		ellipse3 = create3DObject(GL_TRIANGLES, 360*3, vertex_buffer_data, color_buffer_data, GL_LINE);
	else if(num==4)
		ellipse4 = create3DObject(GL_TRIANGLES, 360*3, vertex_buffer_data, color_buffer_data, GL_LINE);
	else if(num==5)
		lher=create3DObject(GL_TRIANGLES, 360*3, vertex_buffer_data, color_buffer_data, GL_FILL);
	else if(num==6)
		lher2=create3DObject(GL_TRIANGLES, 360*3, vertex_buffer_data, color_buffer_data, GL_FILL);
	else if(num==7)
		eyec=create3DObject(GL_TRIANGLES, 360*3, vertex_buffer_data, color_buffer_data, GL_FILL);
	else
		eyec2=create3DObject(GL_TRIANGLES, 360*3, vertex_buffer_data, color_buffer_data, GL_FILL);


}
bool checkcircleline(float ang,float x1,float y1,ball* d){
	float m=tan((ang*M_PI)/180);
	float r=(d->y-m*d->x-(y1-m*x1))/sqrt(1.0+pow(m,2));


	//cout<<r<<'\n';			
	if(r-0.071<=0 && r>=0 && d->y>=y1-sin((ang*M_PI)/180)/2 && d->y<=y1+sin((ang*M_PI)/180)/2){
		return true;
	}
	else
		return false;
}
void checkcollision(){
	//	cout<<g.speed<<'\n';
	//			cout<<h2<<'\n';

	for(map<string,ball>::iterator it=cannon.begin();it!=cannon.end();it++){
		string current = it->first;
		ball* d=&cannon[current];
		if(d->isMoving==1){
			for(int i=0;i<50;i++){
				bricks* f=&(g.brick[i]);
				if(f->live==1){
					if(collision(d->x,d->y,f->x,f->y,0.1,0.3,0.07,0.07)){
						f->live=0;
						d->isMoving=0;
						if(f->type!=2){
							g.score-=5;
							g.numlife--;
						}
						else{
							g.score+=5;
						}

						//   cout<<i<<'\n';
						break;
					}
				}
			}
			if(d->x>4 || d->y> 4 || d->x<-4 || d->y<-4){
				d->isMoving=0;
			}
			//	float m=tan((60*M_PI)/180);
			//	float r=(d->y-m*d->x-(0.4+m*0))/sqrt(1+pow(m,2));
			if(checkcircleline(60.0,-0.45,0.4,d)){
				d->angle=2*60-d->angle;
			}
			if(checkcircleline(45.0,3.4,-1,d)){
				d->angle=2*45.0-d->angle;
			}
			if(checkcircleline(150.0,-0.5,3.35,d)){
				d->angle=2*150.0-d->angle;
			}
			if(checkcircleline(120.0,3.3,3.5,d)){
				d->angle=2*120.0-d->angle;
			}

			//	if(collision(d->x,d->y,-1,2.6,1*cos((30*M_PI)/180),0.05*sin((30*M_PI)/180),0.07*cos((30*M_PI)/180),0.07*sin((30*M_PI)/180)))
			//	d->isMoving=0;	
			//	if(collision(d->x,d->y,-0.4,0.4,sin((60*M_PI)/180)*1,cos((60*M_PI)/180)*0.05,sin((60*M_PI)/180)*0.07,cos((60*M_PI)/180)*0.07))
			//	d->isMoving=0;
			//if(collision(d->x,d->y,3,3,sin((120*M_PI)/180)*1,cos((60*M_PI)/180)*0.05,sin((120*M_PI)/180)*0.07,cos((60*M_PI)/180)*0.07))
			//d->isMoving=0;
			//	if(collision(d->x,d->y,3,-1.2,cos((45*M_PI)/180)*1,sin((120*M_PI)/180)*0.05,cos((45*M_PI)/180)*0.07,sin((120*M_PI)/180)*0.07))
			//d->isMoving=0;
			//cout<<d->x<<' '<<d->y<<'\n';	
		}

	}
	for(int i=0;i<50;i++){
		bricks* f=&(g.brick[i]);
		if(f->live==1){

			if(collision(d2.x,d2.y,f->x,f->y,0.3,1.2,0.1,0.6)){
				f->live=0;
				if(f->type==0)
					g.score+=50;
				else if(f->type==1)
					g.score+=-10;
				else
				{
					g.score-=20;
					g.numlife--;
				}

				//cout<<g.score<<'\n';			
			}
			if(collision(d1.x,d1.y,f->x,f->y,0.3,1.2,0.1,0.6)){
				f->live=0;
				if(f->type==1)
					g.score+=50;
				else if(f->type==0)
					g.score+=-10;
				else{
					g.score-=20;
					g.numlife--;
				}
				//	cout<<g.score<<'\n';
			}
		}
	}
}


void movemouse(GLFWwindow* window){
	// cout<<"asas"<<'\n';
	if(mouse_clicked==1)
	{
		//	cout<<"mouse"<<'\n';
		glfwGetCursorPos(window, &new_mouse_pos_x, &new_mouse_pos_y);        
		float dist1 = (((new_mouse_pos_y-300)*4)/300+(cannonh));
		float dist2 = ((new_mouse_pos_x-600)*4/600-(3.9));
		float angle_rad = atan(dist1/dist2);
		//    cout<<dist1<<' '<<dist2<<'\n';
		// cout<<angle_rad<<'\n';
		cannonangle=(angle_rad*180)/M_PI;
	}
	else if(mouse_clicked==2){
		float eq=new_mouse_pos_x;
		glfwGetCursorPos(window, &new_mouse_pos_x, &new_mouse_pos_y);        
		// cout<<eq<<' '<<new_mouse_pos_x<<'\n';
		eq=(new_mouse_pos_x-eq)*4/600;
		if(d1.x+eq>=-3.8 && d1.x+eq<=3.8){
			d1.x+=eq;
		}
	}
	else if(mouse_clicked==3){
		float eq=new_mouse_pos_x;
		glfwGetCursorPos(window, &new_mouse_pos_x, &new_mouse_pos_y);        
		eq=(new_mouse_pos_x-eq)*4/600;
		if(d2.x+eq>=-3.8 && d2.x+eq<=3.8){
			d2.x+=eq;
		}
	}
	else if(mouse_clicked==4){
		float eq=new_mouse_pos_y;
		glfwGetCursorPos(window, &new_mouse_pos_x, &new_mouse_pos_y);        
		eq=(new_mouse_pos_y-eq)*4/400;
		if(cannonh+eq>=-3.8 && cannonh+eq<=3.8){
			cannonh-=eq;
		}

	}
}
VAO* create7Segment (map<string,ball> curChar)
{
	int f1=curChar["top"].status;
	int f2=curChar["bottom"].status;
	int f3=curChar["middle"].status;
	int f4=curChar["left1"].status;
	int f5=curChar["left2"].status;
	int f6=curChar["right1"].status;
	int f7=curChar["right2"].status;
	//cout<<~1<<'\n';
	int f8=curChar["middle1"].status;
	int f9=curChar["middle2"].status;
	int f10=curChar["diagonal"].status;
	int f11= curChar["m1"].status;
	int f12=curChar["m2"].status;
	/* boolvertices between the bounds specified in glm::ortho will be visible on screen */

	/* Define vertex array as used in glBegin (GL_TRIANGLES) */
	const	 float vertex_buffer_data [] = {
		-0.1, 0.2,0, // vertex 0
		-0.1,0.2,0, // vertex 1
		0.1,0.2,0, // vertex 2

		0.1,0.2,0,
		0.1,0,0,
		0.1,0,0,

		0.1,0,0,
		0.1,-0.2,0,
		0.1,-0.2,0,

		0.1,-0.2,0,
		-0.1, -0.2,0, // vertex 0
		-0.1,-0.2,0, // vertex 1

		-0.1,-0.2,0,
		-0.1,0,0,
		-0.1,0,0,

		-0.1,0,0,
		-0.1,0.2,0,
		-0.1,0.2,0,

		0, 0.2,0, // vertex 0
		0,0,0, // vertex 1
		0,0,0, // vertex 2	

		0,0,0, // vertex 0
		0.1,0,0, // vertex 1
		0.1,0,0, // vertex 2	

		0,0,0, // vertex 0
		-0.1,0,0, // vertex 1
		-0.1,0,0, // vertex 2	

		0,0,0, // vertex 0
		0,-0.2,0, // vertex 1
		0,-0.2,0, // vertex 2	

		-0.1	,0,0,
		0.1,-0.2,0,
		0.1,-0.2,0,

		-0.1, 0.2,0,
		0,0,0,
		0,0,0,

		0.1, 0.2,0,
		0,0,0,
		0,0,0,


		// vertex 2

	};
	int r[11];
	r[0]=f1;
	r[1]=f6;
	r[2]=f7;
	r[3]=f2;
	r[4]=f5;
	r[5]=f4;
	r[6]=f8;
	r[7]=f3;
	r[8]=f3;
	r[9]=f9;

	r[10]=f10;
	r[11]=f11;
	r[12]=f12;
	const float color_buffer_data [] = {
		1,1,1,
		1,1,1,
		1,1,1,

		1,1,1,
		1,1,1,
		1,1,1,

		1,1,1,
		1,1,1,
		1,1,1,

		1,1,1,
		1,1,1,
		1,1,1,

		1,1,1,
		1,1,1,
		1,1,1,

		1,1,1,
		1,1,1,
		1,1,1,

		1,1,1,
		1,1,1,
		1,1,1,

		1,1,1,
		1,1,1,
		1,1,1,

		1,1,1,
		1,1,1,
		1,1,1,

		1,1,1,
		1,1,1,
		1,1,1,


		1,1,1,
		1,1,1,
		1,1,1,

		1,1,1,
		1,1,1,
		1,1,1,

		1,1,1,
		1,1,1,
		1,1,1,
	};
	int counts=0;
	GLfloat *glColor = (GLfloat *)malloc((f1+f2+2*f3+f4+f5+f6+f7+f8+f9+f10+f11+f12)*9 * sizeof(GLfloat));

	GLfloat *glVertices = (GLfloat *)malloc((f1+f2+2*f3+f4+f5+f6+f7+f8+f9+f10+f11+f12)*9 * sizeof(GLfloat));
	for(int k=0;k<13;k++){
		if(r[k]==1){
			for(int j=0;j<9;j++)
			{			glVertices[counts]=vertex_buffer_data[k*9+j];
				glColor[counts++]=color_buffer_data[k*9+j];
				//	counts++;

			}	



		}
	}

	//cout<<counts<<'\n';

	// create3DObject creates and returns a handle to a VAO that can be used later
	return create3DObject(GL_TRIANGLES, (f1+f2+2*f3+f4+f5+f6+f7+f8+f9+f10+f11+f12)*3	, glVertices, glColor, GL_LINE);
}

void setStrokes(char val, int charNo,float y){
	map<string,ball> curChar;
	curChar["top"].status=0;
	curChar["bottom"].status=0;
	curChar["middle"].status=0;
	curChar["left1"].status=0;
	curChar["left2"].status=0;
	curChar["right1"].status=0;
	curChar["right2"].status=0;
	curChar["middle1"].status=0;
	curChar["middle2"].status=0;
	curChar["diagonal"].status=0;
	curChar["m1"].status=0;
	curChar["m2"].status=0;
	if(val=='0' || val=='2' || val=='3' || val=='5' || val=='6'|| val=='7' ||val=='A' || val=='8' || val=='9' || val=='P' || val=='I' || val=='O' || val=='N' || val=='T' || val=='S' || val=='E' || val=='C' || val=='R'){
		curChar["top"].status=1;
	}
	if(val=='2' || val=='3' || val=='4' || val=='5' || val=='6' || val=='8' ||val=='A'|| val=='9' || val=='P' || val=='S' || val=='Y' || val=='E' || val=='R'){
		curChar["middle"].status=1;
	}
	if(val=='0' || val=='2' || val=='3' || val=='5' || val=='6' || val=='8' || val=='9' || val=='O' || val=='S' || val=='I' || val=='Y' || val=='U' || val=='L' || val=='E' || val=='W' || val=='C'){
		curChar["bottom"].status=1;
	}
	if(val=='0' || val=='4' || val=='5' || val=='6' || val=='8' || val=='9' ||val=='A'|| val=='P' || val=='O'|| val=='M' || val=='N' || val=='S' || val=='Y' || val=='U' || val=='L' || val=='E' || val=='W' || val=='C'|| val=='R'){
		curChar["left1"].status=1;
	}
	if(val=='0' || val=='2' || val=='6' || val=='8' || val=='P' || val=='O' ||val=='A'|| val=='M'|| val=='N' || val=='U' || val=='L' || val=='E' || val=='W' || val=='C' || val=='R'){
		curChar["left2"].status=1;
	}
	if(val=='0' || val=='1' || val=='2' || val=='3' || val=='4' || val=='7' ||val=='A'|| val=='M'|| val=='8' || val=='9' || val=='P' || val=='O' || val=='N' || val=='Y' || val=='U' || val=='W' || val=='R'){
		curChar["right1"].status=1;
	}
	if(val=='0' || val=='1' || val=='3' || val=='4' || val=='5' || val=='6' ||val=='A'|| val=='M'|| val=='7' || val=='8' || val=='9' || val=='O' || val=='N' || val=='S' || val=='Y' || val=='U' || val=='W' ){
		curChar["right2"].status=1;
	}
	if(val=='I' || val=='T'){
		curChar["middle1"].status=1;
	}
	if(val=='I' || val=='T' || val=='W'){
		curChar["middle2"].status=1;
	}
	if(val=='R'){
		curChar["diagonal"].status=1;
	}
	if(val==':'){
		curChar["middle"].status=1;
	}
	if(val=='M')
	{
		curChar["m1"].status=1;
		curChar["m2"].status=1;
	}
	VAO* d=create7Segment(curChar);
	translate(3-charNo*0.3,y,0,d);
	// *characters[charNo]=curChar;
}
void game_over(){
	translate(0,0,0,background3);
	string f="PRESS Y TO RESTART";
	int count2=4;
	int y=f.length();
	for(int j=y-1;j>=0;j--){
		setStrokes(f[j],count2,1.5);
		count2++;
	}
	int x=g.score;
	count2=5;
	if(x==0){
		setStrokes('0',count2,0);
		count2++;				
	}

	while(x!=0){
		int r=x%10;
		x=x/10;
		char d=(char)r+'0';
		setStrokes(d,count2,0);
		count2++;
	}
	f="SCORE:";
	string f2="LIFE";
	count2++;
	y=f.length();
	for(int j=y-1;j>=0;j--){
		setStrokes(f[j],count2,0);
		count2++;
	}


}
VAO* createwelcomeblocks (string name, float weight, COLOR color, float x, float y, float r, int NoOfParts, string component, int fill)
{
	int parts = NoOfParts;
	float radius = r;
	GLfloat vertex_buffer_data[parts*9];
	GLfloat color_buffer_data[parts*9];
	int i,j;
	float angle=fill*(M_PI/parts);
	float current_angle = 0;
	for(i=0;i<parts;i++){
		for(j=0;j<3;j++){
			color_buffer_data[i*9+j*3]=color.r;
			color_buffer_data[i*9+j*3+1]=color.g;
			color_buffer_data[i*9+j*3+2]=color.b;
		}
		vertex_buffer_data[i*9]=0;
		vertex_buffer_data[i*9+1]=0;
		vertex_buffer_data[i*9+2]=0;
		vertex_buffer_data[i*9+3]=radius*cos(current_angle);
		vertex_buffer_data[i*9+4]=(radius+x)*sin(current_angle);
		vertex_buffer_data[i*9+5]=0;
		vertex_buffer_data[i*9+6]=radius*cos(current_angle+angle);
		vertex_buffer_data[i*9+7]=(radius+x)*sin(current_angle+angle);
		vertex_buffer_data[i*9+8]=0;
		current_angle+=angle;
	}

	return create3DObject(GL_TRIANGLES, (parts*9)/3, vertex_buffer_data, color_buffer_data, GL_FILL);

}
/* Render the scene with openGL */
/* Edit this function according to your assignment */
void createwelcome(){
	createBackground3();
	COLOR brown={0.6275,0.4196,0.0118};
	COLOR Grey={0.3647,0.3647,0.3647};
	COLOR white={1,1,1};
	COLOR black={0,0,0};
	COLOR gr={0,0.7725,0.018};
	COLOR yel={0.7725,0.7529,0.0471};
	COLOR gr2={0.1412,0.5137,0.1137};
	COLOR gr3={0.1412,0.6196,0.1451};
	COLOR bl={0.1804,0.1765,0.1685};
	COLOR mag={0.4,0.0078,0.3529};
	VAO* mb=createBeach(0,brown);
	VAO* f1=createwelcomeblocks("cloud1ac1",10000,Grey,0.82,110,0.6,1150,"background",1);
	VAO* f2=createwelcomeblocks("cloud1ac1",10000,yel,0.52,110,0.4,1150,"background",1);
	VAO* eye1=createwelcomeblocks("eye1",10000,white,0.15,110,0.1,1150,"ssas",1);
	VAO* eye1b=createwelcomeblocks("eye1",10000,white,0.08,110,0.05,1150,"ssas",1);
	VAO* eye1d=createwelcomeblocks("eye1",10000,black,0.10,110,0.07,1150,"ssas",1);
	VAO* eye1bd=createwelcomeblocks("eye1",10000,black,0.05,110,0.03,1150,"ssas",1);
	VAO* np=createwelcomeblocks("eye1",10000,white,0.10,110,0.05,1150,"ssas",2);
	VAO* eye3=createwelcomeblocks("eye1",10000,white,0.15,110,0.1,1150,"ssas",2);
	VAO* eye3b=createwelcomeblocks("eye1",10000,black,0.12,110,0.06,1150,"ssas",2);
	VAO* np2=createwelcomeblocks("eye1",10000,black,0.06,110,0.03,1150,"ssas",2);
	VAO* np3=createwelcomeblocks("eye1",10000,white,0.05,110,0.03,1150,"ssas",2);
	VAO* np4=createwelcomeblocks("eye1",10000,black,0.03,110,0.02,1150,"ssas",2);
	VAO* f3=createwelcomeblocks("cloud1ac1",10000,gr,0.92,110,0.45,1150,"background",1);
	VAO* t1=createtank();
	VAO* me=createMirror2(bl);
	VAO* t2=createwelcomeblocks("eye1",10000,gr3,0,110,0.5,1150,"ssas",1);
	VAO* f4=createwelcomeblocks("cloud1ac1",10000,mag,0.52,110,0.4,1150,"background",1);
	translate(0,0,0,background3);
	translate(0,-1,0,mb);
	translate(0,-2,0,mb);
	translate(0,-3,0,mb);
	translate(0,-4,0,mb);
	translate(1,-1.5,0,f1);
	translate(2,-0.4,0,f4);
	translate(0,-0.5,0,f2);
	translate(-0.6,-1,0,f3);
	translate(-0.8,-0.4,0,eye3);
	translate(1.9,0,0,np);
	translate(1.885,0,0,np2);
	translate(-3,-3,0,t1);
	translate(-0.83,-0.4,0,eye3b);
	translateandrotate(0.8,-0.2-0.5,0,200,eye1);
	translateandrotate(0.6,-0.7,0,160,eye1b);
	translateandrotate(-0.2+1,-0.7,0,200,eye1d);
	translateandrotate(-0.4+1,-0.7,0,160,eye1bd);
	translateandrotate(-2,-4,0,50,me);
	translate(-3.5,-2.95,0,t2);

	translate(0.1,0,0,np);
	translate(0.085,0,0,np2);
	translate(-0.1,0,0,np3);
	translate(-0.09,0,0,np4);
	string f="WELCOME";
	int count2=7;
	int y=f.length();
	for(int j=y-1;j>=0;j--){
		setStrokes(f[j],count2,4.5);
		count2++;
	}
	f="PRESS Y";
	count2=7;
	y=f.length();
	for(int j=y-1;j>=0;j--){
		setStrokes(f[j],count2,2.5);
		count2++;
	}

}
void draw (GLFWwindow* window)
{

	if(g.score<0)
		g.score=0;
	movemouse(window);
	// clear the color and depth in the frame buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);
	time(&end);
	elapsed = difftime(end, start);

	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (0, 0, 0);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);

	// Compute Camera matrix (view)
	// Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	// MVP = Projection * View * Model

	// Load identity to model matrix
	Matrices.model = glm::mat4(1.0f);
	draw3DObject(background);

	VAO* m=create2bg();

	float x1=-5;
	int w=0;
	while(x1<5){
		if(w%2){
			translateandrotate(x1,4,0,0,m);

		}
		else{
			translateandrotate(x1,5,0,180,m);

		}
		x1+=0.5;
		w++;
		if(g.start==0)
		{
			createwelcome();
			return;
		}

	}
	COLOR b={0.9608,0.7961,0.6078};
	COLOR w1={0.0,0.7725,0.8078,};
	COLOR w2={0.078,0.5373,0.7529};

	VAO* beach=createBeach(0,w1);
	VAO* e=createBeach(1,b);
	VAO* dc=createBeach(0.5,w2);
	/* Render your scene */
	translate(-1,-4,0,beach);
	/* Render your scene */
	translate(-1,-3,0,dc);
	translate(-1,-3,0,e);
	float er=-4;
	while(er<=5){
		translate(er,-3,0,lher);
		er+=0.5;
	}
	VAO* stem=createStem();
	translate(4.3,0.05,0,stem);
	VAO* le=createtreetop(0);
	translate(4.2,0.1,0,le);
	le=createtreetop(0.2);
	translate(4.4,1.1,0,le);
	le=createtreetop(0.4);
	translate(4.6,2.1,0,le);
	// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
	// glPopMatrix ();
	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateRectangle = glm::translate (glm::vec3(-4, cannonh, 0));        // glTranslatef
	glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= (translateRectangle * rotateRectangle);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	draw3DObject(circle2);
	draw3DObject(circle);

	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateRectangle2 = glm::translate (glm::vec3(-3.9, cannonh, 0));        // glTranslatef
	glm::mat4 rotateRectangle2 = glm::rotate((float)(cannonangle*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= (translateRectangle2 * rotateRectangle2);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	draw3DObject(rectangle);
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateRectangle4 = glm::translate (glm::vec3(d2.x, d2.y-0.5, 0));        // glTranslatef
	glm::mat4 rotateRectangle4 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= (translateRectangle4 * rotateRectangle4);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(ellipse2);
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateRectangle5 = glm::translate (glm::vec3(d2.x, d2.y, 0));        // glTranslatef
	glm::mat4 rotateRectangle5 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= (translateRectangle5 * rotateRectangle5);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(bucket);
	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateRectangle3 = glm::translate (glm::vec3(d2.x, d2.y+0.5, 0));        // glTranslatef
	glm::mat4 rotateRectangle3 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= (translateRectangle3 * rotateRectangle3);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(ellipse);
	Matrices.model = glm::mat4(1.0f);

	translate(d1.x,d1.y-0.5,0,ellipse4);

	translate(d1.x,d1.y,0,bucket2);
	translate(d1.x,d1.y+0.5,0,ellipse3);

	Matrices.model = glm::mat4(1.0f);

	translateRectangle2 = glm::translate (glm::vec3(-3.7+0.2*cos((cannonangle*M_PI)/180)-0.2, cannonh+0.2*sin((cannonangle*M_PI)/180), 0));        // glTranslatef
	rotateRectangle2 = glm::rotate((float)(cannonangle*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= (translateRectangle2 * rotateRectangle2);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	draw3DObject(rectangle2);

	COLOR color={0.2157,0.3765,0.5725};
	mirror[0]=createMirror(color);
	mirror[1]=createMirror(color);
	mirror[2]=createMirror(color);
	mirror[3]=createMirror(color);
	translateandrotate(4.6,-3,0,45,mirror[3]);
	translateandrotate(1,5,0,150,mirror[0]);
	translateandrotate(1,-1,0,60,mirror[1]);
	translateandrotate(5.4,4,0,120,mirror[2]);

	VAO* line=createLine(color);
	translate(0,0,0,line);
	translate(0,6,0,line);
	VAO* h=createheart();
	for(int i=0;i<g.numlife;i++){
		translateandrotate(-3.75+i*0.5,4.6,0,180,semicircle);
		COLOR RED={1,0,0};
		VAO* ci=createCircle("cloud1ac1",10000,RED,-250,110,0.11,1150,"background",1);
		translateandrotate(-3.85+i*0.5,4.6,0,180,ci);
		VAO* ci2=createCircle("cloud1ac1",10000,RED,-250,110,0.11,1150,"background",1);
		translateandrotate(-3.65+i*0.5,4.6,0,180,ci2);
		translate(-3.75+i*0.5,4.2,0,h);

	}
	// Increment angles
	counter=elapsed*10;
	float increments = 1;
	for(int i=0;i<50;i++){
		bricks* f=&(g.brick[i]);
		//   cout<<"cr "<<i<<' '<<f.live<<'\n';
		if(f->live==1){
			translate(f->x,f->y,0,f->b);
			f->y-=g.speed;
			//   cout<<f->y<<'\n';
			if(f->y<-3){
				f->live=0;
			}

		}
	}
	float stime=glfwGetTime();
	// cout<<glfwGetTime()<<'\n';
	float r=stime-g.time;
	//cout<<ela<<'\n';
	// cout<<elapsed<<'\n';
	if(r>=1.5/g.increase){
		g.time=stime;
		int j;

		for(int i=0;i<50;i++){
			bricks f=g.brick[i];
			if(f.live==0){
				j=i;
				break;
			}  
		}
		srand(time(NULL));
		int r1 = rand();
		int r2=rand();
		int r3=rand()%3;
		int v1=r2%3+1;
		int v2=r1%2;
		if(v2==0){
			v2=-1;
		}
		bricks* f=&(g.brick[j]);
		f->live=1;
		f->b=block[r3];
		f->type=r3;
		f->x=v1*v2;
		f->y=4;
		start=end;

	}
	for(map<string,ball>::iterator it=cannon.begin();it!=cannon.end();it++){
		string current = it->first;
		ball* d=&cannon[current];
		if(d->isMoving==1){
			translate(d->x,d->y,0,d->object); 
			d->x+=d->dx*cos((d->angle*M_PI)/180);
			d->y+=d->dy*sin((d->angle*M_PI)/180);
		}        //The name of the current object
	}
	checkcollision();

	
	int count2=0;
	int x=g.score;
	if(x==0){
		setStrokes('0',count2,4.5);
		count2++;				
	}

	while(x!=0){
		int r=x%10;
		x=x/10;
		char d=(char)r+'0';
		setStrokes(d,count2,4.5);
		count2++;
	}
	string f="SCORE:";
	string f2="LIFE";
	count2++;
	int y=f.length();
	for(int j=y-1;j>=0;j--){
		setStrokes(f[j],count2,4.5);
		count2++;
	}

	//camera_rotation_angle++; // Simulating camera rotation
	//triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
	//rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

void createHalfCircle (string name, float weight, COLOR color, float x, float y, float r, int NoOfParts, string component, int fill)
{
	int parts = NoOfParts;
	float radius = r;
	GLfloat vertex_buffer_data[parts*9];
	GLfloat color_buffer_data[parts*9];
	int i,j;
	float angle=(M_PI/parts);
	float current_angle = 0;
	for(i=0;i<parts;i++){
		for(j=0;j<3;j++){
			color_buffer_data[i*9+j*3]=color.r;
			color_buffer_data[i*9+j*3+1]=color.g;
			color_buffer_data[i*9+j*3+2]=color.b;
		}
		vertex_buffer_data[i*9]=0;
		vertex_buffer_data[i*9+1]=0;
		vertex_buffer_data[i*9+2]=0;
		vertex_buffer_data[i*9+3]=radius*cos(current_angle);
		vertex_buffer_data[i*9+4]=(radius+0.12)*sin(current_angle);
		vertex_buffer_data[i*9+5]=0;
		vertex_buffer_data[i*9+6]=radius*cos(current_angle+angle);
		vertex_buffer_data[i*9+7]=(radius+0.12)*sin(current_angle+angle);
		vertex_buffer_data[i*9+8]=0;
		current_angle+=angle;
	}

	if(fill==1)
		semicircle = create3DObject(GL_TRIANGLES, (parts*9)/3, vertex_buffer_data, color_buffer_data, GL_FILL);
	else
		semicircle = create3DObject(GL_TRIANGLES, (parts*9)/3, vertex_buffer_data, color_buffer_data, GL_LINE);

}
void createCircle2 (string name, float weight, COLOR color, float x, float y, float r, int NoOfParts, string component, int fill)
{
	int parts = NoOfParts;
	float radius = r;
	GLfloat vertex_buffer_data[parts*9];
	GLfloat color_buffer_data[parts*9];
	int i,j;
	float angle=(2*M_PI/parts);
	float current_angle = 0;
	for(i=0;i<parts;i++){
		for(j=0;j<3;j++){
			color_buffer_data[i*9+j*3]=color.r;
			color_buffer_data[i*9+j*3+1]=color.g;
			color_buffer_data[i*9+j*3+2]=color.b;
		}
		vertex_buffer_data[i*9]=0;
		vertex_buffer_data[i*9+1]=0;
		vertex_buffer_data[i*9+2]=0;
		vertex_buffer_data[i*9+3]=radius*cos(current_angle);
		vertex_buffer_data[i*9+4]=radius*sin(current_angle);
		vertex_buffer_data[i*9+5]=0;
		vertex_buffer_data[i*9+6]=radius*cos(current_angle+angle);
		vertex_buffer_data[i*9+7]=radius*sin(current_angle+angle);
		vertex_buffer_data[i*9+8]=0;
		current_angle+=angle;
	}

	if(fill==1)
		circle2 = create3DObject(GL_TRIANGLES, (parts*9)/3, vertex_buffer_data, color_buffer_data, GL_FILL);
	else
		circle2 = create3DObject(GL_TRIANGLES, (parts*9)/3, vertex_buffer_data, color_buffer_data, GL_LINE);

}
/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		//        exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

	if (!window) {
		glfwTerminate();
		//        exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );
	COLOR cloudwhite = {229/255.0,255/255.0,255/255.0};
	COLOR cloudwhite2 = {229/255.0,255/255.0,0/255.0};
	COLOR lightblue = {79/255.0,129/255.0,189/255.0};
	COLOR lightpink={0.9098,0.7451,0.749};
	COLOR red={0.9804,0.2353,0.2431};
	COLOR green={0.1412,0.5137,0.1137};
	COLOR lightgreen={0.4471,0.6196,0.3804};
	COLOR color={0.9804,0.2353,0.2431};
	COLOR color2={ 0.9098,0.7451,0.749};  
	COLOR gr={0.6078,0.7333,0.349};
	COLOR black={0,0,0};
	COLOR red2={0.7529,0.3137,0.302};
	COLOR RED={1,0,0};
	block[0]=createBlock(red2);
	block[1]=createBlock(gr);
	block[2]=createBlock(black);
	createEllipse(0.3,0.2,lightpink,1);
	createEllipse(0.3,0.2,red,2);
	COLOR w1={0.0,0.7725,0.8078,};
	COLOR w2={0.078,0.5373,0.7529};
	createEllipse(0.5,0.3,w1,5);
	createEllipse(0.5,0.3,w2,6);
	createEllipse(0.3,0.2,lightgreen,3);
	createEllipse(0.3,0.2,green,4);
	createBucket(color,color2,1);
	createBucket(green,lightgreen,2);
	circle=createCircle("cloud1ac1",10000,cloudwhite,-250,110,0.25,1150,"background",1);
	createHalfCircle("cloud1ac1",10000,RED,-250,110,0.212,1150,"background",0);
	createCircle2("cloud1ac1",10000,lightblue,-250,110,0.27,1150,"background",1);
	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	   is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
	glfwSetScrollCallback(window, mousescroll); 

	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	/* Objects should be created before any other gl function and shaders */
	// Create the models
	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createRectangle ();



	createRectangle2 ();
	createBackground();

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}
mpg123_handle *mh = NULL;
ao_device *dev = NULL;
int main (int argc, char** argv)
{
	mpg123_handle *mh;
    unsigned char *buffer;
    size_t buffer_size;
    size_t done;
    int err;

    int driver;
    ao_device *dev;

    ao_sample_format format;
    int channels, encoding;
    long rate;

  

    /* initializations */
    ao_initialize();
    driver = ao_default_driver_id();
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    buffer_size = 3000;
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

    /* open the file and get the decoding format */
    mpg123_open(mh, "breakout.mp3");
    mpg123_getformat(mh, &rate, &channels, &encoding);

    /* set the output format and open the output device */
    format.bits = mpg123_encsize(encoding) * BITS;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;
    dev = ao_open_live(driver, &format, NULL);

    /* decode and play */

	GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);
	time(&start);  
	g.speed=0.01;
	g.increase=1;
	g.score=0;
	g.numlife=5;
	g.time=0;
	g.stage=1;
	g.pause=0;

	double last_update_time = glfwGetTime(), current_time;
	for(int i=0;i<50;i++){
		bricks f=g.brick[i];
		f.live=0;
	}
	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) {
 if (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK)
        ao_play(dev,(char*)buffer, done);
else mpg123_seek(mh, 0, SEEK_SET);

    /* clean up */


		// OpenGL Draw commands
		if(g.pause==0)
			draw(window);
		if(g.numlife==0){
			g.start=0;
			game_over();
		}

		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();

		// Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
		current_time = glfwGetTime(); // Time in seconds
		if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
			// do something every 0.5 seconds ..
			last_update_time = current_time;
		}
	}
    free(buffer);
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_shutdown();
	glfwTerminate();
	//    exit(EXIT_SUCCESS);
}
