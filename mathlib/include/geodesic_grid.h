#ifndef __GEODESIC_GRID_H__
#define __GEODESIC_GRID_H__

#include <assert.h>
#include <stdio.h>
#include "math_utility.h"
#include "geodesic_cell.h"
#include "kdtree.h"

//#define GEODESIC_GRID_RECTILINEAR

#define GEODESIC_GRID_FILE_VERSION 1

//a subdividible geodesic grid on the surface of the sphere
//we can use this data structure to simulate cellular automata

enum CellIndex {
  CELL_ERROR = -1,
  CELL_N = 0, //N pole
  CELL_A = 0,
  CELL_B,
  CELL_C,
  CELL_D,
  CELL_E,
  CELL_F,
  CELL_G,
  CELL_H,
  CELL_I,
  CELL_J,
  CELL_K,
  CELL_L = 11,
  CELL_S = 11, //S pole
};

template <class T>
class GeodesicEdge
{
public:
  GeodesicEdge() { a = NULL; b = NULL; }
  ~GeodesicEdge() {}

  bool operator==(const GeodesicEdge<T> &r)
  {
    if(r.a == this->a && r.b == this->b)
    {
      return true;
    }
    if(r.a == this->b && r.b == this->a)
    {
      return true;
    }

    return false;
  }

  GeodesicCell<T> *a;
  GeodesicCell<T> *b;
};


template <class T>
class GeodesicFace
{
public:
  GeodesicFace() {a = b = c = NULL; }
  ~GeodesicFace() {}

  void set_faces(GeodesicCell<T> *_a, GeodesicCell<T> *_b, GeodesicCell<T> *_c)
  {
    a = _a; b = _b; c = _c;
  }

  GeodesicCell<T> *a, *b, *c;
};

template <class T>
class GeodesicGrid
{
private:
  int _file_version;

  uint32_t _subdivision_levels;        //current subdivision level

  uint32_t _num_cells;                 //number of geodesic cells
  uint32_t _num_edges;                 //number of edges
  uint32_t _num_faces;                 //number of faces

#if defined(GEODESIC_GRID_RECTILINEAR)
  uint32_t            _cell_idx_grid_size;  //total number of cells we will allocate (not actual number of unique cells, since some are duped / empty)
  uint32_t           *_cell_idx_grid;       //linear array of cells that we will index in 2 dimensions
#else
  GeodesicCell<T>    *_cells;          //the array of cells that store the actual data
  GeodesicEdge<T>    *_edges;          //the array of cell pairs (edges)
  GeodesicFace<T>    *_faces;          //the array of edge triangles (faces)

  Structures::KDTree3D<GeodesicCell<T> *>        *_kd_tree;
#endif // GEODESIC_GRID_RECTILINEAR

public:
#if defined GEODESIC_GRID_RECTILINEAR
  GeodesicGrid(unsigned int sub_levels = 0)
  {
    // start with an icosahedron
    // E = F + V - 2 (Euler)
    // E = 30
    // V = 12
    // F = 20

    _subdivision_levels = sub_levels;

    _num_cells = 12;
    _num_edges = 30;
    _num_faces = 20;

    _cells = new GeodesicCell<T>[_num_cells];
    // fill the 12 vertices with the cartesian coordinates of the icosahedron
    // (0, ±1, ± φ)
    // (±1, ± φ, 0)
    // (± φ, 0, ±1)

    cells[0].pos =  Float3(-1.0f,  M_PHI, 0.0f);
    cells[1].pos =  Float3( 1.0f,  M_PHI, 0.0f);
    cells[2].pos =  Float3(-1.0f, -M_PHI, 0.0f);
    cells[3].pos =  Float3( 1.0f, -M_PHI, 0.0f);

    cells[4].pos =  Float3( 0.0f, -1.0f,  M_PHI);
    cells[5].pos =  Float3( 0.0f,  1.0f,  M_PHI);
    cells[6].pos =  Float3( 0.0f, -1.0f, -M_PHI);
    cells[7].pos =  Float3( 0.0f,  1.0f, -M_PHI);

    cells[8].pos =  Float3( M_PHI, 0.0f, -1.0f);
    cells[9].pos =  Float3( M_PHI, 0.0f,  1.0f);
    cells[10].pos = Float3(-M_PHI, 0.0f, -1.0f);
    cells[11].pos = Float3(-M_PHI, 0.0f,  1.0f);

    //                       dU
    //             A-B-C     4
    //             |/|/|
    //           A-K-L-F     3
    //           |/|/|
    //         A-I-J-F       2
    //         |/|/|                =>
    //       A-G-H-F         1
    //       |/|/|                       .-.-A-A-A-.-.
    //     A-D-E-F           0           .-A-G-I-K-B-.
    //     |/|/|                         A-D-E-H-J-L-C
    //     B-C-F             0           B-C-F-F-F-F-F
    // dV: 0-0-0-1-2-3-4                 0-0-0-1-2-3-4

    unsigned int w, h;
    w = 7; h = 6;
    _cell_idx_grid = new uint32_t[w * h];
  }
#else
  GeodesicGrid()
  {
    // start with an icosahedron
    // E = F + V - 2 (Euler)
    // E = 30
    // V = 12
    // F = 20

    // each vertex of the icosahedron is a cell. So we have 12 cells @ the
    // first subdivision level.
    //dual_cells = NULL;
    _kd_tree = NULL;

    _subdivision_levels = 0;
    _num_cells = 12;
    //num_dual_cells = 30;
    _num_edges = 30;
    _num_faces = 20;

    _cells = new GeodesicCell<T>[12];
    _edges = new GeodesicEdge<T>[30];
    _faces = new GeodesicFace<T>[20];

    // fill the 12 vertices with the cartesian coordinates of the icosahedron
    // (0, ±1, ± φ)
    // (±1, ± φ, 0)
    // (± φ, 0, ±1)

    /*
    cells[0].pos =  Float3(-1.0f,  M_PHI, 0.0f);
    cells[1].pos =  Float3( 1.0f,  M_PHI, 0.0f);
    cells[2].pos =  Float3(-1.0f, -M_PHI, 0.0f);
    cells[3].pos =  Float3( 1.0f, -M_PHI, 0.0f);

    cells[4].pos =  Float3( 0.0f, -1.0f,  M_PHI);
    cells[5].pos =  Float3( 0.0f,  1.0f,  M_PHI);
    cells[6].pos =  Float3( 0.0f, -1.0f, -M_PHI);
    cells[7].pos =  Float3( 0.0f,  1.0f, -M_PHI);

    cells[8].pos =  Float3( M_PHI, 0.0f, -1.0f);
    cells[9].pos =  Float3( M_PHI, 0.0f,  1.0f);
    cells[10].pos = Float3(-M_PHI, 0.0f, -1.0f);
    cells[11].pos = Float3(-M_PHI, 0.0f,  1.0f);
    */

    // But... actually, the above coordinates, while mathematically attractive,
    // give me an icosahedron that is rotated oddly for my use (I want vertical poles)
    // so, here are vertex positions from Blender:
    //
    // P.S. I could also fairly easily take the above pretty vertices and rotate them by some
    //      amount no doubt related to PHI, but it seems easier to do this the brute-force
    //      way right now. (also, I don't have a matrix class in my math library yet)
    //

    _cells[CELL_N].pos = Math::Float3( 0.00000f,  1.00000f,  0.00000f);  // A (N pole)

    _cells[CELL_B].pos = Math::Float3( 0.89442f,  0.44721f, 0.00000f);
    _cells[CELL_C].pos = Math::Float3( 0.27639f, 0.44721f, -0.85064f);
    _cells[CELL_D].pos = Math::Float3(-0.72360f, 0.44721f, -0.52572f);
    _cells[CELL_E].pos = Math::Float3(-0.72360f,  0.44721f, 0.52572f);
    _cells[CELL_F].pos = Math::Float3( 0.27639f,  0.44721f, 0.85064f);

    _cells[CELL_G].pos = Math::Float3( 0.72360f, -0.44721f, -0.52572f);
    _cells[CELL_H].pos = Math::Float3(-0.27639f, -0.44721f, -0.85064f);
    _cells[CELL_I].pos = Math::Float3(-0.89442f, -0.44721f, 0.00000f);
    _cells[CELL_J].pos = Math::Float3(-0.27639f, -0.44721f, 0.85064f);
    _cells[CELL_K].pos = Math::Float3( 0.72360f, -0.44721f, 0.52572f);

    _cells[CELL_S].pos = Math::Float3( 0.00000f, -1.00000f,  0.00000f);  // F (S pole)q

    //add edges
    _edges[0].a = &_cells[CELL_A];    _edges[0].b = &_cells[CELL_B];
    _edges[1].a = &_cells[CELL_A];    _edges[1].b = &_cells[CELL_C];
    _edges[2].a = &_cells[CELL_A];    _edges[2].b = &_cells[CELL_D];
    _edges[3].a = &_cells[CELL_A];    _edges[3].b = &_cells[CELL_E];
    _edges[4].a = &_cells[CELL_A];    _edges[4].b = &_cells[CELL_F];

    _edges[5].a = &_cells[CELL_B];    _edges[5].b = &_cells[CELL_F];
    _edges[6].a = &_cells[CELL_B];    _edges[6].b = &_cells[CELL_K];
    _edges[7].a = &_cells[CELL_B];    _edges[7].b = &_cells[CELL_G];
    _edges[8].a = &_cells[CELL_B];    _edges[8].b = &_cells[CELL_C];

    _edges[9].a = &_cells[CELL_C];     _edges[9].b = &_cells[CELL_G];
    _edges[10].a = &_cells[CELL_C];    _edges[10].b = &_cells[CELL_H];
    _edges[11].a = &_cells[CELL_C];    _edges[11].b = &_cells[CELL_D];

    _edges[12].a = &_cells[CELL_D];    _edges[12].b = &_cells[CELL_H];
    _edges[13].a = &_cells[CELL_D];    _edges[13].b = &_cells[CELL_I];
    _edges[14].a = &_cells[CELL_D];    _edges[14].b = &_cells[CELL_E];

    _edges[15].a = &_cells[CELL_E];    _edges[15].b = &_cells[CELL_I];
    _edges[16].a = &_cells[CELL_E];    _edges[16].b = &_cells[CELL_J];
    _edges[17].a = &_cells[CELL_E];    _edges[17].b = &_cells[CELL_F];

    _edges[18].a = &_cells[CELL_F];    _edges[18].b = &_cells[CELL_J];
    _edges[19].a = &_cells[CELL_F];    _edges[19].b = &_cells[CELL_K];

    _edges[20].a = &_cells[CELL_G];    _edges[20].b = &_cells[CELL_K];
    _edges[21].a = &_cells[CELL_G];    _edges[21].b = &_cells[CELL_L];
    _edges[22].a = &_cells[CELL_G];    _edges[22].b = &_cells[CELL_H];

    _edges[23].a = &_cells[CELL_H];    _edges[23].b = &_cells[CELL_L];
    _edges[24].a = &_cells[CELL_H];    _edges[24].b = &_cells[CELL_I];

    _edges[25].a = &_cells[CELL_I];    _edges[25].b = &_cells[CELL_L];
    _edges[26].a = &_cells[CELL_I];    _edges[26].b = &_cells[CELL_J];

    _edges[27].a = &_cells[CELL_J];    _edges[27].b = &_cells[CELL_L];
    _edges[28].a = &_cells[CELL_J];    _edges[28].b = &_cells[CELL_K];

    _edges[29].a = &_cells[CELL_K];    _edges[29].b = &_cells[CELL_L];

    //set up faces
    _faces[0].set_faces(&_cells[CELL_A], &_cells[CELL_B], &_cells[CELL_C]);      //ABC
    _faces[1].set_faces(&_cells[CELL_A], &_cells[CELL_C], &_cells[CELL_D]);      //ACD
    _faces[2].set_faces(&_cells[CELL_A], &_cells[CELL_D], &_cells[CELL_E]);      //ADE
    _faces[3].set_faces(&_cells[CELL_A], &_cells[CELL_E], &_cells[CELL_F]);      //AEF
    _faces[4].set_faces(&_cells[CELL_A], &_cells[CELL_F], &_cells[CELL_B]);      //AFB

    _faces[5].set_faces(&_cells[CELL_B], &_cells[CELL_G], &_cells[CELL_C]);      //BGC
    _faces[6].set_faces(&_cells[CELL_C], &_cells[CELL_G], &_cells[CELL_H]);      //CGH
    _faces[7].set_faces(&_cells[CELL_C], &_cells[CELL_H], &_cells[CELL_D]);      //CHD
    _faces[8].set_faces(&_cells[CELL_D], &_cells[CELL_H], &_cells[CELL_I]);      //DHI
    _faces[9].set_faces(&_cells[CELL_D], &_cells[CELL_I], &_cells[CELL_E]);      //DIE
    _faces[10].set_faces(&_cells[CELL_E], &_cells[CELL_I], &_cells[CELL_J]);    //EIJ
    _faces[11].set_faces(&_cells[CELL_E], &_cells[CELL_J], &_cells[CELL_F]);    //EJF
    _faces[12].set_faces(&_cells[CELL_F], &_cells[CELL_J], &_cells[CELL_K]);    //FJK
    _faces[13].set_faces(&_cells[CELL_F], &_cells[CELL_K], &_cells[CELL_B]);    //FKB
    _faces[14].set_faces(&_cells[CELL_B], &_cells[CELL_K], &_cells[CELL_G]);    //BKG

    _faces[15].set_faces(&_cells[CELL_L], &_cells[CELL_G], &_cells[CELL_K]);    //LGK
    _faces[16].set_faces(&_cells[CELL_L], &_cells[CELL_K], &_cells[CELL_J]);    //LKJ
    _faces[17].set_faces(&_cells[CELL_L], &_cells[CELL_J], &_cells[CELL_I]);    //LJI
    _faces[18].set_faces(&_cells[CELL_L], &_cells[CELL_I], &_cells[CELL_H]);    //LIH
    _faces[19].set_faces(&_cells[CELL_L], &_cells[CELL_H], &_cells[CELL_G]);    //LHG


    // set up adjacency (graph version)
    // IMPORTANT: THESE MUST BE CLOCKWISE FOR RENDERING TO WORK PROPERLY!!!
    // N - BCDEF
    _cells[CELL_N].add_neighbor(&_cells[CELL_B]);
    _cells[CELL_N].add_neighbor(&_cells[CELL_C]);
    _cells[CELL_N].add_neighbor(&_cells[CELL_D]);
    _cells[CELL_N].add_neighbor(&_cells[CELL_E]);
    _cells[CELL_N].add_neighbor(&_cells[CELL_F]);

    // B - AFKGC
    _cells[CELL_B].add_neighbor(&_cells[CELL_N]);
    _cells[CELL_B].add_neighbor(&_cells[CELL_F]);
    _cells[CELL_B].add_neighbor(&_cells[CELL_K]);
    _cells[CELL_B].add_neighbor(&_cells[CELL_G]);
    _cells[CELL_B].add_neighbor(&_cells[CELL_C]);

    // C - ABGHD
    _cells[CELL_C].add_neighbor(&_cells[CELL_N]);
    _cells[CELL_C].add_neighbor(&_cells[CELL_B]);
    _cells[CELL_C].add_neighbor(&_cells[CELL_G]);
    _cells[CELL_C].add_neighbor(&_cells[CELL_H]);
    _cells[CELL_C].add_neighbor(&_cells[CELL_D]);

    // D - ACHIE
    _cells[CELL_D].add_neighbor(&_cells[CELL_N]);
    _cells[CELL_D].add_neighbor(&_cells[CELL_C]);
    _cells[CELL_D].add_neighbor(&_cells[CELL_H]);
    _cells[CELL_D].add_neighbor(&_cells[CELL_I]);
    _cells[CELL_D].add_neighbor(&_cells[CELL_E]);

    // E - ADIJF
    _cells[CELL_E].add_neighbor(&_cells[CELL_N]);
    _cells[CELL_E].add_neighbor(&_cells[CELL_D]);
    _cells[CELL_E].add_neighbor(&_cells[CELL_I]);
    _cells[CELL_E].add_neighbor(&_cells[CELL_J]);
    _cells[CELL_E].add_neighbor(&_cells[CELL_F]);

    // F - AEJKB
    _cells[CELL_F].add_neighbor(&_cells[CELL_N]);
    _cells[CELL_F].add_neighbor(&_cells[CELL_E]);
    _cells[CELL_F].add_neighbor(&_cells[CELL_J]);
    _cells[CELL_F].add_neighbor(&_cells[CELL_K]);
    _cells[CELL_F].add_neighbor(&_cells[CELL_B]);

    // G - BKLHC
    _cells[CELL_G].add_neighbor(&_cells[CELL_B]);
    _cells[CELL_G].add_neighbor(&_cells[CELL_K]);
    _cells[CELL_G].add_neighbor(&_cells[CELL_S]);
    _cells[CELL_G].add_neighbor(&_cells[CELL_H]);
    _cells[CELL_G].add_neighbor(&_cells[CELL_C]);

    // H - CGLID
    _cells[CELL_H].add_neighbor(&_cells[CELL_C]);
    _cells[CELL_H].add_neighbor(&_cells[CELL_G]);
    _cells[CELL_H].add_neighbor(&_cells[CELL_S]);
    _cells[CELL_H].add_neighbor(&_cells[CELL_I]);
    _cells[CELL_H].add_neighbor(&_cells[CELL_D]);

    // I - DHLJE
    _cells[CELL_I].add_neighbor(&_cells[CELL_D]);
    _cells[CELL_I].add_neighbor(&_cells[CELL_H]);
    _cells[CELL_I].add_neighbor(&_cells[CELL_S]);
    _cells[CELL_I].add_neighbor(&_cells[CELL_J]);
    _cells[CELL_I].add_neighbor(&_cells[CELL_E]);

    // J - EILKF
    _cells[CELL_J].add_neighbor(&_cells[CELL_E]);
    _cells[CELL_J].add_neighbor(&_cells[CELL_I]);
    _cells[CELL_J].add_neighbor(&_cells[CELL_S]);
    _cells[CELL_J].add_neighbor(&_cells[CELL_K]);
    _cells[CELL_J].add_neighbor(&_cells[CELL_F]);

    // K - BFJLG
    _cells[CELL_K].add_neighbor(&_cells[CELL_B]);
    _cells[CELL_K].add_neighbor(&_cells[CELL_F]);
    _cells[CELL_K].add_neighbor(&_cells[CELL_J]);
    _cells[CELL_K].add_neighbor(&_cells[CELL_S]);
    _cells[CELL_K].add_neighbor(&_cells[CELL_G]);

    // S - GKJIH
    _cells[CELL_S].add_neighbor(&_cells[CELL_G]);
    _cells[CELL_S].add_neighbor(&_cells[CELL_K]);
    _cells[CELL_S].add_neighbor(&_cells[CELL_J]);
    _cells[CELL_S].add_neighbor(&_cells[CELL_I]);
    _cells[CELL_S].add_neighbor(&_cells[CELL_H]);

    //assign random colors to each vertex for now
    for(int i = 0; i < 12; i++)
    {
      _cells[i].color = Math::Float3(Math::random(0.0f, 1.0f), Math::random(0.0f, 1.0f), Math::random(0.0f, 1.0f));
      //cells[i].make_neighbors_clockwise();
    }
  }


  ~GeodesicGrid()
  {
    delete[] _cells;
    delete[] _edges;
    delete[] _faces;

    //delete[] _adjacency_grid;
    if (_kd_tree)
    {
      delete _kd_tree;
    }
  }
  
  GeodesicGrid<T> &operator=(GeodesicGrid<T> &g)
  {
    _file_version = g._file_version;
    _subdivision_levels = g._subdivision_levels;

    _num_cells = g._num_cells;
    _num_edges = g._num_edges;
    _num_faces = g._num_faces;

    if (_cells) { delete[] _cells; }
    if (_edges) { delete[] _edges; }
    if (_faces) { delete[] _faces; }
    //if (_adjacency_grid) { delete[] _adjacency_grid; }
    if (_kd_tree) { delete _kd_tree; }

    _cells = new GeodesicCell<T>[_num_cells];
    memcpy(_cells, g._cells, sizeof(GeodesicCell<T>) * _num_cells);
    _edges = new GeodesicEdge<T>[_num_edges];
    memcpy(_edges, g._edges, sizeof(GeodesicEdge<T>) * _num_edges);
    _faces = new GeodesicFace<T>[_num_edges];
    memcpy(_faces, g._faces, sizeof(GeodesicFace<T>) * _num_faces);
    
    return *this;
  }
#endif // GEODESIC_GRID_RECTILINEAR

  float get_cell_surface_area()
  {
    return 4.0f * M_PI / (float)num_cells;
  }

#ifndef GEODESIC_GRID_RECTILINEAR
  void generate_kd_tree()
  {
    if (!_kd_tree)
    {
      _kd_tree = new Structures::KDTree3D<GeodesicCell<T> *>;
    }
    else
    {
      _kd_tree->reset();
    }

    for (uint32_t i = 0; i < _num_cells; i++)
    {
      GeodesicCell<T> *c = &(_cells[i]);
      _kd_tree->insert_element(c->pos, c);
    }
  }


  void subdivide(int levels)
  {
    for(int i = 0; i < levels; i++)
    {
      sub();
    }
  }
#endif //GEODESIC_GRID_RECTILINEAR

  int get_subdivision_levels() const { return _subdivision_levels; }

  void make_selected_neighbors_clockwise()
  {
    GeodesicCell<T> *c = &cells[selected_cell];
    c->make_neighbors_clockwise();
  }

  GeodesicCell<T> *get_cell_array() { return _cells; }
  uint32_t get_num_cells() const { return _num_cells; }
  //int get_num_dual_cells() const { return num_dual_cells; }
  uint32_t get_num_edges() const { return _num_edges; }
  uint32_t get_num_faces() const { return _num_faces; }

  GeodesicCell<T> *get_cell_by_uv(const Math::Float2 uv)
  {
    float theta = 2.0f * (float)M_PI * uv[0];
    float phi = ((float)M_PI / 2.0f) + (float)M_PI * (1.0f - uv[1]);

    Math::Float3 xyz = Math::polar_to_cartesian(theta, phi, 1.0f);
    return get_cell_by_xyz(xyz);
  }
  
  GeodesicCell<T> *get_cell_by_xyz(const Math::Float3 xyz)
  {
    if (!_kd_tree)
    {
      generate_kd_tree();
    }
    Structures::KDNode<GeodesicCell<T> *> *n = _kd_tree->find_nearest_neighbor(xyz);
    return n->data;
  }


/*
  void generate_dual()
  {
    if(dual_cells)
    {
      delete[] dual_cells;
    }

    std::cout<<"num cells: "<<num_cells<<std::endl;
    std::cout<<"num edges: "<<num_edges<<std::endl;

    num_dual_cells = num_edges;
    dual_cells = new GeodesicCell<T>[num_edges];
  }
  */

  void fread_grid(FILE *f)
  {
    assert(f);

    fread(&_file_version, sizeof(int), 1, f);
    fread(&_subdivision_levels, sizeof(int), 1, f);

    fread(&_num_cells, sizeof(int), 1, f);
    //fread(&num_dual_cells, sizeof(int), 1, f);
    fread(&_num_edges, sizeof(int), 1, f);
    fread(&_num_faces, sizeof(int), 1, f);

    delete [] _cells;
    _cells = new GeodesicCell<T>[_num_cells];
    for(uint32_t i = 0; i < _num_cells; i++)
    {
      fread_cell(&_cells[i], f);
    }

    /*delete [] dual_cells;
    dual_cells = new GeodesicCell<T>[num_dual_cells];
    for(int i = 0; i < num_dual_cells; i++)
    {
      fread_cell(&dual_cells[i], f);
    }*/

    delete [] _edges;
    _edges = new GeodesicEdge<T>[_num_edges];
    for(uint32_t i = 0; i < _num_edges; i++)
    {
      fread_edge(&_edges[i], f);
    }

    generate_neighbor_centroids();

    //delete [] _faces;
    //_faces = new GeodesicFace<T>[_num_faces];
  }

  void fwrite_grid(FILE *f)
  {
    //assume f is already open
    assert(f);

    int file_version = GEODESIC_GRID_FILE_VERSION;
    fwrite(&file_version, sizeof(int), 1, f);
    fwrite(&_subdivision_levels, sizeof(int), 1, f);

    fwrite(&_num_cells, sizeof(int), 1, f);
    //fwrite(&num_dual_cells, sizeof(int), 1, f);
    fwrite(&_num_edges, sizeof(int), 1, f);
    fwrite(&_num_faces, sizeof(int), 1, f);

    //write the actual arrays
    for(uint32_t i = 0; i < _num_cells; i++)
    {
      fwrite_cell(&_cells[i], f);
    }

    /*for(int i = 0; i < num_dual_cells; i++)
    {
      fwrite_cell(&dual_cells[i], f);
    }*/

    for(uint32_t i = 0; i < _num_edges; i++)
    {
      fwrite_edge(&_edges[i], f);
    }

    /*
    for(int i = 0; i < num_faces; i++)
    {
      fwrite(&faces[i], f);
    }*/
  }
private:

  //helper functions
  GeodesicCell<T> *add_unique_cell_at_point(Math::Float3 &p, GeodesicCell<T> *_cells, uint32_t &cell_count, GeodesicCell<T> *a, GeodesicCell<T> *b)
  {
    float weld_threshold = 0.0001f;
    float weld_threshold_2 = weld_threshold * weld_threshold;
    //maybe add ab to the new cell list (if there are no other verts *super* close)

    // thoughts for optimization:
    // 1. Use a KD tree (other space partition tree) to find the closest cell to a
    //    particular point
    // 2. compare x, y and z distances first before doing the dist_squared calculation
    //
    // 3. For our purposes, I'm pretty sure we just need to check the neighbors
    //    of our neighbors, and not the entire grid.
    //
    // 4. This all goes away if we get the adjacency grid working correctly
    //
    for(uint32_t i = 0; i < a->num_neighbors; i++)
    {
      GeodesicCell<T> *c = a->neighbors[i];
      float d2 = dist_squared(p, c->pos);
      if(d2 < weld_threshold_2)
      {
        return c;
      }
    }

    for(uint32_t i = 0; i < b->num_neighbors; i++)
    {
      GeodesicCell<T> *c = b->neighbors[i];
      float d2 = dist_squared(p, c->pos);
      if(d2 < weld_threshold_2)
      {
        return c;
      }
    }

    //didn't find any... "allocate" a new one off the end of the array
    _cells[cell_count].init();
    _cells[cell_count].pos = p;
    _cells[cell_count].color = Math::Float3(1.0f, 0.0f, 0.0f);
    return &_cells[cell_count++];
  }
  GeodesicCell<T> *add_cell_at_point(Math::Float3 &p, GeodesicCell<T> *_cells, int &cell_count)
  {
    _cells[cell_count].init();
    _cells[cell_count].pos = p;
    _cells[cell_count].color = Math::Float3(1.0f, 0.0f, 0.0f);
    return &_cells[cell_count++];
  }

#if defined GEODESIC_GRID_RECTILINEAR

#else
  void sub()
  {
    _subdivision_levels++;

    // E = F + V - 2 (Euler)
    uint32_t old_num_verts = _num_cells;
    uint32_t new_num_verts = _num_cells + _num_edges;
    GeodesicCell<T> *new_cells = new GeodesicCell<T>[new_num_verts];

    uint32_t old_num_edges = _num_edges;
    uint32_t new_num_edges = _num_edges * 2 * 2;
    GeodesicEdge<T> *new_edges = new GeodesicEdge<T>[new_num_edges];

    uint32_t old_num_faces = _num_faces;
    uint32_t new_num_faces = new_num_edges + 2 - new_num_verts; //E + 2 - V (Euler)
    GeodesicFace<T> *new_faces = new GeodesicFace<T>[new_num_faces];

    //first, add all existing vertices to the new vertex list
    uint32_t new_vert_idx = 0;
    for(uint32_t i = 0; i < old_num_verts; i++)
    {
      //copy the old vert to the temp list (operator=)
      _cells[i].pos.normalize();
      new_cells[new_vert_idx] = _cells[i];

      GeodesicCell<T> *c = &new_cells[new_vert_idx];
      c->num_neighbors = 0;

      //fix up edge list
      for(uint32_t j = 0; j < old_num_edges; j++)
      {
        if(_edges[j].a == &_cells[i]) { _edges[j].a = c; }
        if(_edges[j].b == &_cells[i]) { _edges[j].b = c; }
      }

      //fix up face list
      for(uint32_t j = 0; j < old_num_faces; j++)
      {
        if(_faces[j].a == &_cells[i]) { _faces[j].a = c; }
        if(_faces[j].b == &_cells[i]) { _faces[j].b = c; }
        if(_faces[j].c == &_cells[i]) { _faces[j].c = c; }
      }
      new_vert_idx++;
    }

    uint32_t new_face_idx = 0;
    for(uint32_t i = 0; i < old_num_faces; i++)
    {
      //cout<<"old face idx: "<<i<<endl;
      //find the midpoints of the three edges in each face
      GeodesicCell<T> *ab = NULL;
      GeodesicCell<T> *bc = NULL;
      GeodesicCell<T> *ca = NULL;

      GeodesicFace<T> *f = &_faces[i];
      Math::Float3 mid_ab = midpoint(f->a->pos, f->b->pos);
      Math::Float3 mid_bc = midpoint(f->b->pos, f->c->pos);
      Math::Float3 mid_ca = midpoint(f->c->pos, f->a->pos);

      ab = add_unique_cell_at_point(mid_ab, new_cells, new_vert_idx, f->a, f->b);
      bc = add_unique_cell_at_point(mid_bc, new_cells, new_vert_idx, f->b, f->c);
      ca = add_unique_cell_at_point(mid_ca, new_cells, new_vert_idx, f->c, f->a);
      assert(new_vert_idx <= new_num_verts);

      ab->add_unique_neighbor(f->b);
      ab->add_unique_neighbor(bc);
      ab->add_unique_neighbor(ca);
      ab->add_unique_neighbor(f->a);

      bc->add_unique_neighbor(f->c);
      bc->add_unique_neighbor(ca);
      bc->add_unique_neighbor(ab);
      bc->add_unique_neighbor(f->b);

      ca->add_unique_neighbor(f->a);
      ca->add_unique_neighbor(ab);
      ca->add_unique_neighbor(bc);
      ca->add_unique_neighbor(f->c);

      //questionable
      f->a->add_unique_neighbor(ab);
      f->a->add_unique_neighbor(ca);

      f->b->add_unique_neighbor(bc);
      f->b->add_unique_neighbor(ab);

      f->c->add_unique_neighbor(ca);
      f->c->add_unique_neighbor(bc);

      f->a->make_neighbors_clockwise();
      f->b->make_neighbors_clockwise();
      f->c->make_neighbors_clockwise();

      //so now, we are going to ditch the original face and add 4 new ones
      assert(new_face_idx + 4 <= new_num_faces);
      GeodesicFace<T> *a_ab_ca = &new_faces[new_face_idx++];
      GeodesicFace<T> *ab_b_bc = &new_faces[new_face_idx++];
      GeodesicFace<T> *bc_c_ca = &new_faces[new_face_idx++];
      GeodesicFace<T> *ab_bc_ca = &new_faces[new_face_idx++];

      a_ab_ca->a = f->a;    a_ab_ca->b = ab;    a_ab_ca->c = ca;
      ab_b_bc->a = ab;      ab_b_bc->b = f->b;  ab_b_bc->c = bc;
      bc_c_ca->a = bc;      bc_c_ca->b = f->c;  bc_c_ca->c = ca;
      ab_bc_ca->a = ab;     ab_bc_ca->b = bc;   ab_bc_ca->c = ca;
    }

    for(uint32_t i = 0; i < new_num_verts; i++)
    {
      //push the vertex out to the unit vector
      GeodesicCell<T> *c = &new_cells[i];
      c->pos.normalize();
      c->generate_uvh();

      assert(c->num_neighbors > 4);
    }

    //update edges
    //brute force: visit every cell, accumulate neighbors,
    //checking for duplicate edges
    uint32_t edge_count = 0;
    for(uint32_t i = 0; i < new_num_verts; i++)
    {
      GeodesicCell<T> *cell = &new_cells[i];

      for(uint32_t j = 0; j < cell->num_neighbors; j++)
      {
        GeodesicCell<T> *n = cell->neighbors[j];

        //check for duplicate edges
        bool found_match = false;
        for(uint32_t k = 0; k < edge_count; k++)
        {
          GeodesicEdge<T> *e = &new_edges[k];
          if((e->a == cell && e->b == n) || (e->a == n && e->b == cell))
          {
            found_match = true;
          }
        }
        if(!found_match)
        {
          new_edges[edge_count].a = cell;
          new_edges[edge_count].b = n;
          edge_count++;
        }
      }
    }

    //generate_neighbor_centroids();
    //std::cout<<"edge_count: "<<edge_count<<std::endl;

    delete[] _cells;
    _cells = new_cells;
    _num_cells = new_num_verts;

    generate_neighbor_centroids();

    //std::cout<<"new_vert_idx: "<<new_vert_idx<<std::endl;
    //std::cout<<"new_num_verts: "<<new_num_verts<<std::endl;
    //std::cout<<"num_cells: "<<num_cells<<std::endl;
    //std::cout<<"new_num_edges: "<<new_num_edges<<std::endl;

    delete[] _faces;
    _faces = new_faces;
    _num_faces = new_num_faces;

    delete[] _edges;
    _edges = new_edges;
    _num_edges = new_num_edges;
  }
#endif // GEODESIC_GRID_RECTILINEAR

  void generate_neighbor_centroids()
  {
    //compute the neighbor centroids
    for(uint32_t i = 0; i < _num_cells; i++)
    {
      GeodesicCell<T> *cell = &_cells[i];
      for(uint32_t j = 0; j < cell->num_neighbors; j++)
      {
        GeodesicCell<T> *b = cell->neighbors[j];
        GeodesicCell<T> *c = cell->neighbors[(j + 1) % cell->num_neighbors];

        cell->neighbor_centroids[j] = (cell->pos + b->pos + c->pos) / 3.0f;
      }
    }
  }

  void fread_cell(GeodesicCell<T> *c, FILE *f)
  {
    fread(&c->pos, sizeof(Math::Float3), 1, f);
    fread(&c->uv, sizeof(Math::Float2), 1, f);
    fread(&c->wt, sizeof(Math::Float2), 1, f);

    fread(&c->color, sizeof(Math::Float3), 1, f);
    fread(&c->data, sizeof(T), 1, f);
    fread(&c->num_neighbors, sizeof(uint32_t), 1, f);

    for(uint32_t i = 0; i < c->num_neighbors; i++)
    {
      uint32_t cell_idx;
      fread(&cell_idx, sizeof(uint32_t), 1, f);
      assert(cell_idx >= 0 && cell_idx < _num_cells);
      c->neighbors[i] = &_cells[cell_idx];
    }
  }

  void fwrite_cell(GeodesicCell<T> *c, FILE *f)
  {
    fwrite(&c->pos, sizeof(Math::Float3), 1, f);
    fwrite(&c->uv, sizeof(Math::Float2), 1, f);
    fwrite(&c->wt, sizeof(Math::Float2), 1, f);

    fwrite(&c->color, sizeof(Math::Float3), 1, f);
    fwrite(&c->data, sizeof(T), 1, f);
    fwrite(&c->num_neighbors, sizeof(uint32_t), 1, f);
    for(uint32_t j = 0; j < c->num_neighbors; j++)
    {
      for(uint32_t k = 0; k < _num_cells; k++)
      {
        if(&_cells[k] == c->neighbors[j])
        {
          fwrite(&k, sizeof(uint32_t), 1, f);
          k = _num_cells;
        }
      }
    }
  }

  void fread_edge(GeodesicEdge<T> *e, FILE *f)
  {
    uint32_t a_idx, b_idx;
    fread(&a_idx, sizeof(int), 1, f);
    fread(&b_idx, sizeof(int), 1, f);

    assert(a_idx >= 0 && a_idx < _num_cells);
    assert(b_idx >= 0 && b_idx < _num_cells);

    e->a = &_cells[a_idx];
    e->b = &_cells[b_idx];
  }

  void fwrite_edge(GeodesicEdge<T> *e, FILE *f)
  {
    for(uint32_t i = 0; i < _num_cells; i++)
    {
      if(&_cells[i] == e->a)
      {
        fwrite(&i, sizeof(int), 1, f);
      }
    }
    for(uint32_t i = 0; i < _num_cells; i++)
    {
      if(&_cells[i] == e->b)
      {
        fwrite(&i, sizeof(int), 1, f);
      }
    }
  }
};

#endif //__GEODESIC_GRID_H__
