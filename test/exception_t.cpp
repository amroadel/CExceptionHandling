//#include"../libsupcpp.cpp"
#include <stdio.h> 
#include "exception_t.h"
void exception_func()
{
    try
  {
      printf("try\n");
  }
  catch (...)
  {
      printf("catch\n");
  }
}