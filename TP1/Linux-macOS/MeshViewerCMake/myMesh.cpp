#include "myMesh.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <utility>
#include <GL/glew.h>
#include "myVector3D.h"

using namespace std;

myMesh::myMesh(void)
{
	/**** TODO ****/
}


myMesh::~myMesh(void)
{
	/**** TODO ****/
}

void myMesh::clear()
{
	for (unsigned int i = 0; i < vertices.size(); i++) if (vertices[i]) delete vertices[i];
	for (unsigned int i = 0; i < halfedges.size(); i++) if (halfedges[i]) delete halfedges[i];
	for (unsigned int i = 0; i < faces.size(); i++) if (faces[i]) delete faces[i];

	vector<myVertex *> empty_vertices;    vertices.swap(empty_vertices);
	vector<myHalfedge *> empty_halfedges; halfedges.swap(empty_halfedges);
	vector<myFace *> empty_faces;         faces.swap(empty_faces);
}

void myMesh::checkMesh()
{
	vector<myHalfedge *>::iterator it;
	for (it = halfedges.begin(); it != halfedges.end(); it++)
	{
		if ((*it)->twin == NULL)
			break;
	}
	if (it != halfedges.end())
		cout << "Error! Not all edges have their twins!\n";
	else cout << "Each edge has a twin!\n";
}


bool myMesh::readFile(std::string filename)
{
	string s, t, u;
	vector<int> faceids;
	myHalfedge **hedges;

	ifstream fin(filename);
	if (!fin.is_open()) {
		cout << "Unable to open file!\n";
		return false;
	}
	name = filename;

	map<pair<int, int>, myHalfedge *> twin_map;
	map<pair<int, int>, myHalfedge *>::iterator it;

	while (getline(fin, s))
	{
		t = "";
		stringstream myline(s);
		myline >> t;
		if (t == "g") {}
		else if (t == "v")
		{
			float x, y, z;
			myline >> x >> y >> z;
			myVertex *v = new myVertex();
			v->point = new myPoint3D(x, y, z);
			vertices.push_back(v);
		}
		else if (t == "mtllib") {}
		else if (t == "usemtl") {}
		else if (t == "s") {}
		else if (t == "f")
		{
			faceids.clear();
			while (myline >> u)
				faceids.push_back(atoi((u.substr(0, u.find("/"))).c_str()) - 1);
			if (faceids.size() < 3)
				continue;

			hedges = new myHalfedge *[faceids.size()];
			for (unsigned int i = 0; i < faceids.size(); i++)
				hedges[i] = new myHalfedge();

			myFace *f = new myFace();
			f->adjacent_halfedge = hedges[0];

			for (unsigned int i = 0; i < faceids.size(); i++) {
				int iplusone  = (i + 1) % faceids.size();
				int iminusone = (i - 1 + faceids.size()) % faceids.size();

				hedges[i]->next = hedges[iplusone];
				hedges[i]->prev = hedges[iminusone];
				hedges[i]->source = vertices[faceids[i]];
				hedges[i]->adjacent_face = f;

				if (vertices[faceids[i]]->originof == NULL)
					vertices[faceids[i]]->originof = hedges[i];

				pair<int, int> key      = make_pair(faceids[i], faceids[iplusone]);
				pair<int, int> twin_key = make_pair(faceids[iplusone], faceids[i]);
				it = twin_map.find(twin_key);
				if (it != twin_map.end()) {
					hedges[i]->twin = it->second;
					it->second->twin = hedges[i];
				} else {
					twin_map[key] = hedges[i];
				}

				halfedges.push_back(hedges[i]);
			}

			delete[] hedges;
			faces.push_back(f);
		}
	}

	checkMesh();
	normalize();

	return true;
}


void myMesh::computeNormals()
{
	for (unsigned int i = 0; i < faces.size(); i++)
		if (faces[i] != NULL)
			faces[i]->computeNormal();

for (unsigned int i = 0; i < vertices.size(); i++)
		if (vertices[i] != NULL)
			vertices[i]->computeNormal();
}

void myMesh::normalize()
{
	if (vertices.size()<1) return;
	int tmpxmin = 0, tmpymin = 0, tmpzmin = 0, tmpxmax = 0, tmpymax = 0, tmpzmax = 0;
	for (unsigned int i = 0; i < vertices.size(); i++) {
		if (vertices[i]->point->X < vertices[tmpxmin]->point->X) tmpxmin = i;
		if (vertices[i]->point->X > vertices[tmpxmax]->point->X) tmpxmax = i;

		if (vertices[i]->point->Y < vertices[tmpymin]->point->Y) tmpymin = i;
		if (vertices[i]->point->Y > vertices[tmpymax]->point->Y) tmpymax = i;

		if (vertices[i]->point->Z < vertices[tmpzmin]->point->Z) tmpzmin = i;
		if (vertices[i]->point->Z > vertices[tmpzmax]->point->Z) tmpzmax = i;
	}

	double xmin = vertices[tmpxmin]->point->X, xmax = vertices[tmpxmax]->point->X,
		ymin = vertices[tmpymin]->point->Y, ymax = vertices[tmpymax]->point->Y,
		zmin = vertices[tmpzmin]->point->Z, zmax = vertices[tmpzmax]->point->Z;
	double scale = (xmax - xmin) > (ymax - ymin) ? (xmax - xmin) : (ymax - ymin);
	scale = scale > (zmax - zmin) ? scale : (zmax - zmin);
	for (unsigned int i=0; i<vertices.size(); i++) {
		vertices[i]->point->X -=(xmax+ xmin)/ 2;
		vertices[i]->point->Y -=(ymax+ ymin)/ 2;
		vertices[i]->point->Z -=(zmax+ zmin)/ 2;
		vertices[i]->point->X /= scale;
		vertices[i]->point->Y /= scale;
		vertices[i]->point->Z /= scale;
	}
}


void myMesh::splitFaceTRIS(myFace *f, myPoint3D *p)
{
	/**** TODO ****/
}

void myMesh::splitEdge(myHalfedge *e1, myPoint3D *p)
{

	/**** TODO ****/
}

void myMesh::splitFaceQUADS(myFace *f, myPoint3D *p)
{
	/**** TODO ****/
}


void myMesh::subdivisionCatmullClark()
{
	/**** TODO ****/
}

void myMesh::simplify()
{
	/**** TODO ****/
}

void myMesh::simplify(myVertex *)
{
	/**** TODO ****/
}

bool myMesh::triangulate(myFace *f)
{
	vector<myHalfedge *> bords;
	myHalfedge *h = f->adjacent_halfedge;
	do {
		if (h == NULL) return false;
		bords.push_back(h);
		h = h->next;
		if ((int)bords.size() > 1000) return false;
	} while (h != f->adjacent_halfedge);

	int nb = (int)bords.size();
	if (nb <= 3)
		return false;

	int nb_diag = nb - 3;
	vector<myHalfedge *> diag(nb_diag);
	vector<myHalfedge *> diag_inv(nb_diag);

	for (int i = 0; i < nb_diag; i++) {
		diag[i]     = new myHalfedge();
		diag_inv[i] = new myHalfedge();

		diag[i]->source     = bords[i + 2]->source;
		diag_inv[i]->source = bords[0]->source;

		diag[i]->twin     = diag_inv[i];
		diag_inv[i]->twin = diag[i];

		halfedges.push_back(diag[i]);
		halfedges.push_back(diag_inv[i]);
	}

	f->adjacent_halfedge = bords[0];
	bords[0]->adjacent_face = f;
	bords[1]->adjacent_face = f;
	diag[0]->adjacent_face  = f;
	bords[0]->next = bords[1]; 
	bords[1]->next = diag[0]; 
	diag[0]->next = bords[0];
	bords[0]->prev = diag[0];  
	bords[1]->prev = bords[0];
	 diag[0]->prev = bords[1];

	for (int i = 0; i < nb_diag - 1; i++) {
		myFace *nv_face = new myFace();
		nv_face->adjacent_halfedge = diag_inv[i];

		diag_inv[i]->adjacent_face = nv_face;
		bords[i +2]->adjacent_face= nv_face;
		diag[i+ 1]->adjacent_face = nv_face;

		diag_inv[i]->next  = bords[i+2]; 
		bords[i +2]->next = diag[i+1];
		 diag[i +1]->next = diag_inv[i];
		diag_inv[i]->prev  = diag[i+1]; 
		 bords[i+ 2]->prev = diag_inv[i]; diag[i + 1]->prev = bords[i + 2];

		faces.push_back(nv_face);
	}

	myFace *derniere = new myFace();
	derniere->adjacent_halfedge = diag_inv[nb_diag - 1];

	diag_inv[nb_diag - 1]->adjacent_face = derniere;
	bords[nb-2]->adjacent_face = derniere;
	bords[nb-1]->adjacent_face= derniere;

	diag_inv[nb_diag - 1]->next =bords[nb-2]; 
	bords[nb-2]->next = bords[nb-1]; 
	bords[nb-1]->next = diag_inv[nb_diag- 1];
	diag_inv[nb_diag-1]->prev =bords[nb- 1];
	 bords[nb- 2]->prev= diag_inv[nb_diag -1]; 
	 bords[nb-1]->prev = bords[nb- 2];

	faces.push_back(derniere);
	return true;
}

void myMesh::triangulate()
{
	int nb_faces = faces.size();
	for (int i=0; i<nb_faces;i++)
		triangulate(faces[i]);
}

