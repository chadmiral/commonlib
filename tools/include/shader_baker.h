#ifndef __SHADER_BAKER_H__
#define __SHADER_BAKER_H__

namespace Tool
{
  class ShaderBaker
  {
    public:
      ShaderBaker();
      ~ShaderBaker();
      void init() {}
      void bake(mxml_node_t *tree, std::string output_fname, std::string tabs = "");
    private:
      void parse_shader_block(std::string fname);
  };
};

#endif //__SHADER_BAKER_H__
