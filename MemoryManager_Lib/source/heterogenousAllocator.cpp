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
      ::new (memory) Page(pageSize);

      return reinterpret_cast<Page*>(memory);
    }


    HeterogeneousAllocator::Page::Page() :
      c_pageSize(0),
      m_parentBlock(0),
      m_freeList(0),
      m_usedList(0),
      m_availableBlocks(0)
    {
    }


    HeterogeneousAllocator::Page::Page(const unsigned pageSize) :
      c_pageSize(pageSize),
      m_parentBlock(Page::Block::allocateNew(reinterpret_cast<char*>(this) + sizeof(Page), pageSize, 0, 0)),
      m_freeList(m_parentBlock),
      m_usedList(0),
      m_availableBlocks(1)
    {
    }


    const unsigned HeterogeneousAllocator::Page::getLargestAvailableSize() const
    {
      // m_freeList is sorted with the largest block first
      return m_freeList->m_size;
    }


    HeterogeneousAllocator::Page* const HeterogeneousAllocator::Page::getNext() const
    {
      return m_next;
    }


    void HeterogeneousAllocator::Page::setNext(Page* const nextPage)
    {
      m_next = nextPage;
    }

    
    void HeterogeneousAllocator::Page::removeBlockFromList(Block* const block)
    {
      if(block->m_prev)
      {
        block->m_prev->m_next = block->m_next;
      }

      if(block->m_next)
      {
        block->m_next->m_prev = block->m_prev;
      }
    }


    void HeterogeneousAllocator::Page::addBlockToFreeList(Block* const block)
    {
      if(!m_freeList)
      {
        m_freeList = block;

        ++m_availableBlocks;

        return;
      }

      // Walk through the m_freeList so that it remains sorted by size
      // Only try to consolidate the Blocks that we walk past; attempts to consolidate *all* possible Blocks
      // will be made explicitly when a call to allocate() finds that the m_largestSize isn't enough, but the
      // number of available blocks is equivalent to the size of the requested allocation
      Block* currentBlock = m_freeList;

      while(currentBlock)
      {
        if(block->m_size >= currentBlock->m_size)
        {
          // if the currentBlock is smaller, add to the m_freeList before it
          if(currentBlock->m_prev)
          {
            currentBlock->m_prev->m_next = block;
          }

          block->m_prev = currentBlock->m_prev;
          block->m_next = currentBlock;

          currentBlock->m_prev = block;

          if(currentBlock == m_freeList)
          {
            m_freeList = block;
          }

          ++m_availableBlocks;

          return;
        }
        else if(!currentBlock->m_next)
        {
          // if there's no next Block, add to the very end of the m_freeList
          currentBlock->m_next = block;

          block->m_prev = currentBlock;
          block->m_next = 0;

          ++m_availableBlocks;

          return;
        }
        else
        {
          // if we need to walk forward, consolidate Blocks if we can
          checkToConsolidateBlocks(currentBlock, currentBlock->m_next);

          currentBlock = currentBlock->m_next;
        }
      }
    }


    void HeterogeneousAllocator::Page::addBlockToUsedList(Block* const block)
    {
      // Add the Block to the beginning of the m_usedList
      if(m_usedList)
      {
        m_usedList->m_prev = block;
      }

      block->m_next = m_usedList;
      block->m_prev = 0;

      m_usedList = block;

      --m_availableBlocks;
    }


    const bool HeterogeneousAllocator::Page::checkToConsolidateBlocks(Block* const first, Block* const second)
    {
      const unsigned firstSize = first->m_size;
      const unsigned secondSize = second->m_size;
      char* const firstMemory = reinterpret_cast<char*>(first);
      char* const secondMemory = reinterpret_cast<char*>(second);

      if(firstMemory + sizeof(Block) + firstSize == secondMemory)
      {
        removeBlockFromList(second);
        
        first->m_size += sizeof(Block) + secondSize;

        return true;
      }
      else
      {
        return false;
      }
      
    }


    HeterogeneousAllocator::Page::Block* const HeterogeneousAllocator::Page::traverseFreeListAllocateNewBlock(const unsigned size)
    {
      Block* currentBlock = m_freeList;

      while(currentBlock)
      {
        // if the current Block has capacity
        if(currentBlock->m_size >= size)
        {
          // if there's a next Block that's a better fit
          if(currentBlock->m_next && currentBlock->m_next->m_size >= size &&
            (currentBlock->m_next->m_size == currentBlock->m_size ||
            currentBlock->m_next->m_size - size < currentBlock->m_size - size))
          {
            // noop, move on to the next Block
          }
          else
          {
            if(currentBlock == m_freeList)
            {
              m_freeList = currentBlock->m_next;
            }

            removeBlockFromList(currentBlock);
            addBlockToUsedList(currentBlock);

            unsigned currentSize = currentBlock->m_size;
            const unsigned leftoverSize = currentSize - size;

            // if there's capacity for another Block after this one
            if(leftoverSize > sizeof(Block))
            {
              currentSize -= leftoverSize;

              char* const memory = reinterpret_cast<char*>(currentBlock);
              Block* const leftoverBlock = Block::allocateNew(memory + sizeof(Block) + currentSize, leftoverSize - sizeof(Block), 0, 0);

              addBlockToFreeList(leftoverBlock);
            }

            currentBlock->m_size = currentSize;

            return currentBlock;
          }
          
        }

        currentBlock = currentBlock->m_next;
      }

      return 0;
    }


    bool const HeterogeneousAllocator::Page::traverseFreeListCheckToConsolidateBlocks()
    {
      Block* currentBlock = m_freeList;
      bool consolidatedAtLeastOneBlock = false;

      while(currentBlock && currentBlock->m_next)
      {
        if(checkToConsolidateBlocks(currentBlock, currentBlock->m_next))
        {
          if(!consolidatedAtLeastOneBlock)
          {
            consolidatedAtLeastOneBlock = true;
          }
        }
        else
        {
          currentBlock = currentBlock->m_next;
        }
        
      }

      return consolidatedAtLeastOneBlock;
    }
    
    
    HeterogeneousAllocator::Page::Block* const HeterogeneousAllocator::Page::allocate(const unsigned trueSize)
    {
      bool shouldCheck = m_freeList->m_size >= trueSize;

      if(!shouldCheck && m_availableBlocks >= trueSize)
      {
        shouldCheck = traverseFreeListCheckToConsolidateBlocks();
      }

      return (shouldCheck) ? traverseFreeListAllocateNewBlock(trueSize) : 0;
    }

    
    void HeterogeneousAllocator::Page::deallocate(Block* const block, unsigned size)
    {
      removeBlockFromList(block);
      addBlockToFreeList(block);
    }
    
    
    HeterogeneousAllocator::Page::Block* const HeterogeneousAllocator::Page::Block::allocateNew(char* const memoryLocation, const unsigned blockSize, Block* const prev, Block* const next)
    {
      ::new (memoryLocation) Block(blockSize, prev, next);

      return reinterpret_cast<Block*>(memoryLocation);
    }


    HeterogeneousAllocator::Page::Block::Block() :
      m_size(0),
      m_prev(0),
      m_next(0)
    {
    }


    HeterogeneousAllocator::Page::Block::Block(const unsigned size, Block* const prev, Block* const next) :
      m_size(size),
      m_prev(prev),
      m_next(next)
    {
    }


  }
}
