#ifndef HETEROGENEOUSALLOCATOR_H
#define HEREROGENEOUSALLOCATOR_H


namespace Ballast {
  namespace MemoryManager {


    class HeterogeneousAllocator
    {
      public:
        HeterogeneousAllocator(const unsigned size);
        ~HeterogeneousAllocator();


      private:
        class Node
        {
          public:
            Node(const unsigned size, Node* const left, Node* const right );

            void setLeft(Node* left);
            void setRight(Node* right);

            Node* const getLeft() const;
            Node* const getRight() const;


          private:
            const unsigned c_size;
            Node* m_left;
            Node* m_right;
        };


        const unsigned c_size;
        char* m_freePool;
    };
    

  }
}

#endif
