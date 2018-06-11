#include "voronoi.h"

using namespace Math;

Voronoi2D::Voronoi2D()
{
  //delaunay.set_vertices(&pc);
}

Voronoi2D::~Voronoi2D() {}

void Voronoi2D::reset()
{
  pc.clear();
  delaunay.reset();
}

void Voronoi2D::add_point(const Float2 &p)
{
  pc.push_back(p);
}

Float2 Voronoi2D::get_point(const int i) const
{
  return pc[i];
}

unsigned int Voronoi2D::get_num_points() const
{
  return (unsigned int)pc.size();
}

void Voronoi2D::triangulate()
{
  if(pc.size() <= 0) { return; }
  delaunay.copy_vertices(pc);
  delaunay.generate_delaunay_triangulation();
  //delaunay.generate_convex_hull();
}

Triangulation2D *Voronoi2D::get_triangulation()
{
  return &delaunay;
}

Voronoi3D::Voronoi3D()
{
  delaunay.set_vertices(&pc);
}

Voronoi3D::~Voronoi3D() {}

void Voronoi3D::reset()
{
  pc.clear();
  delaunay.reset();
}

void Voronoi3D::add_point(const Float3 &p)
{
  pc.push_back(p);
}

Float3 Voronoi3D::get_point(const int i) const
{
  return pc[i];
}

unsigned int Voronoi3D::get_num_points() const
{
  return (unsigned int)pc.size();
}

void Voronoi3D::triangulate()
{
  if(pc.size() <= 0) { return; }
  delaunay.set_vertices(&pc);
  delaunay.generate_delaunay_triangulation();
  //delaunay.generate_convex_hull();
}

Triangulation3D *Voronoi3D::get_triangulation()
{
  return &delaunay;
}
