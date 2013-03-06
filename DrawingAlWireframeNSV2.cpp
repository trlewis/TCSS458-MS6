/*
Al with movable limbs. Created by Travis Lewis
*/

#include "Angel.h"
#include <string.h>

#include <iostream>

#include "materials.hpp"
#include "TriangleGroup.hpp"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

// shader variable locations
GLuint programID;
GLuint bufferID;
GLuint vPositionLoc;
GLuint vColorLoc;
GLuint ctmID;

//list of materials
MaterialList mtll;

float yRot = 0.0;//current y rotation

//decided to use defines instead of an enum for reasons.
#define FOOT_L 		0
#define FOOT_R 		1
#define LOWER_LEG_L 2
#define LOWER_LEG_R 3
#define UPPER_LEG_L 4
#define UPPER_LEG_R 5
#define LOWER_ARM_L 6
#define LOWER_ARM_R 7
#define UPPER_ARM_L 8
#define UPPER_ARM_R 9
#define BODY 		10
#define TOTAL_GROUPS 11

TriangleGroup bodyparts[TOTAL_GROUPS];
int selectedBodyPart = BODY;

point4 joints[] = {point4(0.8,-2.9,0.05,1), //ankleL
		point4(-0.8,-2.9,0.05,1), //ankleR
		point4(0.7,-2.1,0.16,1), //kneeL
		point4(-0.7,-2.1,0.16,1), //kneeR
		point4(0.55,-1.5,0.04,1), //hipL
		point4(-0.55,-1.5,0.04,1), //hipR
		point4(2, .06, -.30, 1), //elbowL
		point4(-2, .06, -.30, 1), //elbowR
		point4(1.3, .28, -.32, 1), //shoulderL
		point4(-1.3, .28, -.32, 1), //shoulderR
		point4(0,0,0,1) //body
};

vec3 angles[TOTAL_GROUPS]; //current angle of rotation for each body part
#define X 0
#define Y 1
#define Z 2
int control_axis = X; //which axis to rotate around
float rotate_amount = 5.0;



int parseVertString(char * verticesString, int v[]) {
	char token[20];
	int vCount = 0;
	int t = 0;
	for (int s = 0; verticesString[s] != '\n'; s++) {
		if (verticesString[s] != ' ') {
			token[t++] = verticesString[s];
		} else if (t > 0) {
			token[t++] = 0;
			v[vCount++] = atoi(token);
			t = 0;
		}
	}

	if (t > 0) {
		token[t++] = 0;
		v[vCount++] = atoi(token);
	}
	return vCount;
}

int getGroupIndex(char* str) 
{
	if(strcmp(str,"shoe1R")==0 || strcmp(str,"shoe2R")==0 
			|| strcmp(str,"shoe3R")==0 || strcmp(str,"shoe4R")==0)
		return FOOT_R;
	else if(strcmp(str,"shoe1L")==0 || strcmp(str,"shoe2L")==0
			|| strcmp(str,"shoe3L")==0 || strcmp(str,"shoe4L")==0)
		return FOOT_L;
	else if(strcmp(str,"lowlegR") == 0)
		return LOWER_LEG_R;
	else if(strcmp(str,"lowlegL")==0)
		return LOWER_LEG_L;
	else if(strcmp(str,"uplegR")==0)
		return UPPER_LEG_R;
	else if(strcmp(str,"uplegL")==0)
		return UPPER_LEG_L;
	else if(strcmp(str,"lowarmR")==0 || strcmp(str,"handR")==0
			|| strcmp(str,"sleaveR")==0)
		return LOWER_ARM_R;
	else if(strcmp(str,"lowarmL")==0 || strcmp(str,"handL")==0
			|| strcmp(str,"sleaveL")==0)
		return LOWER_ARM_L;
	else if(strcmp(str,"uparmR")==0)
		return UPPER_ARM_R;
	else if(strcmp(str,"uparmL")==0)
		return UPPER_ARM_L;

	//default return
	return BODY;
}

void readFile() {
	Material m;
	bool readfirstg = false;

    char fileName[50];
    char str1[300];
    char str2[300];
    FILE *input;
    float x,y,z;

    vec4 vertices[4000];
    int numVertices = 1;   // .obj input file numbers the first vertex starting at index 1
	int facetCount = 0;

	input = fopen("al.obj","r+");
	int bodygroup = BODY;

    if (input == NULL)
        perror ("Error opening file");
    else {
        while(feof(input)== 0){
            fscanf(input,"%s",str1);
			if(strcmp(str1,"mtllib")==0) {
				fscanf(input,"%s",str1);
				mtll = MaterialList(str1);
			}
			if(strcmp(str1,"g")==0) {
				if(readfirstg) {
					fscanf(input,"%s",str1);
					m = mtll.getMaterial(str1);
					bodygroup = getGroupIndex(str1);
				}
				else
					readfirstg = true;
			}
			if (strcmp(str1,"v")==0) {  // lines starting with v  =  a vertex 
				fscanf(input,"%f %f %f", &x, &y, &z);
				vertices[numVertices++] = vec4(x,y,z,1);
			} else if (strcmp(str1,"f")==0) {  // lines starting with f = a facet
				facetCount++;

				char verticesString[200];
				fgets(verticesString, 200, input);

				int v[50];
				int vCount = parseVertString(verticesString,v);

				//add the triangles to the appropriate group
				for(int i = 0 ; i < vCount-1 ; i++) {
					bodyparts[bodygroup].addTriangle(vertices[v[0]],
							vertices[v[i]],vertices[v[i+1]],m.diffuse);
				}
			} else {  // line began with something else - ignore for now
				fscanf(input, "%[^\n]%*c", str2);
				//printf("Junk line : %s %s\n", str1, str2);
			}
		}
	}
	printf("Facet count = %d\n",facetCount);
}

void init()
{
	// setting light properties
	color4 light_diffuse( 0.4, 0.4, 0.4, 1.0 );
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);

	//projection stuff
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glEnable( GL_DEPTH_TEST );

    readFile();
    glClearColor(1.0, 1.0, 1.0, 1.0); // white background 
}

void tri(point4 a, point4 b, point4 c, color4 color) 
{
	vec4 normal = normalize(cross(c-a,b-a));
	glMaterialfv(GL_FRONT,GL_DIFFUSE,color);
	glBegin(GL_POLYGON);
	glNormal3f(normal.x,normal.y,normal.z);
	glVertex3fv(a);
	glVertex3fv(b);
	glVertex3fv(c);
	glEnd();
}

void displayParts(int index) 
{
	for(std::vector<Triangle>::iterator it = bodyparts[index].triangles.begin(),
			end = bodyparts[index].triangles.end() ; it != end ; ++it) 
		tri(it->a,it->b,it->c,it->rgb);
}

void setupRotationMatrix(int part) 
{
	//I took this stuff out of display to save lines... a lot of lines.
	glTranslatef(joints[part].x,joints[part].y,joints[part].z);
	glRotatef(angles[part].x,1,0,0);
	glRotatef(angles[part].y,0,1,0);
	glRotatef(angles[part].z,0,0,1);
	glTranslatef(-joints[part].x,-joints[part].y,-joints[part].z);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for(int i = 0 ; i < TOTAL_GROUPS ; i++) {
		//setup for every transformation
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(yRot,0,1,0);
		glScalef(0.3,0.3,-0.3);

		//each body part requires different stuff to be done to it
		if(i == BODY) {
			displayParts(BODY);
		}
		else if(i == UPPER_LEG_L) {
			setupRotationMatrix(UPPER_LEG_L);
			displayParts(UPPER_LEG_L);
		}
		else if(i == UPPER_LEG_R) {
			setupRotationMatrix(UPPER_LEG_R);
			displayParts(UPPER_LEG_R);
		}
		else if(i == LOWER_LEG_L) {
			setupRotationMatrix(UPPER_LEG_L);
			setupRotationMatrix(LOWER_LEG_L);
			displayParts(LOWER_LEG_L);
		}
		else if(i == LOWER_LEG_R) {
			setupRotationMatrix(UPPER_LEG_R);
			setupRotationMatrix(LOWER_LEG_R);
			displayParts(LOWER_LEG_R);
		}
		else if(i == FOOT_L) {
			setupRotationMatrix(UPPER_LEG_L);
			setupRotationMatrix(LOWER_LEG_L);
			setupRotationMatrix(FOOT_L);
			displayParts(FOOT_L);
		}
		else if(i == FOOT_R) {
			setupRotationMatrix(UPPER_LEG_R);
			setupRotationMatrix(LOWER_LEG_R);
			setupRotationMatrix(FOOT_R);
			displayParts(FOOT_R);
		}
		else if(i == UPPER_ARM_L) {
			setupRotationMatrix(UPPER_ARM_L);
			displayParts(UPPER_ARM_L);
		}
		else if(i == UPPER_ARM_R) {	
			setupRotationMatrix(UPPER_ARM_R);
			displayParts(UPPER_ARM_R);
		}
		else if(i == LOWER_ARM_L) {
			setupRotationMatrix(UPPER_ARM_L);
			setupRotationMatrix(LOWER_ARM_L);
			displayParts(LOWER_ARM_L);
		}
		else if(i == LOWER_ARM_R) {	
			setupRotationMatrix(UPPER_ARM_R);
			setupRotationMatrix(LOWER_ARM_R);
			displayParts(LOWER_ARM_R);
		}
	}

    glutSwapBuffers();
}

void myspecialkey(int key, int mousex, int mousey)
{
	//rotation for Al as a whole
	if(key == GLUT_KEY_LEFT) {
        yRot += 1;
        if (yRot > 360.0 ) {
	        yRot -= 360.0;
	    }
	}
	if(key == GLUT_KEY_RIGHT) {
        yRot -= 1;
        if (yRot < -360.0 ) {
	        yRot += 360.0;
	    }
	}

	//rotation for limbs
	if(key == GLUT_KEY_DOWN) {
		if(control_axis == X) angles[selectedBodyPart].x += rotate_amount;
		if(control_axis == Y) angles[selectedBodyPart].y += rotate_amount;
		if(control_axis == Z) angles[selectedBodyPart].z += rotate_amount;
	}
	if(key == GLUT_KEY_UP) {
		if(control_axis == X) angles[selectedBodyPart].x -= rotate_amount;
		if(control_axis == Y) angles[selectedBodyPart].y -= rotate_amount;
		if(control_axis == Z) angles[selectedBodyPart].z -= rotate_amount;
	}

    glutPostRedisplay();
}

void mykey(unsigned char key, int mousex, int mousey)
{
    if(key=='q'||key=='Q') exit(0);

	//for selecting which body part to control
    if(key=='a') selectedBodyPart = FOOT_L;
    if(key=='A') selectedBodyPart = FOOT_R;
    if(key=='k') selectedBodyPart = LOWER_LEG_L;
    if(key=='K') selectedBodyPart = LOWER_LEG_R;
    if(key=='h') selectedBodyPart = UPPER_LEG_L;
    if(key=='H') selectedBodyPart = UPPER_LEG_R;
    if(key=='e') selectedBodyPart = LOWER_ARM_L;
    if(key=='E') selectedBodyPart = LOWER_ARM_R;
    if(key=='s') selectedBodyPart = UPPER_ARM_L;
    if(key=='S') selectedBodyPart = UPPER_ARM_R;
    if(key=='b' || key=='B') selectedBodyPart = BODY;

	//selecting which axis to rotate the selected body part around
	if(key=='x' || key=='X') control_axis = X;
	if(key=='y' || key=='Y') control_axis = Y;
	if(key=='z' || key=='Z') control_axis = Z;

    glutPostRedisplay();
}

void reshape(GLsizei ww, GLsizei hh)
{
    glViewport(0,0, ww, hh);
}


int main(int argc, char** argv)
{
	for(int i = 0 ; i < TOTAL_GROUPS ; i++)
		angles[i] = vec3(0,0,0);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
    glutCreateWindow("Non-Defective Al Capone - No Shaders (Travis Lewis)");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(mykey);
	glutSpecialFunc(myspecialkey);

    glEnable(GL_DEPTH_TEST);
    glutMainLoop();
    return 0;
}

