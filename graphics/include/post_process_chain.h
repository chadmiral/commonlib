#pragma once

#include "render_surface.h"

namespace Graphics
{
  
  class PostProcessNode
  {
    friend class PostProcessChain;
  private:
    RenderSurface                   *_surface;
    //std::vector<PostProcessNode *>   _inputs;
    //std::vector<PostProcessNode *>   _outputs;
    
  public:
    PostProcessNode() {}
    ~PostProcessNode() {}
  };
  
  class PostProcessChain
  {
  private:
    PostProcessNode *_head;
  public:
    PostProcessChain() {}
    ~PostProcessChain() {}
  };
};
