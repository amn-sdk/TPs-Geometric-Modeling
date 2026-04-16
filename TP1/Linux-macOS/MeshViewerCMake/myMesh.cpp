#include "myMesh.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <utility>
#include <array>
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
		if ((int)bords.size()>1000) return false;
	} while (h != f->adjacent_halfedge);
	int nb = (int)bords.size();
	if (nb <= 3)
		return false;

	vector<myVertex *> poly(nb);
	for (int i = 0; i < nb; i++)
		poly[i] = bords[i]->source;

	auto proj = [&](myVertex *v, int axe) -> pair<double, double> {
		if (axe == 0) return make_pair(v->point->Y, v->point->Z);
		if (axe == 1) return make_pair(v->point->X, v->point->Z);
		return make_pair(v->point->X, v->point->Y);
	};

	myVector3D n(0.0, 0.0, 0.0);
	for (int i = 0; i < nb; i++) {
		myPoint3D *p = poly[i]->point;
		myPoint3D *q = poly[(i + 1) % nb]->point;
		n.dX += (p->Y - q->Y) * (p->Z + q->Z);
		n.dY += (p->Z - q->Z) * (p->X + q->X);
		n.dZ += (p->X - q->X) * (p->Y + q->Y);
	}
	double ax = fabs(n.dX), ay = fabs(n.dY), az = fabs(n.dZ);
	int axe = 2;
	if (ax >= ay && ax >= az) axe = 0;
	else if (ay >= ax && ay >= az) axe = 1;

	vector<int> ids(nb);
	for (int i = 0; i < nb; i++) ids[i] = i;

	auto orient2d = [&](int i0, int i1, int i2) -> double {
		pair<double, double> a = proj(poly[i0], axe);
		pair<double, double> b = proj(poly[i1], axe);
		pair<double, double> c = proj(poly[i2], axe);
		return (b.first - a.first) * (c.second - a.second) - (b.second - a.second) * (c.first - a.first);
	};

	double aire = 0.0;
	for (int i = 0; i < nb; i++) {
		pair<double, double> p = proj(poly[i], axe);
		pair<double, double> q = proj(poly[(i + 1) % nb], axe);
		aire += p.first * q.second - q.first * p.second;
	}
	bool ccw = (aire > 0.0);

	auto dansTriangle = [&](int ia, int ib, int ic, int ip) -> bool {
		double c1 = orient2d(ia, ib, ip);
		double c2 = orient2d(ib, ic, ip);
		double c3 = orient2d(ic, ia, ip);
		double eps = 1e-12;
		if (ccw) return c1 >= -eps && c2 >= -eps && c3 >= -eps;
		return c1 <= eps && c2 <= eps && c3 <= eps;
	};

	vector<array<int, 3>> tris;
	int garde = 0;
	while ((int)ids.size() > 3 && garde < 10000) {
		garde++;
		bool coupe = false;
		int m = (int)ids.size();
		for (int i = 0; i < m; i++) {
			int ip = ids[(i - 1 + m) % m];
			int ic = ids[i];
			int in = ids[(i + 1) % m];
			double o = orient2d(ip, ic, in);
			if ((ccw && o <= 1e-12) || (!ccw && o >= -1e-12))
				continue;
			bool contient = false;
			for (int j = 0; j < m; j++) {
				int it = ids[j];
				if (it == ip || it == ic || it == in) continue;
				if (dansTriangle(ip, ic, in, it)) {
					contient = true;
					break;
				}
			}
			if (contient) continue;
			tris.push_back({ ip, ic, in });
			ids.erase(ids.begin() + i);
			coupe = true;
			break;
		}
		if (!coupe) return false;
	}
	if ((int)ids.size() == 3)
		tris.push_back({ ids[0], ids[1], ids[2] });
	if ((int)tris.size() != nb - 2)
		return false;

	map<pair<myVertex *, myVertex *>, myHalfedge *> bord_map;
	for (int i = 0; i < nb; i++) {
		myVertex *a = poly[i];
		myVertex *b = poly[(i + 1) % nb];
		bord_map[make_pair(a, b)] = bords[i];
	}

	map<pair<myVertex *, myVertex *>, myHalfedge *> interne_map;
	vector<myFace *> nv_faces;
	nv_faces.push_back(f);
	for (int i = 1; i < (int)tris.size(); i++)
		nv_faces.push_back(new myFace());

	for (int t = 0; t < (int)tris.size(); t++) {
		int ia = tris[t][0], ib = tris[t][1], ic = tris[t][2];
		myVertex *va = poly[ia], *vb = poly[ib], *vc = poly[ic];
		myVertex *vs[3] = { va, vb, vc };
		myHalfedge *hes[3] = { NULL, NULL, NULL };

		for (int e = 0; e < 3; e++) {
			myVertex *s = vs[e];
			myVertex *d = vs[(e + 1) % 3];
			pair<myVertex *, myVertex *> k = make_pair(s, d);
			map<pair<myVertex *, myVertex *>, myHalfedge *>::iterator itb = bord_map.find(k);
			if (itb != bord_map.end()) {
				hes[e] = itb->second;
			} else {
				map<pair<myVertex *, myVertex *>, myHalfedge *>::iterator iti = interne_map.find(k);
				if (iti != interne_map.end()) {
					hes[e] = iti->second;
				} else {
					myHalfedge *hn = new myHalfedge();
					hn->source = s;
					if (s->originof == NULL) s->originof = hn;
					map<pair<myVertex *, myVertex *>, myHalfedge *>::iterator itr = interne_map.find(make_pair(d, s));
					if (itr != interne_map.end()) {
						hn->twin = itr->second;
						itr->second->twin = hn;
					}
					interne_map[k] = hn;
					halfedges.push_back(hn);
					hes[e] = hn;
				}
			}
		}

		myFace *ft = nv_faces[t];
		ft->adjacent_halfedge = hes[0];
		for (int e = 0; e < 3; e++) {
			hes[e]->adjacent_face = ft;
			hes[e]->next = hes[(e + 1) % 3];
			hes[e]->prev = hes[(e + 2) % 3];
		}
	}

	for (int i = 1; i < (int)nv_faces.size(); i++)
		faces.push_back(nv_faces[i]);

	return true;
}

void myMesh::triangulate()
{
	int nb_faces = faces.size();
	for (int i=0; i<nb_faces;i++)
		triangulate(faces[i]);
}

