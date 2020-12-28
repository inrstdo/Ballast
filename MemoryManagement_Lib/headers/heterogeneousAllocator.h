#ifndef HETEROGENEOUSALLOCATOR_H
#define HEREROGENEOUSALLOCATOR_H


#include "../../TestManager_Lib/headers/useTestManager.h"


namespace Ballast {
  namespace MemoryManagement {
    

    class HeterogeneousAllocator
    {
      private:
        class Page
        {
          private:
            struct Block
            {
              TESTMANAGER_FRIEND
              Block();
              Block(const unsigned size, Block* const prev, Block* const next);
              ~Block();


              static Block* const allocateNew(char* const memoryLocation, const unsigned blockSize, Block* const prev, Block* const next);


              unsigned m_size;
              Block* m_prev,
                   * m_next;
            };


            void removeBlockFromList(Block* const block);
            void addBlockToFreeList(Block* const block);
            void addBlockToUsedList(Block* const block);

            const bool checkToConsolidateBlocks(Block* const first, Block* const second);

            Block* const traverseFreeListAllocateNewBlock(unsigned const size);
            const bool traverseFreeListCheckToConsolidateBlocks();

            
            const unsigned c_pageSize;
            Block*  m_parentBlock,
                 *  m_freeList,
                 *  m_usedList;
            Page* m_next;
            unsigned m_availableBlocks;


          public:
            TESTMANAGER_FRIEND
            Page();
            Page(const unsigned pageSize);
            ~Page();


            const unsigned getLargestAvailableSize() const;
            Page* const getNext() const;

            void setNext(Page* const nextPage);

            Block* const allocate(const unsigned trueSize);
            void deallocate(Block* const block, unsigned size);


            static Page* const allocateNew(const unsigned pageSize);
        };


        Page* const getLastPage() const;


        const unsigned c_pageSize;
        Page* const c_pageList;


      public:
        TESTMANAGER_FRIEND
        HeterogeneousAllocator(const unsigned pageSize);
        ~HeterogeneousAllocator();


        template<typename T>
        T* allocate(unsigned size)
        {
          const unsigned trueSize = size * sizeof(T);

          if (trueSize > c_pageSize)
          {
            throw "Cannot allocate any size larger than c_pageSize";
          }

          Page* currentPage = c_pageList;
          Page::Block* block = 0;

          while(true)
          {
            block = currentPage->allocate(trueSize);
            
            if(block)
            {
              break;
            }

            if(!currentPage->getNext())
            {
              Page* nextPage = Page::allocateNew(c_pageSize);
              currentPage->setNext(nextPage);
            }

            currentPage = currentPage->getNext();
          }

          return reinterpret_cast<T*>(reinterpret_cast<char*>(block) + sizeof(Page::Block));
        }

        template<typename T>
        void deallocate(T* memory, unsigned size)
        {
          Page* currentPage = c_pageList;
          char* const memoryAsChar = reinterpret_cast<char*>(memory);

          while(currentPage)
          {
            char* const pageMemory = reinterpret_cast<char*>(currentPage);

            if(pageMemory < memoryAsChar && pageMemory + c_pageSize > memoryAsChar)
            {
              Page::Block* block = reinterpret_cast<Page::Block*>(memoryAsChar - sizeof(Page::Block));

              currentPage->deallocate(block, size);

              return;
            }

            currentPage = currentPage->getNext();
          }

          throw "Cannot deallocate memory";
        }
    };
    

  }
}

#endif
