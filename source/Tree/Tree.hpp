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
private:

   struct StateAndIndex
   {
      bool state;
      std::size_t index;
   };

   struct Metadata
   {
      std::size_t index;
      std::size_t childCount;

      StateAndIndex parent;
      StateAndIndex nextSibling;
      StateAndIndex previousSibling;
      StateAndIndex firstChild;
      StateAndIndex lastChild;
   };

   constexpr static StateAndIndex UNINITIALIZED{ false, 0 };

   static_assert(std::is_trivial_v<Metadata>,"Metadata is not trivial copyable and trivially defaulted!");
   static_assert(std::is_trivially_destructible_v<Metadata>, "Metadata is not trivial destructible!");

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

      m_metadata.reserve(128);
      m_metadata.emplace_back(Metadata{ });
   }

   /**
   * @brief Tree constructs a new Tree with the provided data encapsulated in a new
   * Node.
   */
   Tree(DataType data)
   {
      m_data.reserve(128);
      m_data.emplace_back(std::move(data));

      m_metadata.reserve(128);
      m_metadata.emplace_back(Metadata{ });
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
   Node GetHead() noexcept
   {
      assert(m_data.size() > 0 && m_metadata.size() > 0);

      return { this, m_data[0], m_metadata[0] };
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
   auto Size(Node* node = nullptr) const noexcept
   {
      assert(m_data.size() == m_metadata.size());

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
   void Detach(Node& node)
   {
      node->DetachFromTree();
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
   const auto& GetDataAsVector() const noexcept
   {
      return m_data;
   }

#if 0
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

#endif

private:

   std::vector<DataType> m_data;
   std::vector<Metadata> m_metadata;
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
   Node(Tree* tree, DataType& data, Metadata& metadata) noexcept :
      m_tree{ tree },
      m_data{ data },
      m_metadata{ metadata }
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
   * @brief AppendChild will construct and append a new TreeNode as the last child of the TreeNode.
   *
   * @param[in] data                The underlying data to be stored in the new TreeNode.
   *
   * @returns The newly appended TreeNode.
   */
   Node AppendChild(const DataType& data)
   {
      assert(m_tree);
      assert(m_tree->m_data.size() == m_tree->m_metadata.size());

      // It's important that this be done before inserting (or you'll be off by one):
      const auto indexOfNewNode = m_tree->m_data.size();

      const Metadata childNode
      {
         /* index =            */ indexOfNewNode,
         /* childCount =       */ 0,
         /* parent =           */ StateAndIndex{ true, m_metadata.index },
         /* nextSibling =      */ UNINITIALIZED,
         /* previousSibling =  */ m_metadata.lastChild,
         /* firstChild =       */ UNINITIALIZED,
         /* lastChild =        */ UNINITIALIZED
      };

      m_tree->m_metadata[m_metadata.lastChild.index].nextSibling = StateAndIndex{ true, indexOfNewNode };

      m_metadata.lastChild = StateAndIndex{ true, indexOfNewNode };
      ++(m_metadata.childCount);

      m_tree->m_data.emplace_back(data);
      m_tree->m_metadata.emplace_back(childNode);

      return { m_tree, m_tree->m_data.back(), m_tree->m_metadata.back() };
   }

   /**
   * @overload
   */
   //template<typename Type>
   //Node& AppendChild(Type&& data)
   //{
   //   const auto& newNode = Node{ std::forward<Type>(data) };
   //   return AppendChild(newNode);
   //}

   /**
   * @brief AppendChild will append the specified TreeNode as a child of the TreeNode.
   *
   * @param[in] child               The new TreeNode to set as the last child of the TreeNode.
   *
   * @returns A pointer to the newly appended child.
   */
   //Node& AppendChild(Node<DataType>& child) noexcept
   //{
   //   child.m_parent = this;

   //   if (!m_lastChild)
   //   {
   //      return AddFirstChild(child);
   //   }

   //   assert(m_lastChild);

   //   m_lastChild->m_nextSibling = &child;
   //   m_lastChild->m_nextSibling->m_previousSibling = m_lastChild;
   //   m_lastChild = m_lastChild->m_nextSibling;

   //   m_childCount++;

   //   return m_lastChild;
   //}



   /**
   * @brief Detaches and then deletes the Node from the Tree it's part of.
   */
   void DeleteFromTree() noexcept
   {
   }

   /**
   *
   */
   auto HasChildren() const noexcept
   {
      return m_metadata.childCount > 0;
   }

   /**
   *
   */
   auto GetChildCount() const noexcept
   {
      return m_metadata.childCount;
   }

   /**
   *
   */
   auto GetFirstChild() const noexcept
   {
      return m_metadata.firstChild;
   }

   /**
   *
   */
   auto GetLastChild() const noexcept
   {
      return m_metadata.lastChild;
   }

   /**
   *
   */
   auto GetParent() const noexcept
   {
      return m_metadata.parent;
   }

   /**
   *
   */
   auto GetNextSibling() const noexcept
   {
      return m_metadata.nextSibling;
   }

   /**
   *
   */
   auto GetPreviousSibling() const noexcept
   {
      return m_metadata.previousSibling;
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
   friend auto operator<(const Node& lhs, const Node& rhs)
   {
      return lhs.GetData() < rhs.GetData();
   }

   /**
   * @returns True if the data encapsulated in the left-hand side Node is less than
   * or equal to the data encapsulated in the right-hand side Node.
   */
   friend auto operator<=(const Node& lhs, const Node& rhs)
   {
      return !(lhs.GetData() > rhs.GetData());
   }

   /**
   * @returns True if the data encapsulated in the left-hand side Node is greater than
   * the data encapsulated in the right-hand side Node.
   */
   friend auto operator>(const Node& lhs, const Node& rhs)
   {
      return rhs.GetData() < lhs.GetData();
   }

   /**
   * @returns True if the data encapsulated in the left-hand side Node is greater than
   * or equal to the data encapsulated in the right-hand side Node.
   */
   friend auto operator>=(const Node& lhs, const Node& rhs)
   {
      return !(lhs.GetData() < rhs.GetData());
   }

   /**
   * @returns True if the data encapsulated in the left-hand side Node is equal to
   * the data encapsulated in the right-hand side Node.
   */
   friend auto operator==(const Node& lhs, const Node& rhs)
   {
      return lhs.GetData() == rhs.GetData();
   }

   /**
   * @returns True if the data encapsulated in the left-hand side Node is not equal
   * to the data encapsulated in the right-hand side Node.
   */
   friend auto operator!=(const Node& lhs, const Node& rhs)
   {
      return !(lhs.GetData() == rhs.GetData());
   }

private:

   Tree* m_tree;
   DataType& m_data;
   Metadata& m_metadata;
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
