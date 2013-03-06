
#ifndef __MATERIALS_H__
#define __MATERIALS_H__

#include <string>
#include <vector>
#include <iostream>

#include "vec.h"

typedef Angel::vec4 color4;

struct Material {
	std::string name;
	color4 ambient;
	color4 diffuse;
	color4 specular;
};

class MaterialList {
public:
	std::vector<Material> materials;

	MaterialList() { materials.clear(); }

	MaterialList(char* filename) {
		materials.clear();
		readFile(filename);
	}

	Material getMaterial(char* material_name) {
		Material m;
		for(std::vector<Material>::iterator it = materials.begin(), 
				end = materials.end() ; it != end ; ++it) {
			if(strcmp(material_name,it->name.c_str()) == 0)
				m = *it;
		}
		return m;
	}

private:
	void readFile(char* filename) {
		char str[100];
		FILE* input;
		float r,g,b;

		input = fopen(filename,"r+");

		Material cm;
		if(input == NULL)
			perror("Error opening material file");
		else {
			while(feof(input) == 0) {
				fscanf(input,"%s",str);
				if(strcmp(str,"newmtl") == 0) {
					materials.push_back(cm);
					fscanf(input,"%s",str);
					//cm.name = *str;
					cm.name = std::string(str);
					whiteAlpha(cm);
				}
				else if(strcmp(str,"Ka") == 0 ) {
					fscanf(input,"%f %f %f",&r,&g,&b);
					cm.ambient.x = r;
					cm.ambient.y = g;
					cm.ambient.z = b;
				}
				else if(strcmp(str,"Kd") == 0 ) {
					fscanf(input,"%f %f %f",&r,&g,&b);
					cm.diffuse.x = r;
					cm.diffuse.y = g;
					cm.diffuse.z = b;
				}

			}//while not eof
		}// else file not null
	}

	void whiteAlpha(Material& m) {
		m.ambient.w = 1;
		m.diffuse.w = 1;
		m.specular.w = 1;
	}
};

#endif //__MATERIALS_H__