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


    HeterogeneousAllocator::Page::Page(const unsigned pageSize, char* const freePool) :
      c_totalSize(pageSize),
      m_availableSize(c_totalSize),
      m_freePool(freePool),
      m_parentNode(Block::allocateNew(m_freePool + sizeof(Page), pageSize, true, 0, 0, 0))
    {
    }


    HeterogeneousAllocator::Page* const HeterogeneousAllocator::Page::allocateNew(const unsigned pageSize)
    {
      const unsigned trueSize = pageSize + sizeof(Page) + sizeof(Block);
      const char* memory = new char[trueSize];
      ::new reinterpret_cast<Page*>(memory) Page(pageSize, memory + sizeof(Page));

      return reinterpret_cast<Page*>(memory);
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

    
    HetereogeneousAllocator::Page::Block* const HeterogeneousAllocator::Page::traverseTreeCreateNewBlock(unsigned const size, Block* const parentBlock)
    {
      // if the current Block has capacity and is free
      if(parentBlock->m_size >= size && parentBlock->m_free)
      {
        Block* newLeftBlock = 0;

        // if there's capacity for another Block after this one
        if(parentBlock->m_size - size > sizeof(Block))
        {
          char* const memory = reinterpret_cast<const char*>(parentBlock);
          newLeftBlock = Block::allocateNew(memory + size, parentBlock->m_size - size - sizeof(Block), true, m_parentNode->m_left, 0, m_parentNode);
        }

        parentBlock->m_size = size;
        parentBlock->m_size = false;

        if(newLeftBlock)
        {
          parentBlock->m_left = newLeftBlock;
        }

        return parentBlock;
      }

      if(parentBlock->m_left)
      {
        Block* allocatedBlock = traverseTreeCreateNewBlock(size, parentBlock->m_left);

        if (allocatedBlock)
        {
          return allocatedBlock;
        }
      }

      const char* freePoolLocation = reinterpret_cast<const char*>(parentBlock);
      freePoolLocation += sizeof(Block) + parentBlock->c_size;

      Block* newBlock = Block::allocateNew(size, 
    }


    HeterogeneousAllocator::Page::Block* const HeterogeneousAllocator::Page::Block::allocateNew(char* const freePoolLocation, const unsigned blockSize, const bool free, Block* const left, Block* const right, Block* parent)
    {
      ::new reinterpret_cast<Block*>(freePoolLocation) Block(blockSize, free, left, right, parent);

      return reinterpret_cast<Block*>(freePoolLocation);
    }


    HeterogeneousAllocator::Page::Block::Block(const unsigned size, const bool free, Block* const left, Block* const right, Block* const parent) :
      m_size(size),
      m_free(free),
      m_left(left),
      m_right(right),
      m_parent(parent)
    {

    }


  }
}
