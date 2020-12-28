#include "./testManager.h"


using namespace Ballast::Test;


int main (void)
{
  TestManager testManager;

  testManager.heterogeneousAllocator_verify_allocate_1();

  return 0;
}