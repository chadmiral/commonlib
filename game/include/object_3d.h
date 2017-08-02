#ifndef __OBJECT_3D_H__
#define __OBJECT_3D_H__

#include "platform_gl.h"

#include "math_utility.h"
#include "quaternion.h"
#include "matrix.h"


#define OBJECT_FLAG_PHYSICS_XYZ       (1<<0)   //should we simulate xyz physics?
#define OBJECT_FLAG_PHYSICS_ROT       (1<<1)   //should we simulate rotational physics?

namespace Game
{
  class Object3D
  {
  protected:
    uint32_t            _object_flags;
    uint32_t            _type;    // to be defined by client
    void               *_data;   // to be filled by client

    Math::Float3        _pos;
    Math::Float3        _vel;
    Math::Float3        _scale;

    double              _birth_time;

    Math::Quaternion    _ang_vel_quat;
    Math::Quaternion    _rot_quat;
    //Math::Matrix3x3     _rot;

  public:
    Object3D();
    ~Object3D() {}

    virtual void init(const double game_time);
    virtual void simulate(const double game_time, const double frame_time);
    virtual void render(const double game_time);

    void apply_flag(uint32_t f) { _object_flags = _object_flags | f; }
    void clear_flag(uint32_t f) { _object_flags = _object_flags & ~f; }
    void toggle_flag(uint32_t f) { _object_flags = _object_flags ^ f; }
    bool is_flag_set(uint32_t f) { return ((_object_flags & f) != 0); }

    void set_type(const uint32_t t) { _type = t; }
    uint32_t get_type() const { return _type; }
    void set_data(void *d) { _data = d; }
    void *get_data() { return _data; }

    void set_pos(const Math::Float3 &p) { _pos = p; }
    void set_vel(const Math::Float3 &v) { _vel = v; }
    void set_scale(const Math::Float3 &s) { _scale = s; }
    void set_angular_vel(const Math::Float3 &axis, const float theta) { _ang_vel_quat.rotation_from_axis_angle(axis, theta); }

    double get_age(const double gt) const { return gt - _birth_time; }
    double get_birth_time() const { return _birth_time; }

    Math::Float3 get_pos() { return _pos; }
    Math::Float3 get_vel() { return _vel; }
    Math::Float3 get_scale() { return _scale; }
    //Math::Quaternion get_orientation() { return rot; }
    //Math::Matrix3x3 get_orientation() const { return _rot; }
    //void set_orientation(const Math::Matrix3x3 &m) { _rot = m; }
  };
};

#endif //__OBJECT_3D_H__
