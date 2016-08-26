#include "switchboard.h"

using namespace Game;

void Switchboard::assign_socket(ChannelSocket src, std::string name)
{
  uint32_t hash_id = Math::hash_value_from_string(name.c_str());
  sockets.insert({ hash_id, src });
}

ChannelSocket *Switchboard::get_socket_handle(uint32_t hash_id)
{
  return &sockets[hash_id];
}