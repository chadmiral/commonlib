#include <iostream>
#include <assert.h>
#include <cstring>

#include "gpu_compute.h"

using namespace GPUCompute;
using namespace std;

GPUComputeContext::GPUComputeContext() :
  _initialized(false),
  _num_elements(0),
  _max_elements(1000)
{}

GPUComputeContext::~GPUComputeContext()
{
  deinit();
}

void GPUComputeContext::init()
{
  bool use_gpu = true;

  //connect to compute device
  int err = clGetDeviceIDs(NULL, use_gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1, &_device_id, NULL);
  assert(err == CL_SUCCESS);

  //create a compute context
  _context = clCreateContext(0, 1, &_device_id, NULL, NULL, &err);
  assert(_context);

  _initialized = true;
}

void GPUComputeContext::deinit()
{
  if(_initialized)
  {
    clReleaseMemObject(_input_buffer);
    clReleaseMemObject(_output_buffer);
    clReleaseProgram(_program);
    clReleaseKernel(_kernel);
    clReleaseCommandQueue(_commands);
    clReleaseContext(_context);
  }
  _initialized = false;
}

void GPUComputeContext::load_and_build_kernel(const char *fname, const char *kernel_name, std::ostream &log)
{
  int err;

  //create command queue
  _commands = clCreateCommandQueue(_context, _device_id, 0, &err);
  assert(_commands);

  log << "Loading OpenCL kernel " << kernel_name << "..." << endl;

  //load the file from disk
  FILE *f = fopen(fname, "r");
  if(f)
  {
    fseek(f, 0, SEEK_END);
    int string_size = ftell(f);
    rewind(f);

    char *kernel_src = (char *)malloc(sizeof(char) * (string_size + 1));
    memset(kernel_src, 0, string_size + 1);
    fread(kernel_src, sizeof(char), string_size, f);

    //create and build the kernel program executable and check for errors
    _program = clCreateProgramWithSource(_context, 1, (const char **) & kernel_src, NULL, &err);
    assert(_program);

    err = clBuildProgram(_program, 0, NULL, NULL, NULL, NULL);
    if(err != CL_SUCCESS)
    {
      size_t len;
      char buffer[2048];
      log<<"GPUCompute: Failed to build kernel program executable!"<<endl;
      clGetProgramBuildInfo(_program, _device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
      log<<buffer<<endl;
    }

    _kernel = clCreateKernel(_program, kernel_name, &err);
    assert(err == CL_SUCCESS && _kernel);

    _input_buffer =  clCreateBuffer(_context, CL_MEM_READ_ONLY,  sizeof(float) * _max_elements, NULL, NULL);
    _output_buffer = clCreateBuffer(_context, CL_MEM_WRITE_ONLY, sizeof(float) * _max_elements, NULL, NULL);

    assert(_input_buffer && _output_buffer);
  }
  else
  {
    log << "GPUCompute: could not open kernel file! (no file handle)" << endl;
  }
}

void GPUComputeContext::upload_input_array(void *data)
{
  // Write our data set into the input array in device memory
  int err = clEnqueueWriteBuffer(_commands, _input_buffer, CL_TRUE, 0, sizeof(float) * _num_elements, data, 0, NULL, NULL);
  if (err != CL_SUCCESS)
  {
    cerr << "Error: Failed to write to source array!" << endl;
  }
}

void GPUComputeContext::download_results_array(void *results)
{
  // Wait for the command commands to get serviced before reading back results
  clFinish(_commands);

  // Read back the results from the device to verify the output
  int err = clEnqueueReadBuffer(_commands, _output_buffer, CL_TRUE, 0, sizeof(float) * _num_elements, results, 0, NULL, NULL);
  if (err != CL_SUCCESS)
  {
    cerr << "Error: Failed to read output array!" << endl;
  }
}

//
//run the actual OpenCL kernel program (input buffer should already be uploaded)
void GPUComputeContext::execute()
{
  int err;

  size_t global;                      // global domain size for our calculation
  size_t local;                       // local domain size for our calculation

  // Set the arguments to our compute kernel
  err = 0;
  err  = clSetKernelArg(_kernel, 0, sizeof(cl_mem), &_input_buffer);
  err |= clSetKernelArg(_kernel, 1, sizeof(cl_mem), &_output_buffer);
  err |= clSetKernelArg(_kernel, 2, sizeof(uint32_t), &_num_elements);
  if (err != CL_SUCCESS)
  {
    cerr<<"Error: Failed to set kernel arguments!"<<endl;
  }

  // Get the maximum work group size for executing the kernel on the device
  err = clGetKernelWorkGroupInfo(_kernel, _device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
  if (err != CL_SUCCESS)
  {
   cerr<<"Error: Failed to retrieve kernel work group info!"<<endl;
  }

  // Execute the kernel over the entire range of our 1d input data set
  // using the maximum number of work group items for this device
  global = _num_elements;
  err = clEnqueueNDRangeKernel(_commands, _kernel, 1, NULL, &global, &local, 0, NULL, NULL);
  if (err)
  {
    cerr<<"local size: "<<local<<endl;
    cerr<<"global size: "<<global<<endl;
    cerr<<"Error: Failed to execute kernel! err: "<<err<<endl;
  }

  // Wait for the command commands to get serviced before reading back results
  clFinish(_commands);
}
