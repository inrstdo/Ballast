#include <new>
#include "../headers/heterogeneousAllocator.h"


namespace Ballast {
  namespace MemoryManager {


    HeterogeneousAllocator::HeterogeneousAllocator(const unsigned pageSize) :
      c_pageSize(pageSize),
      c_pageList(Page::allocateNew(c_pageSize))
    {
    }


    HeterogeneousAllocator::~HeterogeneousAllocator()
    {
      Page* currentPage = c_pageList;

      while(currentPage)
      {
        char* memory = reinterpret_cast<char*>(currentPage);

        currentPage = currentPage->getNext();
        
        delete [] memory;
      }
    }


    HeterogeneousAllocator::Page* const HeterogeneousAllocator::getLastPage() const
    {
      Page* currentPage = c_pageList;

      while(currentPage->getNext())
      {
        currentPage = currentPage->getNext();
      }

      return currentPage;
    }


    HeterogeneousAllocator::Page* const HeterogeneousAllocator::Page::allocateNew(const unsigned pageSize)
    {
      const unsigned trueSize = pageSize + sizeof(Page) + sizeof(Block);
      char* const memory = new char[trueSize]; 
      ::new (memory) Page(pageSize, memory + sizeof(Page));

      return reinterpret_cast<Page*>(memory);
    }


    HeterogeneousAllocator::Page::Page() :
      c_totalSize(0),
      m_availableSize(0),
      m_freePool(0),
      m_parentBlock(0)
    {
    }


    HeterogeneousAllocator::Page::Page(const unsigned pageSize, char* const freePool) :
      c_totalSize(pageSize),
      m_availableSize(c_totalSize),
      m_freePool(freePool),
      m_parentBlock(Block::allocateNew(m_freePool + sizeof(Page), pageSize, true, 0, 0))
    {
    }


    const unsigned HeterogeneousAllocator::Page::getAvailableSize() const
    {
      return m_availableSize;
    }


    HeterogeneousAllocator::Page* const HeterogeneousAllocator::Page::getNext() const
    {
      return m_next;
    }


    void HeterogeneousAllocator::Page::setNext(Page* const nextPage)
    {
      m_next = nextPage;
    }

    
    HeterogeneousAllocator::Page::Block* const HeterogeneousAllocator::Page::traverseListAllocateNewBlock(unsigned const size)
    {
      Block* currentBlock = m_parentBlock;

      while(currentBlock)
      {
        // if the current Block has capacity and is free
        if(currentBlock->m_size >= size && currentBlock->m_free)
        {
          Block* newNextBlock = 0;
          const unsigned leftoverSize = currentBlock->m_size - size;
          unsigned currentSize = size;

          // if there's capacity for another Block after this one
          if(leftoverSize > sizeof(Block))
          {
            char* const memory = reinterpret_cast<char*>(currentBlock);
            newNextBlock = Block::allocateNew(memory + size, leftoverSize - sizeof(Block), true, currentBlock, currentBlock->m_next);

            currentSize -= leftoverSize;
          }

          currentBlock->m_size = currentSize;
          currentBlock->m_free = false;

          if(newNextBlock)
          {
            currentBlock->m_next->m_prev = newNextBlock;
            currentBlock->m_next = newNextBlock;
          }

          return currentBlock;
        }

        currentBlock = currentBlock->m_next;
      }

      if(!m_next)
      {
        m_next = Page::allocateNew(c_totalSize);
      }

      return m_next->traverseListAllocateNewBlock(size);
    }


    HeterogeneousAllocator::Page::Block* const HeterogeneousAllocator::Page::Block::allocateNew(char* const freePoolLocation, const unsigned blockSize, const bool free, Block* const prev, Block* const next)
    {
      ::new (freePoolLocation) Block(blockSize, free, prev, next);

      return reinterpret_cast<Block*>(freePoolLocation);
    }


    HeterogeneousAllocator::Page::Block::Block() :
      m_size(0),
      m_free(false),
      m_prev(0),
      m_next(0)
    {
    }


    HeterogeneousAllocator::Page::Block::Block(const unsigned size, const bool free, Block* const prev, Block* const next) :
      m_size(size),
      m_free(free),
      m_prev(prev),
      m_next(next)
    {
    }


  }
}
