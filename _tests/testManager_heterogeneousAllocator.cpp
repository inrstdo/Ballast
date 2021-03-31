#include <ostream>
#include <string>
#include <string.h>

#include "./testManager.h"
#include "../TestManager_Lib/headers/useTestManager.h"

#include "../MemoryManagement_Lib/headers/heterogeneousAllocator.h"


using namespace Ballast::MemoryManagement;


namespace Ballast {
  namespace Test {

    
    struct SmallStruct
    {
      static HeterogeneousAllocator* s_allocator;
      int m_variableOne,
          m_variableTwo,
          m_variableThree;


      static void InitializeAllocator(HeterogeneousAllocator* allocator)
      {
        SmallStruct::s_allocator = allocator;
      }


      void* operator new(size_t size)
      {
        return reinterpret_cast<void*>(SmallStruct::s_allocator->allocate<SmallStruct>(size));
      }


      void* operator new[](size_t size)
      {
        return reinterpret_cast<void*>(SmallStruct::s_allocator->allocate<SmallStruct>(size));
      }


      void operator delete(void* memory)
      {
        SmallStruct::s_allocator->deallocate<SmallStruct>(reinterpret_cast<SmallStruct*>(memory));
      }


      void operator delete[](void* memory)
      {
        SmallStruct::s_allocator->deallocate<SmallStruct>(reinterpret_cast<SmallStruct*>(memory));
      }
    };


    HeterogeneousAllocator* SmallStruct::s_allocator = 0;


    struct BigStruct
    {
      static HeterogeneousAllocator* s_allocator;
      int m_integerArray[10];
      float m_floatArray[10];
      char m_charArray[64];


      static void InitializeAllocator(HeterogeneousAllocator* allocator)
      {
        BigStruct::s_allocator = allocator;
      }


      void* operator new(size_t size)
      {
        return reinterpret_cast<void*>(BigStruct::s_allocator->allocate<BigStruct>(size));
      }


      void* operator new[](size_t size)
      {
        return reinterpret_cast<void*>(BigStruct::s_allocator->allocate<BigStruct>(size));
      }


      void operator delete(void* memory)
      {
        BigStruct::s_allocator->deallocate<BigStruct>(reinterpret_cast<BigStruct*>(memory));
      }


      void operator delete[](void* memory)
      {
         // 0 for the size because there's no way to know the count
        BigStruct::s_allocator->deallocate<BigStruct>(reinterpret_cast<BigStruct*>(memory));
      }
    };


    HeterogeneousAllocator* BigStruct::s_allocator = 0;


    struct FancyStruct
    {
      static HeterogeneousAllocator* s_allocator;
      char* const c_string;


      FancyStruct(const char* const string) :
        c_string(FancyStruct::s_allocator->allocate<char>(strlen(string) + 1))
      {
        strcpy(c_string, string);
      }


      ~FancyStruct()
      {
        FancyStruct::s_allocator->deallocate(c_string);
      }


      static void InitializeAllocator(HeterogeneousAllocator* allocator)
      {
        FancyStruct::s_allocator = allocator;
      }


      void* operator new(size_t size)
      {
        return reinterpret_cast<void*>(FancyStruct::s_allocator->allocate<BigStruct>(size));
      }


      void* operator new[](size_t size)
      {
        return reinterpret_cast<void*>(FancyStruct::s_allocator->allocate<BigStruct>(size));
      }


      void operator delete(void* memory)
      {
        FancyStruct::s_allocator->deallocate<BigStruct>(reinterpret_cast<BigStruct*>(memory));
      }


      void operator delete[](void* memory)
      {
        FancyStruct::s_allocator->deallocate<BigStruct>(reinterpret_cast<BigStruct*>(memory));
      }
    };


    HeterogeneousAllocator* FancyStruct::s_allocator = 0;
    
    
    const std::string TestManager::heterogeneousAllocator_constructor_shouldInitializeAllMembers()
    {
      const int PAGE_SIZE = 512;

      HeterogeneousAllocator allocator(PAGE_SIZE);

      if(allocator.c_pageList == 0)
      {
        return TEST_FAILURE("HeterogeneousAllocator::c_pageList not initialized");
      }

      if(allocator.c_pageSize != PAGE_SIZE)
      {
        return TEST_FAILURE("HeterogeneousAllocator::c_pageSize not initialized");
      }

      if(allocator.c_pageList->c_pageSize != PAGE_SIZE)
      {
        return TEST_FAILURE("HeterogeneousAllocator::Page::c_pageSize not initialized");
      }

      if(allocator.c_pageList->m_parentBlock == 0)
      {
        return TEST_FAILURE("HeterogeneousAllocator::Page::m_parentBlock not initialized");
      }

      if(allocator.c_pageList->m_freeList == 0)
      {
        return TEST_FAILURE("HeterogeneousAllocator::Page::m_freeList not initialized");
      }

      if(allocator.c_pageList->m_usedList != 0)
      {
        return TEST_FAILURE("HeterogeneousAllocator::Page::m_usedList not initialized");
      }

      if(allocator.c_pageList->m_next != 0)
      {
        return TEST_FAILURE("HeterogeneousAllocator::Page::m_next not initialized");
      }

      if(allocator.c_pageList->m_freeList != allocator.c_pageList->m_parentBlock)
      {
        return TEST_FAILURE("HeterogeneousAllocator::Page::m_freeList not initialized to m_parentBlock");
      }

      if(allocator.c_pageList->m_availableBlocks != 1)
      {
        return TEST_FAILURE("HeterogeneousAllocator::Page::m_availableBlocks not initialized");
      }

      if(allocator.c_pageList->m_parentBlock->m_size != PAGE_SIZE)
      {
        return TEST_FAILURE("HeterogeneousAllocator::Page::Block::m_size not initialized");
      }

      if(allocator.c_pageList->m_parentBlock->m_prev != 0)
      {
        return TEST_FAILURE("HeterogeneousAllocator::Page::Block::m_prev not initialized");
      }

      if(allocator.c_pageList->m_parentBlock->m_next != 0)
      {
        return TEST_FAILURE("HeterogeneousAllocator::Page::Block::m_next not initialized");
      }

      return "";
    }
    
    
    const std::string TestManager::heterogeneousAllocator_instanceAllocation_shouldCreateInstancesAtCorrectLocation()
    {
      HeterogeneousAllocator allocator(256);

      SmallStruct::InitializeAllocator(&allocator);

      HeterogeneousAllocator::Page::Block* initialBlock = allocator.c_pageList->m_parentBlock;
      const unsigned initialSize = allocator.c_pageList->m_freeList->m_size;

      SmallStruct *smallStruct1 = new SmallStruct(),
                  *smallStruct2 = new SmallStruct();

      if((char*)(smallStruct1) != (char*)(initialBlock) + sizeof(HeterogeneousAllocator::Page::Block))
      {
        return TEST_FAILURE("SmallStruct1 not allocated at the correct location");
      }

      if((char*)(smallStruct2) != (char*)(smallStruct1) + sizeof(SmallStruct) + sizeof(HeterogeneousAllocator::Page::Block))
      {
        return TEST_FAILURE("SmallStruct2 not allocated at the correct location");
      }

      unsigned numBlocks;
      HeterogeneousAllocator::Page::Block* blockIter;

      for(numBlocks = 0, blockIter = allocator.c_pageList->m_usedList; blockIter; blockIter = blockIter->m_next, ++numBlocks) ;

      if(numBlocks != 2)
      {
        return TEST_FAILURE("Incorrect count in HeteroegeneousAllocator::Page::m_usedList");
      }

      for(numBlocks = 0, blockIter = allocator.c_pageList->m_freeList; blockIter; blockIter = blockIter->m_next, ++numBlocks) ;

      if(numBlocks != 1)
      {
        return TEST_FAILURE("Incorrect count in HeteroegeneousAllocator::Page::m_freeList");
      }

      if(allocator.c_pageList->m_freeList->m_size != initialSize - (sizeof(HeterogeneousAllocator::Page::Block) * 2 + sizeof(SmallStruct) * 2))
      {
        return TEST_FAILURE("Incorrect count in HeteroegeneousAllocator::Page::m_freeList calculated incorrect size after allocations");
      }

      return "";
    }
    
    
    const std::string TestManager::heterogeneousAllocator_instanceAllocation_shouldNotAllocateOutOfBoundsWhenSizeOfInstanceIsExact()
    {
      HeterogeneousAllocator allocator(sizeof(SmallStruct) * 3 + sizeof(HeterogeneousAllocator::Page::Block));

      SmallStruct::InitializeAllocator(&allocator);

      SmallStruct *smallStruct1 = new SmallStruct(),
                  *smallStruct2 = new SmallStruct(),
                  *smallStruct3 = new SmallStruct();

      if((char*)smallStruct2 != (char*)smallStruct1 + sizeof(SmallStruct) + sizeof(HeterogeneousAllocator::Page::Block))
      {
        return TEST_FAILURE("SmallStruct* allocated out of bounds");
      }

      if((char*)smallStruct3 == (char*)smallStruct2 + sizeof(SmallStruct) + sizeof(HeterogeneousAllocator::Page::Block))
      {
        return TEST_FAILURE("SmallStruct* allocated out of bounds");
      }

      return "";
    }


    const std::string TestManager::heterogeneousAllocator_instanceAllocationAndDeletion_shouldUpdateUsedAndFreeListCorrectly()
    {
      HeterogeneousAllocator allocator(1024);

      BigStruct::InitializeAllocator(&allocator);

      HeterogeneousAllocator::Page::Block* initialBlock = allocator.c_pageList->m_parentBlock;
      const unsigned initialSize = allocator.c_pageList->m_freeList->m_size;

      BigStruct *bigStruct1 = new BigStruct(),
                *bigStruct2 = new BigStruct(),
                *bigStruct3 = new BigStruct();

      if((char*)(bigStruct1) != (char*)(initialBlock) + sizeof(HeterogeneousAllocator::Page::Block))
      {
        return TEST_FAILURE("BigStruct1 not allocated at the correct location");
      }

      if((char*)(bigStruct2) != (char*)(bigStruct1) + sizeof(BigStruct) + sizeof(HeterogeneousAllocator::Page::Block))
      {
        return TEST_FAILURE("BigStruct2 not allocated at the correct location");
      }

      if((char*)(bigStruct3) != (char*)(bigStruct2) + sizeof(BigStruct) + sizeof(HeterogeneousAllocator::Page::Block))
      {
        return TEST_FAILURE("BigStruct2 not allocated at the correct location");
      }

      if(allocator.c_pageList->m_freeList->m_size != initialSize - (sizeof(HeterogeneousAllocator::Page::Block) * 3 + sizeof(BigStruct) * 3))
      {
        return TEST_FAILURE("Incorrect count in HeteroegeneousAllocator::Page::m_freeList calculated incorrect size after allocations");
      }

      const char* const bigStruct2Block = (char*)bigStruct2 - sizeof(HeterogeneousAllocator::Page::Block);

      delete bigStruct2;

      unsigned numBlocks = 0;
      bool bigStruct2BlockOnFreeList = false;
      HeterogeneousAllocator::Page::Block* blockIter = 0;

      for(numBlocks = 0, blockIter = allocator.c_pageList->m_usedList; blockIter; blockIter = blockIter->m_next, ++numBlocks) ;

      if(numBlocks != 2)
      {
        return TEST_FAILURE("Incorrect count in HeteroegeneousAllocator::Page::m_usedList");
      }

      for(numBlocks = 0, blockIter = allocator.c_pageList->m_freeList; blockIter; blockIter = blockIter->m_next, ++numBlocks)
      {
        if(blockIter->m_next && blockIter->m_size < blockIter->m_next->m_size)
        {
          return TEST_FAILURE("HeteroegeneousAllocator::Page::m_freeList not sorted by size after delete");
        }

        if((char*)blockIter == bigStruct2Block)
        {
          bigStruct2BlockOnFreeList = true;
        }
      }

      if(numBlocks != 2)
      {
        return TEST_FAILURE("Incorrect count in HeteroegeneousAllocator::Page::m_freeList");
      }

      if(bigStruct2BlockOnFreeList == false)
      {
        return TEST_FAILURE("BigStruct not added to HeteroegeneousAllocator::Page::m_freeList after delete");
      }

      return "";
    }
    
    
    const std::string TestManager::heterogeneousAllocator_instanceAllocationAndDeletion_shouldConsolidateBlocks()
    {
      HeterogeneousAllocator allocator(512);

      SmallStruct::InitializeAllocator(&allocator);
      BigStruct::InitializeAllocator(&allocator);

      BigStruct *bigStruct1 = new BigStruct(),
                *bigStruct2 = new BigStruct();
      SmallStruct *smallStruct1 = new SmallStruct(),
                  *smallStruct2 = new SmallStruct();

      const char *initialSmallStruct1 = (char*)smallStruct1,
                 *initialBigStruct2 = (char*)bigStruct2;

      delete smallStruct1;
      delete bigStruct2;

      if((char*)allocator.c_pageList->m_freeList + sizeof(HeterogeneousAllocator::Page::Block) != initialBigStruct2)
      {
        return TEST_FAILURE("Block on m_freeList does not include deallocated BigStruct");
      }

      if((char*)allocator.c_pageList->m_freeList + sizeof(HeterogeneousAllocator::Page::Block) * 2 + sizeof(BigStruct) != initialSmallStruct1)
      {
        return TEST_FAILURE("Block on m_freeList does not include deallocated SmallStruct");
      }

      if(allocator.c_pageList->m_freeList->m_size != sizeof(BigStruct) + sizeof(SmallStruct) + sizeof(HeterogeneousAllocator::Page::Block))
      {
        return TEST_FAILURE("Block on m_freeList does not have correctly calculated m_size");
      }

      if((char*)allocator.c_pageList->m_usedList + sizeof(HeterogeneousAllocator::Page::Block) != (char*)smallStruct2)
      {
        return TEST_FAILURE("m_usedList not updated correctly");
      }

      if(!allocator.c_pageList->m_usedList->m_next ||
        (char*)allocator.c_pageList->m_usedList->m_next + sizeof(HeterogeneousAllocator::Page::Block) != (char*)bigStruct1)
      {
        return TEST_FAILURE("m_usedList not updated correctly");
      }

      return "";
    }
    
    
    const std::string TestManager::heterogeneousAllocator_instanceAllocationAndDeletion_shouldUpdateFreeListDuringConsolidation()
    {
      HeterogeneousAllocator allocator(sizeof(BigStruct) * 3 + sizeof(SmallStruct) + sizeof(HeterogeneousAllocator::Page::Block) * 2);

      BigStruct::InitializeAllocator(&allocator);

      BigStruct *bigStruct1 = new BigStruct(),
                *bigStruct2 = new BigStruct(),
                *bigStruct3 = new BigStruct();

      const char *initialBigStruct1 = (char*)bigStruct1;

      delete bigStruct2;
      delete bigStruct1;

      if((char*)allocator.c_pageList->m_freeList + sizeof(HeterogeneousAllocator::Page::Block) != initialBigStruct1)
      {
        return TEST_FAILURE("m_freeList not updated correctly");
      }

      delete bigStruct3;

      if((char*)allocator.c_pageList->m_freeList + sizeof(HeterogeneousAllocator::Page::Block) != initialBigStruct1)
      {
        return TEST_FAILURE("m_freeList not updated correctly");
      }

      return "";
    }
    
    
    const std::string TestManager::heterogeneousAllocator_instanceAllocationAndDeletion_shouldUpdateAvailableBlocksCorrectly()
    {
      HeterogeneousAllocator allocator(1024);

      SmallStruct::InitializeAllocator(&allocator);

      const unsigned numStructs = 20;
      const unsigned structsToDelete[] = { 10, 11, 0, 2, 1, 17 };
      SmallStruct* smallStructs[numStructs];

      for(unsigned i = 0; i < numStructs; ++i)
      {
        smallStructs[i] = new SmallStruct();
      }

      if(allocator.c_pageList->m_availableBlocks != 1)
      {
        return TEST_FAILURE("m_availableBlocks not updated correctly after allocation");
      }

      delete smallStructs[structsToDelete[0]];

      if(allocator.c_pageList->m_availableBlocks != 2)
      {
        return TEST_FAILURE("m_availableBlocks not updated correctly after deallocation");
      }

      delete smallStructs[structsToDelete[1]];

      if(allocator.c_pageList->m_availableBlocks != 3)
      {
        return TEST_FAILURE("m_availableBlocks not updated correctly after deallocation");
      }

      delete smallStructs[structsToDelete[2]];

      if(allocator.c_pageList->m_availableBlocks != 4)
      {
        return TEST_FAILURE("m_availableBlocks not updated correctly after deallocation");
      }

      delete smallStructs[structsToDelete[3]];

      if(allocator.c_pageList->m_availableBlocks != 5)
      {
        return TEST_FAILURE("m_availableBlocks not updated correctly after deallocation");
      }

      delete smallStructs[structsToDelete[4]];

      if(allocator.c_pageList->m_availableBlocks != 5)
      {
        return TEST_FAILURE("m_availableBlocks not updated correctly after deallocation");
      }

      delete smallStructs[structsToDelete[5]];

      if(allocator.c_pageList->m_availableBlocks != 6)
      {
        return TEST_FAILURE("m_availableBlocks not updated correctly after deallocation");
      }

      return "";
    }


    const std::string TestManager::heterogeneousAllocator_instanceAllocationAndDeletion_shouldUpdateAvailableSizeCorrectly()
    {
      return TEST_NOT_IMPLEMENTED();
    }
    
    
    const std::string TestManager::heterogeneousAllocator_instanceAllocationAndDeletion_shouldCheckForAllPossibleConsoliations()
    {
      HeterogeneousAllocator allocator(750);

      SmallStruct::InitializeAllocator(&allocator);
      BigStruct::InitializeAllocator(&allocator);

      const unsigned numStructs = 20;
      SmallStruct* smallStructs[numStructs];

      for(unsigned i = 0; i < numStructs; ++i)
      {
        smallStructs[i] = new SmallStruct();
      }

      if(allocator.c_pageList->m_availableBlocks != 1)
      {
        return TEST_FAILURE("m_availableBlocks not updated correctly after allocation");
      }

      for(unsigned i = 0; i < numStructs; ++i)
      {
        delete smallStructs[i];
      }

      if(allocator.c_pageList->m_availableBlocks != 20)
      {
        return TEST_FAILURE("m_availableBlocks not updated correctly after deallocation");
      }

      BigStruct* bigStruct = new BigStruct();

      if(allocator.c_pageList->m_availableBlocks != 1)
      {
        return TEST_FAILURE("m_availableBlocks not updated correctly after allocation");
      }

      return "";
    }


    const std::string TestManager::heterogeneousAllocator_instanceAllocationAndDeletion_shouldReuseBlocks()
    {
      return TEST_NOT_IMPLEMENTED();
    }
    
    
    const std::string TestManager::heterogeneousAllocator_instanceAllocationAndDeletion_shouldUseMultiplePages()
    {
      return TEST_NOT_IMPLEMENTED();
    }
    
    
    const std::string TestManager::heterogeneousAllocator_instanceAllocationAndDeletion_shouldBeFaster()
    {
      return TEST_NOT_IMPLEMENTED();
    }


    const std::string TestManager::heterogeneousAllocator_arrayAllocation_shouldCreateInstancesAtCorrectLocation()
    {
      HeterogeneousAllocator allocator(256);

      SmallStruct::InitializeAllocator(&allocator);

      HeterogeneousAllocator::Page::Block* initialBlock = allocator.c_pageList->m_parentBlock;
      const unsigned initialSize = allocator.c_pageList->m_freeList->m_size;

      SmallStruct *smallStructs = new SmallStruct[3];

      if((char*)(smallStructs) != (char*)(initialBlock) + sizeof(HeterogeneousAllocator::Page::Block))
      {
        return TEST_FAILURE("SmallStruct* not allocated at the correct location");
      }

      unsigned numBlocks;
      HeterogeneousAllocator::Page::Block* blockIter;

      for(numBlocks = 0, blockIter = allocator.c_pageList->m_usedList; blockIter; blockIter = blockIter->m_next, ++numBlocks) ;

      if(numBlocks != 1)
      {
        return TEST_FAILURE("Incorrect count in HeteroegeneousAllocator::Page::m_usedList");
      }

      for(numBlocks = 0, blockIter = allocator.c_pageList->m_freeList; blockIter; blockIter = blockIter->m_next, ++numBlocks) ;

      if(numBlocks != 1)
      {
        return TEST_FAILURE("Incorrect count in HeteroegeneousAllocator::Page::m_freeList");
      }

      if(allocator.c_pageList->m_freeList->m_size != initialSize - (sizeof(HeterogeneousAllocator::Page::Block) + sizeof(SmallStruct) * 3))
      {
        return TEST_FAILURE("Incorrect count in HeteroegeneousAllocator::Page::m_freeList calculated incorrect size after allocations");
      }

      return "";
    }


    const std::string (*TestManager::s_heterogeneousTests[])() = {
        TestManager::heterogeneousAllocator_constructor_shouldInitializeAllMembers,
        TestManager::heterogeneousAllocator_instanceAllocation_shouldCreateInstancesAtCorrectLocation,
        TestManager::heterogeneousAllocator_instanceAllocation_shouldNotAllocateOutOfBoundsWhenSizeOfInstanceIsExact,
        TestManager::heterogeneousAllocator_instanceAllocationAndDeletion_shouldUpdateUsedAndFreeListCorrectly,
        TestManager::heterogeneousAllocator_instanceAllocationAndDeletion_shouldConsolidateBlocks,
        TestManager::heterogeneousAllocator_instanceAllocationAndDeletion_shouldUpdateFreeListDuringConsolidation,
        TestManager::heterogeneousAllocator_instanceAllocationAndDeletion_shouldUpdateAvailableBlocksCorrectly,
        TestManager::heterogeneousAllocator_instanceAllocationAndDeletion_shouldUpdateAvailableSizeCorrectly,
        TestManager::heterogeneousAllocator_instanceAllocationAndDeletion_shouldCheckForAllPossibleConsoliations,
        TestManager::heterogeneousAllocator_instanceAllocationAndDeletion_shouldReuseBlocks,
        TestManager::heterogeneousAllocator_instanceAllocationAndDeletion_shouldUseMultiplePages,
        TestManager::heterogeneousAllocator_instanceAllocationAndDeletion_shouldBeFaster,
        TestManager::heterogeneousAllocator_arrayAllocation_shouldCreateInstancesAtCorrectLocation,
        0
    };


  }
}
