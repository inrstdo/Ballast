#ifndef HETEROGENEOUSALLOCATOR_H
#define HEREROGENEOUSALLOCATOR_H


namespace Ballast {
  namespace MemoryManager {


    class HeterogeneousAllocator
    {
      private:
        class Page
        {
          private:
            struct Block
            {
              Block();
              Block(const unsigned size, const bool free, Block* const prev, Block* const next);
              ~Block();


              static Block* const allocateNew(char* const freePoolLocation, const unsigned blockSize, const bool free, Block* const prev, Block* const next);


              unsigned m_size;
              bool m_free;
              Block* m_prev;
              Block* m_next;
            };


            Block* const traverseListAllocateNewBlock(unsigned const size);

            
            const unsigned c_totalSize;
            unsigned m_availableSize;
            char* m_freePool;
            Block* m_parentBlock;
            Page* m_next;


          public:
            Page();
            Page(const unsigned pageSize, char* const freePool);
            ~Page();

            const unsigned getAvailableSize() const;
            Page* const getNext() const;

            void setNext(Page* const nextPage);

            template<typename T>
            T* allocate(const unsigned trueSize)
            {
              Block* currentNode = m_parentBlock;

              return 0;
            }

            template<typename T>
            void deallocate(T* memory, unsigned size)
            {

            }


            static Page* const allocateNew(const unsigned pageSize);
        };


        Page* const getLastPage() const;


        const unsigned c_pageSize;
        Page* const c_pageList;


      public:
        HeterogeneousAllocator(const unsigned pageSize);
        ~HeterogeneousAllocator();


        template<typename T>
        T* allocate(unsigned size)
        {
          const trueSize = size * sizeof(T);

          if (trueSize > c_pageSize)
          {
            throw "Cannot allocate any object larger than c_pageSize";
          }

          Page* currentPage = c_pageList;

          while(currentPage->getAvailableSize() < trueSize)
          {
            if(!currentPage->getNext())
            {
              Page* nextPage = Page::allocateNew(c_pageSize);
              currentPage->setNext(nextPage);
            }

            currentPage = currentPage->getNext();
          }

          return currentPage->allocate<T>(size);
        }

        template<typename T>
        void deallocate(T* memory, unsigned size)
        {

        }
    };
    

  }
}

#endif
