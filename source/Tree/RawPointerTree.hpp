#pragma once
#pragma once

#include <cassert>
#include <functional>
#include <iostream>
#include <iterator>

template<typename DataType> class Tree;
template<typename DataType> class TreeNode;

/**
* @returns True if the data encapsulated in the left-hand side TreeNode is less than
* the data encapsulated in the right-hand side TreeNode.
*/
template<typename DataType>
inline bool operator<(const TreeNode<DataType>& lhs, const TreeNode<DataType>& rhs)
{
   return lhs.GetData() < rhs.GetData();
}

/**
* @returns True if the data encapsulated in the left-hand side TreeNode is less than or equal to
* the data encapsulated in the right-hand side TreeNode.
*/
template<typename DataType>
inline bool operator<=(const TreeNode<DataType>& lhs, const TreeNode<DataType>& rhs)
{
   return !(lhs.GetData() > rhs.GetData());
}

/**
* @returns True if the data encapsulated in the left-hand side TreeNode is greater than
* the data encapsulated in the right-hand side TreeNode.
*/
template<typename DataType>
inline bool operator>(const TreeNode<DataType>& lhs, const TreeNode<DataType>& rhs)
{
   return rhs.GetData() < lhs.GetData();
}

/**
* @returns True if the data encapsulated in the left-hand side TreeNode is greater than
* or equal to the data encapsulated in the right-hand side TreeNode.
*/
template<typename DataType>
inline bool operator>=(const TreeNode<DataType>& lhs, const TreeNode<DataType>& rhs)
{
   return !(lhs.GetData() < rhs.GetData());
}

/**
* @returns True if the data encapsulated in the left-hand side TreeNode is equal to
* the data encapsulated in the right-hand side TreeNode.
*/
template<typename DataType>
inline bool operator==(const TreeNode<DataType>& lhs, const TreeNode<DataType>& rhs)
{
   return lhs.GetData() == rhs.GetData();
}

/**
* @returns True if the data encapsulated in the left-hand side TreeNode is not equal
* to the data encapsulated in the right-hand side TreeNode.
*/
template<typename DataType>
inline bool operator!=(const TreeNode<DataType>& lhs, const TreeNode<DataType>& rhs)
{
   return !(lhs.GetData() == rhs.GetData());
}

/**
* The TreeNode class represents the nodes that make up the Tree.
*
* Each node has a pointer to its parent, its first and last child, its previous and next
* sibling, and, of course, to the data it encapsulates.
*/
template<typename DataType>
class TreeNode
{
public:
   typedef DataType           value_type;
   typedef DataType&          reference;
   typedef const DataType&    const_reference;

   /**
   * @brief TreeNode default constructs a new TreeNode. All outgoing links from this new node will
   * initialized to a nullptr.
   */
   TreeNode() :
      m_parent(nullptr),
      m_firstChild(nullptr),
      m_lastChild(nullptr),
      m_previousSibling(nullptr),
      m_nextSibling(nullptr),
      m_data(),
      m_childCount(0),
      m_visited(false)
   {
   }

   /**
   * @brief TreeNode constructs a new TreeNode encapsulating the specified data. All outgoing links
   * from the node will be initialized to nullptr.
   */
   TreeNode(DataType data) :
      m_parent(nullptr),
      m_firstChild(nullptr),
      m_lastChild(nullptr),
      m_previousSibling(nullptr),
      m_nextSibling(nullptr),
      m_data(data),
      m_childCount(0),
      m_visited(false)
   {
   }

   /**
   * @brief TreeNode performs a shallow copy-construction of the specified TreeNode.
   */
   TreeNode(TreeNode<DataType>& other) :
      m_parent(nullptr),
      m_firstChild(nullptr),
      m_lastChild(nullptr),
      m_previousSibling(nullptr),
      m_nextSibling(nullptr),
      m_data(other.m_data),
      m_childCount(0),
      m_visited(false)
   {
      // @todo Const &!

      copy(other, *this);
   }

   /**
   *
   */
   ~TreeNode()
   {
      m_parent = nullptr;
      m_firstChild = nullptr;
      m_lastChild = nullptr;
      m_previousSibling = nullptr;
      m_nextSibling = nullptr;
   }

   /**
   * @brief Exception-safe swap function.
   */
   friend void swap(TreeNode<DataType>& lhs, TreeNode<DataType>& rhs) /* noexcept */
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
   * @returns The encapsulated data.
   */
   DataType* operator->()
   {
      return &m_data;
   }

   /**
   * @overload
   */
   const DataType* operator->() const
   {
      return &m_data;
   }

   /**
   * @brief MarkVisited sets node visitation status.
   *
   * @param[in] visited             Whether the node should be marked as having been visited.
   */
   inline void MarkVisited(const bool visited = true)
   {
      m_visited = visited;
   }

   /**
   * @returns True if the node has been marked as visited.
   */
   inline bool GetVisited() const
   {
      return m_visited;
   }

   /**
   * @brief PrependChild will prepend the specified TreeNode as the first child of the TreeNode.
   *
   * @param[in] child               The new TreeNode to set as the first child of the TreeNode.
   *
   * @returns A std::shared_ptr to the newly appended child.
   */
   TreeNode<DataType>* PrependChild(const TreeNode<DataType>& child)
   {
      child->m_parent = this;

      if (!m_firstChild)
      {
         return AddFirstChild(child);
      }

      assert(m_firstChild);

      m_firstChild->m_previousSibling = child;
      m_firstChild->m_previousSibling->m_nextSibling = m_firstChild;
      m_firstChild = m_firstChild->m_previousSibling;

      m_childCount++;

      return m_firstChild;
   }

   /**
   * @brief PrependChild will construct and prepend a new TreeNode as the first child of the TreeNode.
   *
   * @param[in] data                The underlying data to be stored in the new TreeNode.
   *
   * @returns The newly prepended TreeNode.
   */
   TreeNode<DataType>* PrependChild(DataType& data)
   {
      const auto* newNode = new TreeNode<DataType>(data);
      return PrependChild(*newNode);
   }

   /**
   * @overload
   */
   TreeNode<DataType>* PrependChild(const DataType& data)
   {
      const auto* newNode = new TreeNode<DataType>(data);
      return PrependChild(*newNode);
   }

   /**
   * @overload
   */
   TreeNode<DataType>* PrependChild(DataType&& data)
   {
      const auto* newNode = new TreeNode<DataType>(std::forward<DataType>(data));
      return PrependChild(*newNode);
   }

   /**
   * @brief AppendChild will append the specified TreeNode as a child of the TreeNode.
   *
   * @param[in] child               The new TreeNode to set as the last child of the TreeNode.
   *
   * @returns A std::shared_ptr to the newly appended child.
   */
   TreeNode<DataType>* AppendChild(TreeNode<DataType>& child)
   {
      child.m_parent = this;

      if (!m_lastChild)
      {
         return AddFirstChild(child);
      }

      assert(m_lastChild);

      m_lastChild->m_nextSibling = &child;
      m_lastChild->m_nextSibling->m_previousSibling = m_lastChild;
      m_lastChild = m_lastChild->m_nextSibling;

      m_childCount++;

      return m_lastChild;
   }

   /**
   * @brief AppendChild will construct and append a new TreeNode as the last child of the TreeNode.
   *
   * @param[in] data                The underlying data to be stored in the new TreeNode.
   *
   * @returns The newly appended TreeNode.
   */
   TreeNode<DataType>* AppendChild(DataType& data)
   {
      const auto* const newNode = new TreeNode<DataType>(data);
      return AppendChild(*newNode);
   }

   /**
   * @overload
   */
   TreeNode<DataType>* AppendChild(const DataType& data)
   {
      auto* const newNode = new TreeNode<DataType>(data);
      return AppendChild(*newNode);
   }

   /**
   * @overload
   */
   TreeNode<DataType>* AppendChild(DataType&& data)
   {
      auto* const newNode = new TreeNode<DataType>(std::forward<DataType>(data));
      return AppendChild(*newNode);
   }

   /**
   * @returns The underlying data stored in the TreeNode.
   */
   inline DataType& GetData()
   {
      return m_data;
   }

   /**
   * @overload
   */
   inline const DataType& GetData() const
   {
      return m_data;
   }

   /**
   * @returns A std::shared_ptr to the TreeNode's parent, if it exists; nullptr otherwise.
   */
   inline TreeNode<DataType>* const GetParent() const
   {
      return m_parent;
   }

   /**
   * @returns A std::shared_ptr to the TreeNode's first child.
   */
   inline TreeNode<DataType>* const GetFirstChild() const
   {
      return m_firstChild;
   }

   /**
   * @returns A std::shared_ptr to the TreeNode's last child.
   */
   inline TreeNode<DataType>* const GetLastChild() const
   {
      return m_lastChild;
   }

   /**
   * @returns A std::shared_ptr to the TreeNode's next sibling.
   */
   inline TreeNode<DataType>* const GetNextSibling() const
   {
      return m_nextSibling;
   }

   /**
   * @returns A std::shared_ptr to the TreeNode's previous sibling.
   */
   inline TreeNode<DataType>* const GetPreviousSibling() const
   {
      return m_previousSibling;
   }

   /**
   * @returns True if this node has direct descendants.
   */
   inline bool HasChildren() const
   {
      return m_childCount > 0;
   }

   /**
   * @returns The number of direct descendants that this node has.
   *
   * @note This does not include grandchildren.
   */
   inline unsigned int GetChildCount() const
   {
      return m_childCount;
   }

   /**
   * @returns The total number of descendant nodes belonging to the node.
   */
   inline size_t CountAllDescendants()
   {
      const auto itr = Tree<DataType>::PostOrderIterator(this);
      const auto end = Tree<DataType>::PostOrderIterator();
      const auto nodeCount = std::count_if(itr, end,
         [](Tree<DataType>::const_reference) { return true; });

      return nodeCount - 1;
   }

   /**
   * @brief SortChildren performs a merge sort of the direct descendants nodes.
   *
   * @param[in] comparator          The function to be used as the basis for the sorting comparison.
   */
   void SortChildren(const std::function<bool(TreeNode<DataType>, TreeNode<DataType>)>& comparator)
   {
      if (!m_firstChild)
      {
         return;
      }

      MergeSort(m_firstChild, comparator);
   }

private:

   /**
   * @brief MergeSort is the main entry point into the merge sort implementation.
   *
   * @param[in] list                The first TreeNode in the list to be sorted.
   * @param[in] comparator          The comparator function to be called to figure out which node
   *                                is the lesser of the two.
   */
   void MergeSort(
      TreeNode<DataType>*& list,
      const std::function<bool(TreeNode<DataType>, TreeNode<DataType>)>& comparator)
   {
      if (!list || !list->m_nextSibling)
      {
         return;
      }

      TreeNode<DataType>* head = list;
      TreeNode<DataType>* lhs = nullptr;
      TreeNode<DataType>* rhs = nullptr;

      DivideList(head, lhs, rhs);

      assert(lhs);
      assert(rhs);

      MergeSort(lhs, comparator);
      MergeSort(rhs, comparator);

      list = MergeSortedHalves(lhs, rhs, comparator);
   }

   /**
   * @brief DivideList is a helper function that will divide the specified TreeNode list in two.
   *
   * @param[in] head                The head of the TreeNode list to be divided in two.
   * @param[out] lhs                The first TreeNode of the left hand side list.
   * @param[out] rhs                The first TreeNode of the right hand side list.
   */
   void DivideList(
      TreeNode<DataType>* head,
      TreeNode<DataType>*& lhs,
      TreeNode<DataType>*& rhs)
   {
      if (!head || !head->m_nextSibling)
      {
         return;
      }

      TreeNode<DataType>* tortoise = head;
      TreeNode<DataType>* hare = head->m_nextSibling;

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
   * @returns The first node of the merged TreeNode list.
   */
   TreeNode<DataType>* MergeSortedHalves(
      TreeNode<DataType>*& lhs,
      TreeNode<DataType>*& rhs,
      const std::function<bool(TreeNode<DataType>, TreeNode<DataType>)>& comparator)
   {
      TreeNode<DataType>* result = nullptr;
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

      TreeNode<DataType>* tail = result;

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

   /**
   * @brief AddFirstChild is a helper function to make it easier to add the first descendant.
   *
   * @param[in] child               The TreeNode to be added as a child.
   *
   * @returns The newly added node.
   */
   TreeNode<DataType>* AddFirstChild(TreeNode<DataType>& child)
   {
      assert(m_childCount == 0);

      m_firstChild = &child;
      m_lastChild = m_firstChild;

      m_childCount++;

      return m_firstChild;
   }

   /**
   * @brief Helper function to assist on copy TreeNodes.
   *
   * @param[in] source              The TreeNode to copy information from.
   * @param[out] sink               The TreeNode to place a copy of the information into.
   */
   void copy(TreeNode<DataType>& source, TreeNode<DataType>& sink)
   {
      sink.m_data = source.m_data;

      if (!source.HasChildren())
      {
         return;
      }

      std::for_each(
         Tree<DataType>::SiblingIterator(source.GetFirstChild()),
         Tree<DataType>::SiblingIterator(),
         [&](Tree<DataType>::const_reference node)
      {
         sink.AppendChild(node.GetData());
      });

      const auto end = Tree<DataType>::SiblingIterator();

      auto sourceItr = Tree<DataType>::SiblingIterator(source.GetFirstChild());
      auto sinkItr = Tree<DataType>::SiblingIterator(sink.GetFirstChild());

      while (sourceItr != end)
      {
         copy(*sourceItr++, *sinkItr++);
      }
   }

   TreeNode<DataType>* m_parent;
   TreeNode<DataType>* m_firstChild;
   TreeNode<DataType>* m_lastChild;
   TreeNode<DataType>* m_previousSibling;
   TreeNode<DataType>* m_nextSibling;

   DataType m_data;

   unsigned int m_childCount;

   bool m_visited;
};

/**
* The Tree class declares a basic n-ary tree, built on top of templatized TreeNode nodes.
*
* Each tree consists of a simple head TreeNode and nothing else.
*/
template<typename DataType>
class Tree
{
public:
   friend class TreeNode<DataType>;

   class Iterator;
   class PreOrderIterator;
   class PostOrderIterator;
   class LeafIterator;
   class SiblingIterator;

   // Typedefs needed for STL compliance:
   typedef TreeNode<DataType>                value_type;
   typedef TreeNode<DataType>&               reference;
   typedef const TreeNode<DataType>&         const_reference;

   /**
   * @brief Default constructor.
   */
   Tree() :
      m_head(new TreeNode<DataType>())
   {
   }

   /**
   * @brief Tree constructs a new Tree with the provided data encapsulated in a new
   * TreeNode.
   */
   Tree(DataType data) :
      m_head(new TreeNode<DataType>(data))
   {
   }

   /**
   * @brief Copy constructor.
   */
   Tree(const Tree<DataType>& other) :
      m_head(new TreeNode<DataType>(*other.m_head))
   {
   }

   /**
   * @brief Assignment operator.
   *
   * @todo Is this signature right?
   */
   Tree<DataType>& operator=(Tree<DataType> other)
   {
      swap(*this, other);
      return *this;
   }

   /**
   *
   */
   friend void swap(Tree<DataType>& lhs, Tree<DataType>& rhs)
   {
      // Enable Argument Dependent Lookup (ADL):
      using std::swap;

      swap(lhs.m_head, rhs.m_head);
   }

   /**
   * @brief Destructor deletes all TreeNodes in the Tree.
   */
   ~Tree()
   {
      TreeNode<DataType>* lastNodeVisited = nullptr;

      std::for_each(
         Tree<DataType>::PostOrderIterator(m_head),
         Tree<DataType>::PostOrderIterator(),
         [&](Tree<DataType>::reference node)
      {
         if (lastNodeVisited)
         {
            delete lastNodeVisited;
         }

         lastNodeVisited = &node;
      });

      delete lastNodeVisited;
   }

   /**
   * @returns A std::shared_ptr to the head TreeNode.
   */
   TreeNode<DataType>* GetHead() const
   {
      return m_head;
   }

   /**
   * @brief Computes the size of nodes in the Tree.
   *
   * @complexity Linear in the size of the sub-tree.
   *
   * @returns The total number of nodes in the Tree (both leaf and non-leaf).
   */
   size_t Size() const
   {
      return std::count_if(std::begin(*this), std::end(*this),
         [](const_reference) { return true; });
   }

   /**
   * @returns The zero-indexed depth of the TreeNode in its Tree.
   */
   static unsigned int Depth(TreeNode<DataType> node)
   {
      unsigned int depth = 0;

      TreeNode<DataType>* nodePtr = &node;
      while (nodePtr->GetParent())
      {
         ++depth;
         nodePtr = nodePtr->GetParent();
      }

      return depth;
   }

   /**
   * @brief beginSibling Creates a sibling iterator starting at the specified node.
   *
   * @param node                The TreeNode at which to start iteration.
   *
   * @returns A sibling iterator that advances over the siblings of the node.
   */
   typename Tree::SiblingIterator beginSibling(const TreeNode<DataType>& node) const
   {
      assert(!"Test me!");

      const auto iterator = Tree<DataType>::SiblingIterator(node);
      return iterator;
   }

   /**
   * @brief endSibling Creates a sibling iterator that points past the last sibling to iterate
   * over.
   *
   * @param[in] node                Any node that is a sibling of the target range.
   *
   * @returns A sibling iterator pointing past the last sibling.
   */
   typename Tree::SiblingIterator endSibling(const TreeNode<DataType>& node) const
   {
      assert(!"Test me!");

      Tree<DataType>::SiblingIterator siblingIterator(nullptr);
      siblingIterator.m_parent = node->GetParent();

      return siblingIterator;
   }

   /**
   * @returns A pre-order iterator that will iterate over all TreeNodes in the tree.
   */
   typename Tree::PreOrderIterator beginPreOrder() const
   {
      const auto iterator = Tree<DataType>::PreOrderIterator(m_head);
      return iterator;
   }

   /**
   * @returns A pre-order iterator pointing "past" the end of the tree.
   */
   typename Tree::PreOrderIterator endPreOrder() const
   {
      const auto iterator = Tree<DataType>::PreOrderIterator(nullptr);
      return iterator;
   }

   /**
   * @returns A post-order iterator that will iterator over all nodes in the tree, starting
   * with the head of the Tree.
   */
   typename Tree::PostOrderIterator begin() const
   {
      const auto iterator = Tree<DataType>::PostOrderIterator(m_head);
      return iterator;
   }

   /**
   * @returns A post-order iterator that points past the end of the Tree.
   */
   typename Tree::PostOrderIterator end() const
   {
      const auto iterator = Tree<DataType>::PostOrderIterator(nullptr);
      return iterator;
   }

   /**
   * @returns An iterator that will iterator over all leaf nodes in the Tree, starting with the
   * left-most leaf in the Tree.
   */
   typename Tree::LeafIterator beginLeaf() const
   {
      const auto iterator = Tree<DataType>::LeafIterator(m_head);
      return iterator;
   }

   /**
   * @return A LeafIterator that points past the last leaf TreeNode in the Tree.
   */
   typename Tree::LeafIterator endLeaf() const
   {
      const auto iterator = Tree<DataType>::LeafIterator(nullptr);
      return iterator;
   }

private:
   TreeNode<DataType>* m_head;
};

/**
* @brief The Iterator class
*
* This is the base iterator class that all other iterators (sibling, post-, pre-, in-order)
* will derive from.
*/
template<typename DataType>
class Tree<DataType>::Iterator
{
public:
   friend class Tree<DataType>;

   // Typedefs needed for STL compliance:
   typedef DataType                             value_type;
   typedef DataType*                            pointer;
   typedef DataType&                            reference;
   typedef const DataType&                      const_reference;
   typedef std::size_t                          size_type;
   typedef std::ptrdiff_t                       difference_type;
   typedef std::bidirectional_iterator_tag      iterator_category;

   /**
   * @returns True if the Tree::Iterator points to a valid TreeNode; false otherwise.
   */
   explicit operator bool() const
   {
      const bool isValid = (m_currentNode != nullptr);
      return isValid;
   }

   /**
   * @returns The TreeNode pointed to by the Tree::Iterator.
   */
   TreeNode<DataType>& operator*()
   {
      return *m_currentNode;
   }

   /**
   * @overload
   */
   const TreeNode<DataType>& operator*() const
   {
      return *m_currentNode;
   }

   /**
   * @returns A pointer to the TreeNode.
   */
   TreeNode<DataType>* const operator&()
   {
      return m_currentNode;
   }

   /**
   * @overload
   */
   const TreeNode<DataType>* const operator&() const
   {
      return m_currentNode;
   }

   /**
   * @returns A pointer to the TreeNode pointed to by the Tree:Iterator.
   */
   TreeNode<DataType>* const operator->()
   {
      return m_currentNode;
   }

   /**
   * @overload
   */
   const TreeNode<DataType>* const operator->() const
   {
      return m_currentNode;
   }

   /**
   * @returns True if the Iterator points to the same node as the other Iterator.
   */
   bool operator==(const Iterator& other) const
   {
      return m_currentNode == other.m_currentNode;
   }

   /**
   * @returns True if the Iterator points to the same node as the other Iterator.
   */
   bool operator!=(const Iterator& other) const
   {
      return m_currentNode != other.m_currentNode;
   }

protected:

   /**
   * Default constructor.
   */
   explicit Iterator() :
      m_currentNode(nullptr),
      m_startingNode(nullptr),
      m_endingNode(nullptr)
   {
   }

   /**
   * Copy constructor.
   */
   explicit Iterator(const Iterator& other) :
      m_currentNode(other.m_currentNode),
      m_startingNode(other.m_startingNode),
      m_endingNode(other.m_endingNode)
   {
   }

   explicit Iterator(TreeNode<DataType>* node) :
      m_currentNode(node),
      m_startingNode(node),
      m_endingNode(nullptr)
   {
   }

   TreeNode<DataType>* m_currentNode;
   TreeNode<DataType>* m_startingNode;
   TreeNode<DataType>* m_endingNode;
};

/**
* @brief The PreOrderIterator class
*/
template<typename DataType>
class Tree<DataType>::PreOrderIterator : public Tree<DataType>::Iterator
{
public:
   /**
   * Default constructor.
   */
   explicit PreOrderIterator() :
      Iterator()
   {
   }

   /**
   * Copy constructor.
   */
   explicit PreOrderIterator(const Iterator& other) :
      Iterator(other)
   {
      assert(!"Test me!");
   }

   /**
   * Constructs an iterator that starts and ends at the specified node.
   */
   explicit PreOrderIterator(TreeNode<DataType>* node) :
      Iterator(node)
   {
      m_endingNode = (node && node->GetNextSibling())
         ? node->GetNextSibling()
         : nullptr;
   }

   /**
   * Prefix increment operator.
   */
   typename Tree::PreOrderIterator& operator++()
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
   typename Tree::PreOrderIterator operator++(int)
   {
      const auto result = *this;
      ++(*this);

      return result;
   }

   /**
   * Prefix decrement operator.
   */
   typename Tree::PreOrderIterator operator--()
   {
      assert(!"Test me!");

      auto* traversingNode = m_currentNode;

      if (!traversingNode)
      {
         traversingNode = m_startingNode;

         while (traversingNode->GetLastChild())
         {
            traversingNode = traversingNode->GetLastChild();
         }
      }
      else if (traversingNode->GetPreviousSibling())
      {
         traversingNode = traversingNode->GetPreviousSibling();

         while (traversingNode->GetLastChild())
         {
            traversingNode = traversingNode->GetLastChild();
         }
      }
      else if (traversingNode->GetParent())
      {
         traversingNode = traversingNode->GetParent();
      }
      else
      {
         traversingNode = nullptr;
      }

      // @todo Check against ending node.

      m_currentNode = traversingNode;
      return *this;
   }

   /**
   * Postfix decrement operator.
   */
   typename Tree::PreOrderIterator& operator--(int)
   {
      assert(!"Test me!");

      const auto result = *this;
      --(*this);

      return result;
   }
};

/**
* @brief The PostOrderIterator class
*/
template<typename DataType>
class Tree<DataType>::PostOrderIterator : public Tree<DataType>::Iterator
{
public:
   /**
   * Default constructor.
   */
   explicit PostOrderIterator() :
      Iterator(),
      m_traversingUpTheTree(false)
   {
   }

   /**
   * Copy constructor.
   */
   explicit PostOrderIterator(const Iterator& other) :
      Iterator(other),
      m_traversingUpTheTree(false)
   {
      assert(!"Test me!");
   }

   /**
   * Constructs an iterator that starts and ends at the specified node.
   */
   explicit PostOrderIterator(TreeNode<DataType>* node) :
      Iterator(node),
      m_traversingUpTheTree(false)
   {
      if (node && node->GetNextSibling())
      {
         auto* traversingNode = node->GetNextSibling();
         while (traversingNode->HasChildren())
         {
            traversingNode = traversingNode->GetFirstChild();
         }

         m_endingNode = traversingNode;
      }

      if (node)
      {
         while (node->GetFirstChild())
         {
            node = node->GetFirstChild();
         }

         m_currentNode = node;
      }
   }

   /**
   * Prefix increment operator.
   */
   typename Tree::PostOrderIterator& operator++()
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
   typename Tree::PostOrderIterator operator++(int)
   {
      const auto result = *this;
      ++(*this);

      return result;
   }

   /**
   * Prefix decrement operator.
   */
   typename Tree::PostOrderIterator& operator--()
   {
      assert(!"Test me!");

      auto* traversingNode = m_currentNode;

      // When the iterator is at the end(), then the next position should be the head:
      if (!traversingNode)
      {
         assert(traversingNode);

         traversingNode = m_startingNode;
      }
      else if (traversingNode->HasChildren())
      {
         traversingNode = traversingNode->GetLastChild();
      }
      else if (traversingNode->GetPreviousSibling())
      {
         traversingNode = traversingNode->GetPreviousSibling();
      }
      else if (traversingNode->GetParent())
      {
         while (traversingNode->GetParent() && !traversingNode->GetParent()->GetPreviousSibling())
         {
            traversingNode = traversingNode->GetParent();
         }

         traversingNode = traversingNode->GetParent();

         if (traversingNode)
         {
            traversingNode = traversingNode->GetPreviousSibling();
         }
      }

      m_currentNode = traversingNode;
      return *this;
   }

   /**
   * Postfix decrement operator.
   */
   typename Tree::PostOrderIterator operator--(int)
   {
      assert(!"Test me!");

      const auto result = *this;
      --(*this);

      return result;
   }

private:
   bool m_traversingUpTheTree;
};

/**
* @brief The LeafIterator class
*/
template<typename DataType>
class Tree<DataType>::LeafIterator : public Tree<DataType>::Iterator
{
public:
   /**
   * Default constructor.
   */
   explicit LeafIterator() :
      Iterator()
   {
   }

   /**
   * Copy constructor.
   */
   explicit LeafIterator(const Iterator& other) :
      Iterator(other)
   {
      assert(!"Test me!");
   }

   /**
   * Constructs an iterator that starts at the specified node and iterates to the end.
   */
   explicit LeafIterator(TreeNode<DataType>* node) :
      Iterator(node)
   {
      if (!m_currentNode)
      {
         return;
      }

      auto* firstNode = m_currentNode;
      if (firstNode->HasChildren())
      {
         while (firstNode->GetFirstChild())
         {
            firstNode = firstNode->GetFirstChild();
         }
      }

      m_currentNode = firstNode;

      auto* lastNode = node;
      if (lastNode->GetNextSibling())
      {
         lastNode = lastNode->GetNextSibling();

         while (lastNode->HasChildren())
         {
            lastNode = lastNode->GetFirstChild();
         }

         m_endingNode = lastNode;
      }
      else
      {
         m_endingNode = nullptr;
      }
   }

   /**
   * Prefix increment operator.
   */
   typename Tree::LeafIterator& operator++()
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
   typename Tree::LeafIterator operator++(int)
   {
      const auto result = *this;
      ++(*this);

      return result;
   }

   /**
   * Prefix decrement operator.
   */
   typename Tree::LeafIterator& operator--()
   {
      assert(!"Test me!");

      assert(m_currentNode);
      auto* traversingNode = m_currentNode;

      if (!traversingNode)
      {
         traversingNode = m_startingNode;
      }

      if (traversingNode->HasChildren())
      {
         while (traversingNode->GetLastChild())
         {
            traversingNode = traversingNode->GetLastChild();
         }
      }
      else if (traversingNode->GetPreviousSibling())
      {
         traversingNode = traversingNode->GetPreviousSibling();

         while (traversingNode->GetLastChild())
         {
            traversingNode->GetLastChild();
         }
      }
      else if (traversingNode->GetParent())
      {
         while (traversingNode->GetParent() && !traversingNode->GetParent()->GetPreviousSibling())
         {
            traversingNode = traversingNode->GetParent();
         }

         if (traversingNode->GetParent())
         {
            traversingNode = traversingNode->GetParent()->GetPreviousSibling();

            while (traversingNode->HasChildren())
            {
               traversingNode = traversingNode->GetLastChild();
            }

            return *this;
         }

         // Otherwise, the traversal is at the end:
         traversingNode = nullptr;
      }

      m_currentNode = traversingNode;
      return *this;
   }

   /**
   * Postfix decrement operator.
   */
   typename Tree::LeafIterator operator--(int)
   {
      assert(!"Test me!");

      const auto result = *this;
      --(*this);

      return result;
   }
};

/**
* @brief The SiblingIterator class
*/
template<typename DataType>
class Tree<DataType>::SiblingIterator : public Tree<DataType>::Iterator
{
public:
   /**
   * Default constructor.
   */
   explicit SiblingIterator() :
      Iterator()
   {
   }

   /**
   * Copy constructor.
   */
   explicit SiblingIterator(const Iterator& other) :
      Iterator(other)
   {
      assert(!"Test me!");
   }

   /**
   * Constructs an iterator that starts at the specified node and iterates to the end.
   */
   explicit SiblingIterator(TreeNode<DataType>* node) :
      Iterator(node)
   {
   }

   /**
   * Prefix increment operator.
   */
   typename Tree::SiblingIterator& operator++()
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
   typename Tree::SiblingIterator operator++(int)
   {
      const auto result = *this;
      ++(*this);

      return result;
   }

   /**
   * Prefix decrement operator.
   */
   typename Tree::SiblingIterator& operator--()
   {
      assert(!"Test me!");

      if (!m_currentNode)
      {
         if (m_parent)
         {
            m_currentNode = m_parent->GetLastChild();
         }
         else
         {
            // If no parent exists, then the node in question must be the one "past" the head,
            // so decrementing from there should return the head node:
            m_currentNode = m_startingNode;
         }
      }
      else
      {
         m_currentNode = m_currentNode->GetPreviousSibling();
      }

      return *this;
   }

   /**
   * Postfix decrement operator.
   */
   typename Tree::SiblingIterator operator--(int)
   {
      assert(!"Test me!");

      const auto result = *this;
      --(*this);

      return result;
   }

private:
   TreeNode<DataType>* m_parent;
};
