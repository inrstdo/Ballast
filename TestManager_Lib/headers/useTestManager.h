#ifndef USE_TESTMANAGER_LIB
namespace Ballast {
  namespace Test
  {
    class TestManager;
  }
}
#define TESTMANAGER_FRIEND friend class Ballast::Test::TestManager;
#else
#define TESTMANAGER_FRIEND
#endif