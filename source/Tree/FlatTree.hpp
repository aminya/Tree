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
#include <functional>
#include <memory>
#include <vector>
#include <type_traits>

namespace
{
   static constexpr auto NONE{ std::numeric_limits<std::size_t>::max() };
}

/**
* @brief A basic tree, built on top of two std::vectors. Each node in the tree can have any number
* of children.
*
* The data is stored in a dedicated vector of DataType objects, while the metadata is stored in a
* separate vector. This data-oriented design should allow for lightning fast traversals of the data
* for those cases where the nature of the tree cannot be exploited to further increase operations.
*
* For increased cache-friendliness, this class also enables consumers to easily rearrange the
* data in the underlying vectors so that everything is contiguous relative to a chosen traversal
* order.
*
* See the individual member functions for further documentation.
*/
template<typename DataType>
class FlatTree
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
   FlatTree()
   {
      m_data.emplace_back(DataType{ });
      m_nodes.emplace_back(Node{ this, /* ownIndex = */ 0 });
   }

   /**
   * @brief Constructs a new tree with the provided data encapsulated in a new node.
   */
   template<typename DatumType>
   FlatTree(DatumType&& datum)
   {
      m_data.emplace_back(std::forward<DatumType>(datum));
      m_nodes.emplace_back(Node{ this, /* ownIndex = */ 0 });
   }

   /**
   * @brief Copy constructor.
   *
   * @todo Worry about this later.
   */
   FlatTree(const FlatTree<DataType>&) = delete;

   /**
   * @brief Assignment operator.
   *
   * @todo Worry about this later.
   */
   FlatTree<DataType>& operator=(const FlatTree<DataType>&) = delete;

   /**
   * @brief Reserves space for the specified number of nodes in the underlying vectors.
   */
   void Reserve(std::size_t numberOfNodes)
   {
      m_data.reserve(numberOfNodes);
      m_nodes.reserve(numberOfNodes);
   }

   /**
   * @returns The node at the specified index.
   */
   auto& GetNodeAtIndex(std::size_t index)
   {
      return m_nodes[index];
   }

   /**
   * @returns The node at the specified index.
   */
   const auto& GetNodeAtIndex(std::size_t index) const
   {
      return m_nodes[index];
   }

   /**
   * @returns A pointer to the head node.
   */
   Node* GetRoot() noexcept
   {
      assert(m_data.size() > 0 && m_nodes.size() > 0);
      assert(m_data.size() == m_nodes.size());
      assert(m_data.size() > m_rootIndex);

      return &m_nodes[m_rootIndex];
   }

   /**
   * @overload
   */
   const Node* GetRoot() const noexcept
   {
      assert(m_data.size() > 0 && m_nodes.size() > 0);
      assert(m_data.size() == m_nodes.size());
      assert(m_data.size() > m_rootIndex);

      return &m_nodes[m_rootIndex];
   }

   /**
   * @returns The total number of nodes in the tree. This includes leaf and non-leaf nodes,
   * in addition to the root node.
   */
   auto Size() const noexcept
   {
      assert(m_data.size() == m_nodes.size());

      return m_data.size();
   }

   /**
   * @returns A post-order iterator that will iterator over all nodes in the tree, ending
   * at the root of the tree.
   */
   typename FlatTree::PostOrderIterator begin() const noexcept
   {
      assert(m_data.size() > 0);
      return FlatTree<DataType>::PostOrderIterator{ GetRoot() };
   }

   /**
   * @returns A post-order iterator that points "past the end" of the tree.
   */
   typename FlatTree::PostOrderIterator end() const noexcept
   {
      return FlatTree<DataType>::PostOrderIterator{ };
   }

   /**
   * @returns A pre-order iterator that will iterate over all nodes in the tree, starting
   * at the root of the tree.
   */
   typename FlatTree::PreOrderIterator beginPreOrder() const noexcept
   {
      assert(m_data.size() > 0);

      return FlatTree<DataType>::PreOrderIterator{ GetRoot() };
   }

   /**
   * @returns A pre-order iterator pointing "past the end" of the tree.
   */
   typename FlatTree::PreOrderIterator endPreOrder() const noexcept
   {
      return FlatTree<DataType>::PreOrderIterator{ };
   }

   /**
   * @returns A leaf iterator that will iterate over all leaf nodes in the tree, starting
   * with the left-most leaf in the tree.
   */
   typename FlatTree::LeafIterator beginLeaf() const noexcept
   {
      assert(m_data.size() > 0);

      return FlatTree<DataType>::LeafIterator{ GetRoot() };
   }

   /**
   * @returns A leaf iterator that points "past" the last leaf node in the tree.
   */
   typename FlatTree::LeafIterator endLeaf() const noexcept
   {
      return FlatTree<DataType>::LeafIterator{ };
   }

   /**
   * @returns A sibling iterator that will iterator over all subsequent siblings of a node.
   *
   * @todo There is probably no test coverage for this function...
   */
   typename FlatTree::SiblingIterator beginSibling(const Node& node) const noexcept
   {
      return FlatTree<DataType>::SiblingIterator{ node };
   }

   /**
   * @returns A sibling iterator that points "past" the last sibling of a node.
   *
   * @todo There is probably no test coverage for this function...
   */
   typename FlatTree::SiblingIterator endSibling() const noexcept
   {
      return FlatTree<DataType>::SiblingIterator{ };
   }

   /**
   * @brief Each node in the traversal range for which the supplied predicate returns true is
   * detached from the tree.
   *
   * @todo Rewrite this function to eliminate the need for the additional vector.
   * @todo Add additional handling to deal with unreachable nodes.
   *
   * @param[in] begin               An iterator pointing to the starting location.
   * @param[in] end                 An iterator pointing past the last node to iterate over.
   * @param[in] shouldRemove        Predicate that will return true if the node is to be removed,
   *                                and false otherwise. This predicate should have the following
   *                                signature: [...] (const Node& node) -> bool { ... }
   *
   * @returns The number of nodes that have been removed from the tree.
   */
   template<
      typename IteratorType,
      typename PredicateType
   >
   static auto DetachNodeIf(
      const IteratorType& begin,
      const IteratorType& end,
      const PredicateType& shouldRemove)
   {
      std::vector<std::reference_wrapper<Node>> victims;

      std::copy_if(begin, end, std::back_inserter(victims),
         [&] (const auto& node)
      {
         return shouldRemove(node);
      });

      std::size_t bodyCount{ 0 };

      for (auto& node : victims)
      {
         bodyCount += node.get().Detach();
      }

      return bodyCount;
   }

   /**
   * @returns A const reference to the underlying data vector.
   */
   const auto& GetDataAsVector() const noexcept
   {
      return m_data;
   }

   /**
   * @brief Swaps around the nodes in the underlying vectors so that all of the nodes are
   * in the proper traversal order relative to the desired iteration sequence.
   *
   * By making sure that all nodes are contiguous (relative to a specified traversal order),
   * the processor should hopefully be able to take full advantage of cache locality.
   *
   * This function would ideally be called once the tree has been fully constructed, and the
   * majority of subsequent operations avoid modifying the tree's structure.
   *
   * @tparam TraversalTag           One of the supported Traversal types tags:
   *                                   @li PreOrderTraversal
   *                                   @li PostOrderTraversal
   *                                   @li LeafTraversal
   *                                   @li SiblingTraversal
   */
   template<typename TraversalTag>
   void OptimizeMemoryLayoutFor()
   {
      using std::swap; //< Enable Argument Dependent Lookup (ADL)

      TraversalTag::Iterator<DataType> itr{ GetRoot() };
      for (std::size_t sinkIndex{ 0u }; sinkIndex < m_data.size(); ++sinkIndex, ++itr)
      {
         if (!itr)
         {
            break;
         }

         const auto sourceIndex = itr->m_ownIndex;

         if (sourceIndex == sinkIndex)
         {
            continue;
         }
         else if (sourceIndex == m_rootIndex)
         {
            m_rootIndex = sinkIndex;
         }
         else if (sinkIndex == m_rootIndex)
         {
            m_rootIndex = sourceIndex;
         }

         const auto source = m_nodes[sourceIndex];
         const auto sink = m_nodes[sinkIndex];

         std::swap(m_data[sinkIndex], m_data[sourceIndex]);
         std::swap(m_nodes[sinkIndex], m_nodes[sourceIndex]);

         m_nodes[sinkIndex].m_ownIndex = sinkIndex;
         m_nodes[sourceIndex].m_ownIndex = sourceIndex;

         UpdateSiblingRelationships(source, sink, sourceIndex, sinkIndex);
         UpdateAncestryRelationships(source, sink, sourceIndex, sinkIndex);

         itr.m_currentNode = &m_nodes[sinkIndex];
      }
   }

   /**
   *
   */
   void FuzzTestOptimizer(
      std::size_t sourceIndex,
      std::size_t sinkIndex)
   {
      if (sourceIndex == sinkIndex)
      {
         return;
      }
      else if (sourceIndex == m_rootIndex)
      {
         m_rootIndex = sinkIndex;
      }
      else if (sinkIndex == m_rootIndex)
      {
         m_rootIndex = sourceIndex;
      }

      const auto source = m_nodes[sourceIndex];
      const auto sink = m_nodes[sinkIndex];

      std::swap(m_data[sinkIndex], m_data[sourceIndex]);
      std::swap(m_nodes[sinkIndex], m_nodes[sourceIndex]);

      m_nodes[sinkIndex].m_ownIndex = sinkIndex;
      m_nodes[sourceIndex].m_ownIndex = sourceIndex;

      UpdateSiblingRelationships(source, sink, sourceIndex, sinkIndex);
      UpdateAncestryRelationships(source, sink, sourceIndex, sinkIndex);
   }

private:

   /**
   * @brief Helper function to update the tree structure to reflect the fact that the
   * source and sink nodes are swapping places in the underlying vector.
   */
   void UpdateAncestryRelationships(
      const Node& source,
      const Node& sink,
      std::size_t sourceIndex,
      std::size_t sinkIndex) noexcept
   {
      // Update source node's children to point to the new parent location:
      if (source.m_childCount > 0)
      {
         auto childIndex = source.m_firstChildIndex;

         // If the source's first child happens to be the sink:
         if (childIndex == sinkIndex)
         {
            m_nodes[sinkIndex].m_firstChildIndex = sourceIndex;
            childIndex = m_nodes[sourceIndex].m_nextSiblingIndex;
         }

         while (childIndex != NONE)
         {
            if (childIndex == sinkIndex)
            {
               // If the source's last child happens to be the sink:
               if (source.m_lastChildIndex == sinkIndex)
               {
                  m_nodes[sinkIndex].m_lastChildIndex = sourceIndex;
               }

               childIndex = m_nodes[childIndex].m_nextSiblingIndex;
               continue;
            }

            m_nodes[childIndex].m_parentIndex = sinkIndex;
            childIndex = m_nodes[childIndex].m_nextSiblingIndex;
         }
      }

      // Update sink's parent to point to its child's new location:
      if (sink.m_parentIndex != NONE)
      {
         if (sink.m_parentIndex == sourceIndex)
         {
            m_nodes[sourceIndex].m_parentIndex = sinkIndex;
         }
         else
         {
            if (m_nodes[sink.m_parentIndex].m_firstChildIndex == sinkIndex)
            {
               m_nodes[sink.m_parentIndex].m_firstChildIndex = sourceIndex;
            }

            if (m_nodes[sink.m_parentIndex].m_lastChildIndex == sinkIndex)
            {
               m_nodes[sink.m_parentIndex].m_lastChildIndex = sourceIndex;
            }
         }
      }

      // Update sink node's children to point to the new parent location:
      if (sink.m_childCount > 0)
      {
         auto childIndex = sink.m_firstChildIndex;

         // If the sink's first child happens to be the source:
         if (childIndex == sourceIndex)
         {
            m_nodes[sourceIndex].m_firstChildIndex = sinkIndex;
            childIndex = m_nodes[sinkIndex].m_nextSiblingIndex;
         }

         while (childIndex != NONE)
         {
            if (childIndex == sourceIndex)
            {
               // If the sink's last child happens to be the source:
               if (sink.m_lastChildIndex == sourceIndex)
               {
                  m_nodes[sourceIndex].m_lastChildIndex = sinkIndex;
               }

               childIndex = m_nodes[childIndex].m_nextSiblingIndex;
               continue;
            }

            m_nodes[childIndex].m_parentIndex = sourceIndex;
            childIndex = m_nodes[childIndex].m_nextSiblingIndex;
         }
      }

      // Update source node's parent to point to the new child location:
      if (source.m_parentIndex != NONE)
      {
         if (source.m_parentIndex == sinkIndex)
         {
            m_nodes[sinkIndex].m_parentIndex = sourceIndex;
         }
         else
         {
            if (m_nodes[source.m_parentIndex].m_firstChildIndex == sourceIndex)
            {
               m_nodes[source.m_parentIndex].m_firstChildIndex = sinkIndex;
            }

            if (m_nodes[source.m_parentIndex].m_lastChildIndex == sourceIndex)
            {
               m_nodes[source.m_parentIndex].m_lastChildIndex = sinkIndex;
            }
         }
      }
   }

   /**
   * @brief Helper function to update the tree structure to reflect the fact that the
   * source and sink nodes are swapping places in the underlying vector.
   */
   void UpdateSiblingRelationships(
      const Node& source,
      const Node& sink,
      std::size_t sourceIndex,
      std::size_t sinkIndex) noexcept
   {
      // Update the source node's neighbors:
      if (source.m_nextSiblingIndex != NONE)
      {
         if (source.m_nextSiblingIndex != sinkIndex)
         {
            m_nodes[source.m_nextSiblingIndex].m_previousSiblingIndex = sinkIndex;
         }
         else
         {
            m_nodes[sinkIndex].m_nextSiblingIndex = sourceIndex;
         }
      }

      if (source.m_previousSiblingIndex != NONE)
      {
         if (source.m_previousSiblingIndex != sinkIndex)
         {
            m_nodes[source.m_previousSiblingIndex].m_nextSiblingIndex = sinkIndex;
         }
         else
         {
            m_nodes[sinkIndex].m_previousSiblingIndex = sourceIndex;
         }
      }

      // Update the sink node's neighbors:
      if (sink.m_nextSiblingIndex != NONE)
      {
         if (sink.m_nextSiblingIndex != sourceIndex)
         {
            m_nodes[sink.m_nextSiblingIndex].m_previousSiblingIndex = sourceIndex;
         }
         else
         {
            m_nodes[sourceIndex].m_nextSiblingIndex = sinkIndex;
         }
      }

      if (sink.m_previousSiblingIndex != NONE)
      {
         if (sink.m_previousSiblingIndex != sourceIndex)
         {
            m_nodes[sink.m_previousSiblingIndex].m_nextSiblingIndex = sourceIndex;
         }
         else
         {
            m_nodes[sourceIndex].m_previousSiblingIndex = sinkIndex;
         }
      }
   }

   std::vector<DataType> m_data;
   std::vector<Node> m_nodes;

   std::size_t m_rootIndex{ 0 };
};

/**
* @brief The node that the tree is built out of.
*/
template<typename DataType>
class FlatTree<DataType>::Node
{
   friend class FlatTree;

public:
   using value_type = DataType;
   using reference = DataType&;
   using const_reference = const DataType&;

   /**
   * @brief Constructs and appends a new node as the last child of this node.
   *
   * @warning Since the Tree class stores nodes in a vector, calls to this function may result
   * in a reallocation of the vector's buffer. If that happens, all pointers and references to
   * existing nodes will be invalidated!
   *
   * @param[in] data                The underlying data to be stored in the new node.
   *
   * @returns A pointer to the newly appended node.
   */
   Node* AppendChild(DataType datum)
   {
      assert(m_tree);
      assert(m_tree->m_data.size() == m_tree->m_nodes.size());

      auto& tree = *m_tree;
      const auto ownIndex = m_ownIndex;

      tree.m_data.emplace_back(std::move(datum));
      tree.m_nodes.emplace_back(Node{ &tree, tree.m_nodes.size() });

      Node& appendee = tree.m_nodes.back();
      appendee.m_parentIndex = ownIndex;

      auto& nodes = tree.m_nodes;
      auto& firstChildIndex = nodes[ownIndex].m_firstChildIndex;
      auto& lastChildIndex = nodes[ownIndex].m_lastChildIndex;

      if (lastChildIndex == NONE)
      {
         firstChildIndex = appendee.m_ownIndex;
         lastChildIndex = firstChildIndex;

         ++nodes[ownIndex].m_childCount;

         return &nodes[appendee.m_ownIndex];
      }

      nodes[lastChildIndex].m_nextSiblingIndex = appendee.m_ownIndex;
      nodes[nodes[lastChildIndex].m_nextSiblingIndex].m_previousSiblingIndex = lastChildIndex;
      lastChildIndex = nodes[lastChildIndex].m_nextSiblingIndex;

      ++nodes[ownIndex].m_childCount;

      return &nodes[lastChildIndex];
   }

   /**
   * @brief Appends an existing node (along with its children) to this node.
   *
   * @note The source node must be from a different tree.
   */
   void AppendChild(const Node& node)
   {
      if (node.m_tree == m_tree)
      {
         assert(!"The node you're trying to add is already in this tree!");
         return;
      }

      auto endItr = FlatTree<DataType>::PreOrderIterator{ &node };
      auto endIndex = endItr.m_endingNode ? endItr.m_endingNode->GetIndex() : NONE;

      auto* source = &node;
      auto sinkIndex = m_ownIndex;

      auto& nodes = m_tree->m_nodes;

      auto lastAppendeeIndex = NONE;

      while (source && source->GetIndex() != endIndex)
      {
         lastAppendeeIndex = nodes[sinkIndex].AppendChild(source->GetData())->GetIndex();

         if (source->HasChildren())
         {
            source = source->GetFirstChild();
            sinkIndex = nodes[lastAppendeeIndex].GetIndex();
         }
         else if (source->GetNextSibling())
         {
            source = source->GetNextSibling();
         }
         else
         {
            while (source->GetParent() && !source->GetParent()->GetNextSibling())
            {
               source = source->GetParent();
               sinkIndex = nodes[sinkIndex].GetParent()->GetIndex();
            }

            if (source->GetParent())
            {
               source = source->GetParent()->GetNextSibling();
               sinkIndex = nodes[sinkIndex].GetParent()->GetIndex();
            }
            else
            {
               source = nullptr;
            }
         }
      }
   }

   /**
   * @brief Constructs and prepends a new node as the first child of this node.
   *
   * @warning Since the Tree class stores nodes in a vector, calls to this function may result
   * in a reallocation of the vector's buffer. If that happens, all pointers and references to
   * existing nodes will be invalidated!
   *
   * @param[in] data                The underlying data to be stored in the new node.
   *
   * @returns A pointer to the newly prepended node.
   */
   Node* PrependChild(DataType datum)
   {
      assert(m_tree);
      assert(m_tree->m_data.size() == m_tree->m_nodes.size());

      auto& tree = *m_tree;
      const auto ownIndex = m_ownIndex;

      tree.m_data.emplace_back(std::move(datum));
      tree.m_nodes.emplace_back(Node{ &tree, tree.m_nodes.size() });

      Node& prependee = tree.m_nodes.back();
      prependee.m_parentIndex = ownIndex;

      auto& nodes = tree.m_nodes;
      auto& firstChildIndex = nodes[ownIndex].m_firstChildIndex;
      auto& lastChildIndex = nodes[ownIndex].m_lastChildIndex;

      if (firstChildIndex == NONE)
      {
         firstChildIndex = prependee.m_ownIndex;
         lastChildIndex = firstChildIndex;

         ++nodes[ownIndex].m_childCount;

         return &nodes[prependee.m_ownIndex];
      }

      nodes[firstChildIndex].m_previousSiblingIndex = prependee.m_ownIndex;
      nodes[nodes[firstChildIndex].m_previousSiblingIndex].m_nextSiblingIndex = firstChildIndex;
      firstChildIndex = nodes[firstChildIndex].m_previousSiblingIndex;

      ++nodes[ownIndex].m_childCount;

      return &nodes[firstChildIndex];
   }

   /**
   * @brief Appends an existing node (along with its children) to this node.
   *
   * @note The source node must be from a different tree.
   */
   void PrependChild(const Node& node)
   {
      if (node.m_tree == m_tree)
      {
         assert(!"The node you're trying to add is already in this tree!");
         return;
      }

      auto endItr = FlatTree<DataType>::PreOrderIterator{ &node };
      auto endIndex = endItr.m_endingNode ? endItr.m_endingNode->GetIndex() : NONE;

      auto* source = &node;
      auto sinkIndex = m_ownIndex;

      auto& nodes = m_tree->m_nodes;

      auto lastAppendeeIndex = NONE;

      while (source && source->GetIndex() != endIndex)
      {
         if (lastAppendeeIndex != NONE)
         {
            lastAppendeeIndex = nodes[sinkIndex].AppendChild(source->GetData())->GetIndex();
         }
         else
         {
            lastAppendeeIndex = nodes[sinkIndex].PrependChild(source->GetData())->GetIndex();
         }

         if (source->HasChildren())
         {
            source = source->GetFirstChild();
            sinkIndex = nodes[lastAppendeeIndex].GetIndex();
         }
         else if (source->GetNextSibling())
         {
            source = source->GetNextSibling();
         }
         else
         {
            while (source->GetParent() && !source->GetParent()->GetNextSibling())
            {
               source = source->GetParent();
               sinkIndex = nodes[sinkIndex].GetParent()->GetIndex();
            }

            if (source->GetParent())
            {
               source = source->GetParent()->GetNextSibling();
               sinkIndex = nodes[sinkIndex].GetParent()->GetIndex();
            }
            else
            {
               source = nullptr;
            }
         }
      }
   }

   /**
   * @returns The index of the node in the underlying vectors.
   */
   inline auto GetIndex() const
   {
      return m_ownIndex;
   }

   /**
   *
   */
   inline auto& GetTree()
   {
      return *m_tree;
   }

   /**
   * @brief Removes the TreeNode from the tree structure, updating all surrounding links
   * as appropriate.
   *
   * @note This function does not actually reclaim the memory occupied by the doomed node.
   * The memory will be reclaimed once the tree that the node was a part of prior to detachment
   * reaches the end of its life.
   */
   std::size_t Detach() noexcept
   {
      DetachFromParentAndSiblings();

      if (m_childCount == 0)
      {
         return 1;
      }

      std::vector<std::reference_wrapper<Node>> victims;
      victims.reserve(m_childCount);

      std::copy(
         FlatTree<DataType>::PostOrderIterator{ this },
         FlatTree<DataType>::PostOrderIterator{ },
         std::back_inserter(victims));

      for (auto& node : victims)
      {
         node.get().m_tree = nullptr;
         node.get().m_ownIndex = NONE;
      }

      return victims.size();
   }

   /**
   * @returns True if this node has children.
   */
   inline auto HasChildren() const noexcept
   {
      return m_childCount > 0;
   }

   /**
   * @returns The number of children that this node has.
   */
   inline auto GetChildCount() const noexcept
   {
      return m_childCount;
   }

   /**
   * @returns The total number of descendants that this node has.
   */
   auto CountAllDescendants() const noexcept
   {
      const auto nodeCount = std::count_if(
         FlatTree<DataType>::PostOrderIterator{ this },
         FlatTree<DataType>::PostOrderIterator{ },
         [] (const auto&) noexcept
      {
         return true;
      });

      constexpr decltype(nodeCount) currentNode{ 1 };
      return nodeCount - currentNode;
   }

   /**
   * @returns The zero-indexed depth of this node in the tree.
   */
   auto Depth() const noexcept
   {
      std::size_t depth = 0;

      auto traversalIndex = m_ownIndex;
      while (m_tree->m_nodes[traversalIndex].m_parentIndex != NONE)
      {
         ++depth;
         traversalIndex = m_tree->m_nodes[traversalIndex].m_parentIndex;
      }

      return depth;
   }

   /**
   * @returns The first (or left-most) child of this node.
   */
   inline Node* GetFirstChild() const noexcept
   {
      return (m_firstChildIndex != NONE)
         ? &m_tree->m_nodes[m_firstChildIndex]
         : nullptr;
   }

   /**
   * @returns The last (or right-most) child of this node.
   */
   inline Node* GetLastChild() const noexcept
   {
      return (m_lastChildIndex != NONE)
         ? &m_tree->m_nodes[m_lastChildIndex]
         : nullptr;
   }

   /**
   * @returns The parent of this node.
   */
   inline Node* GetParent() const noexcept
   {
      return (m_parentIndex != NONE)
         ? &m_tree->m_nodes[m_parentIndex]
         : nullptr;
   }

   /**
   * @returns The node to the right of this node in the tree.
   */
   inline Node* GetNextSibling() const noexcept
   {
      return (m_nextSiblingIndex != NONE)
         ? &m_tree->m_nodes[m_nextSiblingIndex]
         : nullptr;
   }

   /**
   * @returns The node to the left of this node in the tree.
   */
   inline Node* GetPreviousSibling() const noexcept
   {
      return (m_previousSiblingIndex != NONE)
         ? &m_tree->m_nodes[m_previousSiblingIndex]
         : nullptr;
   }

   /**
   * @returns The encapsulated data.
   */
   inline auto& GetData() noexcept
   {
      return m_tree->m_data[m_ownIndex];
   }

   /**
   * @returns The encapsulated data.
   */
   inline const auto& GetData() const noexcept
   {
      return m_tree->m_data[m_ownIndex];
   }

   /**
   * @returns True if this node is a valid member of a tree.
   */
   inline explicit operator bool() const noexcept
   {
      return m_ownIndex != NONE;
   }

   /**
   * @returns True if the data encapsulated in the left-hand side node is less than
   * the data encapsulated in the right-hand side node.
   */
   friend auto operator<(const Node& lhs, const Node& rhs) noexcept
   {
      const auto& lhsVector = lhs.m_tree->GetDataAsVector();
      const auto& rhsVector = rhs.m_tree->GetDataAsVector();

      return lhsVector[lhs.m_ownIndex] < rhsVector[rhs.m_ownIndex];
   }

   /**
   * @returns True if the data encapsulated in the left-hand side node is less than
   * or equal to the data encapsulated in the right-hand side node.
   */
   friend auto operator<=(const Node& lhs, const Node& rhs) noexcept
   {
      return !(lhs > rhs);
   }

   /**
   * @returns True if the data encapsulated in the left-hand side node is greater than
   * the data encapsulated in the right-hand side node.
   */
   friend auto operator>(const Node& lhs, const Node& rhs) noexcept
   {
      return rhs < lhs;
   }

   /**
   * @returns True if the data encapsulated in the left-hand side node is greater than
   * or equal to the data encapsulated in the right-hand side node.
   */
   friend auto operator>=(const Node& lhs, const Node& rhs) noexcept
   {
      return !(lhs < rhs);
   }

   /**
   * @returns True if the data encapsulated in the left-hand side node is equal to
   * the data encapsulated in the right-hand side node.
   */
   friend auto operator==(const Node& lhs, const Node& rhs) noexcept
   {
      const auto& lhsVector = lhs.m_tree->GetDataAsVector();
      const auto& rhsVector = rhs.m_tree->GetDataAsVector();

      return lhsVector[lhs.m_ownIndex] == rhsVector[rhs.m_ownIndex];
   }

   /**
   * @returns True if the data encapsulated in the left-hand side node is not equal
   * to the data encapsulated in the right-hand side node.
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

   /**
   * @brief Performs a merge sort of the node's children.
   *
   * @param[in] comparator          A callable type to be used as the basis for the sorting
   *                                comparison. This type should have the following signature:
   *                                   [...] (const Tree<DataType>::Node& lhs,
                                              const Tree<DataType>::Node& rhs) -> bool {...}
   */
   template<typename ComparatorType>
   void SortChildren(const ComparatorType& comparator) noexcept(noexcept(comparator))
   {
      if (m_childCount == 0)
      {
         return;
      }

      MergeSort(m_firstChildIndex, comparator);
   }

private:

   /**
   * @brief Main private entry point into the merge sort implementation.
   *
   * @param[in, out] leftMostChildIndex   The index of the left-most child where sorting is to
   *                                      begin.
   * @param[in] comparator                The predicate to be invoked in order to figure out which
   *                                      node is the lesser of the two.
   */
   template<typename ComparatorType>
   void MergeSort(
      std::size_t& leftMostChildIndex,
      const ComparatorType& comparator) noexcept(noexcept(comparator))
   {
      assert(m_tree);
      auto& nodes = m_tree->m_nodes;

      if (leftMostChildIndex == NONE || nodes[leftMostChildIndex].m_nextSiblingIndex == NONE)
      {
         return;
      }

      auto lhs = NONE;
      auto rhs = NONE;

      DivideChildren(leftMostChildIndex, lhs, rhs);

      MergeSort(lhs, comparator);
      MergeSort(rhs, comparator);

      leftMostChildIndex = MergeSortedHalves(lhs, rhs, comparator);
   }

   /**
   * @brief DivideList is a helper function that will divide the specified TreeNode list in two.
   *
   * @param[in] start               The first child in the sequence of consecutive children that
   *                                is to be divided in half.
   * @param[in, out] lhs            The index of the first child of the left subdivision.
   * @param[in, out] rhs            The index of the first child of the right subdivision.
   */
   void DivideChildren(
      std::size_t start,
      std::size_t& lhs,
      std::size_t& rhs) noexcept
   {
      assert(m_tree);
      auto& nodes = m_tree->m_nodes;

      auto tortoise = start;
      auto hare = nodes[tortoise].m_nextSiblingIndex;

      while (hare != NONE)
      {
         hare = nodes[hare].m_nextSiblingIndex;
         if (hare != NONE)
         {
            tortoise = nodes[tortoise].m_nextSiblingIndex;
            hare = nodes[hare].m_nextSiblingIndex;
         }
      }

      lhs = start;
      rhs = nodes[tortoise].m_nextSiblingIndex;

      nodes[tortoise].m_nextSiblingIndex = NONE;
   }

   /**
   * @brief Helper function to merge the two sorted halves back together again.
   *
   * @param[in, out] lhs            The index of the first child of the sorted left subdivision.
   * @param[in, out] rhs            The index of the first child of the sorted right subdivision.
   * @param[in] comparator          The predicate to be invoked in order to figure out which node
   *                                is the lesser of the two.
   *
   * @returns The index of the first child in the sequence of merged and sorted children.
   */
   template<typename ComparatorType>
   auto MergeSortedHalves(
      std::size_t& lhs,
      std::size_t& rhs,
      const ComparatorType& comparator) noexcept(noexcept(comparator))
   {
      assert(m_tree);
      auto& nodes = m_tree->m_nodes;

      auto head = NONE;
      if (comparator(nodes[lhs], nodes[rhs]))
      {
         head = lhs;
         lhs = nodes[lhs].m_nextSiblingIndex;
      }
      else
      {
         head = rhs;
         rhs = nodes[rhs].m_nextSiblingIndex;
      }

      nodes[head].m_previousSiblingIndex = NONE;

      auto tail = head;

      while (lhs != NONE && rhs != NONE)
      {
         if (comparator(nodes[lhs], nodes[rhs]))
         {
            nodes[tail].m_nextSiblingIndex = lhs;
            tail = nodes[tail].m_nextSiblingIndex;

            lhs = nodes[lhs].m_nextSiblingIndex;

            if (lhs != NONE)
            {
               nodes[lhs].m_previousSiblingIndex = NONE;
            }
         }
         else
         {
            nodes[tail].m_nextSiblingIndex = rhs;
            tail = nodes[tail].m_nextSiblingIndex;

            rhs = nodes[tail].m_nextSiblingIndex;

            if (rhs != NONE)
            {
               nodes[rhs].m_previousSiblingIndex = NONE;
            }
         }
      }

      while (lhs != NONE)
      {
         nodes[tail].m_nextSiblingIndex = lhs;
         tail = nodes[tail].m_nextSiblingIndex;

         lhs = nodes[lhs].m_nextSiblingIndex;

         if (lhs != NONE)
         {
            nodes[lhs].m_previousSiblingIndex = NONE;
         }
      }

      while (rhs != NONE)
      {
         nodes[tail].m_nextSiblingIndex = rhs;
         tail = nodes[tail].m_nextSiblingIndex;

         rhs = nodes[rhs].m_nextSiblingIndex;

         if (rhs != NONE)
         {
            nodes[rhs].m_previousSiblingIndex = NONE;
         }
      }

      return head;
   }

   /**
   * @brief Detaches the node from its parent and siblings.
   *
   * @note This function leaves the relationship with its children intact. This is useful for
   * further processing.
   */
   void DetachFromParentAndSiblings()
   {
      assert(m_tree);
      auto& nodes = m_tree->m_nodes;

      if (m_previousSiblingIndex != NONE && m_nextSiblingIndex != NONE)
      {
         nodes[m_previousSiblingIndex].m_nextSiblingIndex = m_nextSiblingIndex;
         nodes[m_nextSiblingIndex].m_previousSiblingIndex = m_previousSiblingIndex;
      }
      else if (m_previousSiblingIndex != NONE)
      {
         nodes[m_previousSiblingIndex].m_nextSiblingIndex = NONE;
      }
      else if (m_nextSiblingIndex != NONE)
      {
         nodes[m_nextSiblingIndex].m_previousSiblingIndex = NONE;
      }

      if (m_parentIndex == NONE)
      {
         return;
      }

      if (nodes[m_parentIndex].m_firstChildIndex == nodes[m_parentIndex].m_lastChildIndex)
      {
         nodes[m_parentIndex].m_firstChildIndex = NONE;
         nodes[m_parentIndex].m_lastChildIndex = NONE;
      }
      else if (nodes[m_parentIndex].m_firstChildIndex == m_ownIndex)
      {
         assert(nodes[nodes[m_parentIndex].m_firstChildIndex].m_nextSiblingIndex != NONE);

         nodes[m_parentIndex].m_firstChildIndex =
            nodes[nodes[m_parentIndex].m_firstChildIndex].m_nextSiblingIndex;
      }
      else if (nodes[m_parentIndex].m_lastChildIndex == m_ownIndex)
      {
         assert(nodes[nodes[m_parentIndex].m_lastChildIndex].m_previousSiblingIndex != NONE);

         nodes[m_parentIndex].m_lastChildIndex =
            nodes[nodes[m_parentIndex].m_lastChildIndex].m_previousSiblingIndex;
      }

      --(nodes[m_parentIndex].m_childCount);
   }

   /**
   * @brief Private constructor to be used by the Tree class when it creates new nodes and inserts
   * them into the tree.
   */
   Node(
      FlatTree* tree,
      std::size_t ownIndex)
      :
      m_tree{ tree },
      m_ownIndex{ ownIndex }
   {
   }

   FlatTree* m_tree{ nullptr };

   std::size_t m_ownIndex{ NONE };
   std::size_t m_parentIndex{ NONE };
   std::size_t m_firstChildIndex{ NONE };
   std::size_t m_lastChildIndex{ NONE };
   std::size_t m_previousSiblingIndex{ NONE };
   std::size_t m_nextSiblingIndex{ NONE };

   std::size_t m_childCount{ 0 };
};

/**
* @brief The Iterator class
*
* This is the base iterator class that all other iterators (sibling, leaf, pre-, and post-order)
* will derive from. This class can only instantiated by derived types.
*/
template<typename DataType>
class FlatTree<DataType>::Iterator
{
   friend class FlatTree;
   friend class Node;

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
   * @returns True if the iterator is not "past the end."
   */
   explicit operator bool() const noexcept
   {
      return m_currentNode != nullptr;
   }

   /**
   * @returns The node pointed to by the Tree::Iterator.
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
   * @returns A pointer to the node.
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
   * @returns A pointer to the node pointed to by the Tree:Iterator.
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
   * @returns True if the iterator points to the same node as the other iterator,
   * and false otherwise.
   */
   auto operator==(const Iterator& other) const noexcept
   {
      return m_currentIndex == other.m_currentIndex;
   }

   /**
   * @returns True if the iterator points to the same node as the other iterator,
   * and false otherwise.
   */
   auto operator!=(const Iterator& other) const noexcept
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
   Iterator(const Iterator&) noexcept = default;

   /**
   * Constructs a iterator started at the specified node.
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
class FlatTree<DataType>::PreOrderIterator final : public FlatTree<DataType>::Iterator
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
   typename FlatTree::PreOrderIterator& operator++() noexcept
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
   typename FlatTree::PreOrderIterator operator++(int) noexcept
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
class FlatTree<DataType>::PostOrderIterator final : public FlatTree<DataType>::Iterator
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

      // Compute and set the ending node:

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
   typename FlatTree::PostOrderIterator& operator++() noexcept
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
   typename FlatTree::PostOrderIterator operator++(int) noexcept
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
class FlatTree<DataType>::LeafIterator final : public FlatTree<DataType>::Iterator
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
   typename FlatTree::LeafIterator& operator++() noexcept
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
   typename FlatTree::LeafIterator operator++(int) noexcept
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
class FlatTree<DataType>::SiblingIterator final : public FlatTree<DataType>::Iterator
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
   typename FlatTree::SiblingIterator& operator++() noexcept
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
   typename FlatTree::SiblingIterator operator++(int) noexcept
   {
      const auto result = *this;
      ++(*this);

      return result;
   }
};

/**
* @brief Type tag for use with Tree<DataType>::OptimizeMemoryLayoutFor<...>.
*/
struct PreOrderTraversal
{
   template<typename DataType>
   using Iterator = typename FlatTree<DataType>::PreOrderIterator;
};

/**
* @brief Type tag for use with Tree<DataType>::OptimizeMemoryLayoutFor<...>.
*/
struct PostOrderTraversal
{
   template<typename DataType>
   using Iterator = typename FlatTree<DataType>::PostOrderIterator;
};

/**
* @brief Type tag for use with Tree<DataType>::OptimizeMemoryLayoutFor<...>.
*/
struct LeafTraversal
{
   template<typename DataType>
   using Iterator = typename FlatTree<DataType>::LeafIterator;
};

/**
* @brief Type tag for use with Tree<DataType>::OptimizeMemoryLayoutFor<...>.
*/
struct SiblingTraversal
{
   template<typename DataType>
   using Iterator = typename FlatTree<DataType>::SiblingIterator;
};

#if _WIN64
#define X64
#else
#define X86
#endif

#ifdef X86
static_assert(
   sizeof(FlatTree<int>::Node) <= 32,
   "Two Node instances will no longer fit on a typical cache line.");
#elif defined(X64)
static_assert(
   sizeof(FlatTree<int>::Node) <= 64,
   "A single Node instance will not fit on a typical cache line.");
#endif
