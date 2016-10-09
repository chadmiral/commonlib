#ifndef __VR_H__
#define __VR_H__

//TODO:
// InitGL() - distortion init, etc... vAO


//#define __USE_OCULUS_SDK

#define VR_LEFT_EYE  0
#define VR_RIGHT_EYE 1

#if defined (_USE_OCULUS_SDK)
#include "OVR_CAPI_GL.h"
#endif

#include <openvr.h>

#include "sdl_game.h"
#include "camera.h"
#include "shader.h"

namespace VR
{
  struct VertexDataLens
  {
    Math::Float2 position;
    Math::Float2 texCoordRed;
    Math::Float2 texCoordGreen;
    Math::Float2 texCoordBlue;
  };

  class VRContext
  {
  private:
    GameContext *game_context;
    vr::IVRSystem *hmd;
    vr::TrackedDevicePose_t device_poses[vr::k_unMaxTrackedDeviceCount];

    float near_clip;
    float far_clip;

    GLuint eye_fbo[2];          //frame buffer object id
    GLuint eye_dbo[2];          //depth buffer object id
    GLuint eye_tex[2];          //eye render texture id
    GLuint eye_resolve_fbo[2];  //resolve fbo id
    GLuint eye_resolve_tex[2];  //resolve tex id
    
    GLuint lens_vao;            //vertex array object for lens distortion
    GLuint lens_vbo;
    GLuint lens_ibo;

    Graphics::Shader lens_shader;

    uint32_t num_lens_indices;
    
    uint32_t render_target_dim[2];
    uint32_t window_dim[2];

#if defined (_USE_OCULUS_SDK)
    ovrHmdDesc            hmd_desc;
    ovrSession            ovr_session;
    ovrSizei              window_size;

    ovrTextureSwapChain   eye_tex_chain[2];
    ovrSizei              eye_tex_size[2];
    //GLuint                eye_tex[2];
    //GLuint                eye_fbo[2];
    GLuint                depth_tex[2];

    GLuint                mirror_fbo;

    double                sensor_sample_time;
    ovrPosef              eye_render_pose[2];
    long long             frame_index;
#endif //_USE_OCULUS_SDK
  public:
    VRContext(GameContext *gc);
    ~VRContext() {}

    void init();
    void init_gl();
    void init_render_models();
    void deinit();

    void bind(SDLGame *game);
    void retrieve_eye_poses();
    void get_eye_camera(const uint32_t eye, Graphics::Camera *cam) const;
    void render_capture(const uint32_t eye);
    void render_release(const uint32_t eye);
    void finalize_render();

    void simulate(const double game_time, const double frame_time);
  private:
    void create_eye_texture(const int eye_idx);
    void create_eye_depth_texture(const int eye_idx);

    void init_compositor();

    void setup_cameras();
    void setup_distortion_geo();
    void setup_distortion_shader();
    void setup_stereo_render_targets();

    void render_stereo_targets();
    void render_distortion();

    void update_hmd_matrix_pose();
  };
};

#endif //__VR_H__