/*
 * TriangleGroup.hpp
 *
 *  Created on: Mar 5, 2013
 *      Author: econobeing5
 */

#ifndef TRIANGLEGROUP_HPP_
#define TRIANGLEGROUP_HPP_

#include <vector>
#include "vec.h"

typedef Angel::vec4  point4;
typedef Angel::vec4  color4;

//if storing the points directly doesn't work just store
//the indices of a huge array that has all the points
struct Triangle{
	point4 a, b, c; //the three points of the triangle
	color4 rgb; //diffuse lighting
};

class TriangleGroup{
public:
	std::vector<Triangle> triangles;

	TriangleGroup() {triangles.clear();}

	void addTriangle(point4 pa, point4 pb, point4 pc, color4 color) {
		Triangle tri;
		tri.a = pa;
		tri.b = pb;
		tri.c = pc;
		tri.rgb = color;
		triangles.push_back(tri);
	}

	void addTriangle(float ax, float ay, float az, float bx, float by,
			float bz, float cx, float cy, float cz, color4 color) {
		point4 aa(ax,ay,az,1.0);
		point4 bb(bx,by,bz,1.0);
		point4 cc(cx,cy,cz,1.0);
		addTriangle(aa,bb,cc,color);
	}
private:
};



#endif /* TRIANGLEGROUP_HPP_ */
