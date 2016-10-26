#include <iostream>
#include <assert.h>
#include <cstring>

#include "gpu_compute.h"

using namespace GPUCompute;
using namespace std;

GPUComputeContext::GPUComputeContext()
{
  initialized = false;
  num_elements = 0;
  max_elements = 1000;
}

GPUComputeContext::~GPUComputeContext()
{
  deinit();
}

void GPUComputeContext::init()
{
  bool use_gpu = true;

  //connect to compute device
  int err = clGetDeviceIDs(NULL, use_gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1, &device_id, NULL);
  assert(err == CL_SUCCESS);

  //create a compute context
  context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
  assert(context);

  initialized = true;
}

void GPUComputeContext::deinit()
{
  if(initialized)
  {
    clReleaseMemObject(input_buffer);
    clReleaseMemObject(output_buffer);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(commands);
    clReleaseContext(context);
  }
  initialized = false;
}

void GPUComputeContext::load_and_build_kernel(const char *fname, const char *kernel_name)
{
  int err;

  //create command queue
  commands = clCreateCommandQueue(context, device_id, 0, &err);
  assert(commands);

  cout<<"Loading OpenCL kernel "<<kernel_name<<"..."<<endl;

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
    program = clCreateProgramWithSource(context, 1, (const char **) & kernel_src, NULL, &err);
    assert(program);

    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if(err != CL_SUCCESS)
    {
      size_t len;
      char buffer[2048];
      cerr<<"GPUCompute: Failed to build kernel program executable!"<<endl;
      clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
      cerr<<buffer<<endl;
    }

    kernel = clCreateKernel(program, kernel_name, &err);
    assert(err == CL_SUCCESS && kernel);

    input_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * max_elements, NULL, NULL);
    output_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * max_elements, NULL, NULL);

    assert(input_buffer && output_buffer);
  }
  else
  {
    cerr<<"GPUCompute: could not open kernel file! (no file handle)"<<endl;
  }
}

void GPUComputeContext::upload_input_array(void *data)
{
  int err;

  // Write our data set into the input array in device memory
  err = clEnqueueWriteBuffer(commands, input_buffer, CL_TRUE, 0, sizeof(float) * num_elements, data, 0, NULL, NULL);
  if (err != CL_SUCCESS)
  {
    cerr<<"Error: Failed to write to source array!"<<endl;
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
  err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_buffer);
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &output_buffer);
  err |= clSetKernelArg(kernel, 2, sizeof(uint32_t), &num_elements);
  if (err != CL_SUCCESS)
  {
    cerr<<"Error: Failed to set kernel arguments!"<<endl;
  }

  // Get the maximum work group size for executing the kernel on the device
  err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
  if (err != CL_SUCCESS)
  {
   cerr<<"Error: Failed to retrieve kernel work group info!"<<endl;
  }

  // Execute the kernel over the entire range of our 1d input data set
  // using the maximum number of work group items for this device
  global = num_elements;
  err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
  if (err)
  {
    cerr<<"Error: Failed to execute kernel! err: "<<err<<endl;
  }

  // Wait for the command commands to get serviced before reading back results
  clFinish(commands);
}
