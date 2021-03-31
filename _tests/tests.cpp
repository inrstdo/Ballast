#include <iostream>
#include <list>
#include <string>

#include "./testManager.h"


using namespace Ballast::Test;


int main (void)
{
  std::list<std::string> errors;
  std::string error;
  
  auto heterogeneousTests = TestManager::s_heterogeneousTests;

  for(unsigned i = 0; heterogeneousTests[i]; ++i)
  {
    error = heterogeneousTests[i]();
    if(error.length())
    {
      errors.push_back(std::string(error));
    }
  }

  std::list<std::string>::iterator errorsIterator = errors.begin();

  for(; errorsIterator != errors.end(); ++errorsIterator)
  {
    std::cout << *errorsIterator << std::endl;
  }

  return 0;
}