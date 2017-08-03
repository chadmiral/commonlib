__kernel void test(__global float2* input, __global float2* output, const unsigned int count)
{
   int i = get_global_id(0);
   if(i < count)
       output[i] = input[i] * input[i];
}
