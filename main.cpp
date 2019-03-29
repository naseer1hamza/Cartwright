/*
* Final Project comuter graphics
*
* Authors Hamza Naseer, Matthew Werzbicki, Amar Al-adil 
*
*Date: 201903-20
*
*
*/

#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <GL/glew.h>
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "apis/stb_image.h"

#include "ShaderProgram.h"
#include "ObjMesh.h"

#define SCALE_FACTOR 2.0f
#define SHININESS_STEP 1.0f

int width, height;

GLuint programId;
GLuint vertexBuffer;
GLuint indexBuffer;
GLenum positionBufferId;
GLuint positions_vbo = 0;
float angle = 0.0f;
GLuint textureCoords_vbo = 0;
GLuint normals_vbo = 0;
GLuint colours_vbo = 0;
GLuint textureId;
bool animateLight = true;
bool rotateObject = true;
bool scaleObject = true;
float lightOffsetY = 0.0f;
float scaleFactor = 10.0f;
float lastX = std::numeric_limits<float>::infinity();
float lastY = std::numeric_limits<float>::infinity();
float moveX = 0.0f;
float moveY = 0.0f;
unsigned int numVertices;
glm::vec3 eyePosition(40, 30, 30);

bool rotating = true;

unsigned int loadTexture(char const * path);

static void createTexture(std::string filename) {
   int imageWidth, imageHeight;
   int numComponents;

   // get the bitmap and load it with its height and weight and components 
   unsigned char *bitmap = stbi_load(filename.c_str(),
                                     &imageWidth,
                                     &imageHeight,
                                     &numComponents, 4);

   glGenTextures(1, &textureId);
   glBindTexture(GL_TEXTURE_2D, textureId);
   glGenerateTextureMipmap(textureId);
   glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, imageWidth, imageHeight,
                0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);   
   glBindTexture(GL_TEXTURE_2D, textureId);
   glActiveTexture(GL_TEXTURE0);
 
   stbi_image_free(bitmap);
}

/*
*
* this function makes a mesh of the object we want to load. 
* it gets the number of verticies and texture position to render
*
*/
static void drawLogo(void) {
  ObjMesh mesh;
  mesh.load("meshes/oval.obj", true, true);

  numVertices = mesh.getNumIndexedVertices();
  Vector3* vertexPositions = mesh.getIndexedPositions();
  Vector2* vertexTextureCoords = mesh.getIndexedTextureCoords();
  Vector3* vertexNormals = mesh.getIndexedNormals();

  glGenBuffers(1, &positions_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, positions_vbo);
  glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vector3), vertexPositions, GL_STATIC_DRAW);

  glGenBuffers(1, &textureCoords_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, textureCoords_vbo);
  glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vector2), vertexTextureCoords, GL_STATIC_DRAW);

  glGenBuffers(1, &normals_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
  glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vector3), vertexNormals, GL_STATIC_DRAW);

  unsigned int* indexData = mesh.getTriangleIndices();
  int numTriangles = mesh.getNumTriangles();

  glGenBuffers(1, &indexBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * numTriangles * 3, indexData, GL_STATIC_DRAW);
}



/*
 * 
 * this function allows us to update the logo position
 * it allow us to to translate the object by using sine and cosine
 * we are also rotating the object unitl it becomes straight
 */
static void update(void) {
	int milliseconds = glutGet(GLUT_ELAPSED_TIME);
	float degrees = (float)milliseconds / 80.0f;

	// rotate the shape about the y-axis so that we can see the shading
	if (rotateObject && angle <= 54) {
		angle = degrees;
	}

	//std::cout << 20sinf(degrees) << std::endl;
	moveX = 20*sinf((float)milliseconds / 1000.0f);
	moveY = 20*cosf((float)milliseconds / 1000.0f);


	if (scaleFactor <= 20.0f) {
		scaleFactor = 10.0f + (float)milliseconds / 70.0f;
	}


	// move the light position over time along the x-axis, so we can see how it affects the shading
	if (animateLight) {
		float t = milliseconds / 500.0f;
		if (t < 3.4) {
			lightOffsetY = sinf(t) * 100;
		}

	}

	glutPostRedisplay();
}

static void render(void) {
   float moveZ = 0.0f;
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // activate our shader program
	glUseProgram(programId);

   // turn on depth buffering
   glEnable(GL_DEPTH_TEST);
   
   float aspectRatio = (float)width / (float)height;
   glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 1000.0f);

   glm::mat4 view = glm::lookAt(
	   eyePosition,
	   glm::vec3(0, 0, 0),   
	   glm::vec3(0, 1, 0)     
   );

   // model matrix: translate, scale, and rotate the model
   glm::vec3 rotationAxis(0,1,0);
   glm::mat4 model = glm::mat4(1.0f);
   // translate the object in x y z
   model = glm::translate(model, glm::vec3(moveX, moveY, moveZ));
   model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0)); // rotate about the y-axis
   // scaling the object
   model = glm::scale(model, glm::vec3(25.0f, 25.0f, 25.0f));

   // model-view-projection matrix
   glm::mat4 mvp = projection * view * model;
   GLuint mvpMatrixId = glGetUniformLocation(programId, "MVP");
   glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, &mvp[0][0]);

   // texture sampler - a reference to the texture we've previously created
   // send the texture id to the texture sampler
   GLuint textureUniformId = glGetUniformLocation(programId, "textureSampler");
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, textureId);
   glUniform1i(textureUniformId, 0);

   // the position of our camera/eye
   GLuint eyePosId = glGetUniformLocation(programId, "u_EyePosition");
   glUniform3f(eyePosId, eyePosition.x, eyePosition.y, eyePosition.z);

   // the position of our light
   GLuint lightPosId = glGetUniformLocation(programId, "u_LightPos");
   glUniform3f(lightPosId, 10, 8, 10);

   // find the names (ids) of each vertex attribute
   GLint positionAttribId = glGetAttribLocation(programId, "position");
   GLint colourAttribId = glGetAttribLocation(programId, "colour");
   GLint textureCoordsAttribId = glGetAttribLocation(programId, "textureCoords");
   GLint normalAttribId = glGetAttribLocation(programId, "normal");


   // provide the vertex positions to the shaders
   glBindBuffer(GL_ARRAY_BUFFER, positions_vbo);
   glEnableVertexAttribArray(positionAttribId);
   glVertexAttribPointer(positionAttribId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

   // provide the vertex texture coordinates to the shaders
   glBindBuffer(GL_ARRAY_BUFFER, textureCoords_vbo);
   glEnableVertexAttribArray(textureCoordsAttribId);
   glVertexAttribPointer(textureCoordsAttribId, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

   // provide the vertex normals to the shaders
   glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
   glEnableVertexAttribArray(normalAttribId);
   glVertexAttribPointer(normalAttribId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// draw the triangles
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glDrawElements(GL_TRIANGLES, numVertices, GL_UNSIGNED_INT, (void*)0);

	// disable the attribute arrays
   glDisableVertexAttribArray(positionAttribId);
   glDisableVertexAttribArray(textureCoordsAttribId);
   glDisableVertexAttribArray(normalAttribId);
   glDisableVertexAttribArray(colourAttribId);

	// make the draw buffer to display buffer (i.e. display what we have drawn)
	glutSwapBuffers();
}

/**
 * 
 * function  changes the height and weiht of the shape
 */
static void reshape(int w, int h) {

  glViewport(0, 0, w, h);

  width = w;
  height = h;
}

/**
 * 
 * this function lets you drad using scale.
 */
static void drag(int x, int y) {
	if (!isinf(lastX) && !isinf(lastY)) {
		float dx = lastX - (float)x;
		float dy = lastY - (float)y;
		float distance = sqrt(dx * dx + dy * dy);

		if (dy > 0.0f) {
			scaleFactor = SCALE_FACTOR / distance;
		}
		else {
			scaleFactor = distance / SCALE_FACTOR;
		}
	}
	else {
		lastX = (float)x;
		lastY = (float)y;
	}
}
/*
* function ets the last x and y position of you mouse click
*/
static void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		lastX = std::numeric_limits<float>::infinity();
		lastY = std::numeric_limits<float>::infinity();
	}
}


int main(int argc, char** argv) {
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
   glutInitWindowSize(800, 600);
   glutCreateWindow("Cartwright Final Project- Amar, Hamza, Matthew");
   glutIdleFunc(&update);
   glutDisplayFunc(&render);
   glutReshapeFunc(&reshape);
   glutMotionFunc(&drag);
   glutMouseFunc(&mouse);
   glClearColor(0.5, 0.5, 0.5, 0.1);
   
   glewInit();
   if (!GLEW_VERSION_2_0) {
      std::cerr << "OpenGL 2.0 not available" << std::endl;
      return 1;
   }

   drawLogo();

   createTexture("textures/stone.jpg");

   ShaderProgram program;
  	program.loadShaders("shaders/vertex.glsl", "shaders/fragment.glsl");
	
  	programId = program.getProgramId();

   glutMainLoop();

   return 0;
}
