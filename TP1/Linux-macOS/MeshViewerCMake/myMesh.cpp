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

bool myMesh::testTwins()
{
	int sans_twin = 0;
	for (int i = 0; i < (int)halfedges.size(); i++)
		if (halfedges[i]->twin == NULL) sans_twin++;
	if (sans_twin > 0)
		cout << "Warning: " << sans_twin << " demi-aretes sans twin (bord)\n";
	else
		cout << "OK twins\n";
	return (sans_twin == 0);
}

bool myMesh::testNext()
{
	bool ok = true;
	for (int i = 0; i < (int)halfedges.size(); i++) {
		myHalfedge *h = halfedges[i];
		if (h->next == NULL || h->prev == NULL) { ok = false; break; }
		if (h->next->prev != h) { ok = false; break; }
	}
	if (!ok) cout << "Erreur: next/prev incohérents\n";
	else cout << "OK next/prev\n";
	return ok;
}

bool myMesh::testFaces()
{
	bool ok = true;
	for (int i = 0; i < (int)faces.size(); i++) {
		if (faces[i]->adjacent_halfedge == NULL) { ok = false; break; }
		if (faces[i]->adjacent_halfedge->adjacent_face != faces[i]) { ok = false; break; }
	}
	if (!ok) cout << "Erreur: les faces mal connectées\n";
	else cout << "OK faces\n";
	return ok;
}

bool myMesh::testVertices()
{
	bool ok = true;
	for (int i = 0; i < (int)vertices.size(); i++) {
		if (vertices[i]->originof == NULL) { ok = false; break; }
		if (vertices[i]->originof->source != vertices[i]) { ok = false; break; }
	}
	if (!ok) cout << "Erreur: les sommets mal connectés\n";
	else cout << "OK sommets\n";
	return ok;
}

void myMesh::checkMesh()
{
	testTwins();
	testNext();
	testFaces();
	testVertices();
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
	if (halfedges.size() == 0 || vertices.size() < 2) return;
	myHalfedge *e_min = NULL;
	double d_min = 1e100;
	for (int i = 0; i < (int)halfedges.size(); i++) {
		myHalfedge *e = halfedges[i];
		if (e == NULL || e->twin == NULL) continue;
		if (e->source == NULL || e->twin->source == NULL) continue;
		if (e > e->twin) continue;
		double dx=e->source->point->X -e->twin->source->point->X;
		double dy= e->source->point->Y -e->twin->source->point->Y;
		double dz =e->source->point->Z -e->twin->source->point->Z;
		double d2 = dx*dx + dy* dy + dz*dz;
		if (d2 < d_min) {
			d_min = d2;
			e_min = e;
		}
	}
	if (e_min == NULL) return;
	myVertex *v1 = e_min->source;
	myVertex *v2 = e_min->twin->source;
	if (v1 == NULL || v2 == NULL || v1 == v2) return;
	v1->point->X = 0.5*(v1->point->X+v2->point->X);
	v1->point->Y = 0.5*(v1->point->Y+v2->point->Y);
	v1->point->Z = 0.5*(v1->point->Z+v2->point->Z);

	vector<vector<myVertex *> > polys;
	for (int i=0; i<(int)faces.size(); i++) {
		myFace *f = faces[i];
		if (f==NULL || f->adjacent_halfedge == NULL) continue;
		vector<myVertex *> loop;
		myHalfedge *h0 =f->adjacent_halfedge;
		myHalfedge *h =h0;
		int secu = 0;
		do {
			if (h == NULL || h->source == NULL) { loop.clear(); break; }
			myVertex *v = h->source;
			if (v == v2) v = v1;
			loop.push_back(v);
			h = h->next;
			secu++;
		} while (h != h0 && secu < 10000);
		if (loop.size() < 3) continue;

		vector<myVertex *> clean;
		for (int j = 0; j < (int)loop.size(); j++) {
			if (clean.size() == 0 || clean.back() != loop[j]) clean.push_back(loop[j]);
		}
		if (clean.size() >= 2 && clean.front() == clean.back()) clean.pop_back();
		if (clean.size() < 3) continue;

		bool ok = true;
		for (int j = 0; j < (int)clean.size(); j++) {
			for (int k = j + 1; k < (int)clean.size(); k++) {
				if (clean[j] == clean[k]) { ok = false; break; }
			}
			if (!ok) break;
		}
		if (!ok) continue;
		polys.push_back(clean);
	}

	for (int i = 0; i < (int)vertices.size(); i++) {
		if (vertices[i] == v2) {
			delete vertices[i];
			vertices.erase(vertices.begin() + i);
			break;
		}
	}

	for (int i = 0; i < (int)halfedges.size(); i++){
		if (halfedges[i] != NULL) delete halfedges[i];
	}
	for (int i = 0; i < (int)faces.size(); i++){
		if (faces[i] != NULL) delete faces[i];
	}
	halfedges.clear();
	faces.clear();
	for (int i = 0; i < (int)vertices.size(); i++) vertices[i]->originof = NULL;

	map<pair<myVertex *, myVertex *>, myHalfedge *> tmap;
	for (int i = 0; i < (int)polys.size(); i++) {
		vector<myVertex *> &p = polys[i];
		int m = (int)p.size();
		if (m < 3) continue;
		myFace *f = new myFace();
		vector<myHalfedge *> he(m);
		for (int j = 0; j < m; j++) he[j] = new myHalfedge();
		f->adjacent_halfedge = he[0];
		for (int j = 0; j < m; j++) {
			int jn = (j + 1) % m;
			int jp = (j - 1 + m) % m;
			he[j]->source = p[j];
			he[j]->adjacent_face = f;
			he[j]->next = he[jn];
			he[j]->prev = he[jp];
			if (p[j]->originof == NULL) p[j]->originof = he[j];
			pair<myVertex *, myVertex *> key = make_pair(p[j], p[jn]);
			pair<myVertex *, myVertex *> rkey = make_pair(p[jn], p[j]);
			map<pair<myVertex *, myVertex *>, myHalfedge *>::iterator it = tmap.find(rkey);
			if (it != tmap.end()) {
				he[j]->twin = it->second;
				it->second->twin = he[j];
			} else {
				tmap[key] = he[j];
			}
			halfedges.push_back(he[j]);
		}
		faces.push_back(f);
	}

	checkMesh();
}

void myMesh::simplify(myVertex *)
{
	simplify();
}

bool myMesh::triangulate(myFace *f)
{
	vector<myHalfedge *> bord;
	myHalfedge *h = f->adjacent_halfedge;
	do {
		if (h == NULL) return false;
		bord.push_back(h);
		h = h->next;
		if ((int)bord.size() > 1000) return false;
	} while (h != f->adjacent_halfedge);

	int n = (int)bord.size();
	if (n <= 3) return false;

	vector<myVertex *> pt(n);
	for (int i = 0; i < n; i++) pt[i] = bord[i]->source;

	double nx = 0.0, ny = 0.0, nz = 0.0;
	for (int i = 0; i < n; i++) {
		int j = (i + 1) % n;
		double x1 = pt[i]->point->X, y1 = pt[i]->point->Y, z1 = pt[i]->point->Z;
		double x2 = pt[j]->point->X, y2 = pt[j]->point->Y, z2 = pt[j]->point->Z;
		nx += (y1 - y2) * (z1 + z2);
		ny += (z1 - z2) * (x1 + x2);
		nz += (x1 - x2) * (y1 + y2);
	}

	int axe = 2;
	double ax = fabs(nx), ay = fabs(ny), az = fabs(nz);
	if (ax >= ay && ax >= az) axe = 0;
	else if (ay >= ax && ay >= az) axe = 1;

	vector<pair<double, double>> p2(n);
	for (int i = 0; i < n; i++) {
		double x = pt[i]->point->X, y = pt[i]->point->Y, z = pt[i]->point->Z;
		if (axe == 0) p2[i] = make_pair(y, z);
		else if (axe == 1) p2[i] = make_pair(x, z);
		else p2[i] = make_pair(x, y);
	}

	auto orient = [&](int a, int b, int c) -> double {
		double ax2 = p2[a].first, ay2 = p2[a].second;
		double bx2 = p2[b].first, by2 = p2[b].second;
		double cx2 = p2[c].first, cy2 = p2[c].second;
		return (bx2 - ax2) * (cy2 - ay2) - (by2 - ay2) * (cx2 - ax2);
	};

	double aire = 0.0;
	for (int i = 0; i < n; i++) {
		int j = (i + 1) % n;
		aire += p2[i].first * p2[j].second - p2[j].first * p2[i].second;
	}
	bool sens_horaire = (aire < 0.0);

	auto dedans = [&](int a, int b, int c, int p) -> bool {
		double o1 = orient(a, b, p);
		double o2 = orient(b, c, p);
		double o3 = orient(c, a, p);
		if (sens_horaire) return (o1 <= 0.0 && o2 <= 0.0 && o3 <= 0.0);
		return (o1 >= 0.0 && o2 >= 0.0 && o3 >= 0.0);
	};

	auto meme = [&](int i, int j) -> bool {
		double dx = pt[i]->point->X - pt[j]->point->X;
		double dy = pt[i]->point->Y - pt[j]->point->Y;
		double dz = pt[i]->point->Z - pt[j]->point->Z;
		return (dx * dx + dy * dy + dz * dz) < 1e-16;
	};

	vector<int> id;
	for (int i = 0; i < n; i++) id.push_back(i);
	vector<array<int, 3>> tri;

	int secu = 0;
	while ((int)id.size() > 3 && secu < 10000) {
		secu++;
		bool ok = false;
		int m = (int)id.size();
		for (int i = 0; i < m; i++) {
			int a = id[(i - 1 + m) % m];
			int b = id[i];
			int c = id[(i + 1) % m];
			if (meme(a, b) || meme(b, c) || meme(c, a)) continue;
			double o = orient(a, b, c);
			if (!sens_horaire && o <= 0.0) continue;
			if (sens_horaire && o >= 0.0) continue;
			bool bloc = false;
			for (int j = 0; j < m; j++) {
				int p = id[j];
				if (p == a || p == b || p == c) continue;
				if (meme(p, a) || meme(p, b) || meme(p, c)) continue;
				if (dedans(a, b, c, p)) {
					bloc = true;
					break;
				}
			}
			if (bloc) continue;
			tri.push_back({a, b, c});
			id.erase(id.begin() + i);
			ok = true;
			break;
		}
		if (!ok) return false;
	}

	if ((int)id.size() == 3) tri.push_back({id[0], id[1], id[2]});
	if ((int)tri.size() != n - 2) return false;

	map<pair<int, int>, myHalfedge *> bord_map;
	for (int i = 0; i < n; i++) {
		int j = (i + 1) % n;
		bord_map[make_pair(i, j)] = bord[i];
	}

	map<pair<int, int>, myHalfedge *> interne;
	vector<myFace *> face_new;
	face_new.push_back(f);
	for (int i = 1; i < (int)tri.size(); i++) face_new.push_back(new myFace());

	for (int t = 0; t < (int)tri.size(); t++) {
		int a = tri[t][0], b = tri[t][1], c = tri[t][2];
		int v[3] = {a, b, c};
		myHalfedge *e[3] = {NULL, NULL, NULL};

		for (int k = 0; k < 3; k++) {
			int s = v[k];
			int d = v[(k + 1) % 3];
			map<pair<int, int>, myHalfedge *>::iterator itb = bord_map.find(make_pair(s, d));
			if (itb != bord_map.end()) {
				e[k] = itb->second;
			} else {
				map<pair<int, int>, myHalfedge *>::iterator iti = interne.find(make_pair(s, d));
				if (iti != interne.end()) {
					e[k] = iti->second;
				} else {
					myHalfedge *hn = new myHalfedge();
					hn->source = pt[s];
					if (pt[s]->originof == NULL) pt[s]->originof = hn;
					map<pair<int, int>, myHalfedge *>::iterator itr = interne.find(make_pair(d, s));
					if (itr != interne.end()) {
						hn->twin = itr->second;
						itr->second->twin = hn;
					}
					interne[make_pair(s, d)] = hn;
					halfedges.push_back(hn);
					e[k] = hn;
				}
			}
		}

		myFace *ff = face_new[t];
		ff->adjacent_halfedge = e[0];
		for (int k = 0; k < 3; k++) {
			e[k]->adjacent_face = ff;
			e[k]->next = e[(k + 1) % 3];
			e[k]->prev = e[(k + 2) % 3];
		}
	}

	for (int i = 1; i < (int)face_new.size(); i++) faces.push_back(face_new[i]);
	return true;
}

void myMesh::triangulate()
{
	int nb_faces = faces.size();
	for (int i=0; i<nb_faces;i++)
		triangulate(faces[i]);
}

