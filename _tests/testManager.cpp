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
        s_allocator = allocator;
      }


      void* operator new(size_t size)
      {
        return reinterpret_cast<void*>(s_allocator->allocate<SmallStruct>(size));
      }


      void* operator new[](size_t size)
      {
        return reinterpret_cast<void*>(s_allocator->allocate<SmallStruct>(size));
      }


      void operator delete(void* memory)
      {
        s_allocator->deallocate<SmallStruct>(reinterpret_cast<SmallStruct*>(memory), 0);
      }


      void operator delete[](void* memory)
      {
        s_allocator->deallocate<SmallStruct>(reinterpret_cast<SmallStruct*>(memory), 0);
      }
    };


    struct BigStruct
    {
      static HeterogeneousAllocator* s_allocator;
      int m_integerArray[10];
      float m_floatArray[10];
      char m_charArray[64];


      static void InitializeAllocator(HeterogeneousAllocator* allocator)
      {
        s_allocator = allocator;
      }


      void* operator new(size_t size)
      {
        return reinterpret_cast<void*>(s_allocator->allocate<BigStruct>(size));
      }


      void* operator new[](size_t size)
      {
        return reinterpret_cast<void*>(s_allocator->allocate<BigStruct>(size));
      }


      void operator delete(void* memory)
      {
        s_allocator->deallocate<BigStruct>(reinterpret_cast<BigStruct*>(memory), 1);
      }


      void operator delete[](void* memory)
      {
         // 0 for the size because there's no way to know the count
        s_allocator->deallocate<BigStruct>(reinterpret_cast<BigStruct*>(memory), 0);
      }
    };


    struct FancyStruct
    {
      static HeterogeneousAllocator* s_allocator;
      char* const c_string;


      FancyStruct(const char* const string) :
        c_string(s_allocator->allocate<char>(strlen(string) + 1))
      {
        strcpy(c_string, string);
      }


      ~FancyStruct()
      {
        s_allocator->deallocate(c_string, strlen(c_string) + 1);
      }


      static void InitializeAllocator(HeterogeneousAllocator* allocator)
      {
        s_allocator = allocator;
      }


      void* operator new(size_t size)
      {
        return reinterpret_cast<void*>(s_allocator->allocate<BigStruct>(size));
      }


      void* operator new[](size_t size)
      {
        return reinterpret_cast<void*>(s_allocator->allocate<BigStruct>(size));
      }


      void operator delete(void* memory)
      {
        s_allocator->deallocate<BigStruct>(reinterpret_cast<BigStruct*>(memory), 0);
      }


      void operator delete[](void* memory)
      {
        s_allocator->deallocate<BigStruct>(reinterpret_cast<BigStruct*>(memory), 0);
      }
    };


    const bool TestManager::heterogeneousAllocator_verify_allocate_1() const
    {
      HeterogeneousAllocator allocator(1024);
      HeterogeneousAllocator::Page* page = allocator.c_pageList;

      SmallStruct::InitializeAllocator(&allocator);

      SmallStruct *smallStruct1 = new SmallStruct(),
                  *smallStruct2 = new SmallStruct();

      if(!allocator.c_pageList)
      {
        return false;
      }

      HeterogeneousAllocator::Page::Block* 
      (!page->m_usedList

      return true;
    }


  }
}
