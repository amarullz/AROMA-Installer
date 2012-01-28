/*#include <stdlib.h>
#include "amath.h"

int amath_round(float x){
  float v = (float)((int) x);
  if ((v+0.5)<x){
    return ((int) (v+1));
  }
  return (int) v;
}
int amath_floor(float x){
  return (int) x;
}
int amath_ceil(float x){
  float v = (float)((int) x);
  if (v<x){
    return ((int) (v+1));
  }
  return (int) v;
}
float amath_sin(float x)
{
    if (x>6.28318530717958647692){
      return amath_sin(x-6.28318530717958647692);
    }
    else if (x<0){
      return amath_sin(x+6.28318530717958647692);
    }
    const float B   = 1.2732395447351626861521414298709;
    const float C   = -0.40528473456935108577619987817378;
    const float P   = 0.225;
    // unsigned char D = 0;
    if (x>3.14159265358979323846){
      // D = 1;
      x = 3.14159265358979323846-x;
    }
    float y = B * x + C * x * abs(x);
    y = P * (y * abs(y) - y) + y;
    // if (D==1) return -y;
    return y;
}
float amath_cos(float x)
{
    return amath_sin(x + 1.57079632679489661923);
}*/