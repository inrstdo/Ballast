#include "../headers/heterogeneousAllocator.h"


namespace Ballast {
  namespace MemoryManager {


    HeterogeneousAllocator::HeterogeneousAllocator(const unsigned size) :
      c_size(size)
    {
      m_freePool = new char[c_size];
    }


    HeterogeneousAllocator::~HeterogeneousAllocator()
    {
      delete [] m_freePool;
    }
    
    
    HeterogeneousAllocator::Node::Node(const unsigned size, Node* const left, Node* const right) :
      c_size(size),
      m_left(left),
      m_right(right)
    {
    }


    void HeterogeneousAllocator::Node::setLeft(Node* const left)
    {
      m_left = left;
    }


    void HeterogeneousAllocator::Node::setRight(Node* const right)
    {
      m_right = right;
    }


    HeterogeneousAllocator::Node* const HeterogeneousAllocator::Node::getLeft() const
    {
      return m_left;
    }


    HeterogeneousAllocator::Node* const HeterogeneousAllocator::Node::getRight() const
    {
      return m_right;
    }


  }
}
