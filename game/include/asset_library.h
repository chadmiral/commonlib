#ifndef __ASSET_LIBRARY_H__
#define __ASSET_LIBRARY_H__

#include <vector>
#include <unordered_map>
#include <string>
#include <sys/stat.h>

#include "shader.h"
#include "texture.h"
#include "timer.h"
#include "layout.h"
#include "static_mesh.h"
#include "skeleton.h"
#include "lens_flare.h"

namespace Game
{
  enum GameAssetType
  {
    SHADER_ASSET,
    TEXTURE_ASSET,
    TEXTURE_3D_ASSET,
    MATERIAL_ASSET,
    MESH_ASSET,
    SKELETON_ASSET,
    ANIMATION_ASSET,
    UI_LAYOUT_ASSET,
    LENS_FLARE_ASSET,
    AUDIO_ASSET
  };

  class GameAsset
  {
  public:
    GameAsset(GameAssetType t);
    ~GameAsset() {}

    virtual void reload_from_disk() = 0;

    GameAssetType    type;

    //for file monitoring
    struct FileMonitorInfo
    {
      uint32_t      mod_time;
      std::string   fname;
    };
    std::vector<FileMonitorInfo> files_monitored;
  };

  class ShaderAsset : public GameAsset
  {
  public:
    ShaderAsset() : GameAsset(SHADER_ASSET) {}
    ~ShaderAsset() {}

    virtual void reload_from_disk();
  
    Graphics::Shader *s;
  };
  
  class TextureAsset : public GameAsset
  {
  public:
    TextureAsset() : GameAsset(TEXTURE_ASSET) {}
    ~TextureAsset() {}

    virtual void reload_from_disk();

    Graphics::Texture2D *t;
  };

  class Texture3DAsset : public GameAsset
  {
  public:
    Texture3DAsset() : GameAsset(TEXTURE_3D_ASSET) {}
    ~Texture3DAsset() {}

    virtual void reload_from_disk() { assert(false); }

    Graphics::Texture3D *t;
  };

  class MaterialAsset : public GameAsset
  {
  public:
    MaterialAsset() : GameAsset(MATERIAL_ASSET) {}
    ~MaterialAsset() {}

    virtual void reload_from_disk() { assert(false); }

    Graphics::Material *m;
  };

  class MeshAsset : public GameAsset
  {
  public:
    MeshAsset() : GameAsset(MESH_ASSET) {}
    ~MeshAsset() {}

    virtual void reload_from_disk() { assert(false); }

    Graphics::StaticMesh *m;
  };

  class SkeletonAsset : public GameAsset
  {
  public:
    SkeletonAsset() : GameAsset(SKELETON_ASSET) {}
    ~SkeletonAsset() {}

    virtual void reload_from_disk() { assert(false); }

    Animation::Skeleton *s;
  };

  class AnimationAsset : public GameAsset
  {
  public:
    AnimationAsset() : GameAsset(ANIMATION_ASSET) {}
    ~AnimationAsset() {}

    virtual void reload_from_disk() { assert(false); }
    Animation::SkeletonAnimation *a;
  };

  class UILayoutAsset : public GameAsset
  {
  public:
    UILayoutAsset() : GameAsset(UI_LAYOUT_ASSET) {}
    ~UILayoutAsset() {}

    virtual void reload_from_disk() {}

    UI::Layout *l;
  };

  class LensFlareAsset : public GameAsset
  {
  public:
    LensFlareAsset() : GameAsset(LENS_FLARE_ASSET) {}
    ~LensFlareAsset() {}

    virtual void reload_from_disk() { assert(false); }

    Graphics::LensFlare *lf;
  };
  
  class AssetLibrary
  {
  protected:
    std::unordered_map<uint32_t, GameAsset *> lib;
    UI::Timer file_monitor_timer;
    float file_monitor_interval_s;
  public:
    AssetLibrary() { file_monitor_interval_s = 1.0f; }
    ~AssetLibrary() {}

    void add_asset(uint32_t hash_id, GameAsset *a);
    GameAsset *retrieve_asset(std::string name);
    Graphics::Shader *retrieve_shader(std::string name);
    Graphics::Texture2D *retrieve_texture(std::string name);
    Graphics::Texture3D *retrieve_texture_3d(std::string name);
    Graphics::Material *retrieve_material(std::string name);
    Graphics::StaticMesh *retrieve_mesh(std::string name);
    Animation::Skeleton *retrieve_skeleton(std::string name);
    Animation::SkeletonAnimation *retrieve_animation(std::string name);
    Graphics::LensFlare *retrieve_lens_flare(std::string name);

    void init();
    void simulate(const double game_time, const double frame_time);
  };
}

#endif //__ASSET_LIBRARY_H__
