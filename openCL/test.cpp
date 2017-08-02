////////////////////////////////////////////////////////////////////////////////

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>

#include "gpu_compute.h"

using namespace GPUCompute;
using namespace std;

int main(int argc, char** argv)
{
  std::string kernel_fname("data/test.cl");
  std::string kernel_name("test");

  uint32_t data_count = 317;
  if(argc > 1)
  {
    data_count = atoi(argv[1]);
  }

  float *data = new float[data_count];
  float *results = new float[data_count];

  for(uint32_t i = 0; i < data_count; i++)
  {
    data[i] = (float)i;
  }

  GPUComputeContext gpu_cc(data_count);
  gpu_cc.set_num_elements(data_count);
  gpu_cc.set_max_elements(data_count);
  gpu_cc.init();
  gpu_cc.load_and_build_kernel(kernel_fname, kernel_name, cout);
  gpu_cc.upload_input_array(data);
  gpu_cc.execute();
  gpu_cc.download_results_array(results);

  int num_correct = 0;
  for(uint32_t i = 0; i < data_count; i++)
  {
    //cout << results[i] << endl;
    if(data[i] * data[i] == results[i])
    {
      num_correct++;
    }
  }
  cout << "checking results..." << endl;
  cout << "\t" << num_correct << "/" << data_count << " correct." << endl;

  return 0;
}
