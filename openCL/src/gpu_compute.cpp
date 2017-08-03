#include <iostream>
#include <assert.h>
#include <cstring>
#include <string>

#include "gpu_compute.h"

using namespace GPUCompute;
using namespace std;

cl_int GPUCompute::cl_check_error(cl_int err)
{
  switch(err)
  {
    case CL_INVALID_PROGRAM:
      cerr << "CL_INVALID_PROGRAM: Program is not a valid program object." << endl;
      break;
    case CL_INVALID_PROGRAM_EXECUTABLE:
      cerr << "CL_INVALID_PROGRAM_EXECUTABLE: There is no successfully built executable for program." << endl;
      break;
    case CL_INVALID_KERNEL_NAME:
      cerr << "CL_INVALID_KERNEL_NAME: kernel_name is not found in program" << endl;
      break;
    case CL_INVALID_KERNEL_DEFINITION:
      cerr << "CL_INVALID_KERNEL_DEFINITION: The function definition for __kernel function given by kernel_name such as the number of arguments, the argument types are not the same for all devices for which the program executable has been built." << endl;
      break;
    case CL_INVALID_VALUE:
      cerr << "CL_INVALID_VALUE: kernel_name is NULL." << endl;
      break;
    case CL_OUT_OF_HOST_MEMORY:
      cerr << "CL_OUT_OF_HOST_MEMORY: There is a failure to allocate resources required by the OpenCL implementation on the host." << endl;
      break;
    case CL_INVALID_PLATFORM: //-32
      cerr << "an invalid platform was given" << endl;
      break;
    case 	CL_INVALID_DEVICE: //-33
      cerr << "devices contains an invalid device or are not associated with the specified platform." << endl;
      break;

    case CL_INVALID_WORK_GROUP_SIZE:
      cerr << "CL_INVALID_WORK_GROUP_SIZE: if local_work_size is specified and number of work-items specified by global_work_size is not evenly divisable by size of work-group given by local_work_size or does not match the work-group size specified for kernel using the __attribute__ ((reqd_work_group_size(X, Y, Z))) qualifier in program source." << endl;
      cerr << "if local_work_size is specified and the total number of work-items in the work-group computed as local_work_size[0] *… local_work_size[work_dim – 1] is greater than the value specified by CL_DEVICE_MAX_WORK_GROUP_SIZE in the table of OpenCL Device Queries for clGetDeviceInfo.if local_work_size is NULL and the __attribute__ ((reqd_work_group_size(X, Y, Z))) qualifier is used to declare the work-group size for kernel in the program source." << endl;
      break;
  }
  return err;
}

GPUComputeContext::GPUComputeContext(uint32_t max_elements) :
  _initialized(false),
  _num_elements(0),
  _max_elements(max_elements)
{}

GPUComputeContext::~GPUComputeContext()
{
  deinit();
}

void GPUComputeContext::init()
{
  bool use_gpu = true;

  //create a compute context
  int err;

  //query the number of platforms available
  cl_uint num_platforms;
  err = clGetPlatformIDs(0, NULL, &num_platforms);
  cl_check_error(err);

  //query the platform ids
  cl_platform_id *platforms = new cl_platform_id[num_platforms];
  err = clGetPlatformIDs(num_platforms, platforms, NULL);
  cl_check_error(err);

  cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (long)platforms[0], 0 };
  _context = clCreateContextFromType(properties, CL_DEVICE_TYPE_GPU, NULL, NULL, &err);
  cl_check_error(err);

  /*
  _context = clCreateContext(0, 1, &_device_id, NULL, NULL, &err);
  err = cl_check_error(err);
  assert(_context);
  */

  //size_t size
  //err = clGetContextInfo(clGPUContext, CL_CONTEXT_DEVICES, 0, NULL, &dataBytes);

  //connect to compute device
  err = clGetDeviceIDs(platforms[0], use_gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1, &_device_id, NULL);
  cl_check_error(err);
  assert(err == CL_SUCCESS);

  _initialized = true;

  delete [] platforms;
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

void GPUComputeContext::load_and_build_kernel(std::string &fname, std::string &kernel_name, std::ostream &log)
{
  cl_int err;

  //create command queue
  _commands = clCreateCommandQueue(_context, _device_id, 0, &err);
  assert(_commands);

  log << "Loading OpenCL kernel " << kernel_name << "..." << endl;

  //load the file from disk
  FILE *f = fopen(fname.c_str(), "r");
  if(f)
  {
    fseek(f, 0, SEEK_END);
    int string_size = ftell(f);
    rewind(f);

    char *kernel_src = (char *)malloc(sizeof(char) * (string_size + 1));
    memset(kernel_src, 0, string_size + 1);
    fread(kernel_src, sizeof(char), string_size, f);

    //create and build the kernel program executable and check for errors
    _program = clCreateProgramWithSource(_context, 1, (const char **) &kernel_src, NULL, &err);
    assert(_program);

    err = clBuildProgram(_program, 0, NULL, NULL, NULL, NULL);
    cl_check_error(err);
    if(err != CL_SUCCESS)
    {
      size_t len;
      char buffer[2048];
      log << "GPUCompute: Failed to build kernel program executable!" << endl;
      clGetProgramBuildInfo(_program, _device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
      log << buffer << endl;
    }

    _kernel = clCreateKernel(_program, kernel_name.c_str(), &err);
    err = cl_check_error(err);

    assert(err == CL_SUCCESS);
    assert(_kernel);

    _input_buffer =  clCreateBuffer(_context, CL_MEM_READ_ONLY,  sizeof(float) * _num_elements, NULL, NULL);
    _output_buffer = clCreateBuffer(_context, CL_MEM_WRITE_ONLY, sizeof(float) * _num_elements, NULL, NULL);

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
  // Wait for the commands to get serviced before reading back results
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
    cerr << "Error: Failed to set kernel arguments!" << endl;
  }

  // Get the maximum work group size for executing the kernel on the device
  err = clGetKernelWorkGroupInfo(_kernel, _device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
  if (err != CL_SUCCESS)
  {
   cerr << "Error: Failed to retrieve kernel work group info!" << endl;
  }
  global = local * (_num_elements / local + 1);
  if(global < local) { global += local; }

  cout << "local size: " << local << endl;
  cout << "global size: " << global << endl;

  // Execute the kernel over the entire range of our 1d input data set
  // using the maximum number of work group items for this device

  err = clEnqueueNDRangeKernel(_commands, _kernel, 1, NULL, &global, &local, 0, NULL, NULL);
  if (err)
  {
    cerr << "local size: " << local << endl;
    cerr << "global size: " << global << endl;
    cerr << "Error: Failed to execute kernel!" << endl;
    cl_check_error(err);
  }

  // Wait for the command commands to get serviced before reading back results
  //clFinish(_commands);
}
