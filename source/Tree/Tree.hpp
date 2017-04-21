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

#define DECLARE_FALSEY_METADATA                       \
static Metadata falseyMetadata                        \
{                                                     \
   /* self =             */ UNINITIALIZED,            \
   /* parent =           */ UNINITIALIZED,            \
   /* nextSibling =      */ UNINITIALIZED,            \
   /* previousSibling =  */ UNINITIALIZED,            \
   /* firstChild =       */ UNINITIALIZED,            \
   /* lastChild =        */ UNINITIALIZED,            \
   /* childCount =       */ 0                         \
}

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
      StateAndIndex self;

      StateAndIndex parent;
      StateAndIndex nextSibling;
      StateAndIndex previousSibling;
      StateAndIndex firstChild;
      StateAndIndex lastChild;

      std::size_t childCount;
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

      // The root now should consider itself valid, but everything else should be uninitialized:
      const Metadata metadata
      {
         /* self =             */ StateAndIndex{ true, 0 },
         /* parent =           */ UNINITIALIZED,
         /* nextSibling =      */ UNINITIALIZED,
         /* previousSibling =  */ UNINITIALIZED,
         /* firstChild =       */ UNINITIALIZED,
         /* lastChild =        */ UNINITIALIZED,
         /* childCount =       */ 0
      };

      m_metadata.reserve(128);
      m_metadata.emplace_back(std::move(metadata));
   }

   /**
   * @brief Tree constructs a new Tree with the provided data encapsulated in a new
   * Node.
   */
   Tree(DataType data)
   {
      m_data.reserve(128);
      m_data.emplace_back(std::move(data));

      // The root now should consider itself valid, but everything else should be uninitialized:
      const Metadata metadata
      {
         /* self =             */ StateAndIndex{ true, 0 },
         /* parent =           */ UNINITIALIZED,
         /* nextSibling =      */ UNINITIALIZED,
         /* previousSibling =  */ UNINITIALIZED,
         /* firstChild =       */ UNINITIALIZED,
         /* lastChild =        */ UNINITIALIZED,
         /* childCount =       */ 0
      };

      m_metadata.reserve(128);
      m_metadata.emplace_back(std::move(metadata));
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
   Node GetRoot() noexcept
   {
      assert(m_data.size() > 0 && m_metadata.size() > 0);

      return { *this, m_data[0], m_metadata[0] };
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
      assert(m_data.size() == m_metadata.size());

      return m_data.size();
   }

   /**
   * @returns A post-order iterator that will iterator over all nodes in the tree, starting
   * with the head of the Tree.
   */
   typename Tree::PostOrderIterator begin() const noexcept
   {
      assert(m_nodes.size() > 0);
      const auto iterator = Tree<DataType>::PostOrderIterator{ GetRoot() };
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
   * @returns A pre-order iterator that will iterate over all Nodes in the tree.
   */
   typename Tree::PreOrderIterator beginPreOrder() const noexcept
   {
      assert(m_nodes.size() > 0);
      const auto iterator = Tree<DataType>::PreOrderIterator{ GetRoot() };
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
      const auto iterator = Tree<DataType>::LeafIterator{ GetRoot() };
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
   * @returns The underlying data container.
   */
   const auto& GetDataAsVector() const noexcept
   {
      return m_data;
   }

private:

   std::vector<DataType> m_data;
   std::vector<Metadata> m_metadata;
};

template<typename DataType>
class Tree<DataType>::Node
{
public:
   using value_type = DataType;
   using reference = DataType&;
   using const_reference = const DataType&;

   /**
   * @brief Node default constructs a new Node. All outgoing links from this new node will
   * initialized to a nullptr.
   */
   Node(Tree& tree, DataType& data, Metadata& metadata) noexcept :
      m_tree{ tree },
      m_nodeData{ data },
      m_nodeMetadata{ metadata }
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
   * @brief Constructs and appends a new Node as the last child of the Node.
   *
   * @param[in] data                The underlying data to be stored in the new Node.
   *
   * @returns The newly appended Node.
   */
   template<typename DatumType>
   Node AppendChild(DatumType&& datum)
   {
      assert(m_tree.m_data.size() == m_tree.m_metadata.size());

      if (!m_nodeMetadata.self.state)
      {
         assert(!"Attempting to append to a node that's in an invalid state!");

         // @todo Or should this throw instead?
         return
         {
            m_tree,
            m_tree.m_data[m_nodeMetadata.self.index],
            m_tree.m_metadata[m_nodeMetadata.self.index]
         };
      }

      // It's important that this be done before inserting (or you'll be off by one):
      const auto indexOfNewNode = m_tree.m_data.size();
      const auto newNode = StateAndIndex{ true, indexOfNewNode };

      const Metadata childNode
      {
         /* self =             */ newNode,
         /* parent =           */ m_nodeMetadata.self,
         /* nextSibling =      */ UNINITIALIZED,
         /* previousSibling =  */ m_nodeMetadata.lastChild,
         /* firstChild =       */ UNINITIALIZED,
         /* lastChild =        */ UNINITIALIZED,
         /* childCount =       */ 0
      };

      m_tree.m_metadata[m_nodeMetadata.lastChild.index].nextSibling = newNode;

      if (m_nodeMetadata.firstChild.state == false)
      {
         m_nodeMetadata.firstChild = newNode;
      }

      m_nodeMetadata.lastChild = newNode;
      ++(m_nodeMetadata.childCount);

      m_tree.m_data.emplace_back(std::forward<DatumType>(datum));
      m_tree.m_metadata.emplace_back(std::move(childNode));

      return { m_tree, m_tree.m_data.back(), m_tree.m_metadata.back() };
   }

   /**
   * @brief Constructs and prepends a new Node as the first child of the Node.
   *
   * @param[in] data                The underlying data to be stored in the new Node.
   *
   * @returns The newly prepended Node.
   */
   template<typename DatumType>
   Node PrependChild(DatumType&& datum)
   {
      assert(m_tree.m_data.size() == m_tree.m_metadata.size());

      if (!m_nodeMetadata.self.state)
      {
         assert(!"Attempting to prepend to a node that's in an invalid state!");

         // @todo Or should this throw instead?
         return
         {
            m_tree,
            m_tree.m_data[m_nodeMetadata.self.index],
            m_tree.m_metadata[m_nodeMetadata.self.index]
         };
      }

      // It's important that this be done before inserting (or you'll be off by one):
      const auto indexOfNewNode = m_tree.m_data.size();
      const auto newNode = StateAndIndex{ true, indexOfNewNode };

      const Metadata childNode
      {
         /* self =             */ newNode,
         /* parent =           */ m_nodeMetadata.self,
         /* nextSibling =      */ m_nodeMetadata.firstChild,
         /* previousSibling =  */ UNINITIALIZED,
         /* firstChild =       */ UNINITIALIZED,
         /* lastChild =        */ UNINITIALIZED,
         /* childCount =       */ 0
      };

      m_tree.m_metadata[m_nodeMetadata.firstChild.index].previousSibling = newNode;

      if (m_nodeMetadata.lastChild.state == false)
      {
         m_nodeMetadata.lastChild = newNode;
      }

      m_nodeMetadata.firstChild = newNode;
      ++(m_nodeMetadata.childCount);

      m_tree.m_data.emplace_back(std::forward<DatumType>(datum));
      m_tree.m_metadata.emplace_back(std::move(childNode));

      return { m_tree, m_tree.m_data.back(), m_tree.m_metadata.back() };
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
      return m_nodeMetadata.childCount > 0;
   }

   /**
   * @returns The number of children that this Node has.
   */
   auto GetChildCount() const noexcept
   {
      return m_nodeMetadata.childCount;
   }

   /**
   * @returns The first child Node of this Node.
   */
   Node GetFirstChild() const noexcept
   {
      if (m_nodeMetadata.firstChild.state)
      {
         return
         {
            m_tree,
            m_tree.m_data[m_nodeMetadata.firstChild.index],
            m_tree.m_metadata[m_nodeMetadata.firstChild.index]
         };
      }

      DECLARE_FALSEY_METADATA;

      return{ m_tree, m_tree.m_data[0], falseyMetadata };
   }

   /**
   * @returns The last child Node of this Node.
   */
   Node GetLastChild() const noexcept
   {
      if (m_nodeMetadata.lastChild.state)
      {
         return
         {
            m_tree,
            m_tree.m_data[m_nodeMetadata.lastChild.index],
            m_tree.m_metadata[m_nodeMetadata.lastChild.index]
         };
      }

      DECLARE_FALSEY_METADATA;

      return { m_tree, m_tree.m_data[0], falseyMetadata };
   }

   /**
   * @returns The parent Node of this Node.
   */
   Node GetParent() noexcept
   {
      if (m_nodeMetadata.parent.state)
      {
         return
         {
            m_tree,
            m_tree.m_data[m_nodeMetadata.parent.index],
            m_tree.m_metadata[m_nodeMetadata.parent.index]
         };
      }

      DECLARE_FALSEY_METADATA;

      return { m_tree, m_tree.m_data[0], falseyMetadata };
   }

   /**
   * @returns The Node to the right of this Node.
   */
   Node GetNextSibling() noexcept
   {
      if (m_nodeMetadata.nextSibling.state)
      {
         return
         {
            m_tree,
            m_tree.m_data[m_nodeMetadata.nextSibling.index],
            m_tree.m_metadata[m_nodeMetadata.nextSibling.index]
         };
      }

      DECLARE_FALSEY_METADATA;

      return { m_tree, m_tree.m_data[0], falseyMetadata };
   }

   /**
   * @returns The Node to the left of this Node.
   */
   Node GetPreviousSibling() const noexcept
   {
      if (m_nodeMetadata.previousSibling.state)
      {
         return
         {
            m_tree,
            m_tree.m_data[m_nodeMetadata.previousSibling.index],
            m_tree.m_metadata[m_nodeMetadata.previousSibling.index]
         };
      }

      DECLARE_FALSEY_METADATA;

      return { m_tree, m_tree.m_data[0], falseyMetadata };
   }

   /**
   * @returns The encapsulated data.
   */
   auto& GetData() noexcept
   {
      return m_nodeData;
   }

   /**
   * @returns The encapsulated data.
   */
   const auto& GetData() const noexcept
   {
      return m_nodeData;
   }

   /**
   * @returns True is the Node is initialized, or valid.
   */
   explicit operator bool() const noexcept
   {
      return m_nodeMetadata.self.state;
   }

   /**
   * @returns True if the data encapsulated in the left-hand side Node is less than
   * the data encapsulated in the right-hand side Node.
   */
   friend auto operator<(const Node& lhs, const Node& rhs) noexcept
   {
      return lhs.GetData() < rhs.GetData();
   }

   /**
   * @returns True if the data encapsulated in the left-hand side Node is less than
   * or equal to the data encapsulated in the right-hand side Node.
   */
   friend auto operator<=(const Node& lhs, const Node& rhs) noexcept
   {
      return !(lhs.GetData() > rhs.GetData());
   }

   /**
   * @returns True if the data encapsulated in the left-hand side Node is greater than
   * the data encapsulated in the right-hand side Node.
   */
   friend auto operator>(const Node& lhs, const Node& rhs) noexcept
   {
      return rhs.GetData() < lhs.GetData();
   }

   /**
   * @returns True if the data encapsulated in the left-hand side Node is greater than
   * or equal to the data encapsulated in the right-hand side Node.
   */
   friend auto operator>=(const Node& lhs, const Node& rhs) noexcept
   {
      return !(lhs.GetData() < rhs.GetData());
   }

   /**
   * @returns True if the data encapsulated in the left-hand side Node is equal to
   * the data encapsulated in the right-hand side Node.
   */
   friend auto operator==(const Node& lhs, const Node& rhs) noexcept
   {
      return lhs.GetData() == rhs.GetData();
   }

   /**
   * @returns True if the data encapsulated in the left-hand side Node is not equal
   * to the data encapsulated in the right-hand side Node.
   */
   friend auto operator!=(const Node& lhs, const Node& rhs) noexcept
   {
      return !(lhs.GetData() == rhs.GetData());
   }

private:

   Tree& m_tree;
   DataType& m_nodeData;
   Metadata& m_nodeMetadata;
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
