#include <iostream>
#include <assert.h>
#include <png.h>

#include "minilzo.h"
#include "bphys_baker.h"

using namespace std;
using namespace Tool;

void BPhysBaker::init()
{
  cout<<"Initializing Blender Point Cache (Physics) Baker..."<<endl;
  if (lzo_init() != LZO_E_OK)
  {
    cerr<<"BPhysBaker::init() - Failed to initialize lzo compression!"<<endl;
  }
}

void BPhysBaker::bake(FILE *f)
{
  BPhysHeader bph;
  fread(&bph, sizeof(BPhysHeader), 1, f);
  assert(bph.bphys[0] == 'B' && bph.bphys[1] == 'P' && bph.bphys[2] == 'H' && bph.bphys[3] == 'Y');
  assert(bph.bphys[4] == 'S' && bph.bphys[5] == 'I' && bph.bphys[6] == 'C' && bph.bphys[7] == 'S');

  switch(bph.data_type)
  {
    case BPHYS_DATA_SMOKE_LOW:
      read_smoke_data(f);
      break;
  }
}


static int ptcache_file_compressed_read(unsigned char *result, unsigned int len, FILE *f)
{
	int r = 0;
	unsigned char compressed = 0;
	size_t in_len;
#ifdef WITH_LZO
	size_t out_len = len;
#endif
	unsigned char *in;
	//unsigned char *props = MEM_callocN(16 * sizeof(char), "tmp");
  unsigned char *props = new unsigned char[16];

	//ptcache_file_read(pf, &compressed, 1, sizeof(unsigned char));
  fread(&compressed, sizeof(unsigned char), 1, f);
	if (compressed) {
		unsigned int size;
		//ptcache_file_read(pf, &size, 1, sizeof(unsigned int));
    fread(&size, sizeof(unsigned int), 1, f);
		in_len = (size_t)size;
		if (in_len==0) {

		}
		else {
			//in = (unsigned char *)MEM_callocN(sizeof(unsigned char)*in_len, "pointcache_compressed_buffer");
      in = new unsigned char[in_len];
			//ptcache_file_read(pf, in, in_len, sizeof(unsigned char));
      fread(in, sizeof(unsigned char), in_len, f);
#ifdef WITH_LZO
			if (compressed == 1)
				r = lzo1x_decompress_safe(in, (lzo_uint)in_len, result, (lzo_uint *)&out_len, NULL);
#endif
#ifdef WITH_LZMA
			if (compressed == 2) {
				size_t sizeOfIt;
				size_t leni = in_len, leno = len;
				//ptcache_file_read(pf, &size, 1, sizeof(unsigned int));
        fread(&size, sizeof(unsigned int), 1, f);
				sizeOfIt = (size_t)size;
				//ptcache_file_read(pf, props, sizeOfIt, sizeof(unsigned char));
        fread(props, sizeof(unsigned char), sizeOfIt, f);
				r = LzmaUncompress(result, &leno, in, &leni, props, sizeOfIt);
			}
#endif
			//MEM_freeN(in);
      delete in;
		}
	}
	else {
		//ptcache_file_read(pf, result, len, sizeof(unsigned char));
    fread(result, sizeof(unsigned char), len, f);
	}

	//MEM_freeN(props);
  delete props;

	return r;
}

#if 0

typedef struct ModifierData {
	struct ModifierData *next, *prev;

	int type, mode;
	int stackindex, pad;
	char name[64];  /* MAX_NAME */

	/* XXX for timing info set by caller... solve later? (ton) */
	struct Scene *scene;

	char *error;
} ModifierData;

typedef struct SmokeModifierData {
	ModifierData modifier;

	struct SmokeDomainSettings *domain;
	struct SmokeFlowSettings *flow; /* inflow, outflow, smoke objects */
	struct SmokeCollSettings *coll; /* collision objects */
	float time;
	int type;  /* domain, inflow, outflow, ... */
} SmokeModifierData;
#endif


void BPhysBaker::read_smoke_data(FILE *f)
{
  cout<<"reading smoke sim data..."<<endl;

  //WTF DOES THIS BLOCK OF DATA DO?!?!?!
  char unknown_data[11];
  fread(unknown_data, sizeof(char), 11, f);

  char version[5];
  int fluid_fields = -1;
  int active_fields = -1;
  int res[3] = { -1, -1, -1 };
  float dx = -1.0f;

  version[4] = '\0';
  fread(&version[0], sizeof(char), 4, f);

  //fluid info
  fread(&fluid_fields, 1, sizeof(int), f);
  fread(&active_fields, 1, sizeof(int), f);
  fread(&res, 3, sizeof(int), f);
  fread(&dx, 1, sizeof(float), f);

  cout<<"version: "<<version<<endl;
  cout<<"fluid fields: "<<fluid_fields<<endl;
  cout<<"active fields: "<<active_fields<<endl;
  cout<<"resolution: "<<res[0]<<"x"<<res[1]<<"x"<<res[2]<<endl;
  cout<<"dx: "<<dx<<endl;

  unsigned int alloc_res = res[0] * res[1] * res[2];
  const unsigned int out_len = (unsigned int)(alloc_res) * sizeof(float);

  float *shadow_voxels = new float[alloc_res];
  float *density_voxels = new float[alloc_res];
  float *heat_voxels = NULL;
  float *heat_old_voxels = NULL;
  float *flame_voxels = NULL;
  float *fuel_voxels = NULL;
  float *react_voxels = NULL;
  float *color_voxels_r = NULL;
  float *color_voxels_g = NULL;
  float *color_voxels_b = NULL;
  float *velocity_voxels_x = new float[alloc_res];
  float *velocity_voxels_y = new float[alloc_res];
  float *velocity_voxels_z = new float[alloc_res];

  cout<<"reading shadow voxels..."<<endl;
  ptcache_file_compressed_read((unsigned char *)shadow_voxels, out_len, f); //shadow

  cout<<"reading density voxels..."<<endl;
  ptcache_file_compressed_read((unsigned char *)density_voxels, out_len, f); //density

  if(fluid_fields & SM_ACTIVE_HEAT)
  {
    cout<<"reading heat voxels..."<<endl;
    heat_voxels = new float[alloc_res];
    heat_old_voxels = new float[alloc_res];
    ptcache_file_compressed_read((unsigned char *)heat_voxels, out_len, f); //heat
    ptcache_file_compressed_read((unsigned char *)heat_old_voxels, out_len, f); //heat_old
  }
  if(fluid_fields & SM_ACTIVE_FIRE)
  {
    cout<<"reading fire voxels..."<<endl;
    flame_voxels = new float[alloc_res];
    fuel_voxels = new float[alloc_res];
    react_voxels = new float[alloc_res];
    ptcache_file_compressed_read((unsigned char *)flame_voxels, out_len, f); //flame
    ptcache_file_compressed_read((unsigned char *)fuel_voxels, out_len, f); //fuel
    ptcache_file_compressed_read((unsigned char *)react_voxels, out_len, f); //react
  }
  if(fluid_fields & SM_ACTIVE_COLORS)
  {
    cout<<"reading color voxels..."<<endl;
    color_voxels_r = new float[alloc_res];
    color_voxels_g = new float[alloc_res];
    color_voxels_b = new float[alloc_res];
    ptcache_file_compressed_read((unsigned char *)color_voxels_r, out_len, f); //r
    ptcache_file_compressed_read((unsigned char *)color_voxels_g, out_len, f); //g
    ptcache_file_compressed_read((unsigned char *)color_voxels_b, out_len, f); //b
  }

  //velocity
  cout<<"reading velocity voxels..."<<endl;
  ptcache_file_compressed_read((unsigned char *)velocity_voxels_x, out_len, f); //vx
  ptcache_file_compressed_read((unsigned char *)velocity_voxels_y, out_len, f); //vy
  ptcache_file_compressed_read((unsigned char *)velocity_voxels_z, out_len, f); //vz

  if(shadow_voxels)     { delete shadow_voxels; }
  if(density_voxels)    { delete density_voxels; }
  if(heat_voxels)       { delete heat_voxels; }
  if(heat_old_voxels)   { delete heat_old_voxels; }
  if(flame_voxels)      { delete flame_voxels; }
  if(fuel_voxels)       { delete fuel_voxels; }
  if(react_voxels)      { delete react_voxels; }
  if(velocity_voxels_x) { delete velocity_voxels_x; }
  if(velocity_voxels_y) { delete velocity_voxels_y; }
  if(velocity_voxels_z) { delete velocity_voxels_z; }
}
