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
#include <type_traits>
#include <vector>

/**
* The Tree class declares a basic tree, built on top of templatized Tree<DataType>::Node objects.
*
* Each tree consists of a simple head Tree<DataType>::Node and nothing else.
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
      m_nodes.reserve(128);
      m_nodes.emplace_back(std::make_shared<Node>());
   }

   /**
   * @brief Tree constructs a new Tree with the provided data encapsulated in a new
   * Node.
   */
   Tree(DataType data)
   {
      m_nodes.reserve(128);
      m_nodes.emplace_back(std::make_shared<Node>(std::move(data)));
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
   inline std::shared_ptr<Node> GetHead() const noexcept
   {
      assert(m_nodes.size() > 0);
      return m_nodes.front();
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
   auto Size(std::shared_ptr<Node> node = nullptr) const noexcept
   {
      if (!node)
      {
         return m_nodes.size();
      }

      if (m_nodes.size() == 0)
      {
         return std::size_t{ 0 };
      }

      const std::size_t descendantCount = std::count_if(
         PostOrderIterator{ *node },
         PostOrderIterator{ },
         [](const auto&) noexcept
      {
         return true;
      });

      return descendantCount;
   }

   /**
   * @brief Adds a new child node to the specified parent node as the last child.
   *
   * @param[in] data                The data to be encapsulated in the newly created node.
   * @param[in] parent              The node to which the new node is to be appended.
   *
   * @returns A reference to the newly appended node.
   */
   Node& AppendChild(
      const DataType& data,
      Node& parent)
   {
      const auto child = std::make_shared<Node>(data);
      child->m_parent = &parent;

      if (!parent.m_lastChild)
      {
         assert(parent.m_childCount == 0);
         parent.m_firstChild = child;
         parent.m_lastChild = child;

         ++(parent.m_childCount);
         m_nodes.emplace_back(std::move(child));

         assert(m_nodes.size() > 0);
         return *m_nodes.back();
      }

      assert(parent.m_lastChild);

      parent.m_lastChild->m_nextSibling = child;
      parent.m_lastChild->m_nextSibling->m_previousSibling = parent.m_lastChild;
      parent.m_lastChild = child;

      ++(parent.m_childCount);
      m_nodes.emplace_back(std::move(child));

      assert(m_nodes.size() > 0);
      return *m_nodes.back();
   }

   /**
   * @brief Adds a new child node to the specified parent node as the first child.
   *
   * @param[in] data                The data to be encapsulated in the newly created node.
   * @param[in] parent              The node to which the new node is to be prepended.
   *
   * @returns A reference to the newly prepended node.
   */
   Node& PrependChild(
      const DataType& data,
      Node& parent)
   {
      const auto child = std::make_shared<Node>(data);
      child->m_parent = &parent;

      if (!parent.m_firstChild)
      {
         assert(parent.m_childCount == 0);
         parent.m_firstChild = child;
         parent.m_lastChild = child;

         ++(parent.m_childCount);
         m_nodes.emplace_back(std::move(child));

         assert(m_nodes.size() > 0);
         return *m_nodes.back();
      }

      assert(parent.m_firstChild);

      parent.m_firstChild->m_previousSibling = child;
      parent.m_firstChild->m_previousSibling->m_nextSibling = parent.m_firstChild;
      parent.m_firstChild = child;

      ++(parent.m_childCount);
      m_nodes.emplace_back(std::move(child));

      assert(m_nodes.size() > 0);
      return *m_nodes.back();
   }

   /**
   *
   */
   void AppendSibling(
      const DataType& data,
      Node& predecessor)
   {
      // @todo
   }

   /**
   *
   */
   void PrependSibling(
      const DataType& data,
      Node& successor)
   {
      // @todo
   }

   /**
   * @brief Detaches the given node from the tree.
   *
   * If nothing else references the detached node, the node will be freed.
   */
   void Detach(std::shared_ptr<Node>& node)
   {
      node->DetachFromTree();

      // If no one else is referencing the node, then the reference in the std::vector should be
      // the only reference that's left. If that's the case, we can reclaim a bit of memory:
      if (node.use_count() == 1)
      {
         // @todo Add test coverage...
         node.reset();
      }
   }

   /**
   * @returns A post-order iterator that will iterator over all nodes in the tree, starting
   * with the head of the Tree.
   */
   typename Tree::PostOrderIterator begin() const noexcept
   {
      assert(m_nodes.size() > 0);
      const auto iterator = Tree<DataType>::PostOrderIterator{ *GetHead() };
      return iterator;
   }

   /**
   * @returns A post-order iterator that points past the end of the Tree.
   */
   typename Tree::PostOrderIterator end() const noexcept
   {
      const auto iterator = Tree<DataType>::PostOrderIterator{ };
      return iterator;
   }

   /**
   * @returns The zero-indexed depth of the Node in its Tree.
   */
   static auto Depth(Node& node) noexcept
   {
      std::size_t depth = 0;

      Node* nodePtr = &node;
      while (nodePtr->GetParent())
      {
         ++depth;
         nodePtr = nodePtr->GetParent();
      }

      return depth;
   }

   /**
   * @returns A pre-order iterator that will iterate over all Nodes in the tree.
   */
   typename Tree::PreOrderIterator beginPreOrder() const noexcept
   {
      assert(m_nodes.size() > 0);
      const auto iterator = Tree<DataType>::PreOrderIterator{ *GetHead() };
      return iterator;
   }

   /**
   * @returns A pre-order iterator pointing "past" the end of the tree.
   */
   typename Tree::PreOrderIterator endPreOrder() const noexcept
   {
      const auto iterator = Tree<DataType>::PreOrderIterator{ };
      return iterator;
   }

   /**
   * @returns An iterator that will iterator over all leaf nodes in the Tree, starting with the
   * left-most leaf in the Tree.
   */
   typename Tree::LeafIterator beginLeaf() const noexcept
   {
      assert(m_nodes.size() > 0);
      const auto iterator = Tree<DataType>::LeafIterator{ *GetHead() };
      return iterator;
   }

   /**
   * @return A LeafIterator that points past the last leaf Node in the Tree.
   */
   typename Tree::LeafIterator endLeaf() const noexcept
   {
      const auto iterator = Tree<DataType>::LeafIterator{  };
      return iterator;
   }

   /**
   *
   */
   typename Tree::SiblingIterator beginSibling(const Node& node) const noexcept
   {
      const auto iterator = Tree<DataType>::SiblingIterator{ node };
      return iterator;
   }

   /**
   * @todo There is probably no test coverage for this function...
   */
   typename Tree::SiblingIterator endSibling() const noexcept
   {
      const auto iterator = Tree<DataType>::SiblingIterator{ };
      return iterator;
   }

   /**
   *
   */
   auto& GetNodesAsVector() const noexcept
   {
      return m_nodes;
   }

   /**
   * @brief SortChildren performs a merge sort of the direct descendants nodes.
   *
   * @param[in] node                The node whose children are to be sorted.
   * @param[in] comparator          A callable type to be used as the basis for the sorting
   *                                comparison. This type should be equivalent to:
   *                                   bool comparator(
   *                                      const Tree<DataType>::Node& lhs,
   *                                      const Tree<DataType>::Node& rhs);
   */
   template<typename ComparatorType>
   void SortChildren(
      Node& node,
      const ComparatorType& comparator) noexcept(noexcept(comparator))
   {
      if (!node.m_firstChild)
      {
         return;
      }

      MergeSort(node.m_firstChild, comparator);
   }

private:

   /**
   * @brief MergeSort is the main entry point into the merge sort implementation.
   *
   * @param[in] list                The first Node in the list of siblings to be sorted.
   * @param[in] comparator          The comparator function to be called to figure out which node
   *                                is the lesser of the two.
   */
   template<typename ComparatorType>
   void MergeSort(
      std::shared_ptr<Node>& list,
      const ComparatorType& comparator) noexcept(noexcept(comparator))
   {
      if (!list || !list->m_nextSibling)
      {
         return;
      }

      std::shared_ptr<Node> head = list;
      std::shared_ptr<Node> lhs = nullptr;
      std::shared_ptr<Node> rhs = nullptr;

      DivideList(head, lhs, rhs);

      assert(lhs);
      assert(rhs);

      MergeSort(lhs, comparator);
      MergeSort(rhs, comparator);

      list = MergeSortedHalves(lhs, rhs, comparator);
   }

   /**
   * @brief DivideList is a helper function that will divide the specified Node list in two.
   *
   * @param[in] head                The head of the Node list to be divided in two.
   * @param[out] lhs                The first Node of the left hand side list.
   * @param[out] rhs                The first Node of the right hand side list.
   */
   void DivideList(
      std::shared_ptr<Node> head,
      std::shared_ptr<Node>& lhs,
      std::shared_ptr<Node>& rhs) noexcept
   {
      if (!head || !head->m_nextSibling)
      {
         return;
      }

      std::shared_ptr<Node> tortoise = head;
      std::shared_ptr<Node> hare = head->m_nextSibling;

      while (hare)
      {
         hare = hare->m_nextSibling;
         if (hare)
         {
            tortoise = tortoise->m_nextSibling;
            hare = hare->m_nextSibling;
         }
      }

      lhs = head;
      rhs = tortoise->m_nextSibling;

      tortoise->m_nextSibling = nullptr;
   }

   /**
   * @brief MergeSortedHalves is a helper function that will merge the sorted halves.
   *
   * @param[in] lhs                 The first node of the sorted left half.
   * @param[in] rhs                 The first node of the sorted right half.
   *
   * @returns The first node of the merged Node list.
   */
   template<typename ComparatorType>
   std::shared_ptr<Node> MergeSortedHalves(
      std::shared_ptr<Node>& lhs,
      std::shared_ptr<Node>& rhs,
      const ComparatorType& comparator) noexcept(noexcept(comparator))
   {
      std::shared_ptr<Node> result = nullptr;
      if (comparator(*lhs, *rhs))
      {
         result = lhs;
         lhs = lhs->m_nextSibling;
      }
      else
      {
         result = rhs;
         rhs = rhs->m_nextSibling;
      }

      result->m_previousSibling = nullptr;

      std::shared_ptr<Node> tail = result;

      while (lhs && rhs)
      {
         if (comparator(*lhs, *rhs))
         {
            tail->m_nextSibling = lhs;
            tail = tail->m_nextSibling;

            lhs = lhs->m_nextSibling;

            if (lhs)
            {
               lhs->m_previousSibling = nullptr;
            }
         }
         else
         {
            tail->m_nextSibling = rhs;
            tail = tail->m_nextSibling;

            rhs = rhs->m_nextSibling;

            if (rhs)
            {
               rhs->m_previousSibling = nullptr;
            }
         }
      }

      while (lhs)
      {
         tail->m_nextSibling = lhs;
         tail = tail->m_nextSibling;

         lhs = lhs->m_nextSibling;

         if (lhs)
         {
            lhs->m_previousSibling = nullptr;
         }
      }

      while (rhs)
      {
         tail->m_nextSibling = rhs;
         tail = tail->m_nextSibling;

         rhs = rhs->m_nextSibling;

         if (rhs)
         {
            rhs->m_previousSibling = nullptr;
         }
      }

      return result;
   }

   std::vector<std::shared_ptr<Node>> m_nodes;
};

template<typename DataType>
class Tree<DataType>::Node
{
   friend class Tree<DataType>;

public:
   using value_type = DataType;
   using reference = DataType&;
   using const_reference = const DataType&;

   /**
   * @brief Node default constructs a new Node. All outgoing links from this new node will
   * initialized to a nullptr.
   */
   constexpr Node() noexcept = default;

   /**
   * @brief Node constructs a new Node encapsulating the specified data. All outgoing links
   * from the node will be initialized to nullptr.
   */
   Node(DataType data) noexcept(std::is_nothrow_move_constructible_v<DataType>) :
      m_data{ std::move(data) }
   {
   }

   /**
   * @brief Copy-constructor
   *
   * @todo Worry about this later.
   */
   Node(const Node& other) = delete;

   /**
   * @brief Assignment operator.
   *
   * @todo Worry about this later.
   */
   Node& operator=(Node other) = delete;

   /**
   * @brief Destroys the Node and all Nodes under it.
   */
   ~Node()
   {
      if (m_childCount == 0)
      {
         // @todo Even if a node does not have children, it may still have siblings. The links to
         // these siblings needs to be broken in order to ensure the proper freeing of these nodes.

         return;
      }

      // Find the first node to delete:

      auto* victim = this;
      while (victim->GetFirstChild())
      {
         victim = victim->GetFirstChild().get();
      }

      assert(victim);

      // Find the node immediately following the last node to delete:

      decltype(victim) lastVictim = nullptr;
      if (victim->GetNextSibling())
      {
         auto* lastVictim = victim->GetNextSibling().get();
         while (lastVictim->HasChildren())
         {
            lastVictim = lastVictim->GetFirstChild().get();
         }
      }
      else
      {
         lastVictim = victim->GetParent();
      }

      // Perform deletions:

      bool traversingUpTheTree = false;

      const auto AdvanceToNext = [&](auto* node)
      {
         if (node->HasChildren() && !traversingUpTheTree)
         {
            while (node->GetFirstChild())
            {
               node = node->GetFirstChild().get();
            }
         }
         else if (node->GetNextSibling())
         {
            traversingUpTheTree = false;

            node = node->GetNextSibling().get();
            while (node->HasChildren())
            {
               node = node->GetFirstChild().get();
            }
         }
         else
         {
            traversingUpTheTree = true;

            node = node->GetParent();
         }

         return node;
      };

      auto* nextVictim = AdvanceToNext(victim);
      victim->DetachFromTree();

      while (nextVictim != lastVictim)
      {
         victim = nextVictim;
         nextVictim = AdvanceToNext(victim);
         victim->DetachFromTree();
      }
   }

   /**
   * @brief Swaps all member variables of the left-hand side with that of the right-hand side.
   */
   friend void swap(Node& lhs, Node& rhs)
      noexcept(noexcept(swap(lhs.m_data, rhs.m_data)))
   {
      // Enable Argument Dependent Lookup (ADL):
      using std::swap;

      swap(lhs.m_parent, rhs.m_parent);
      swap(lhs.m_firstChild, rhs.m_firstChild);
      swap(lhs.m_lastChild, rhs.m_lastChild);
      swap(lhs.m_previousSibling, rhs.m_previousSibling);
      swap(lhs.m_nextSibling, rhs.m_nextSibling);
      swap(lhs.m_data, rhs.m_data);
      swap(lhs.m_childCount, rhs.m_childCount);
      swap(lhs.m_visited, rhs.m_visited);
   }

   /**
   * @brief Detaches and then deletes the Node from the Tree it's part of.
   */
   inline void DeleteFromTree() noexcept
   {
      delete this;
   }

   /**
   * @returns The encapsulated data.
   */
   inline DataType* operator->() noexcept
   {
      return &m_data;
   }

   /**
   * @overload
   */
   inline const DataType* operator->() const noexcept
   {
      return &m_data;
   }

   /**
   * @brief MarkVisited sets node visitation status.
   *
   * @param[in] visited             Whether the node should be marked as having been visited.
   */
   inline void MarkVisited(const bool visited = true) noexcept
   {
      m_visited = visited;
   }

   /**
   * @returns True if the node has been marked as visited.
   */
   inline constexpr bool HasBeenVisited() const noexcept
   {
      return m_visited;
   }

   /**
   *
   */
   auto HasChildren() const noexcept
   {
      return m_childCount > 0;
   }

   /**
   *
   */
   auto GetChildCount() const noexcept
   {
      return m_childCount;
   }

   /**
   *
   */
   auto GetFirstChild() const noexcept
   {
      return m_firstChild;
   }

   /**
   *
   */
   auto GetLastChild() const noexcept
   {
      return m_lastChild;
   }

   /**
   *
   */
   auto GetParent() const noexcept
   {
      return m_parent;
   }

   /**
   *
   */
   auto GetNextSibling() const noexcept
   {
      return m_nextSibling;
   }

   /**
   *
   */
   auto GetPreviousSibling() const noexcept
   {
      return m_previousSibling;
   }

   /**
   *
   */
   auto& GetData() noexcept
   {
      return m_data;
   }

   /**
   *
   */
   const auto& GetData() const noexcept
   {
      return m_data;
   }
   
   /**
   * @returns True if the data encapsulated in the left-hand side Node is less than
   * the data encapsulated in the right-hand side Node.
   */
   friend bool operator<(const Node& lhs, const Node& rhs)
   {
      return lhs.GetData() < rhs.GetData();
   }

   /**
   * @returns True if the data encapsulated in the left-hand side Node is less than
   * or equal to the data encapsulated in the right-hand side Node.
   */
   friend bool operator<=(const Node& lhs, const Node& rhs)
   {
      return !(lhs.GetData() > rhs.GetData());
   }

   /**
   * @returns True if the data encapsulated in the left-hand side Node is greater than
   * the data encapsulated in the right-hand side Node.
   */
   friend bool operator>(const Node& lhs, const Node& rhs)
   {
      return rhs.GetData() < lhs.GetData();
   }

   /**
   * @returns True if the data encapsulated in the left-hand side Node is greater than
   * or equal to the data encapsulated in the right-hand side Node.
   */
   friend bool operator>=(const Node& lhs, const Node& rhs)
   {
      return !(lhs.GetData() < rhs.GetData());
   }

   /**
   * @returns True if the data encapsulated in the left-hand side Node is equal to
   * the data encapsulated in the right-hand side Node.
   */
   friend bool operator==(const Node& lhs, const Node& rhs)
   {
      return lhs.GetData() == rhs.GetData();
   }

   /**
   * @returns True if the data encapsulated in the left-hand side Node is not equal
   * to the data encapsulated in the right-hand side Node.
   */
   friend bool operator!=(const Node& lhs, const Node& rhs)
   {
      return !(lhs.GetData() == rhs.GetData());
   }

private:

   /**
   * @brief Removes the Node from the tree structure, updating all surrounding links
   * as appropriate.
   *
   * @note This function does not actually delete the node.
   */
   void DetachFromTree() noexcept
   {
      if (m_previousSibling && m_nextSibling)
      {
         m_previousSibling->m_nextSibling = m_nextSibling;
         m_nextSibling->m_previousSibling = m_previousSibling;
      }
      else if (m_previousSibling)
      {
         m_previousSibling->m_nextSibling = nullptr;
      }
      else if (m_nextSibling)
      {
         m_nextSibling->m_previousSibling = nullptr;
      }

      if (!m_parent)
      {
         m_previousSibling = nullptr;
         m_nextSibling = nullptr;
         return;
      }

      if (m_parent->m_firstChild == m_parent->m_lastChild)
      {
         m_parent->m_firstChild = nullptr;
         m_parent->m_lastChild = nullptr;
      }
      else if (m_parent->m_firstChild.get() == this)
      {
         assert(m_parent->m_firstChild->m_nextSibling);
         m_parent->m_firstChild = m_parent->m_firstChild->m_nextSibling;
      }
      else if (m_parent->m_lastChild.get() == this)
      {
         assert(m_parent->m_lastChild->m_previousSibling);
         m_parent->m_lastChild = m_parent->m_lastChild->m_previousSibling;
      }

      m_previousSibling = nullptr;
      m_nextSibling = nullptr;

      m_parent->m_childCount--;

      return;
   }

   // @note Parents should not be a std::shared_ptr<Node>, because we don't want to create cycles.
   Node* m_parent{ nullptr };

   std::shared_ptr<Node> m_firstChild{ nullptr };
   std::shared_ptr<Node> m_lastChild{ nullptr };

   // @todo Having adjacent siblings pointing at one another creates an obvious cycle. While this
   // cycle is properly broken when nodes are detached from the tree, this is likely not to be the
   // case if the underlying vector of nodes goes out of scope when an exception is throw. This
   // should ideally not be cyclic, or ~Node() should handle the breaking of this cycle when it is
   // invoked.
   std::shared_ptr<Node> m_previousSibling{ nullptr };
   std::shared_ptr<Node> m_nextSibling{ nullptr };

   DataType m_data;

   std::size_t m_childCount{ 0 };

   // @todo Add supporting infrastructure for this variable:
   bool m_visited{ false };
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
   * @returns True if the Tree::Iterator points to a valid Node; false otherwise.
   */
   explicit operator bool() const noexcept
   {
      const auto isValid = (m_currentNode != nullptr);
      return isValid;
   }

   /**
   * @returns The Node pointed to by the Tree::Iterator.
   */
   inline Node& operator*() noexcept
   {
      return *m_currentNode;
   }

   /**
   * @overload
   */
   inline const Node& operator*() const noexcept
   {
      return *m_currentNode;
   }

   /**
   * @returns A pointer to the Node.
   */
   inline Node* const operator&() noexcept
   {
      return m_currentNode;
   }

   /**
   * @overload
   */
   inline const Node* const operator&() const noexcept
   {
      return m_currentNode;
   }

   /**
   * @returns A pointer to the Node pointed to by the Tree:Iterator.
   */
   inline Node* const operator->() noexcept
   {
      return m_currentNode;
   }

   /**
   * @overload
   */
   inline const Node* const operator->() const noexcept
   {
      return m_currentNode;
   }

   /**
   * @returns True if the Iterator points to the same node as the other Iterator,
   * and false otherwise.
   */
   inline bool operator==(const Iterator& other) const
   {
      return m_currentNode == other.m_currentNode;
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
   explicit Iterator(const Node& node) noexcept :
      m_currentNode{ const_cast<Node*>(&node) },
      m_startingNode{ const_cast<Node*>(&node) }
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
   explicit PreOrderIterator(const Node& node) noexcept :
      Iterator{ node }
   {
      if (node.GetNextSibling())
      {
         m_endingNode = node.GetNextSibling().get();
      }
      else
      {
         m_endingNode = &node;
         while (m_endingNode->GetParent() && !m_endingNode->GetParent()->GetNextSibling().get())
         {
            m_endingNode = m_endingNode->GetParent();
         }

         if (m_endingNode->GetParent())
         {
            m_endingNode = m_endingNode->GetParent()->GetNextSibling().get();
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
         traversingNode = traversingNode->GetFirstChild().get();
      }
      else if (traversingNode->GetNextSibling())
      {
         traversingNode = traversingNode->GetNextSibling().get();
      }
      else
      {
         while (traversingNode->GetParent() && !traversingNode->GetParent()->GetNextSibling().get())
         {
            traversingNode = traversingNode->GetParent();
         }

         if (traversingNode->GetParent())
         {
            traversingNode = traversingNode->GetParent()->GetNextSibling().get();
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
   explicit PostOrderIterator(const Node& node) noexcept :
      Iterator{ node }
   {
      // Compute and set the starting node:

      auto* traversingNode = &node;
      while (traversingNode->GetFirstChild())
      {
         traversingNode = traversingNode->GetFirstChild().get();
      }

      assert(traversingNode);
      m_currentNode = const_cast<Node*>(traversingNode);

      // Commpute and set the ending node:

      if (node.GetNextSibling())
      {
         auto* traversingNode = node.GetNextSibling().get();
         while (traversingNode->HasChildren())
         {
            traversingNode = traversingNode->GetFirstChild().get();
         }

         m_endingNode = traversingNode;
      }
      else
      {
         m_endingNode = node.GetParent();
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
            traversingNode = traversingNode->GetFirstChild().get();
         }
      }
      else if (traversingNode->GetNextSibling())
      {
         m_traversingUpTheTree = false;

         traversingNode = traversingNode->GetNextSibling().get();
         while (traversingNode->HasChildren())
         {
            traversingNode = traversingNode->GetFirstChild().get();
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
   explicit LeafIterator(const Node& node) noexcept :
      Iterator{ node }
   {
      // Compute and set the starting node:

      if (node.HasChildren())
      {
         auto* firstNode = &node;
         while (firstNode->GetFirstChild().get())
         {
            firstNode = firstNode->GetFirstChild().get();
         }

         m_currentNode = const_cast<Node*>(firstNode);
      }

      // Compute and set the ending node:

      if (node.GetNextSibling().get())
      {
         auto* lastNode = node.GetNextSibling().get();
         while (lastNode->HasChildren())
         {
            lastNode = lastNode->GetFirstChild().get();
         }

         m_endingNode = lastNode;
      }
      else
      {
         m_endingNode = &node;
         while (m_endingNode->GetParent() && !m_endingNode->GetParent()->GetNextSibling().get())
         {
            m_endingNode = m_endingNode->GetParent();
         }

         if (m_endingNode->GetParent())
         {
            m_endingNode = m_endingNode->GetParent()->GetNextSibling().get();
            while (m_endingNode->HasChildren())
            {
               m_endingNode = m_endingNode->GetFirstChild().get();
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
         while (traversingNode->GetFirstChild().get())
         {
            traversingNode = traversingNode->GetFirstChild().get();
         }
      }
      else if (traversingNode->GetNextSibling().get())
      {
         traversingNode = traversingNode->GetNextSibling().get();

         while (traversingNode->GetFirstChild().get())
         {
            traversingNode = traversingNode->GetFirstChild().get();
         }
      }
      else if (traversingNode->GetParent())
      {
         while (traversingNode->GetParent() && !traversingNode->GetParent()->GetNextSibling().get())
         {
            traversingNode = traversingNode->GetParent();
         }

         if (traversingNode->GetParent())
         {
            traversingNode = traversingNode->GetParent()->GetNextSibling().get();

            while (traversingNode && traversingNode->HasChildren())
            {
               traversingNode = traversingNode->GetFirstChild().get();
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
   explicit SiblingIterator(const Node& node) noexcept :
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
         m_currentNode = m_currentNode->GetNextSibling().get();
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
