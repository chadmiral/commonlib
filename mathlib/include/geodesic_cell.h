#ifndef __GEODESIC_CELL_H__
#define __GEODESIC_CELL_H__

#include <assert.h>
#include "math_utility.h"

template <class T>
class GeodesicCell
{
  //friend class GeodesicGrid<T>;
  template<class F> friend class GeodesicGrid;
private:
  Math::Float3 pos;                 //cartesian coordinates
  Math::Float2 uv;                  //latitude / longitude
  Math::Float2 wt;                  //winkel-tripel projection coordinates

  Math::Float3 color;               //temp for debugging
  T data;

  GeodesicCell<T> *neighbors[6];    //max number of neighbors a cell can have: 6
  Math::Float3 neighbor_centroids[6];
  //Float3 neighbor_normals[6];
  uint32_t num_neighbors;                //current number of neighbors (either 4, 5, or 6)
public:
  GeodesicCell() : data(T()) { init(); }
  ~GeodesicCell() {}

  void init()
  {
    for(uint32_t i = 0; i < 6; i++)
    {
      neighbors[i] = NULL;
    }
    num_neighbors = 0;
  }

  GeodesicCell &operator=(const GeodesicCell &r)
  {
    data = r.data;
    pos = r.pos;
    uv = r.uv;
    color = r.color;

    for(uint32_t i = 0; i < num_neighbors; i++)
    {
      neighbors[i] = r.neighbors[i];
    }

    num_neighbors = r.num_neighbors;

    return *this;
  }

  void generate_uvh()
  {
    //atan: [-pi, pi]
    uv[0] = 0.5f + (atan2(pos[2], pos[0]) / (float)M_PI) * 0.5f;
    uv[1] = asin(pos[1]) / (float)(M_PI) + 0.5f;
    //uv[1] = 0.5f * (pos[1] + 1.0f); //probably a better way to do this? asin?
    //h = pos.magnitude();

    //winkel-tripel projection
    float lat = 2.0f * uv[1] - 1.0f;
    float lon = 2.0f * uv[0] - 1.0f;
    float central_meridian = 0.0f;
    float standard_parallel = acos(2.0f / (float)M_PI);//0.0f;
    float lambda = lon - central_meridian;

    float alpha = acos(cos(lat) * cos(lambda / 2.0f));

    float sinc_alpha = (alpha == 0.0f) ? 1.0f : sin(alpha) / alpha;
    float sin_phi = sin(lat);

    wt[0] = 0.5f * (lambda * (2.0f / (float)M_PI) + 2.0f * (cos(lat) * sin(lambda / 2.0f)) / sinc_alpha);
    wt[1] = 0.5f * (lat + sin_phi / sinc_alpha);

    wt[0] = 0.5f + 0.5f * wt[0];
    wt[1] = 0.5f + 0.5f * wt[1];
  } 

  Math::Float3 get_pos() const { return pos; }
  Math::Float2 get_uv() const { return uv; }
  Math::Float2 get_wt() const { return wt; }

  Math::Float3 get_color() const { return color; }
  void set_color(const Math::Float3 c) { color = c; }

  inline T *get_data() { return &data; }
  void set_data(T d) { data = d; }

  void add_neighbor(GeodesicCell *n)
  {
    assert(n);
    assert(num_neighbors < 6);
    neighbors[num_neighbors] = n;
    num_neighbors++;
  }

  void add_unique_neighbor(GeodesicCell<T> *n)
  {
    assert(n);
    for(uint32_t i = 0; i < num_neighbors; i++)
    {
      if(neighbors[i] == n)
      {
        return;
      }
    }
    assert(num_neighbors < 6);
    neighbors[num_neighbors] = n;
    num_neighbors++;
  }

  void make_neighbors_clockwise()
  {
    // bubble sort
    GeodesicCell<T> *tmp = NULL;
    bool swapped = true;
    int j = 0;
    while(swapped)
    {
      swapped = false;
      j++;
      for(uint32_t i = 0; i < num_neighbors - j; i++)
      {
        if(neighbors[0] != neighbors[j] && neighbors[0] != neighbors[j + 1])
        {
          GeodesicCell<T> *res = compare_neighbors_winding_order(neighbors[0], neighbors[i], neighbors[i + 1]);
          if(res == neighbors[i + 1])
          {
            swapped = true;
            tmp = neighbors[i];
            neighbors[i] = neighbors[i + 1];
            neighbors[i + 1] = tmp;
          }
        }
      }
    }
    return;
  }

  int get_num_neighbors() const { return num_neighbors; }
  GeodesicCell<T> *get_neighbor(const int i) { return neighbors[i]; }
  Math::Float3 get_neighbor_centroid(const int i) { return neighbor_centroids[i]; }

  void fwrite(FILE *f, GeodesicCell<T> *cells, int num_cells)
  {
    fwrite(&pos, sizeof(Math::Float3), 1, f);
    fwrite(&uv, sizeof(Math::Float2), 1, f);
    fwrite(&wt, sizeof(Math::Float2), 1, f);
  }

private:

  GeodesicCell<T> *compare_neighbors_winding_order(GeodesicCell<T> *a, GeodesicCell<T> *b, GeodesicCell<T> *c)
  {
    if(a == b) { return b; }
    if(a == c) { return c; }
    if(b == c) { return b; }

    Math::Float3 va = a->pos - pos;
    Math::Float3 vb = b->pos - pos;
    Math::Float3 vc = c->pos - pos;

    Math::Float3 center = pos;
    center.normalize();

    va.normalize();
    vb.normalize();
    vc.normalize();

    // cross products
    Math::Float3 va_x_vb = va ^ vb;
    Math::Float3 va_x_vc = va ^ vc;
    Math::Float3 vb_x_vc = vb ^ vc;

    // dot products
    float dp_ab = va_x_vb * center;
    float dp_ac = va_x_vc * center;
    float dp_bc = vb_x_vc * center;

    // this part literally hurts my brain.
    if(dp_bc < 0)
    {
      if(dp_ab > 0) { return (dp_ac < 0) ? b : c; }
      else { return c; }
    }
    else
    {
      if(dp_ab > 0) { return b; }
      else { return (dp_ac > 0) ? c : b; }
    }

    return b;
  }
};

#endif //__GEODESIC_CELL_H__
