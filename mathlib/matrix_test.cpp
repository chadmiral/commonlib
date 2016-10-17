#include <iostream>
#include "matrix.h"

using namespace std;
using namespace Math;

int main(int argc, char **argv)
{
  srand(time(NULL));
  Matrix4x4 a(
    random(-10.0f, 10.0f), random(-10.0f, 10.0f), random(-10.0f, 10.0f), random(-10.0f, 10.0f),
    random(-10.0f, 10.0f), random(-10.0f, 10.0f), random(-10.0f, 10.0f), random(-10.0f, 10.0f),
    random(-10.0f, 10.0f), random(-10.0f, 10.0f), random(-10.0f, 10.0f), random(-10.0f, 10.0f),
    random(-10.0f, 10.0f), random(-10.0f, 10.0f), random(-10.0f, 10.0f), random(-10.0f, 10.0f)
  );

  cout<<a<<endl;
  Matrix4x4 b = a;
  b.invert();
  cout<<endl<<b<<endl;
  Matrix4x4 c = a * b;
  cout<<endl<<c<<endl;

  return 0;
}
