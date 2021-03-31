#ifndef TESTMANAGER_H
#define TESTMANAGER_H


#include <string>


#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define TEST_FAILURE(reason) getTestFailureMessage(__FILE__, TOSTRING(__LINE__), reason)
#define TEST_NOT_IMPLEMENTED() getTestNotImplementedMessage(__FILE__, TOSTRING(__LINE__), __FUNCTION__)


namespace Ballast {
  namespace Test {


    struct TestManager
    {
        static const std::string getTestFailureMessage(const char* const fileName, const char* const lineNumber, const char* const reason);
        static const std::string getTestNotImplementedMessage(const char* const fileName, const char* const lineNumber, const char* const functionName);

        // HeterogeneousAllocator Tests
        static const std::string heterogeneousAllocator_constructor_shouldInitializeAllMembers();
        static const std::string heterogeneousAllocator_instanceAllocation_shouldCreateInstancesAtCorrectLocation();
        static const std::string heterogeneousAllocator_instanceAllocation_shouldNotAllocateOutOfBoundsWhenSizeOfInstanceIsExact();
        static const std::string heterogeneousAllocator_instanceAllocationAndDeletion_shouldUpdateUsedAndFreeListCorrectly();
        static const std::string heterogeneousAllocator_instanceAllocationAndDeletion_shouldConsolidateBlocks();
        static const std::string heterogeneousAllocator_instanceAllocationAndDeletion_shouldUpdateFreeListDuringConsolidation();
        static const std::string heterogeneousAllocator_instanceAllocationAndDeletion_shouldUpdateAvailableBlocksCorrectly();
        static const std::string heterogeneousAllocator_instanceAllocationAndDeletion_shouldUpdateAvailableSizeCorrectly();
        static const std::string heterogeneousAllocator_instanceAllocationAndDeletion_shouldCheckForAllPossibleConsoliations();
        static const std::string heterogeneousAllocator_instanceAllocationAndDeletion_shouldReuseBlocks();
        static const std::string heterogeneousAllocator_instanceAllocationAndDeletion_shouldUseMultiplePages();
        static const std::string heterogeneousAllocator_instanceAllocationAndDeletion_shouldBeFaster();
        static const std::string heterogeneousAllocator_arrayAllocation_shouldCreateInstancesAtCorrectLocation();
        
        static const std::string (*s_heterogeneousTests[])();
    };


  }
}


#endif
