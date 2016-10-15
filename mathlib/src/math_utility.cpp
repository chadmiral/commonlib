#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include "math_utility.h"

//#define M_PI 3.14159f

using namespace Math;

/*float Math::cerp(float y1, float y2, float mu)
{
   double mu2;

   mu2 = (1.0f - cos(mu * M_PI)) / 2.0f;
   return (y1  *(1.0f - mu2) + y2 * mu2);
}
*/

/*float Math::lerp(float y1, float y2, float mu)
{
   return (y1 * (1.0f - mu) + y2 * mu);
}*/

float Math::clamp(float x, float a, float b)
{
	float low = std::min(a, b);
	float hi = std::max(a, b);
	if(x < low) x = low;
	if(x > hi) x = hi;
  
	return x;
}

#if 0
/*inline */float Math::max(float a, float b)
{
	return (a > b) ? a : b;
}

/*inline*/ float Math::min(float a, float b)
{
	return (a < b) ? a : b;
}
#endif

float Math::random(float a, float b)
{
	return a + (b - a) * ((rand() % RAND_MAX) / (float)RAND_MAX);
}

int Math::random(int a, int b)
{
	return a + rand() % abs(b - a);
}

/*
float Math::degrees_to_radians(float d)
{
	return (d / 180.0f) * M_PI;
}


float Math::radians_to_degrees(float r)
{
	return (r / M_PI) * 180.0f;
}*/

uint32_t Math::hash_value_from_string(const char *str)
{
  uint32_t hash = 5381;
  int c;

  while((c = *str++))
  {
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }

  return hash;
}
