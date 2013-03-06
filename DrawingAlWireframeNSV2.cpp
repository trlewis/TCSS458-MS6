//TODO: load body parts into proper triangle groups
//TODO: be able to select different body parts
//TODO: be able to move different body parts
//TODO: draw the body parts

#include "Angel.h"
#include <string.h>

#include <iostream>

#include "materials.hpp"

#include "TriangleGroup.hpp"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

#define MAX_VERT 35000//20000
point4 data[MAX_VERT];
point4 colors[MAX_VERT];
int dataSize = 0;  // how many vertices

point4 light_position( 0.0, 0.0, 1.0, 0.0 );
mat4 CTM;

// shader variable locations
GLuint programID;
GLuint bufferID;
GLuint vPositionLoc;
GLuint vColorLoc;
GLuint ctmID;

//list of materials
MaterialList mtll;

float yRot = 0.0;

//decided to use defines instead of an enum so i could use indices for
//the triangles groups.
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

//hipL = (.55, -1.5, .04, 1));
//kneeL = (.7, -2.1, .16, 1));
//ankleL =  (.8, -2.9, .05, 1));
//shoulderL =(1.3, .28, -.32, 1));
//elbowL = (2, .06, -.30, 1));

//locations of joints
//point4 hipL(0.55,-1.5,0.04,1);
//point4 hipR(-0.55,-1.5,0.04,1);
//point4 kneeL(0.7,-2.1,0.16,1);
//point4 kneeR(-0.7,-2.1,0.16,1);
//point4 ankleL(0.8,-2.9,0.05,1);
//point4 ankleR(-0.8,-2.9,0.05,1);
//point4 shoulderL(1.3, .28, -.32, 1);
//point4 shoulderR(-1.3, .28, -.32, 1);
//point4 elbowL(2, .06, -.30, 1);
//point4 elbowR(-2, .06, -.30, 1);
//point4 body(0,0,0,1);

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

float joint_angles[TOTAL_GROUPS];

float rotate_amount = 5.0;

int selectedBodyPart = BODY;

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

int getGroupIndex(char* str) {
	if(strcmp(str,"shoe1R")==0 || strcmp(str,"shoe2R")==0 ||
			strcmp(str,"shoe3R")==0 || strcmp(str,"shoe4R")==0)
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
    //int v1,v2,v3;

    vec4 vertices[4000];
    int numVertices = 1;   // .obj input file numbers the first vertex starting at index 1

	int facetCount = 0;

	//printf("Enter the name of the input file (no blanks): ");
	//gets(fileName);
	//input = fopen(fileName, "r+");
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

				if (dataSize+3 > MAX_VERT) {
					printf("Arrays are not big enough!\n");
					system("PAUSE");
	                exit(1);
				}

				for(int i = 0 ; i < vCount-1 ; i++) {
					//this should add the triangles to the correct group
					bodyparts[bodygroup].addTriangle(vertices[v[0]],
							vertices[v[i]],vertices[v[i+1]],m.diffuse);

					//old code
//					colors[dataSize] = m.diffuse;
//					data[dataSize++] = vertices[v[0]];
//					colors[dataSize] = m.diffuse;
//					data[dataSize++] = vertices[v[i]];
//					colors[dataSize] = m.diffuse;
//					data[dataSize++] = vertices[v[i+1]];
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

	glEnable( GL_DEPTH_TEST );

    readFile();
    glClearColor(1.0, 1.0, 1.0, 1.0); // white background 
}

void tri(int i) {
	vec4 v1 = data[i];
	vec4 v2 = data[i+1];
	vec4 v3 = data[i+2];
	vec4 normal = normalize(cross(v3-v1,v2-v1));

	glMaterialfv(GL_FRONT, GL_DIFFUSE, colors[i]);

	glBegin(GL_POLYGON);
	glNormal3f(normal.x,normal.y,normal.z);
	glVertex3fv(v1);
	glVertex3fv(v2);
	glVertex3fv(v3);
	glEnd();
}

void tri(point4 a, point4 b, point4 c, color4 color) {
	vec4 normal = normalize(cross(c-a,b-a));
	glMaterialfv(GL_FRONT,GL_DIFFUSE,color);
	glBegin(GL_POLYGON);
	glNormal3f(normal.x,normal.y,normal.z);
	glVertex3fv(a);
	glVertex3fv(b);
	glVertex3fv(c);
	glEnd();
}

void displayParts(int index) {
	for(std::vector<Triangle>::iterator it = bodyparts[index].triangles.begin(),
			end = bodyparts[index].triangles.end() ; it != end ; ++it) {
		tri(it->a,it->b,it->c,it->rgb);
	}
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);


	glLoadIdentity();
	glRotatef(yRot,0,1,0);
	glScalef(0.3,0.3,-0.3);
	for(int i = 0 ; i < TOTAL_GROUPS ; i++) {
		glLoadIdentity();
		//glTranslatef(-joints[i].x,-joints[i].y,-joints[i].z);
		//glRotatef(joint_angles[i],1,0,0);
		//glTranslatef(joints[i].x,joints[i].y,joints[i].z);
		glRotatef(yRot,0,1,0);
		glScalef(0.3,0.3,-0.3);

		displayParts(i);

//		for(std::vector<Triangle>::iterator it = bodyparts[i].triangles.begin(),
//				end = bodyparts[i].triangles.end() ; it != end ; ++it) {
//			tri(it->a,it->b,it->c,it->rgb);
//		}
	}

	
    glutSwapBuffers();
}

//DON'T UPDATE LOCATIONS MANUALLY, USE GL FUNCTIONS

/*
void translateParts(int index, vec4 newLocation)
{
	mat4 toOrigin = Translate(-joints[index].x,-joints[index].y,-joints[index].z);
	mat4 fromOrigin = Translate(newLocation.x,newLocation.y,newLocation.z);
	mat4 allops = fromOrigin * toOrigin;

	joints[index] = newLocation;

	for(std::vector<Triangle>::iterator it = bodyparts[index].triangles.begin(),
			end = bodyparts[index].triangles.end() ; it != end ; ++it) {
		it->a = allops * it->a;
		it->b = allops * it->b;
		it->c = allops * it->c;
	}
}

void rotateParts(int index, float deg) {
	//translate rotate translate
	//make sure to change the joint location too!
	mat4 toOrigin, rotate, fromOrigin, allops;

	toOrigin = Translate(0.0,-joints[index].y,-joints[index].z);
	rotate = RotateX(deg);
	fromOrigin = Translate(0.0,joints[index].y,joints[index].z);
	allops = fromOrigin * rotate * toOrigin;

	for(std::vector<Triangle>::iterator it =
			bodyparts[index].triangles.begin(),
			end = bodyparts[index].triangles.end(); it != end ; ++it) {
		it->a = allops * it->a;
		it->b = allops * it->b;
		it->c = allops * it->c;
	}

	switch(index){
	case FOOT_L:
		break;
	case LOWER_LEG_L:
		translateParts(FOOT_L, allops * joints[FOOT_L]);
		rotateParts(FOOT_L,deg);
		break;
	case UPPER_LEG_L:
		translateParts(LOWER_LEG_L, allops * joints[LOWER_LEG_L]);
		//translateParts(FOOT_L,allops*joints[FOOT_L]);
		rotateParts(LOWER_LEG_L,deg);
	}
}


void rotateParts(int index, float deg, point4 about) {
	//translate rotate translate
	//make sure to change the joint location too!
	mat4 toOrigin, rotate, fromOrigin, allops;

	toOrigin = Translate(0.0,-about.y,-about.z);
	rotate = RotateX(deg);
	fromOrigin = Translate(0.0,about.y,about.z);
	allops = fromOrigin * rotate * toOrigin;

	for(std::vector<Triangle>::iterator it =
			bodyparts[index].triangles.begin(),
			end = bodyparts[index].triangles.end(); it != end ; ++it) {
		it->a = allops * it->a;
		it->b = allops * it->b;
		it->c = allops * it->c;
	}

	switch(index){
	case FOOT_L:
		break;
	case LOWER_LEG_L:
		translateParts(FOOT_L, allops * joints[FOOT_L]);
		rotateParts(FOOT_L,deg,about);
		break;
	case UPPER_LEG_L:
		translateParts(LOWER_LEG_L, allops * joints[LOWER_LEG_L]);
		//translateParts(FOOT_L,allops*joints[FOOT_L]);
		rotateParts(LOWER_LEG_L,deg,about);
	}
}
*/

//void rotateParts(int index, float deg)
//{
//	glMatrixMode(MODEL_VIEW);
//	glLoadIdentity();
//	glTranslate()
//	glRotatef(joint_angles[i],1,0,0);
//
//	switch(index)
//	{
//	case UPPER_LEG_L:
//
//	}
//
//	//glRotatef(joint_angles[i],1,0,0);
//	//glRotatef(yRot,0,1,0);
//	//glScalef(0.3,0.3,-0.3);
//}

void myspecialkey(int key, int mousex, int mousey)
{

    //if(key == 100) {
	if(key == GLUT_KEY_LEFT) {
        yRot += 1;
        if (yRot > 360.0 ) {
	        yRot -= 360.0;
	    }
	}
    //if(key == 102) {
	if(key == GLUT_KEY_RIGHT) {
        yRot -= 1;
        if (yRot < -360.0 ) {
	        yRot += 360.0;
	    }
	}
	if(key == GLUT_KEY_DOWN)
		joint_angles[selectedBodyPart] += rotate_amount;
		//rotateParts(selectedBodyPart,rotate_amount);
		//rotateParts(selectedBodyPart,rotate_amount,joints[selectedBodyPart]);
	if(key == GLUT_KEY_UP)
		joint_angles[selectedBodyPart] -= rotate_amount;
		//rotateParts(selectedBodyPart,-rotate_amount);
		//rotateParts(selectedBodyPart,-rotate_amount,joints[selectedBodyPart]);

    glutPostRedisplay();
}

void mykey(unsigned char key, int mousex, int mousey)
{
    //float dr = 3.14159/180.0*5.0;
    if(key=='q'||key=='Q') exit(0);

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

    glutPostRedisplay();
}

void reshape(GLsizei ww, GLsizei hh)
{
    glViewport(0,0, ww, hh);
}


int main(int argc, char** argv)
{
	for(int i = 0 ; i < TOTAL_GROUPS ; i++)
		joint_angles[i] = 0;

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

