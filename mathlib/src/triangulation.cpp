#if defined(__USE_BOOST__)
#include <boost/polygon/voronoi.hpp>
using boost::polygon::voronoi_builder;
using boost::polygon::voronoi_diagram;
#endif //__USE_BOOST__


#include <assert.h>
#include <iostream>
#include "triangulation.h"

using namespace Math;
using namespace std;

Triangulation2D::Triangulation2D() {  }

Triangulation2D::~Triangulation2D() {}

void Triangulation2D::reset()
{
  triangles.clear();
  edges.clear();
}

void Triangulation2D::copy_vertices(std::vector<Float2> &verts)
{
  vertices.clear();
  vertices.insert(vertices.end(), verts.begin(), verts.end());
}

bool sort_compare_x(Float2 a, Float2 b)
{
  //if(a[0] == b[0]) { return a[1] < b[1]; }
  return a[0] < b[0];
}

bool sort_compare_y(Float2 a, Float2 b)
{
  return a[1] < b[1];
}

void Triangulation2D::generate_delaunay_triangulation()
{
  //cout<<"calculating delaunay triangulation..."<<endl;
  triangles.clear();

  //first, sort the vertices by x-coord, w/ y-coord for tie breakers
  //std::sort(vertices->begin(), vertices->end(), sort_compare_x);
  //delaunay_divide_and_conquer(0, 0, vertices->size());

  //delaunay_cgal();
#if defined(__USE_BOOST__)
  if(vertices.size() >= 3)
  {
    delaunay_boost();
  }
#endif //__USE_BOOST__
}

#if defined(__USE_BOOST__)
typedef boost::polygon::point_data<int32_t> VPoint;
void Triangulation2D::delaunay_boost()
{
  edges.clear();
  voronoi_diagram<double> vd;

  std::vector<VPoint> points;
  for(uint32_t i = 0; i < vertices.size(); i++)
  {
    VPoint vp((int)(vertices[i][0] * 1000000.0f), (int)(vertices[i][1] * 1000000.0f));
    points.push_back(vp);
  }

  construct_voronoi(points.begin(), points.end(), &vd);

  voronoi_diagram<double>::const_cell_iterator cit = vd.cells().begin();
  for(; cit != vd.cells().end(); ++cit)
  {
    const voronoi_diagram<double>::cell_type& cell = *cit;
    const voronoi_diagram<double>::edge_type* edge = cell.incident_edge();
    do
    {
      if(edge->is_primary())
      {
        const voronoi_diagram<double>::cell_type *twin_cell = edge->twin()->cell();

        Edge2D delaunay_edge;
        delaunay_edge.vidx[0] = (*cit).source_index();
        delaunay_edge.vidx[1] = twin_cell->source_index();
        edges.push_back(delaunay_edge);
      }

      edge = edge->next();
    } while (edge != cell.incident_edge());
  }
}
#endif //__USE_BOOST__

#if defined(__USE_CGAL2D__)
void Triangulation2D::delaunay_cgal()
{

}
#endif //__USE_CGAL__

void Triangulation2D::delaunay_merge(int start_a, int end_a, int start_b, int end_b)
{
  Triangle2D t;
  t.vidx[0] = end_a - 1;
  t.vidx[1] = end_a;
  t.vidx[2] = start_b;

  triangles.push_back(t);
}

void Triangulation2D::delaunay_divide_and_conquer(int axis, int start_idx, int end_idx)
{
  std::vector<Float2>::iterator start = vertices.begin() + start_idx;
  std::vector<Float2>::iterator end = vertices.begin() + end_idx;
  if(axis == 0)
  {
    std::sort(start, end, sort_compare_x);
  }
  else
  {
    std::sort(start, end, sort_compare_y);
  }

  //find the mean vertex
  int num_verts = (end_idx - start_idx);
  int mean_idx = num_verts / 2;
  Float2 mean_vertex = vertices[start_idx + mean_idx];

  cout<<"num verts: "<<num_verts<<endl;
  cout<<"mean index: "<<mean_idx<<endl;
  cout<<"mean vertex: "<<mean_vertex<<endl;

  if(num_verts >= 4)
  {
    delaunay_divide_and_conquer(axis, start_idx, start_idx + mean_idx);
    delaunay_divide_and_conquer(axis, start_idx + mean_idx, end_idx);
    delaunay_merge(start_idx, start_idx + mean_idx, start_idx + mean_idx, end_idx);
  }
  else if (num_verts == 3)
  {
    Triangle2D t;
    t.vidx[0] = start_idx;
    t.vidx[1] = start_idx + 1;
    t.vidx[2] = start_idx + 2;
    triangles.push_back(t);
  }
}

struct CompareAngle {
  inline bool operator()(int bi, int ai)
  {
    Float2 a = (*vertex_list)[ai];
    Float2 b = (*vertex_list)[bi];

    Float2 va = a - root;
    Float2 vb = b - root;
    Float2 ref = root + Float2(-1.0f, 0.0f);

    double detb = ref[0] * vb[1] - ref[1] * vb[0];
    if(detb == 0 && vb[0] * ref[0] + vb[1] * ref[1] >= 0) return false;

    double deta = ref[0] * va[1] - ref[1] * va[0];
    if(deta == 0 && va[0] * ref[0] + va[1] * ref[1] >= 0) return true;

    if(deta * detb >= 0) { return ((va[0] * vb[1] - va[1] * vb[0]) > 0); }

    return deta > 0.0f;
  }

  Float2 root;
  std::vector<Float2> *vertex_list;
};

void Triangulation2D::generate_convex_hull()
{
  unsigned int num_verts = (unsigned int)vertices.size();
  if(num_verts < 3) { return; }
  edges.clear();

  //
  // Graham Scan Algorithm
  //
  // 1. original vertex list must not change order!!!
  //    instead, we can use a list of indices into the original list?
  //

  std::vector<int> index_list;

  //first, find the vertex with the smallest y coordinate
  int min_y_idx = -1;
  Float2 min_vert;
  for(unsigned int i = 0; i < vertices.size(); i++)
  {
    index_list.push_back(i);
    Float2 v = vertices[i];
    if(min_y_idx < 0 || v[1] < min_vert[1] || (v[1] == min_vert[1] && v[0] < min_vert[0]))
    {
      min_y_idx = i;
      min_vert = v;
    }
  }

  //put that min_y vertex in the first element
  int tmp_idx = index_list[0];
  index_list[0] = index_list[min_y_idx];
  index_list[min_y_idx] = tmp_idx;


  // next sort the vertex list according to the angle betwixt each point
  // and min_y (polar angle)
  CompareAngle ca;
  ca.root = vertices[min_y_idx];
  ca.vertex_list = &vertices;
  //std::sort(vertices->begin(), vertices->end(), ca);
  std::sort(index_list.begin(), index_list.end(), ca);

  std::vector<int> convex_hull;
  for(unsigned int i = 0; i < index_list.size(); i++)
  {
    Float2 p = vertices[index_list[i]];
    while(convex_hull.size() > 2 && ccw(vertices[convex_hull[i - 2]],
                                        vertices[convex_hull[i - 1]], p) <= 0)
    {
      convex_hull.pop_back();
    }
    convex_hull.push_back(i);
  }

  /*
  //next, we visit each vertex in sequence and compare it to the previous 2
  int m = 0;
  //std::vector<int>::iterator it;
  for(int i = 2; i < num_verts; i++)
  {
    //find the next valid point on the convex hull
    Float2 p[3];
    p[0] = (*vertices)[index_list[m - 1]];
    p[1] = (*vertices)[index_list[m]];
    p[2] = (*vertices)[index_list[i]];

    //calculate z-coord of cross product
    float det = ccw(p[0], p[1], p[2]);

    while(det <= 0)
    {
      if(m > 1) { m--; }
      else if(i == num_verts)
      {
        break;
      }
      else
      {
        i++;
      }
    }

    m++;
    tmp_idx = index_list[i];// *it;
    index_list[i] = index_list[m];
    index_list[m] = tmp_idx;
  }
  */

  //push all the indices into the edge list
  for(unsigned int i = 0; i < convex_hull.size() - 1; i++)
  {
    Edge2D new_edge;
    new_edge.vidx[0] = convex_hull[i];
    new_edge.vidx[1] = convex_hull[i + 1];
    edges.push_back(new_edge);
  }
}

std::vector<Triangle2D> *Triangulation2D::get_triangles()
{
  return &triangles;
}

std::vector<Edge2D> *Triangulation2D::get_edges()
{
  return &edges;
}
