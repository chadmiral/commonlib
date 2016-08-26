#pragma once

#include <unordered_map>
#include "math_utility.h"

//signal chain

namespace Game
{
  enum ChannelSocketType
  {
    CHANNEL_SOCKET_FLOAT,
    CHANNEL_SOCKET_DOUBLE,
    CHANNEL_SOCKET_FLOAT2,
    CHANNEL_SOCKET_FLOAT3
  };

  class ChannelSocket
  {
  protected:
    //ChannelSocket *input, *output;
    ChannelSocketType socket_type;
  public:
    ChannelSocket() {}
    ~ChannelSocket() {}
  };

  class ChannelSocketFloat : public ChannelSocket
  {
  private:
    float *source;
  public:
    ChannelSocketFloat(float *src) { source = src; socket_type = CHANNEL_SOCKET_FLOAT; }
    ~ChannelSocketFloat() {}
  };

  class ChannelSocketDouble : public ChannelSocket
  {
  private:
    double *source;
  public:
    ChannelSocketDouble(double *src) { source = src; socket_type = CHANNEL_SOCKET_DOUBLE; }
    ~ChannelSocketDouble() {}
  };

  /*
  class ChannelSocketFloat2 : public ChannelSocket
  {
  private:
    Math::Float2 *source;
  public:
    ChannelSocketFloat3(Math::Float2 *src) { source = src; socket_type = CHANNEL_SOCKET_FLOAT2; }
    ~ChannelSocketFloat3() {}
  };
  */

  class ChannelSocketFloat3 : public ChannelSocket
  {
  private:
    Math::Float3 *source;
  public:
    ChannelSocketFloat3(Math::Float3 *src) { source = src; socket_type = CHANNEL_SOCKET_FLOAT3; }
    ~ChannelSocketFloat3() {}
  };

  class Switchboard
  {
  private:
    std::unordered_map<uint32_t, ChannelSocket> sockets;   // hashed string map of variables
  public:
    Switchboard() {}
    ~Switchboard() {}

    void assign_socket(ChannelSocket socket, std::string name);
    ChannelSocket *get_socket_handle(uint32_t hash_id);
  };
};