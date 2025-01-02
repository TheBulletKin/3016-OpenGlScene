#ifndef VERTATTRIBUTE_H
#define VERTATTRIBUTE_H

enum class VertAttributeTypes {
	POSITION = 0,
	NORMAL = 1,
	TEXCOORD = 2,
	COLOURVERTEX = 3,
	INSTANCEMATRIX = 6
   
};

struct VertAttribute {
    int length;
    VertAttributeTypes type;
};

#endif
