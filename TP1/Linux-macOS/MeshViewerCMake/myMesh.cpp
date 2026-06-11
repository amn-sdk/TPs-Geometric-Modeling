#include "myMesh.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <utility>
#include <array>
#include <algorithm>
#include <cmath>
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

	for (myVertex *vertex : vertices)
		if (vertex != NULL && vertex->normal != NULL)
			vertex->normal->clear();

	for (myFace *face : faces) {
		if (face == NULL || face->normal == NULL || face->adjacent_halfedge == NULL)
			continue;
		myHalfedge *first = face->adjacent_halfedge;
		myHalfedge *halfedge = first;
		int guard = 0;
		do {
			if (halfedge == NULL || halfedge->source == NULL ||
				halfedge->source->normal == NULL)
				break;
			*halfedge->source->normal += *face->normal;
			halfedge = halfedge->next;
			guard++;
		} while (halfedge != first && guard <= (int)halfedges.size());
	}

	for (myVertex *vertex : vertices)
		if (vertex != NULL && vertex->normal != NULL && vertex->normal->length() > 1e-12)
			vertex->normal->normalize();
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
	if (vertices.empty() || faces.empty()) return;

	struct FaceData {
		vector<int> vertices;
		myPoint3D point;
	};
	struct EdgeData {
		int v0 = -1;
		int v1 = -1;
		vector<int> faces;
		myPoint3D midpoint;
		myPoint3D point;
		int new_vertex = -1;
	};

	map<myVertex *, int> vertex_ids;
	for (int i = 0; i < (int)vertices.size(); i++)
		vertex_ids[vertices[i]] = i;

	vector<FaceData> old_faces;
	map<pair<int, int>, EdgeData> edges;
	vector<vector<int> > incident_faces(vertices.size());
	vector<vector<pair<int, int> > > incident_edges(vertices.size());

	for (int face_id = 0; face_id < (int)faces.size(); face_id++) {
		myFace *face = faces[face_id];
		if (face == NULL || face->adjacent_halfedge == NULL) {
			cerr << "Catmull-Clark: face invalide, subdivision annulee.\n";
			return;
		}

		FaceData data;
		myHalfedge *first = face->adjacent_halfedge;
		myHalfedge *halfedge = first;
		int guard = 0;
		do {
			if (halfedge == NULL || halfedge->source == NULL ||
				vertex_ids.find(halfedge->source) == vertex_ids.end()) {
				cerr << "Catmull-Clark: contour de face invalide, subdivision annulee.\n";
				return;
			}
			data.vertices.push_back(vertex_ids[halfedge->source]);
			halfedge = halfedge->next;
			guard++;
		} while (halfedge != first && guard <= (int)halfedges.size());

		if (halfedge != first || data.vertices.size() < 3) {
			cerr << "Catmull-Clark: contour de face non ferme, subdivision annulee.\n";
			return;
		}

		for (int vertex_id : data.vertices) {
			data.point.X += vertices[vertex_id]->point->X;
			data.point.Y += vertices[vertex_id]->point->Y;
			data.point.Z += vertices[vertex_id]->point->Z;
			incident_faces[vertex_id].push_back(face_id);
		}
		double face_size = (double)data.vertices.size();
		data.point.X /= face_size;
		data.point.Y /= face_size;
		data.point.Z /= face_size;

		for (int i = 0; i < (int)data.vertices.size(); i++) {
			int a = data.vertices[i];
			int b = data.vertices[(i + 1) % data.vertices.size()];
			pair<int, int> key = minmax(a, b);
			map<pair<int, int>, EdgeData>::iterator it = edges.find(key);
			if (it == edges.end()) {
				EdgeData edge;
				edge.v0 = key.first;
				edge.v1 = key.second;
				edge.midpoint.X = 0.5 * (vertices[edge.v0]->point->X + vertices[edge.v1]->point->X);
				edge.midpoint.Y = 0.5 * (vertices[edge.v0]->point->Y + vertices[edge.v1]->point->Y);
				edge.midpoint.Z = 0.5 * (vertices[edge.v0]->point->Z + vertices[edge.v1]->point->Z);
				edges[key] = edge;
				incident_edges[a].push_back(key);
				incident_edges[b].push_back(key);
			}
			edges[key].faces.push_back(face_id);
		}

		old_faces.push_back(data);
	}

	for (map<pair<int, int>, EdgeData>::iterator it = edges.begin(); it != edges.end(); ++it) {
		EdgeData &edge = it->second;
		if (edge.faces.size() > 2) {
			cerr << "Catmull-Clark: arete non-manifold detectee, subdivision annulee.\n";
			return;
		}
		if (edge.faces.size() == 2) {
			myPoint3D &f0 = old_faces[edge.faces[0]].point;
			myPoint3D &f1 = old_faces[edge.faces[1]].point;
			edge.point.X = 0.25 * (vertices[edge.v0]->point->X + vertices[edge.v1]->point->X + f0.X + f1.X);
			edge.point.Y = 0.25 * (vertices[edge.v0]->point->Y + vertices[edge.v1]->point->Y + f0.Y + f1.Y);
			edge.point.Z = 0.25 * (vertices[edge.v0]->point->Z + vertices[edge.v1]->point->Z + f0.Z + f1.Z);
		} else {
			edge.point = edge.midpoint;
		}
	}

	vector<myPoint3D> repositioned(vertices.size());
	for (int vertex_id = 0; vertex_id < (int)vertices.size(); vertex_id++) {
		myPoint3D &p = *vertices[vertex_id]->point;
		vector<int> boundary_neighbors;
		for (const pair<int, int> &key : incident_edges[vertex_id]) {
			EdgeData &edge = edges[key];
			if (edge.faces.size() == 1)
				boundary_neighbors.push_back(edge.v0 == vertex_id ? edge.v1 : edge.v0);
		}

		if (boundary_neighbors.size() == 2) {
			myPoint3D &p0 = *vertices[boundary_neighbors[0]]->point;
			myPoint3D &p1 = *vertices[boundary_neighbors[1]]->point;
			repositioned[vertex_id].X = 0.75 * p.X + 0.125 * (p0.X + p1.X);
			repositioned[vertex_id].Y = 0.75 * p.Y + 0.125 * (p0.Y + p1.Y);
			repositioned[vertex_id].Z = 0.75 * p.Z + 0.125 * (p0.Z + p1.Z);
		} else if (!boundary_neighbors.empty() || incident_edges[vertex_id].empty()) {
			// Corners and isolated vertices are kept fixed.
			repositioned[vertex_id] = p;
		} else {
			double face_x = 0.0, face_y = 0.0, face_z = 0.0;
			for (int face_id : incident_faces[vertex_id]) {
				face_x += old_faces[face_id].point.X;
				face_y += old_faces[face_id].point.Y;
				face_z += old_faces[face_id].point.Z;
			}
			double face_count = (double)incident_faces[vertex_id].size();
			face_x /= face_count;
			face_y /= face_count;
			face_z /= face_count;

			double edge_x = 0.0, edge_y = 0.0, edge_z = 0.0;
			for (const pair<int, int> &key : incident_edges[vertex_id]) {
				edge_x += edges[key].midpoint.X;
				edge_y += edges[key].midpoint.Y;
				edge_z += edges[key].midpoint.Z;
			}
			double valence = (double)incident_edges[vertex_id].size();
			edge_x /= valence;
			edge_y /= valence;
			edge_z /= valence;

			repositioned[vertex_id].X = (face_x + 2.0 * edge_x + (valence - 3.0) * p.X) / valence;
			repositioned[vertex_id].Y = (face_y + 2.0 * edge_y + (valence - 3.0) * p.Y) / valence;
			repositioned[vertex_id].Z = (face_z + 2.0 * edge_z + (valence - 3.0) * p.Z) / valence;
		}
	}

	vector<myVertex *> new_vertices;
	auto add_vertex = [&](const myPoint3D &point) -> int {
		myVertex *vertex = new myVertex();
		vertex->point = new myPoint3D(point.X, point.Y, point.Z);
		new_vertices.push_back(vertex);
		return (int)new_vertices.size() - 1;
	};

	vector<int> vertex_points(vertices.size());
	for (int i = 0; i < (int)repositioned.size(); i++)
		vertex_points[i] = add_vertex(repositioned[i]);
	for (map<pair<int, int>, EdgeData>::iterator it = edges.begin(); it != edges.end(); ++it)
		it->second.new_vertex = add_vertex(it->second.point);
	vector<int> face_points(old_faces.size());
	for (int i = 0; i < (int)old_faces.size(); i++)
		face_points[i] = add_vertex(old_faces[i].point);

	vector<array<int, 4> > quads;
	for (int face_id = 0; face_id < (int)old_faces.size(); face_id++) {
		vector<int> &loop = old_faces[face_id].vertices;
		for (int i = 0; i < (int)loop.size(); i++) {
			int previous = loop[(i - 1 + loop.size()) % loop.size()];
			int current = loop[i];
			int next = loop[(i + 1) % loop.size()];
			quads.push_back({
				vertex_points[current],
				edges[minmax(current, next)].new_vertex,
				face_points[face_id],
				edges[minmax(previous, current)].new_vertex
			});
		}
	}

	vector<myHalfedge *> new_halfedges;
	vector<myFace *> new_faces;
	map<pair<int, int>, myHalfedge *> directed_edges;
	for (const array<int, 4> &quad : quads) {
		myFace *face = new myFace();
		array<myHalfedge *, 4> face_edges;
		for (int i = 0; i < 4; i++) face_edges[i] = new myHalfedge();
		face->adjacent_halfedge = face_edges[0];

		for (int i = 0; i < 4; i++) {
			int next = (i + 1) % 4;
			int previous = (i + 3) % 4;
			myHalfedge *halfedge = face_edges[i];
			halfedge->source = new_vertices[quad[i]];
			halfedge->adjacent_face = face;
			halfedge->next = face_edges[next];
			halfedge->prev = face_edges[previous];
			if (halfedge->source->originof == NULL)
				halfedge->source->originof = halfedge;

			pair<int, int> reverse = make_pair(quad[next], quad[i]);
			map<pair<int, int>, myHalfedge *>::iterator twin = directed_edges.find(reverse);
			if (twin != directed_edges.end()) {
				halfedge->twin = twin->second;
				twin->second->twin = halfedge;
			}
			directed_edges[make_pair(quad[i], quad[next])] = halfedge;
			new_halfedges.push_back(halfedge);
		}
		new_faces.push_back(face);
	}

	for (myHalfedge *halfedge : halfedges) delete halfedge;
	for (myFace *face : faces) delete face;
	for (myVertex *vertex : vertices) {
		delete vertex->point;
		delete vertex;
	}
	vertices.swap(new_vertices);
	halfedges.swap(new_halfedges);
	faces.swap(new_faces);

	cout << "Catmull-Clark: " << vertices.size() << " sommets, "
		 << faces.size() << " faces.\n";
	checkMesh();
}

struct Quadric {
	double m[4][4];
	Quadric() {
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++) m[i][j] = 0.0;
	}
	void addPlane(double a, double b, double c, double d) {
		double p[4] = {a, b, c, d};
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				m[i][j] += p[i] * p[j];
	}
	void add(const Quadric &q) {
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				m[i][j] += q.m[i][j];
	}
	double eval(double x, double y, double z) const {
		double v[4] = {x, y, z, 1.0};
		double s = 0.0;
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				s += v[i] * m[i][j] * v[j];
		return s;
	}
};

static bool facePlane(myFace *f, double &a, double &b, double &c, double &d)
{
	if (f == NULL || f->adjacent_halfedge == NULL) return false;
	myHalfedge *h0 = f->adjacent_halfedge;
	myHalfedge *h1 = h0->next;
	myHalfedge *h2 = (h1 == NULL) ? NULL : h1->next;
	if (h2 == NULL || h0->source == NULL || h1->source == NULL || h2->source == NULL)
		return false;
	double x0 = h0->source->point->X, y0 = h0->source->point->Y, z0 = h0->source->point->Z;
	double x1 = h1->source->point->X, y1 = h1->source->point->Y, z1 = h1->source->point->Z;
	double x2 = h2->source->point->X, y2 = h2->source->point->Y, z2 = h2->source->point->Z;
	double ux = x1 - x0, uy = y1 - y0, uz = z1 - z0;
	double vx = x2 - x0, vy = y2 - y0, vz = z2 - z0;
	a = uy * vz - uz * vy;
	b = uz * vx - ux * vz;
	c = ux * vy - uy * vx;
	double len = sqrt(a * a + b * b + c * c);
	if (len < 1e-12) return false;
	a /= len; b /= len; c /= len;
	d = -(a * x0 + b * y0 + c * z0);
	return true;
}

static double det3(double m[3][3])
{
	return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
	     - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
	     + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

static void qemOptimalPoint(const Quadric &Q, myVertex *v1, myVertex *v2,
	double &ox, double &oy, double &oz, double &cost)
{
	double A[3][3] = {
		{Q.m[0][0], Q.m[0][1], Q.m[0][2]},
		{Q.m[1][0], Q.m[1][1], Q.m[1][2]},
		{Q.m[2][0], Q.m[2][1], Q.m[2][2]}
	};
	double b[3] = {-Q.m[0][3], -Q.m[1][3], -Q.m[2][3]};
	double detA = det3(A);

	double candidates[4][3];
	int nc = 0;

	if (fabs(detA) > 1e-12) {
		double Ax[3][3] = {{b[0], A[0][1], A[0][2]}, {b[1], A[1][1], A[1][2]}, {b[2], A[2][1], A[2][2]}};
		double Ay[3][3] = {{A[0][0], b[0], A[0][2]}, {A[1][0], b[1], A[1][2]}, {A[2][0], b[2], A[2][2]}};
		double Az[3][3] = {{A[0][0], A[0][1], b[0]}, {A[1][0], A[1][1], b[1]}, {A[2][0], A[2][1], b[2]}};
		candidates[nc][0] = det3(Ax) / detA;
		candidates[nc][1] = det3(Ay) / detA;
		candidates[nc][2] = det3(Az) / detA;
		nc++;
	}

	candidates[nc][0] = 0.5 * (v1->point->X + v2->point->X);
	candidates[nc][1] = 0.5 * (v1->point->Y + v2->point->Y);
	candidates[nc][2] = 0.5 * (v1->point->Z + v2->point->Z);
	nc++;

	candidates[nc][0] = v1->point->X;
	candidates[nc][1] = v1->point->Y;
	candidates[nc][2] = v1->point->Z;
	nc++;

	candidates[nc][0] = v2->point->X;
	candidates[nc][1] = v2->point->Y;
	candidates[nc][2] = v2->point->Z;
	nc++;

	cost = 1e100;
	for (int i = 0; i < nc; i++) {
		double c = Q.eval(candidates[i][0], candidates[i][1], candidates[i][2]);
		if (c < cost) {
			cost = c;
			ox = candidates[i][0];
			oy = candidates[i][1];
			oz = candidates[i][2];
		}
	}
}

void myMesh::simplify()
{
	if (halfedges.size() == 0 || vertices.size() < 2) return;

	map<myVertex *, Quadric> Qv;
	for (int i = 0; i < (int)vertices.size(); i++)
		Qv[vertices[i]] = Quadric();

	for (int i = 0; i < (int)faces.size(); i++) {
		myFace *f = faces[i];
		double a, b, c, d;
		if (!facePlane(f, a, b, c, d)) continue;
		myHalfedge *h0 = f->adjacent_halfedge;
		if (h0 == NULL) continue;
		myHalfedge *h = h0;
		do {
			if (h->source != NULL) Qv[h->source].addPlane(a, b, c, d);
			h = h->next;
		} while (h != NULL && h != h0);
	}

	myHalfedge *e_min = NULL;
	double cost_min = 1e100;
	double optX = 0.0, optY = 0.0, optZ = 0.0;
	for (int i = 0; i < (int)halfedges.size(); i++) {
		myHalfedge *e = halfedges[i];
		if (e == NULL || e->twin == NULL) continue;
		if (e->source == NULL || e->twin->source == NULL) continue;
		if (e > e->twin) continue;
		myVertex *v1 = e->source;
		myVertex *v2 = e->twin->source;
		Quadric Qedge = Qv[v1];
		Qedge.add(Qv[v2]);
		double cx, cy, cz, cost;
		qemOptimalPoint(Qedge, v1, v2, cx, cy, cz, cost);
		if (cost < cost_min) {
			cost_min = cost;
			e_min = e;
			optX = cx; optY = cy; optZ = cz;
		}
	}
	if (e_min == NULL) return;
	myVertex *v1 = e_min->source;
	myVertex *v2 = e_min->twin->source;
	if (v1 == NULL || v2 == NULL || v1 == v2) return;

	unsigned int oldVertices = vertices.size();
	unsigned int oldFaces = faces.size();

	v1->point->X = optX;
	v1->point->Y = optY;
	v1->point->Z = optZ;

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
			if (vertices[i]->point != NULL) delete vertices[i]->point;
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

	cout << "Simplification QEM: " << oldVertices << " -> " << vertices.size()
		 << " sommets, " << oldFaces << " -> " << faces.size()
		 << " faces (cout=" << cost_min << ").\n";
	checkMesh();
}

void myMesh::simplify(myVertex *)
{
	simplify();
}

void myMesh::generateSurface()
{
	clear();
	name = "surface";
	vector<pair<double, double> > profil;
	profil.push_back(make_pair(0.62, 0.95));
	profil.push_back(make_pair(0.58, 0.72));
	profil.push_back(make_pair(0.48, 0.46));
	profil.push_back(make_pair(0.26, 0.20));
	profil.push_back(make_pair(0.20, 0.00));
	profil.push_back(make_pair(0.26, -0.20));
	profil.push_back(make_pair(0.52, -0.48));
	profil.push_back(make_pair(0.66, -0.74));
	profil.push_back(make_pair(0.72, -0.95));
	int n = 32;
	double pi = 3.14159265358979323846;
	vector<vector<int> > ids(profil.size(), vector<int>(n, -1));
	for (int k=0; k<(int)profil.size(); k++) {
		double r = profil[k].first;
		double y = profil[k].second;
		for (int i=0; i<n; i++) {
			double a = 2.0 * pi * (double)i/(double)n;
			myVertex *v = new myVertex();
			v->point = new myPoint3D(r * cos(a), y, r * sin(a));
			vertices.push_back(v);
			ids[k][i] = (int)vertices.size() - 1;
		}
	}
	vector<vector<int> > polys;
	for (int k = 0; k < (int)profil.size() - 1; k++) {
		for (int i =0; i<n; i++) {
			int j= (i + 1) % n;
			int a= ids[k][i];
			int b= ids[k][j];
			int c= ids[k+1][j];
			int d= ids[k+1][i];
			polys.push_back({a, b, c, d});
		}
	}
	map<pair<int, int>, myHalfedge *> twin_map;
	for (int p = 0; p < (int)polys.size(); p++) {
		vector<int> &id = polys[p];
		int m = (int)id.size();
		if (m < 3) continue;
		myFace *f = new myFace();
		vector<myHalfedge *> he(m);
		for (int i=0; i<m; i++) he[i] = new myHalfedge();
		f->adjacent_halfedge = he[0];

		for (int i=0; i<m; i++) {
			int ip = (i-1+ m) % m;
			int in = (i+1) % m;
			int s= id[i];
			int t= id[in];
			he[i]->source = vertices[s];
			he[i]->adjacent_face = f;
			he[i]->prev = he[ip];
			he[i]->next = he[in];
			if (vertices[s]->originof == NULL) vertices[s]->originof = he[i];
			pair<int, int> key = make_pair(s,t);
			pair<int, int> tkey = make_pair(t,s);
			map<pair<int, int>, myHalfedge *>::iterator it = twin_map.find(tkey);
			if (it!= twin_map.end()) {
				he[i]->twin = it->second;
				it->second->twin = he[i];
			} else {
				twin_map[key] = he[i];
			}
			halfedges.push_back(he[i]);
		}
		faces.push_back(f);
	}
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

