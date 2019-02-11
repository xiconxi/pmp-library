//=============================================================================
// Copyright (C) 2011-2019 The pmp-library developers
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//=============================================================================

#include <pmp/algorithms/DifferentialGeometry.h>

#include <limits>
#include <cmath>
#include <cfloat>

//=============================================================================

namespace pmp {

//=============================================================================

Scalar triangle_area(const Point& p0, const Point& p1, const Point& p2)
{
    return Scalar(0.5) * norm(cross(p1 - p0, p2 - p0));
}

//-----------------------------------------------------------------------------

Scalar triangle_area(const SurfaceMesh& mesh, SurfaceMesh::Face f)
{
    assert(mesh.valence(f) == 3);

    auto fv = mesh.vertices(f);
    const auto& p0 = mesh.position(*fv);
    const auto& p1 = mesh.position(*(++fv));
    const auto& p2 = mesh.position(*(++fv));

    return triangle_area(p0, p1, p2);
}

//-----------------------------------------------------------------------------

double cotan_weight(const SurfaceMesh& mesh, SurfaceMesh::Edge e)
{
    double weight = 0.0;

    const SurfaceMesh::Halfedge h0 = mesh.halfedge(e, 0);
    const SurfaceMesh::Halfedge h1 = mesh.halfedge(e, 1);

    const dvec3 p0 = (dvec3)mesh.position(mesh.to_vertex(h0));
    const dvec3 p1 = (dvec3)mesh.position(mesh.to_vertex(h1));

    if (!mesh.is_boundary(h0))
    {
        const dvec3 p2 =
            (dvec3)mesh.position(mesh.to_vertex(mesh.next_halfedge(h0)));
        const dvec3 d0 = p0 - p2;
        const dvec3 d1 = p1 - p2;
        const double area = norm(cross(d0, d1));
        if (area > std::numeric_limits<double>::min())
        {
            const double cot = dot(d0, d1) / area;
            weight += clamp_cot(cot);
        }
    }

    if (!mesh.is_boundary(h1))
    {
        const dvec3 p2 =
            (dvec3)mesh.position(mesh.to_vertex(mesh.next_halfedge(h1)));
        const dvec3 d0 = p0 - p2;
        const dvec3 d1 = p1 - p2;
        const double area = norm(cross(d0, d1));
        if (area > std::numeric_limits<double>::min())
        {
            const double cot = dot(d0, d1) / area;
            weight += clamp_cot(cot);
        }
    }

    assert(!std::isnan(weight));
    assert(!std::isinf(weight));

    return weight;
}

//-----------------------------------------------------------------------------

double voronoi_area(const SurfaceMesh& mesh, SurfaceMesh::Vertex v)
{
    double area(0.0);

    if (!mesh.is_isolated(v))
    {
        SurfaceMesh::Halfedge h0, h1, h2;
        dvec3 p, q, r, pq, qr, pr;
        double dotp, dotq, dotr, triArea;
        double cotq, cotr;

        for (auto h : mesh.halfedges(v))
        {
            h0 = h;
            h1 = mesh.next_halfedge(h0);
            h2 = mesh.next_halfedge(h1);

            if (mesh.is_boundary(h0))
                continue;

            // three vertex positions
            p = (dvec3)mesh.position(mesh.to_vertex(h2));
            q = (dvec3)mesh.position(mesh.to_vertex(h0));
            r = (dvec3)mesh.position(mesh.to_vertex(h1));

            // edge vectors
            (pq = q) -= p;
            (qr = r) -= q;
            (pr = r) -= p;

            // compute and check triangle area
            triArea = norm(cross(pq, pr));
            if (triArea <= std::numeric_limits<double>::min())
                continue;

            // dot products for each corner (of its two emanating edge vectors)
            dotp = dot(pq, pr);
            dotq = -dot(qr, pq);
            dotr = dot(qr, pr);

            // angle at p is obtuse
            if (dotp < 0.0)
            {
                area += 0.25 * triArea;
            }

            // angle at q or r obtuse
            else if (dotq < 0.0 || dotr < 0.0)
            {
                area += 0.125 * triArea;
            }

            // no obtuse angles
            else
            {
                // cot(angle) = cos(angle)/sin(angle) = dot(A,B)/norm(cross(A,B))
                cotq = dotq / triArea;
                cotr = dotr / triArea;

                // clamp cot(angle) by clamping angle to [1,179]
                area += 0.125 * (sqrnorm(pr) * clamp_cot(cotq) +
                                 sqrnorm(pq) * clamp_cot(cotr));
            }
        }
    }

    assert(!std::isnan(area));
    assert(!std::isinf(area));

    return area;
}

//-----------------------------------------------------------------------------

double voronoi_area_barycentric(const SurfaceMesh& mesh, SurfaceMesh::Vertex v)
{
    double area(0.0);

    if (!mesh.is_isolated(v))
    {
        const Point p = mesh.position(v);
        SurfaceMesh::Halfedge h0, h1;
        Point q, r, pq, pr;

        for (auto h : mesh.halfedges(v))
        {
            if (mesh.is_boundary(h))
                continue;

            h0 = h;
            h1 = mesh.next_halfedge(h0);

            pq = mesh.position(mesh.to_vertex(h0));
            pq -= p;

            pr = mesh.position(mesh.to_vertex(h1));
            pr -= p;

            area += norm(cross(pq, pr)) / 3.0;
        }
    }

    return area;
}

//-----------------------------------------------------------------------------

Point laplace(const SurfaceMesh& mesh, SurfaceMesh::Vertex v)
{
    Point laplace(0.0, 0.0, 0.0);

    if (!mesh.is_isolated(v))
    {
        Scalar weight, sumWeights(0.0);

        for (auto h : mesh.halfedges(v))
        {
            weight = cotan_weight(mesh, mesh.edge(h));
            sumWeights += weight;
            laplace += weight * mesh.position(mesh.to_vertex(h));
        }

        laplace -= sumWeights * mesh.position(v);
        laplace /= Scalar(2.0) * voronoi_area(mesh, v);
    }

    return laplace;
}

//-----------------------------------------------------------------------------

Scalar angle_sum(const SurfaceMesh& mesh, SurfaceMesh::Vertex v)
{
    Scalar angles(0.0);

    if (!mesh.is_boundary(v))
    {
        const Point& p0 = mesh.position(v);

        for (auto h : mesh.halfedges(v))
        {
            const Point& p1 = mesh.position(mesh.to_vertex(h));
            const Point& p2 =
                mesh.position(mesh.to_vertex(mesh.ccw_rotated_halfedge(h)));

            const Point p01 = normalize(p1 - p0);
            const Point p02 = normalize(p2 - p0);

            Scalar cos_angle = clamp_cos(dot(p01, p02));

            angles += acos(cos_angle);
        }
    }

    return angles;
}

//-----------------------------------------------------------------------------

VertexCurvature vertex_curvature(const SurfaceMesh& mesh, SurfaceMesh::Vertex v)
{
    VertexCurvature c;

    const Scalar area = voronoi_area(mesh, v);
    if (area > std::numeric_limits<Scalar>::min())
    {
        c.mean = Scalar(0.5) * norm(laplace(mesh, v));
        c.gauss = (2.0 * M_PI - angle_sum(mesh, v)) / area;

        const Scalar s = sqrt(std::max(Scalar(0.0), c.mean * c.mean - c.gauss));
        c.min = c.mean - s;
        c.max = c.mean + s;

        assert(!std::isnan(c.mean));
        assert(!std::isnan(c.gauss));
        assert(!std::isinf(c.mean));
        assert(!std::isinf(c.gauss));

        assert(c.min <= c.mean);
        assert(c.mean <= c.max);
    }

    return c;
}

//=============================================================================
} // namespace pmp
//=============================================================================
