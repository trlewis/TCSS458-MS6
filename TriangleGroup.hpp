/*
 * TriangleGroup.hpp
 *
 *  Created on: Mar 5, 2013
 *      Author: Travis Lewis
 */

#ifndef TRIANGLEGROUP_HPP_
#define TRIANGLEGROUP_HPP_

#include <vector>
#include "vec.h"

typedef Angel::vec4  point4;
typedef Angel::vec4  color4;

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
};

#endif /* TRIANGLEGROUP_HPP_ */
