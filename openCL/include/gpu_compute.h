#ifndef __GPU_COMPUTE_H__
#define __GPU_COMPUTE_H__

#if defined(__APPLE__)
#include <OpenCL/opencl.h>
#else
#include <CL/opencl.h>
#endif //__APPLE__

namespace GPUCompute
{

  cl_int cl_check_error(cl_int err);

  class GPUComputeContext
  {
  private:
    bool                      _initialized;

    uint32_t                  _num_elements;
    uint32_t                  _max_elements;
    cl_context                _context;
    cl_device_id              _device_id;

    cl_mem                    _input_buffer;
    cl_mem                    _output_buffer;

    cl_program                _program;
    cl_kernel                 _kernel;
    cl_command_queue          _commands;

  public:
    GPUComputeContext();
    ~GPUComputeContext();

    void init();
    void deinit();

    void load_and_build_kernel(std::string &fname, std::string &kernel_name, std::ostream &log);

    void upload_input_array(void *data);
    void download_results_array(void *results);

    void set_num_elements(const unsigned int e) { _num_elements = e; }
    uint32_t get_num_elements() const { return _num_elements; }

    void set_max_elements(const uint32_t e) { _max_elements = e; }
    uint32_t get_max_elements() const { return _max_elements; }

    void execute();
  };
};

#endif //__GPU_COMPUTE_H__
