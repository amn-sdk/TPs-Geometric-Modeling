#include "myVertex.h"
#include "myVector3D.h"
#include "myHalfedge.h"
#include "myFace.h"

myVertex::myVertex(void)
{
	point = NULL;
	originof = NULL;
	normal = new myVector3D(1.0,1.0,1.0);
}

myVertex::~myVertex(void)
{
	if (normal) delete normal;
}

void myVertex::computeNormal()
{
	normal->dX=  0.0;
	normal->dY=0.0;
	normal->dZ= 0.0;
	int nb=0;
	if (originof==NULL)
		return;
	myHalfedge *h = originof;
	do {
		if (h->adjacent_face != NULL && h->adjacent_face->normal!= NULL) {
			normal->dX+= h->adjacent_face->normal->dX;
			normal->dY+= h->adjacent_face->normal->dY;
			normal->dZ+= h->adjacent_face->normal->dZ;
			nb++;
		}
		if (h->twin == NULL)
			break;
		h = h->twin->next;
	} while (h != originof);
	if (nb > 0) {
		normal->dX /= nb;
		normal->dY /= nb;
		normal->dZ /= nb;
	}
	normal->normalize();
}
