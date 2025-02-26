#ifndef __OOC_INTERFACE_HPP
#define __OOC_INTERFACE_HPP

#include <map>
#include <utility>

#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_io.h>
#include <geogram/delaunay/delaunay.h>

#include "../core.hpp"

namespace ooc {

    typedef struct
    {
        geogram::vec3 v;
        geogram::vec3 a;
        geogram::vec3 b;
    } InterfacePlane;

    class Interface
    {
    public:
        Interface(const InterfacePlane plane);

        void AddConstraints(geogram::Mesh& mesh);
        void Triangulate();
        geogram::Mesh* Triangulation();

    private:
        geogram::Mesh _constraints;
        geogram::Mesh _triangulation;

        InterfacePlane _plane;
        std::map<std::pair<double, double>, GEO::index_t> _indices;
        std::vector<std::pair<GEO::index_t, GEO::index_t>> _edges;
    };

#ifndef NDEBUG
    void export_delaunay(const std::string filename, geogram::Mesh& mesh, int dimension = 3);
#endif // NDEBUG
}

#endif // __OOC_INTERFACE_HPP
