#include <new>

#include "../headers/heterogeneousAllocator.h"


namespace Ballast {
  namespace MemoryManagement {


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
      m_next(0),
      m_availableBlocks(0),
      m_availableSize(0)
    {
    }


    HeterogeneousAllocator::Page::Page(const unsigned pageSize) :
      c_pageSize(pageSize),
      m_parentBlock(Page::Block::allocateNew(reinterpret_cast<char*>(this) + sizeof(Page), pageSize, 0, 0)),
      m_freeList(m_parentBlock),
      m_usedList(0),
      m_next(0),
      m_availableBlocks(1),
      m_availableSize(pageSize)
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

      block->m_prev = block->m_next = 0;
    }


    void HeterogeneousAllocator::Page::addBlockToFreeList(Block* const block)
    {
      if(!m_freeList)
      {
        m_freeList = block;

        ++m_availableBlocks;
        m_availableSize += block->m_size; // sizeof(Block) not included

        return;
      }

      // Walk through the m_freeList so that it remains sorted by size, from largest to smallest
      // Only try to consolidate the Blocks that we walk past; attempts to consolidate *all* possible Blocks
      // will be made explicitly when a call to allocate() finds that the largest free Block isn't enough, but the
      // number of available Blocks is equivalent to the size of the requested allocation
      Block* currentBlock = m_freeList;

      while(currentBlock)
      {
        if(block->m_size >= currentBlock->m_size)
        {
          // if the Block to add is larger than the currentBlock, add it to the m_freeList in front of the currentBlock
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
          m_availableSize += block->m_size; // sizeof(Block) not included

          // check to consolidate as many sequential Blocks as possible
          checkForAndPerformAllPossibleConsolidations(block);

          return;
        }
        else if(!currentBlock->m_next)
        {
          // if there's no next Block, add to the very end of the m_freeList
          currentBlock->m_next = block;

          block->m_prev = currentBlock;
          block->m_next = 0;

          ++m_availableBlocks;
          m_availableSize += block->m_size; // sizeof(Block) not included

          return;
        }
        else
        {
          // if we need to walk forward, might as well consolidate Blocks if we can
          checkForAndPerformPossibleConsolidation(currentBlock, currentBlock->m_next);

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
      m_availableSize -= block->m_size; // sizeof(Block) not included
    }


    const bool HeterogeneousAllocator::Page::checkForAndPerformAllPossibleConsolidations(Block* const block)
    {
      bool consolidatedAtLeastOneBlock = false;

      for(Block* second = m_freeList; second; second = second->m_next)
      {
        if(block != second)
        {
          const bool consolidated = checkForAndPerformPossibleConsolidation(block, second);
          consolidatedAtLeastOneBlock = (!consolidatedAtLeastOneBlock && consolidated) || consolidatedAtLeastOneBlock;
        }
      }

      return consolidatedAtLeastOneBlock;
    }


    const bool HeterogeneousAllocator::Page::checkForAndPerformPossibleConsolidation(Block* const first, Block* const second)
    {
      char* const firstMemory = reinterpret_cast<char*>(first);
      char* const secondMemory = reinterpret_cast<char*>(second);

      if(firstMemory + sizeof(Block) + first->m_size == secondMemory)
      {
        consolidateBlocks(first, second);

        if(second == m_freeList)
        {
          m_freeList = first;
        }

        return true;
      }
      else
      {
        return false;
      }
    }


    void HeterogeneousAllocator::Page::consolidateBlocks(Block* const first, Block* const second)
    {
      removeBlockFromList(second);

      --m_availableBlocks;
      m_availableSize += sizeof(Block); // second->m_size should already be accounted for
      
      first->m_size += sizeof(Block) + second->m_size;
    }


    HeterogeneousAllocator::Page::Block* const HeterogeneousAllocator::Page::traverseFreeListAllocateNewBlock(const unsigned blockSize)
    {
      Block* currentBlock = m_freeList;

      while(currentBlock)
      {
        // if the current Block has capacity
        if(currentBlock->m_size >= blockSize)
        {
          // if there's a next Block that's a better fit
          if(currentBlock->m_next && currentBlock->m_next->m_size >= blockSize &&
            (currentBlock->m_next->m_size == currentBlock->m_size ||
            currentBlock->m_next->m_size - blockSize < currentBlock->m_size - blockSize))
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
            const unsigned leftoverSize = currentSize - blockSize;

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
      Block* block = m_freeList;
      bool consolidatedAtLeastOneBlock = false;

      while(block)
      {
        const bool consolidated = checkForAndPerformAllPossibleConsolidations(block);

        consolidatedAtLeastOneBlock = (!consolidatedAtLeastOneBlock && consolidated) || consolidatedAtLeastOneBlock;

        // if we consolidated once, there's a chance we've rearranged the Blocks in such a way that we can consolidate again,
        // so don't move onto m_next yet
        if(!consolidated)
        {
          block = block->m_next;
        }
      }

      return consolidatedAtLeastOneBlock;
    }
    
    
    HeterogeneousAllocator::Page::Block* const HeterogeneousAllocator::Page::allocate(const unsigned blockSize, const unsigned templateParameterSize)
    {
      // Remember, blockSize and templateParameterSize aren't always the same, because of array allocation

      bool shouldCheck = m_freeList && m_freeList->m_size >= blockSize;

      if(!shouldCheck && m_availableSize >= blockSize + sizeof(Block))
      {
        shouldCheck = traverseFreeListCheckToConsolidateBlocks();
      }

      return (shouldCheck) ? traverseFreeListAllocateNewBlock(blockSize) : 0;
    }

    
    void HeterogeneousAllocator::Page::deallocate(Block* const block)
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
