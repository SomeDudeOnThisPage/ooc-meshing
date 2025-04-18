#ifndef __OOC_PREDICATES_CPP
#define __OOC_PREDICATES_CPP

#include <cmath>
#include <optional>

#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/numerics/predicates.h>
#include <geogram/basic/vecg.h>

#include "core.hpp"
#include "core-interface.hpp"

// TODO: THIS IS WRONG!
#define in_range(x, p, eps) (std::abs(x - p) <= eps) || (std::abs(x + p) <= eps)

namespace incremental_meshing::predicates
{
    PURE INLINE bool vec_eq_2d(const geogram::vec3 &v0, const geogram::vec3 &v1, const incremental_meshing::AxisAlignedInterfacePlane& plane)
    {
        switch (plane.axis)
        {
            case incremental_meshing::Axis::X:
                return v0.y == v1.y && v0.z == v1.z;
            case incremental_meshing::Axis::Y:
                return v0.x == v1.x && v0.z == v1.z;
            case incremental_meshing::Axis::Z:
                return v0.x == v1.x && v0.y == v1.y;
        }

        return false;
    }

    PURE INLINE bool point_on_plane(const geogram::vec3& point, const incremental_meshing::AxisAlignedInterfacePlane& plane)
    {
        switch (plane.axis)
        {
            case incremental_meshing::Axis::X:
                return in_range(point.x, plane.extent, plane.epsilon);
            case incremental_meshing::Axis::Y:
                return in_range(point.y, plane.extent, plane.epsilon);
            case incremental_meshing::Axis::Z:
                return point.z == plane.extent; // TODO
                //return in_range(point.z, plane.extent, plane.epsilon);
        }

        return false;
    }

    PURE INLINE bool edge_on_plane(const geogram::vec3& p0, const geogram::vec3& p1, const incremental_meshing::AxisAlignedInterfacePlane& plane)
    {
        return point_on_plane(p0, plane) && point_on_plane(p1, plane);
    }

    PURE INLINE bool facet_on_plane(const geogram::index_t facet, const geogram::Mesh& mesh, const incremental_meshing::AxisAlignedInterfacePlane& plane)
    {
    #ifdef OPTION_UNROLL_LOOPS
        #pragma unroll 3
    #endif
        for (l_index lv = 0; lv < /* mesh.facets.nb_vertices(facet) */ 3; lv++)
        {
            const auto point = mesh.vertices.point(mesh.facets.vertex(facet, lv));
            if (!point_on_plane(point, plane))
            {
                return false;
            }
        }

        return true;
    }

    PURE INLINE bool cell_on_plane(const geogram::index_t cell, const geogram::Mesh& mesh, const incremental_meshing::AxisAlignedInterfacePlane& plane)
    {
        unsigned char points_on_plane = 0;
    #ifdef OPTION_UNROLL_LOOPS
        #pragma unroll 6
    #endif
        for (unsigned char i = 0; i < /*mesh.cells.nb_vertices(cell)*/ 6; i++)
        {
            if (point_on_plane(mesh.vertices.point(mesh.cells.vertex(cell, i)), plane))
            {
                points_on_plane++;
            }
        }

        return points_on_plane == 3;
    }

    PURE INLINE bool cell_facet_on_plane(const geogram::index_t cell, const geogram::index_t lf, const geogram::Mesh& mesh, const incremental_meshing::AxisAlignedInterfacePlane& plane)
    {
        for (geogram::index_t lv = 0; lv < /*mesh.cells.facet_nb_vertices(cell, lf)*/ 3; lv++)
        {
            const auto point = mesh.vertices.point(mesh.cells.facet_vertex(cell, lf, lv));
            if (!point_on_plane(point, plane))
            {
                return false;
            }
        }

        return true;
    }

    // Source: geogram/mesh/mesh_AABB.cpp#175
    // TODO: this can likely be replaced with less instructions because we work in 2d...
    PURE INLINE bool point_in_tet(const geogram::Mesh& mesh, geogram::index_t t, const geogram::vec3& p)
    {
        const auto p0 = mesh.vertices.point(mesh.cells.vertex(t, 0));
        const auto p1 = mesh.vertices.point(mesh.cells.vertex(t, 1));
        const auto p2 = mesh.vertices.point(mesh.cells.vertex(t, 2));
        const auto p3 = mesh.vertices.point(mesh.cells.vertex(t, 3));

        geogram::Sign s[4];
        s[0] = geogram::PCK::orient_3d(p, p1, p2, p3);
        s[1] = geogram::PCK::orient_3d(p0, p, p2, p3);
        s[2] = geogram::PCK::orient_3d(p0, p1, p, p3);
        s[3] = geogram::PCK::orient_3d(p0, p1, p2, p);

        return (
            (s[0] >= 0 && s[1] >= 0 && s[2] >= 0 && s[3] >= 0) ||
            (s[0] <= 0 && s[1] <= 0 && s[2] <= 0 && s[3] <= 0)
        );
    }

    namespace xy
    {
        PURE INLINE bool check_lines_aabb(const geogram::vec2& p0, const geogram::vec2& p1, const geogram::vec2& p2, const geogram::vec2& p3)
        {
            return (std::min(p0.x, p1.x) <= std::max(p2.x, p3.x)) && (std::max(p0.x, p1.x) >= std::min(p2.x, p3.x))
                && (std::min(p0.y, p1.y) <= std::max(p2.y, p3.y)) && (std::max(p0.y, p1.y) >= std::min(p2.y, p3.y));
        }

        PURE INLINE bool point_in_facet(const geogram::vec2& point, const g_index facet, const geogram::Mesh& triangulation)
        {
            const auto p0 = reinterpret_cast<const geogram::vec2&>(triangulation.vertices.point(triangulation.facets.vertex(facet, 0)));
            const auto p1 = reinterpret_cast<const geogram::vec2&>(triangulation.vertices.point(triangulation.facets.vertex(facet, 1)));
            const auto p2 = reinterpret_cast<const geogram::vec2&>(triangulation.vertices.point(triangulation.facets.vertex(facet, 2)));

            geogram::Sign s[3];
            s[0] = geogram::PCK::orient_2d(p0, p1, point);
            s[1] = geogram::PCK::orient_2d(p1, p2, point);
            s[2] = geogram::PCK::orient_2d(p2, p0, point);

            bool has_neg = (s[0] < 0) || (s[1] < 0) || (s[2] < 0);
            bool has_pos = (s[0] > 0) || (s[1] > 0) || (s[2] > 0);

            return (
                (s[0] >= 0 && s[1] >= 0 && s[2] >= 0) ||
                (s[0] <= 0 && s[1] <= 0 && s[2] <= 0)
            );
        }

        PURE /* INLINE */ inline std::optional<geogram::vec2> get_line_intersection(const geogram::vec2& p0, const geogram::vec2& p1, const geogram::vec2& p2, const geogram::vec2& p3)
        {
            geogram::Sign s[4];
            s[0] = geogram::PCK::orient_2d(p0, p1, p2);
            s[1] = geogram::PCK::orient_2d(p0, p1, p3);
            s[2] = geogram::PCK::orient_2d(p2, p3, p0);
            s[3] = geogram::PCK::orient_2d(p2, p3, p1);

            if (s[0] != s[1] && s[2] != s[3])
            {
                double denom = (p0.x - p1.x) * (p2.y - p3.y) - (p0.y - p1.y) * (p2.x - p3.x);
                if (denom == 0) return std::nullopt;

                double t = ((p0.x - p2.x) * (p2.y - p3.y) - (p0.y - p2.y) * (p2.x - p3.x)) / denom;
                double u = ((p0.x - p2.x) * (p0.y - p1.y) - (p0.y - p2.y) * (p0.x - p1.x)) / denom;

                const auto eps = 2 * incremental_meshing::__DOUBLE_EPSILON;
                if ((t >= eps && t <= 1.0 - eps) && (u >= eps && u <= 1.0 - eps))
                {
                    // we don't work with real predicates, so we have to have a rounding factor in these vertices...
                    // since these are not inserted interface vertices, but created in both meshes by intersections,
                    // this SHOULD have no effect on the merging process...
                    return vec2 {
                        //p0.x + t * (p1.x - p0.x),
                        //p0.y + t * (p1.y - p0.y)

                        std::round((p0.x + t * (p1.x - p0.x)) * 1e12) / 1e12,
                        std::round((p0.y + t * (p1.y - p0.y)) * 1e12) / 1e12
                    };
                }
            }

            return std::nullopt;
        }
    }

    PURE INLINE bool lines_intersect(const geogram::vec2& p0, const geogram::vec2& p1, const geogram::vec2& p2, const geogram::vec2& p3)
    {
        geogram::Sign s[4];
        s[0] = geogram::PCK::orient_2d(p0, p1, p2);
        s[1] = geogram::PCK::orient_2d(p0, p1, p3);
        s[2] = geogram::PCK::orient_2d(p2, p3, p0);
        s[3] = geogram::PCK::orient_2d(p2, p3, p1);

        if (s[0] != s[1] && s[2] != s[3])
        {
            double denom = (p0.x - p1.x) * (p2.y - p3.y) - (p0.y - p1.y) * (p2.x - p3.x);
            if (denom == 0) return false;

            double t = ((p0.x - p2.x) * (p2.y - p3.y) - (p0.y - p2.y) * (p2.x - p3.x)) / denom;
            double u = ((p0.x - p2.x) * (p0.y - p1.y) - (p0.y - p2.y) * (p0.x - p1.x)) / denom;

            return (t > 0 && t < 1) && (u > 0 && u < 1);
        }

        return false;
    }

}

#endif // __OOC_PREDICATES_CPP
