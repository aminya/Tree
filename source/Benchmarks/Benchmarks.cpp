#include <algorithm>
#include <iostream>
#include <string>

#include "../Tree/Tree.hpp"

#include "Stopwatch.hpp"

/**
* Notes and Observations:
* =======================
*
* The following observations applied to an earlier version of the Tree that made extensive use of
* std::shared_ptr objects.
*
* If functions like TreeNode::GetParent(), -::GetFirstChild(), -::GetLastChild(),
* -::GetNextSibling(), and -::GetPreviousSibling() don't return const references,
* and instead, return copies of the std::shared_ptrs, then a simple post-order traversal
* over 16^5 + 1 nodes goes from taking 46ms to 71ms!
*
* When using an STD algorithm, like std::for_each(...), capturing the parameter by
* Tree<T>::value_type is about 3x slower than capturing that same node by Tree<T>::const_reference,
* or by Tree<T>::reference. Iterating over 16^5 + 1 nodes using the value_type takes approximately
* 125ms in x86/Release (using MSVC2015), while that same traversal takes only 46ms when capturing
* by (const) reference.
*/

namespace
{
   static int NODE_NUMBER = 0;

   void AddChildrenToNode(TreeNode<std::string>& node)
   {
      constexpr int NUMBER_OF_CHILDREN = 16;
      for (int i = 0; i < NUMBER_OF_CHILDREN; i++)
      {
         node.AppendChild(std::to_string(NODE_NUMBER++));
      }
   }

   /**
   * @brief CreateLargeTree will construct a Tree containing 16^5 + 1 nodes.
   */
   Tree<std::string> CreateLargeTree()
   {
      Tree<std::string> tree{ "Head" };
      auto* currentNode = tree.GetHead();

      AddChildrenToNode(*currentNode);

      TreeNode<std::string>* lastNodeVisited = nullptr;

      for (int i = 0; i < 4; i++)
      {
         std::for_each(
            Tree<std::string>::LeafIterator{ tree.GetHead() },
            Tree<std::string>::LeafIterator{ },
            [&] (auto& node)
         {
            assert(node.HasChildren() == false);

            if (lastNodeVisited)
            {
               AddChildrenToNode(*lastNodeVisited);
               assert(lastNodeVisited->HasChildren());
            }

            lastNodeVisited = &node;
         });

         lastNodeVisited = nullptr;
      }

      return tree;
   }
}

#define NEW_LINE std::cout << std::endl

int main()
{
   std::cout.imbue(std::locale{ "" });
   std::cout << "Constructing a large tree..." << std::endl;

   Tree<std::string> tree;
   Stopwatch<std::chrono::milliseconds>([&]
   {
      tree = CreateLargeTree();
   }, "Contructed a large tree in ");

   const auto sizeOfTree = tree.Size();
   std::cout << "Size of Tree: " << sizeOfTree << " nodes!" << std::endl;

   NEW_LINE;

   auto preOrderTraversalTime = Stopwatch<std::chrono::milliseconds>([&]
   {
      std::for_each(tree.beginPreOrder(), tree.endPreOrder(),
         [] (const auto&) noexcept
      {
         return true;
      });
   }).GetElapsedTime();

   preOrderTraversalTime = std::max(preOrderTraversalTime, std::uint64_t{ 1 });

   std::cout << "Pre-Order Traversal: " << std::endl;
   std::cout << "\tEntire Tree:\t" << preOrderTraversalTime << " ms" << std::endl;
   std::cout << "\tNodes per MS:\t" << sizeOfTree / preOrderTraversalTime << std::endl;

   NEW_LINE;

   auto postOrderTraversalTime = Stopwatch<std::chrono::milliseconds>([&]
   {
      std::for_each(std::begin(tree), std::end(tree),
         [] (const auto&) noexcept
      {
         return true;
      });
   }).GetElapsedTime();

   postOrderTraversalTime = std::max(postOrderTraversalTime, std::uint64_t{ 1 });

   std::cout << "Post-Order Traversal: " << std::endl;
   std::cout << "\tEntire Tree:\t" << postOrderTraversalTime << " ms" << std::endl;
   std::cout << "\tNodes per MS:\t" << sizeOfTree / postOrderTraversalTime << std::endl;

   NEW_LINE;

   auto leafTraversalTime = Stopwatch<std::chrono::milliseconds>([&]
   {
      std::for_each(tree.beginLeaf(), tree.endLeaf(),
         [] (const auto&) noexcept
      {
         return true;
      });
   }).GetElapsedTime();

   leafTraversalTime = std::max(leafTraversalTime, std::uint64_t{ 1 });

   std::cout << "Leaf Traversal: " << std::endl;
   std::cout << "\tEntire Tree:\t" << leafTraversalTime << " ms" << std::endl;
   std::cout << "\tNodes per MS:\t" << sizeOfTree / leafTraversalTime << std::endl;

   NEW_LINE;

   return 0;
}
