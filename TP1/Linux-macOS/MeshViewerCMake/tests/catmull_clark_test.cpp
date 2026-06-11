#include "myMesh.h"

#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <utility>

namespace {

bool validateTopology(const myMesh &mesh, bool require_closed)
{
    for (myHalfedge *halfedge : mesh.halfedges) {
        if (halfedge == NULL || halfedge->source == NULL ||
            halfedge->adjacent_face == NULL || halfedge->next == NULL ||
            halfedge->prev == NULL || halfedge->next->prev != halfedge ||
            halfedge->prev->next != halfedge)
            return false;
        if (require_closed && halfedge->twin == NULL)
            return false;
        if (halfedge->twin != NULL && halfedge->twin->twin != halfedge)
            return false;
    }
    for (myFace *face : mesh.faces)
        if (face == NULL || face->adjacent_halfedge == NULL ||
            face->adjacent_halfedge->adjacent_face != face)
            return false;
    for (myVertex *vertex : mesh.vertices)
        if (vertex == NULL || vertex->point == NULL || vertex->originof == NULL ||
            vertex->originof->source != vertex)
            return false;
    return true;
}

size_t uniqueEdgeCount(const myMesh &mesh)
{
    std::map<myVertex *, int> ids;
    for (int i = 0; i < (int)mesh.vertices.size(); i++) ids[mesh.vertices[i]] = i;

    std::set<std::pair<int, int> > edges;
    for (myHalfedge *halfedge : mesh.halfedges) {
        int a = ids[halfedge->source];
        int b = ids[halfedge->next->source];
        if (a > b) std::swap(a, b);
        edges.insert(std::make_pair(a, b));
    }
    return edges.size();
}

bool testCube(const char *filename)
{
    myMesh mesh;
    if (!mesh.readFile(filename)) return false;

    mesh.subdivisionCatmullClark();
    if (mesh.vertices.size() != 26 || mesh.faces.size() != 24 ||
        mesh.halfedges.size() != 96 || !validateTopology(mesh, true))
        return false;

    const double expected = 5.0 / 18.0;
    for (int i = 0; i < 8; i++) {
        myPoint3D &point = *mesh.vertices[i]->point;
        if (std::fabs(std::fabs(point.X) - expected) > 1e-9 ||
            std::fabs(std::fabs(point.Y) - expected) > 1e-9 ||
            std::fabs(std::fabs(point.Z) - expected) > 1e-9)
            return false;
    }
    return true;
}

bool testOpenSurface(const char *filename)
{
    myMesh mesh;
    if (!mesh.readFile(filename)) return false;

    size_t old_vertices = mesh.vertices.size();
    size_t old_faces = mesh.faces.size();
    size_t old_edges = uniqueEdgeCount(mesh);
    size_t old_halfedges = mesh.halfedges.size();

    mesh.subdivisionCatmullClark();
    return mesh.vertices.size() == old_vertices + old_edges + old_faces &&
           mesh.faces.size() == old_halfedges &&
           mesh.halfedges.size() == 4 * old_halfedges &&
           validateTopology(mesh, false);
}

} // namespace

int main(int argc, char **argv)
{
    if (argc != 3) {
        std::cerr << "Usage: CatmullClarkTest cube.obj surface_revolution.obj\n";
        return 2;
    }
    if (!testCube(argv[1])) {
        std::cerr << "Catmull-Clark cube test failed.\n";
        return 1;
    }
    if (!testOpenSurface(argv[2])) {
        std::cerr << "Catmull-Clark open-surface test failed.\n";
        return 1;
    }
    std::cout << "Catmull-Clark tests passed.\n";
    return 0;
}
