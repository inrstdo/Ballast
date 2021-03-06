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

            const bool checkForAndPerformAllPossibleConsolidations(Block* const block);
            const bool checkForAndPerformPossibleConsolidation(Block* const first, Block* const second);
            void consolidateBlocks(Block* const first, Block* const second);

            Block* const traverseFreeListAllocateNewBlock(unsigned const size);
            const bool traverseFreeListCheckToConsolidateBlocks();

            
            const unsigned c_pageSize;
            Block*  m_parentBlock,
                 *  m_freeList,
                 *  m_usedList;
            Page* m_next;
            unsigned m_availableBlocks,
                     m_availableSize;


          public:
            TESTMANAGER_FRIEND
            Page();
            Page(const unsigned pageSize);
            ~Page();


            const unsigned getLargestAvailableSize() const;
            Page* const getNext() const;

            void setNext(Page* const nextPage);

            Block* const allocate(const unsigned blockSize, const unsigned templateParameterSize);
            void deallocate(Block* const block);


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
          if (size > c_pageSize)
          {
            throw "Cannot allocate any size larger than c_pageSize";
          }

          Page* currentPage = c_pageList;
          Page::Block* block = 0;

          while(currentPage)
          {
            block = currentPage->allocate(size, sizeof(T));
            
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
        void deallocate(T* memory)
        {
          Page* currentPage = c_pageList;
          char* const memoryAsChar = reinterpret_cast<char*>(memory);

          while(currentPage)
          {
            char* const pageMemory = reinterpret_cast<char*>(currentPage);

            if(pageMemory < memoryAsChar && pageMemory + c_pageSize + sizeof(Page::Block) > memoryAsChar)
            {
              Page::Block* block = reinterpret_cast<Page::Block*>(memoryAsChar - sizeof(Page::Block));

              currentPage->deallocate(block);

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
