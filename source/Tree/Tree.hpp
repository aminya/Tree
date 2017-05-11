/**
* The MIT License (MIT)
*
* Copyright (c) 2017 Tim Severeijns
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>
#include <memory>
#include <vector>

namespace
{
   static constexpr auto NOT_SPECIFIED{ std::numeric_limits<std::size_t>::max() };
}

/**
* @brief The Tree class declares a basic tree, built on top of two std::vectors.
*
* The data is stored in a dedicated vector of DataType objects, while the metadata is stored in a
* separate vector. This data-oriented design should allow for lightning fast traversals of the data
* for those cases where the nature of the tree cannot be exploited to further increase operations.
*
* For increased cache-friendliness, this class also enables consumers to easily rearrange the
* objects in the vector so that everything is contiguous. This obviously works best for those cases
* where the tree is been completely built and further operations are purely focused on read
* operations.
*
* See the individual member functions for further documentation.
*/
template<typename DataType>
class Tree
{
public:

   class Node;

   class Iterator;
   class PreOrderIterator;
   class PostOrderIterator;
   class LeafIterator;
   class SiblingIterator;

   // Typedefs needed for STL compliance:
   using value_type = Node;
   using reference = Node&;
   using const_reference = const Node&;

   /**
   * @brief Default constructor.
   */
   Tree()
   {
      m_data.reserve(128);
      m_data.emplace_back(DataType{ });

      m_nodes.reserve(128);
      m_nodes.emplace_back(Node{ this, /* ownIndex = */ 0 });
   }

   /**
   * @brief Tree constructs a new Tree with the provided data encapsulated in a new
   * Node.
   */
   template<typename DatumType>
   Tree(DatumType&& datum)
   {
      m_data.reserve(128);
      m_data.emplace_back(std::forward<DatumType>(datum));

      m_nodes.reserve(128);
      m_nodes.emplace_back(Node{ this, /* ownIndex = */ 0 });
   }

   /**
   * @brief Copy constructor.
   *
   * @todo Worry about this later.
   */
   Tree(const Tree<DataType>& other) = delete;

   /**
   * @brief Assignment operator.
   *
   * @todo Worry about this later.
   */
   Tree<DataType>& operator=(Tree<DataType> other) = delete;

   /**
   * @returns A pointer to the head Node.
   */
   Node* GetRoot() noexcept
   {
      assert(m_data.size() > 0 && m_nodes.size() > 0);

      return &m_nodes.front();
   }

   /**
   * @brief Computes the size of either the whole Tree, or a portion thereof.
   *
   * @param[in] node                The node at which to start counting. If this parameter is not
   *                                provided, then the size of the entire tree is returned.
   *
   * @returns The total number of nodes in the Tree. This includes leaf and non-leaf nodes,
   * in addition to the root node.
   */
   auto Size() const noexcept
   {
      assert(m_data.size() == m_nodes.size());

      return m_data.size();
   }

   /**
   * @returns A post-order iterator that will iterator over all nodes in the tree, starting
   * with the head of the Tree.
   */
   typename Tree::PostOrderIterator begin() noexcept
   {
      assert(m_data.size() > 0);
      const auto iterator = Tree<DataType>::PostOrderIterator{ GetRoot() };
      return iterator;
   }

   /**
   * @returns A post-order iterator that points past the end of the Tree.
   */
   typename Tree::PostOrderIterator end() noexcept
   {
      const auto iterator = Tree<DataType>::PostOrderIterator{ };
      return iterator;
   }

   /**
   * @returns A pre-order iterator that will iterate over all Nodes in the tree.
   */
   typename Tree::PreOrderIterator beginPreOrder() noexcept
   {
      assert(m_data.size() > 0);
      const auto iterator = Tree<DataType>::PreOrderIterator{ GetRoot() };
      return iterator;
   }

   /**
   * @returns A pre-order iterator pointing "past" the end of the tree.
   */
   typename Tree::PreOrderIterator endPreOrder() noexcept
   {
      const auto iterator = Tree<DataType>::PreOrderIterator{ };
      return iterator;
   }

   /**
   * @returns An iterator that will iterator over all leaf nodes in the Tree, starting with the
   * left-most leaf in the Tree.
   */
   typename Tree::LeafIterator beginLeaf() noexcept
   {
      assert(m_data.size() > 0);
      const auto iterator = Tree<DataType>::LeafIterator{ GetRoot() };
      return iterator;
   }

   /**
   * @return A LeafIterator that points past the last leaf Node in the Tree.
   */
   typename Tree::LeafIterator endLeaf() noexcept
   {
      const auto iterator = Tree<DataType>::LeafIterator{ };
      return iterator;
   }

   /**
   *
   */
   typename Tree::SiblingIterator beginSibling(const Node& node) noexcept
   {
      const auto iterator = Tree<DataType>::SiblingIterator{ node };
      return iterator;
   }

   /**
   * @todo There is probably no test coverage for this function...
   */
   typename Tree::SiblingIterator endSibling() noexcept
   {
      const auto iterator = Tree<DataType>::SiblingIterator{ };
      return iterator;
   }

   /**
   * @returns The underlying data container.
   */
   const auto& GetDataAsVector() const noexcept
   {
      return m_data;
   }

   /**
   *
   */
   template<typename IteratorType>
   void OptimizeMemoryLayoutFor()
   {
      static_assert(
         std::is_same<IteratorType::value_type, decltype(m_data)::value_type>::value,
         "Types don't match!");

      // Enable Argument Dependent Lookup (ADL):
      using std::swap;

      IteratorType itr{ GetRoot() };

      for (std::size_t sinkIndex{ 0u }; sinkIndex < m_data.size(); ++sinkIndex, ++itr)
      {
         const auto sourceIndex = itr->m_ownIndex;

         if (sourceIndex == sinkIndex)
         {
            continue;
         }

         const auto source = m_nodes[sourceIndex];
         const auto sink = m_nodes[sinkIndex];

         std::swap(m_data[sinkIndex], m_data[sourceIndex]);
         std::swap(m_nodes[sinkIndex], m_nodes[sourceIndex]);

         m_nodes[sinkIndex].m_ownIndex = sinkIndex;
         m_nodes[sourceIndex].m_ownIndex = sourceIndex;

         { // Update parent-child relationships:
            if (source.m_parentIndex == sink.m_ownIndex)
            {
               m_nodes[sinkIndex].m_parentIndex = sourceIndex;

               auto previousSiblingIndex = m_nodes[sinkIndex].m_previousSiblingIndex;
               while (previousSiblingIndex != NOT_SPECIFIED)
               {
                  m_nodes[previousSiblingIndex].m_parentIndex = sourceIndex;
                  previousSiblingIndex = m_nodes[previousSiblingIndex].m_previousSiblingIndex;
               }

               auto nextSiblingIndex = m_nodes[sinkIndex].m_nextSiblingIndex;
               while (nextSiblingIndex != NOT_SPECIFIED)
               {
                  m_nodes[nextSiblingIndex].m_parentIndex = sourceIndex;
                  nextSiblingIndex = m_nodes[nextSiblingIndex].m_nextSiblingIndex;
               }

               if (sink.m_firstChildIndex == source.m_ownIndex)
               {
                  m_nodes[sourceIndex].m_firstChildIndex = sinkIndex;
               }

               if (sink.m_lastChildIndex == source.m_ownIndex)
               {
                  m_nodes[sourceIndex].m_lastChildIndex = sinkIndex;
               }
            }
            else if (sink.m_parentIndex == source.m_ownIndex)
            {
               m_nodes[sourceIndex].m_parentIndex = sinkIndex;

               auto previousSiblingIndex = m_nodes[sourceIndex].m_previousSiblingIndex;
               while (previousSiblingIndex != NOT_SPECIFIED)
               {
                  m_nodes[previousSiblingIndex].m_parentIndex = sinkIndex;
                  previousSiblingIndex = m_nodes[previousSiblingIndex].m_previousSiblingIndex;
               }

               auto nextSiblingIndex = m_nodes[sourceIndex].m_nextSiblingIndex;
               while (nextSiblingIndex != NOT_SPECIFIED)
               {
                  m_nodes[nextSiblingIndex].m_parentIndex = sinkIndex;
                  nextSiblingIndex = m_nodes[nextSiblingIndex].m_nextSiblingIndex;
               }

               if (source.m_firstChildIndex == sink.m_ownIndex)
               {
                  m_nodes[sinkIndex].m_firstChildIndex = sourceIndex;
               }

               if (source.m_lastChildIndex == sink.m_ownIndex)
               {
                  m_nodes[sinkIndex].m_lastChildIndex = sourceIndex;
               }
            }
         }

         { // Update the source node's neighbours:
            if (source.m_nextSiblingIndex != NOT_SPECIFIED)
            {
               m_nodes[source.m_nextSiblingIndex].m_previousSiblingIndex = sinkIndex;
            }

            if (source.m_previousSiblingIndex != NOT_SPECIFIED)
            {
               m_nodes[source.m_previousSiblingIndex].m_nextSiblingIndex = sinkIndex;
            }
         }

         { // Update the sink node's neighbours:
            if (sink.m_nextSiblingIndex != NOT_SPECIFIED)
            {
               m_nodes[sink.m_nextSiblingIndex].m_previousSiblingIndex = sourceIndex;
            }

            if (sink.m_previousSiblingIndex != NOT_SPECIFIED)
            {
               m_nodes[sink.m_previousSiblingIndex].m_nextSiblingIndex = sourceIndex;
            }
         }

         itr.m_currentNode = &m_nodes[sinkIndex];
      }
   }

private:

   std::vector<DataType> m_data;
   std::vector<Node> m_nodes;
};

template<typename DataType>
class Tree<DataType>::Node
{
   friend class Tree;

public:
   using value_type = DataType;
   using reference = DataType&;
   using const_reference = const DataType&;

   /**
   * @brief Constructs and appends a new Node as the last child of the Node.
   *
   * @param[in] data                The underlying data to be stored in the new Node.
   *
   * @returns The newly appended Node.
   */
   template<typename DatumType>
   Node* AppendChild(DatumType&& datum)
   {
      assert(m_tree->m_data.size() == m_tree->m_nodes.size());

      m_tree->m_data.emplace_back(std::forward<DatumType>(datum));
      m_tree->m_nodes.emplace_back(Node{ m_tree, m_tree->m_nodes.size() });

      Node& appendee = m_tree->m_nodes.back();

      appendee.m_parentIndex = m_ownIndex;

      if (m_lastChildIndex == NOT_SPECIFIED)
      {
         assert(m_childCount == 0);

         m_firstChildIndex = appendee.m_ownIndex;
         m_lastChildIndex = m_firstChildIndex;

         ++m_childCount;

         return &m_tree->m_nodes[m_lastChildIndex];
      }

      auto& nodes = m_tree->m_nodes;

      nodes[m_lastChildIndex].m_nextSiblingIndex = appendee.m_ownIndex;
      nodes[nodes[m_lastChildIndex].m_nextSiblingIndex].m_previousSiblingIndex = m_lastChildIndex;
      m_lastChildIndex = nodes[m_lastChildIndex].m_nextSiblingIndex;

      ++m_childCount;

      return &nodes[m_lastChildIndex];
   }

   /**
   * @brief Constructs and prepends a new Node as the first child of the Node.
   *
   * @param[in] data                The underlying data to be stored in the new Node.
   *
   * @returns The newly prepended Node.
   */
   template<typename DatumType>
   Node* PrependChild(DatumType&& datum)
   {
      assert(m_tree->m_data.size() == m_tree->m_nodes.size());

      m_tree->m_data.emplace_back(std::forward<DatumType>(datum));
      m_tree->m_nodes.emplace_back(Node{ m_tree, m_tree->m_nodes.size() });

      Node& prependee = m_tree->m_nodes.back();

      prependee.m_parentIndex = m_ownIndex;

      if (m_firstChildIndex == NOT_SPECIFIED)
      {
         assert(m_childCount == 0);

         m_firstChildIndex = prependee.m_ownIndex;
         m_lastChildIndex = m_firstChildIndex;

         ++m_childCount;

         return &m_tree->m_nodes[m_firstChildIndex];
      }

      auto& nodes = m_tree->m_nodes;

      nodes[m_firstChildIndex].m_previousSiblingIndex = prependee.m_ownIndex;
      nodes[nodes[m_firstChildIndex].m_previousSiblingIndex].m_nextSiblingIndex = m_firstChildIndex;
      m_firstChildIndex = nodes[m_firstChildIndex].m_previousSiblingIndex;

      ++m_childCount;

      return &nodes[m_firstChildIndex];
   }

   /**
   * @todo
   */
   void DeleteFromTree() noexcept
   {
   }

   /**
   * @returns True if this Node has children.
   */
   auto HasChildren() const noexcept
   {
      return m_childCount > 0;
   }

   /**
   * @returns The number of children that this Node has.
   */
   auto GetChildCount() const noexcept
   {
      return m_childCount;
   }

   /**
   *
   */
   auto CountAllDescendants() const noexcept
   {
      const auto nodeCount = std::count_if(
         Tree<DataType>::PostOrderIterator{ this },
         Tree<DataType>::PostOrderIterator{ },
         [](const auto&) noexcept
      {
         return true;
      });

      return nodeCount - 1;
   }

   /**
   * @returns The zero-indexed depth of the TreeNode in its Tree.
   */
   auto Depth() const noexcept
   {
      std::size_t depth = 0;

      auto traversalIndex = m_ownIndex;
      while (m_tree->m_nodes[traversalIndex].m_parentIndex != NOT_SPECIFIED)
      {
         ++depth;
         traversalIndex = m_tree->m_nodes[traversalIndex].m_parentIndex;
      }

      return depth;
   }

   /**
   * @returns The first child Node of this Node.
   */
   Node* GetFirstChild() const noexcept
   {
      return (m_firstChildIndex != NOT_SPECIFIED)
         ? &m_tree->m_nodes[m_firstChildIndex]
         : nullptr;
   }

   /**
   * @returns The last child Node of this Node.
   */
   Node* GetLastChild() const noexcept
   {
      return (m_lastChildIndex != NOT_SPECIFIED)
         ? &m_tree->m_nodes[m_lastChildIndex]
         : nullptr;
   }

   /**
   * @returns The parent Node of this Node.
   */
   Node* GetParent() const noexcept
   {
      return (m_parentIndex != NOT_SPECIFIED)
         ? &m_tree->m_nodes[m_parentIndex]
         : nullptr;
   }

   /**
   * @returns The Node to the right of this Node.
   */
   Node* GetNextSibling() const noexcept
   {
      return (m_nextSiblingIndex != NOT_SPECIFIED)
         ? &m_tree->m_nodes[m_nextSiblingIndex]
         : nullptr;
   }

   /**
   * @returns The Node to the left of this Node.
   */
   Node* GetPreviousSibling() const noexcept
   {
      return (m_previousSiblingIndex != NOT_SPECIFIED)
         ? &m_tree->m_nodes[m_previousSiblingIndex]
         : nullptr;
   }

   /**
   * @returns The encapsulated data.
   */
   auto& GetData() noexcept
   {
      return m_tree->m_data[m_ownIndex];
   }

   /**
   * @returns The encapsulated data.
   */
   const auto& GetData() const noexcept
   {
      return m_tree->m_data[m_ownIndex];
   }

   /**
   * @returns True is the Node is initialized, or valid.
   */
   explicit operator bool() const noexcept
   {
      return m_ownIndex != NOT_SPECIFIED;
   }

   /**
   * @returns True if the data encapsulated in the left-hand side Node is less than
   * the data encapsulated in the right-hand side Node.
   */
   friend auto operator<(const Node& lhs, const Node& rhs) noexcept
   {
      return lhs.m_tree->GetDataAsVector()[lhs.m_ownIndex]
         < rhs.m_tree->GetDataAsVector()[rhs.m_ownIndex];
   }

   /**
   * @returns True if the data encapsulated in the left-hand side Node is less than
   * or equal to the data encapsulated in the right-hand side Node.
   */
   friend auto operator<=(const Node& lhs, const Node& rhs) noexcept
   {
      return !(lhs > rhs);
   }

   /**
   * @returns True if the data encapsulated in the left-hand side Node is greater than
   * the data encapsulated in the right-hand side Node.
   */
   friend auto operator>(const Node& lhs, const Node& rhs) noexcept
   {
      return rhs < lhs;
   }

   /**
   * @returns True if the data encapsulated in the left-hand side Node is greater than
   * or equal to the data encapsulated in the right-hand side Node.
   */
   friend auto operator>=(const Node& lhs, const Node& rhs) noexcept
   {
      return !(lhs < rhs);
   }

   /**
   * @returns True if the data encapsulated in the left-hand side Node is equal to
   * the data encapsulated in the right-hand side Node.
   */
   friend auto operator==(const Node& lhs, const Node& rhs) noexcept
   {
      return lhs.m_tree->GetDataAsVector()[lhs.m_ownIndex]
         == rhs.m_tree->GetDataAsVector()[rhs.m_ownIndex];
   }

   /**
   * @returns True if the data encapsulated in the left-hand side Node is not equal
   * to the data encapsulated in the right-hand side Node.
   */
   friend auto operator!=(const Node& lhs, const Node& rhs) noexcept
   {
      return !(lhs == rhs);
   }

   /**
   * @brief Swaps all member variables of the left-hand side with that of the right-hand side.
   */
   friend void swap(Node& lhs, Node& rhs) noexcept
   {
      // Enable Argument Dependent Lookup (ADL):
      using std::swap;

      swap(lhs.m_tree, rhs.m_tree);
      swap(lhs.m_ownIndex, rhs.m_ownIndex);
      swap(lhs.m_parentIndex, rhs.m_parentIndex);
      swap(lhs.m_firstChildIndex, rhs.m_firstChildIndex);
      swap(lhs.m_lastChildIndex, rhs.m_lastChildIndex);
      swap(lhs.m_previousSiblingIndex, rhs.m_previousSiblingIndex);
      swap(lhs.m_nextSiblingIndex, rhs.m_nextSiblingIndex);
      swap(lhs.m_childCount, rhs.m_childCount);
   }

private:

   /**
   * @todo
   */
   Node(
      Tree* tree,
      std::size_t ownIndex)
      :
      m_tree{ tree },
      m_ownIndex{ ownIndex }
   {
   }

   Tree* m_tree{ nullptr };

   std::size_t m_ownIndex{ NOT_SPECIFIED };
   std::size_t m_parentIndex{ NOT_SPECIFIED };
   std::size_t m_firstChildIndex{ NOT_SPECIFIED };
   std::size_t m_lastChildIndex{ NOT_SPECIFIED };
   std::size_t m_previousSiblingIndex{ NOT_SPECIFIED };
   std::size_t m_nextSiblingIndex{ NOT_SPECIFIED };

   std::size_t m_childCount{ 0 };
};

/**
* @brief The Iterator class
*
* This is the base iterator class that all other iterators (sibling, leaf, post-, pre-, and
* in-order) will derive from. This class can only instantiated by derived types.
*/
template<typename DataType>
class Tree<DataType>::Iterator
{
   friend class Tree;

public:
   // Typedefs needed for STL compliance:
   using value_type = DataType;
   using pointer = DataType*;
   using reference = DataType&;
   using const_reference = const DataType&;
   using size_type = std::size_t;
   using difference_type = std::ptrdiff_t;
   using iterator_category = std::forward_iterator_tag;

   /**
   * @returns The Node pointed to by the Tree::Iterator.
   */
   Node& operator*() noexcept
   {
      return *m_currentNode;
   }

   /**
   * @overload
   */
   const Node& operator*() const noexcept
   {
      return *m_currentNode;
   }

   /**
   * @returns A pointer to the Node.
   */
   Node* const operator&() noexcept
   {
      return m_currentNode;
   }

   /**
   * @overload
   */
   const Node* const operator&() const noexcept
   {
      return m_currentNode;
   }

   /**
   * @returns A pointer to the Node pointed to by the Tree:Iterator.
   */
   Node* const operator->() noexcept
   {
      return m_currentNode;
   }

   /**
   * @overload
   */
   const Node* const operator->() const noexcept
   {
      return m_currentNode;
   }

   /**
   * @returns True if the Iterator points to the same node as the other Iterator,
   * and false otherwise.
   */
   bool operator==(const Iterator& other) const noexcept
   {
      return m_currentIndex == other.m_currentIndex;
   }

   /**
   * @returns True if the Iterator points to the same node as the other Iterator,
   * and false otherwise.
   */
   bool operator!=(const Iterator& other) const noexcept
   {
      return m_currentNode != other.m_currentNode;
   }

protected:

   /**
   * Default constructor.
   */
   Iterator() noexcept = default;

   /**
   * Copy constructor.
   */
   explicit Iterator(const Iterator& other) noexcept :
      m_currentNode{ other.m_currentNode },
      m_startingNode{ other.m_startingNode },
      m_endingNode{ other.m_endingNode }
   {
   }

   /**
   * Constructs a Iterator started at the specified node.
   */
   explicit Iterator(const Node* node) noexcept :
      m_currentNode{ const_cast<Node*>(node) },
      m_startingNode{ const_cast<Node*>(node) }
   {
   }

   Node* m_currentNode{ nullptr };

   const Node* m_startingNode{ nullptr };
   const Node* m_endingNode{ nullptr };
};

/**
* @brief The PreOrderIterator class
*/
template<typename DataType>
class Tree<DataType>::PreOrderIterator final : public Tree<DataType>::Iterator
{
public:
   /**
   * Default constructor.
   */
   PreOrderIterator() noexcept = default;

   /**
   * Constructs an iterator that starts and ends at the specified node.
   */
   explicit PreOrderIterator(const Node* node) noexcept :
      Iterator{ node }
   {
      if (!node)
      {
         return;
      }

      if (node->GetNextSibling())
      {
         m_endingNode = node->GetNextSibling();
      }
      else
      {
         m_endingNode = node;
         while (m_endingNode->GetParent() && !m_endingNode->GetParent()->GetNextSibling())
         {
            m_endingNode = m_endingNode->GetParent();
         }

         if (m_endingNode->GetParent())
         {
            m_endingNode = m_endingNode->GetParent()->GetNextSibling();
         }
         else
         {
            m_endingNode = nullptr;
         }
      }
   }

   /**
   * Prefix increment operator.
   */
   typename Tree::PreOrderIterator& operator++() noexcept
   {
      assert(m_currentNode);
      auto* traversingNode = m_currentNode;

      if (traversingNode->HasChildren())
      {
         traversingNode = traversingNode->GetFirstChild();
      }
      else if (traversingNode->GetNextSibling())
      {
         traversingNode = traversingNode->GetNextSibling();
      }
      else
      {
         while (traversingNode->GetParent() && !traversingNode->GetParent()->GetNextSibling())
         {
            traversingNode = traversingNode->GetParent();
         }

         if (traversingNode->GetParent())
         {
            traversingNode = traversingNode->GetParent()->GetNextSibling();
         }
         else
         {
            traversingNode = nullptr;
         }
      }

      m_currentNode = (traversingNode != m_endingNode) ? traversingNode : nullptr;
      return *this;
   }

   /**
   * Postfix increment operator.
   */
   typename Tree::PreOrderIterator operator++(int) noexcept
   {
      const auto result = *this;
      ++(*this);

      return result;
   }
};

/**
* @brief The PostOrderIterator class
*/
template<typename DataType>
class Tree<DataType>::PostOrderIterator final : public Tree<DataType>::Iterator
{
public:
   /**
   * Default constructor.
   */
   PostOrderIterator() noexcept = default;

   /**
   * Constructs an iterator that starts and ends at the specified node.
   */
   explicit PostOrderIterator(const Node* node) noexcept :
      Iterator{ node }
   {
      assert(node);

      // Compute and set the starting node:

      auto* traversingNode = node;
      while (traversingNode->GetFirstChild())
      {
         traversingNode = traversingNode->GetFirstChild();
      }

      assert(traversingNode);
      m_currentNode = const_cast<Node*>(traversingNode);

      // Commpute and set the ending node:

      if (node->GetNextSibling())
      {
         auto* traversingNode = node->GetNextSibling();
         while (traversingNode->HasChildren())
         {
            traversingNode = traversingNode->GetFirstChild();
         }

         m_endingNode = traversingNode;
      }
      else
      {
         m_endingNode = node->GetParent();
      }
   }

   /**
   * Prefix increment operator.
   */
   typename Tree::PostOrderIterator& operator++() noexcept
   {
      assert(m_currentNode);
      auto* traversingNode = m_currentNode;

      if (traversingNode->HasChildren() && !m_traversingUpTheTree)
      {
         while (traversingNode->GetFirstChild())
         {
            traversingNode = traversingNode->GetFirstChild();
         }
      }
      else if (traversingNode->GetNextSibling())
      {
         m_traversingUpTheTree = false;

         traversingNode = traversingNode->GetNextSibling();
         while (traversingNode->HasChildren())
         {
            traversingNode = traversingNode->GetFirstChild();
         }
      }
      else
      {
         m_traversingUpTheTree = true;

         traversingNode = traversingNode->GetParent();
      }

      m_currentNode = (traversingNode != m_endingNode) ? traversingNode : nullptr;
      return *this;
   }

   /**
   * Postfix increment operator.
   */
   typename Tree::PostOrderIterator operator++(int) noexcept
   {
      const auto result = *this;
      ++(*this);

      return result;
   }

private:

   bool m_traversingUpTheTree{ false };
};

/**
* @brief The LeafIterator class
*/
template<typename DataType>
class Tree<DataType>::LeafIterator final : public Tree<DataType>::Iterator
{
public:
   /**
   * Default constructor.
   */
   LeafIterator() noexcept = default;

   /**
   * Constructs an iterator that starts at the specified node and iterates to the end.
   */
   explicit LeafIterator(const Node* node) noexcept :
      Iterator{ node }
   {
      if (!node)
      {
         return;
      }

      // Compute and set the starting node:

      if (node->HasChildren())
      {
         auto* firstNode = node;
         while (firstNode->GetFirstChild())
         {
            firstNode = firstNode->GetFirstChild();
         }

         m_currentNode = const_cast<Node*>(firstNode);
      }

      // Compute and set the ending node:

      if (node->GetNextSibling())
      {
         auto* lastNode = node->GetNextSibling();
         while (lastNode->HasChildren())
         {
            lastNode = lastNode->GetFirstChild();
         }

         m_endingNode = lastNode;
      }
      else
      {
         m_endingNode = node;
         while (m_endingNode->GetParent() && !m_endingNode->GetParent()->GetNextSibling())
         {
            m_endingNode = m_endingNode->GetParent();
         }

         if (m_endingNode->GetParent())
         {
            m_endingNode = m_endingNode->GetParent()->GetNextSibling();
            while (m_endingNode->HasChildren())
            {
               m_endingNode = m_endingNode->GetFirstChild();
            }
         }
         else
         {
            m_endingNode = nullptr;
         }
      }
   }

   /**
   * Prefix increment operator.
   */
   typename Tree::LeafIterator& operator++() noexcept
   {
      assert(m_currentNode);

      auto* traversingNode = m_currentNode;

      if (traversingNode->HasChildren())
      {
         while (traversingNode->GetFirstChild())
         {
            traversingNode = traversingNode->GetFirstChild();
         }
      }
      else if (traversingNode->GetNextSibling())
      {
         traversingNode = traversingNode->GetNextSibling();

         while (traversingNode->GetFirstChild())
         {
            traversingNode = traversingNode->GetFirstChild();
         }
      }
      else if (traversingNode->GetParent())
      {
         while (traversingNode->GetParent() && !traversingNode->GetParent()->GetNextSibling())
         {
            traversingNode = traversingNode->GetParent();
         }

         if (traversingNode->GetParent())
         {
            traversingNode = traversingNode->GetParent()->GetNextSibling();

            while (traversingNode && traversingNode->HasChildren())
            {
               traversingNode = traversingNode->GetFirstChild();
            }
         }
         else
         {
            traversingNode = nullptr;
         }
      }

      m_currentNode = (traversingNode != m_endingNode) ? traversingNode : nullptr;
      return *this;
   }

   /**
   * Postfix increment operator.
   */
   typename Tree::LeafIterator operator++(int) noexcept
   {
      const auto result = *this;
      ++(*this);

      return result;
   }
};

/**
* @brief The SiblingIterator class
*/
template<typename DataType>
class Tree<DataType>::SiblingIterator final : public Tree<DataType>::Iterator
{
public:
   /**
   * Default constructor.
   */
   SiblingIterator() noexcept = default;

   /**
   * Constructs an iterator that starts at the specified node and iterates to the end.
   */
   explicit SiblingIterator(const Node* node) noexcept :
      Iterator{ node }
   {
   }

   /**
   * Prefix increment operator.
   */
   typename Tree::SiblingIterator& operator++() noexcept
   {
      if (m_currentNode)
      {
         m_currentNode = m_currentNode->GetNextSibling();
      }

      return *this;
   }

   /**
   * Postfix increment operator.
   */
   typename Tree::SiblingIterator operator++(int) noexcept
   {
      const auto result = *this;
      ++(*this);

      return result;
   }
};

#if _WIN64
#define X64 1
#else
#define X86 1
#endif

#if X86
static_assert(
   sizeof(Tree<int>::Node) <= 32,
   "Two Node instances will no longer fit on a typical cache line.");
#elif X64
static_assert(
   sizeof(Tree<int>::Node) <= 64,
   "A single Node instance will not fit on a typical cache line.");
#endif
