#pragma once

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "Catch.hpp"

#include "../Tree/Tree.hpp"

#include <algorithm>
#include <vector>

TEST_CASE("TreeNode Construction and Assignment")
{
   const Tree<std::string>::Node node{ "Bar" };

   SECTION("Default Construction")
   {
      const Tree<std::string>::Node defaultNode{ };
      const std::string defaultString{ };

      REQUIRE(defaultNode.GetChildCount() == 0);
      REQUIRE(defaultNode.GetFirstChild() == nullptr);
      REQUIRE(defaultNode.GetLastChild() == nullptr);
      REQUIRE(defaultNode.GetParent() == nullptr);
      REQUIRE(defaultNode.GetNextSibling() == nullptr);
      REQUIRE(defaultNode.GetPreviousSibling() == nullptr);
      REQUIRE(defaultNode.GetData() == defaultString);
   }
}

TEST_CASE("TreeNode Comparison Operations")
{
   const Tree<int>::Node ten{ 10 };
   const Tree<int>::Node twenty{ 20 };

   SECTION("Less Than")
   {
      REQUIRE(ten < twenty);
      REQUIRE(!(twenty < ten));
   }

   SECTION("Less Than or Equal")
   {
      REQUIRE(ten <= ten);
      REQUIRE(ten <= twenty);
      REQUIRE(!(twenty <= ten));
   }

   SECTION("Equality")
   {
      REQUIRE(ten == ten);
      REQUIRE(!(ten == twenty));
   }

   SECTION("Greater Than")
   {
      REQUIRE(twenty > ten);
      REQUIRE(!(ten > twenty));
   }

   SECTION("Greater Than or Equal")
   {
      REQUIRE(twenty >= twenty);
      REQUIRE(twenty >= ten);
      REQUIRE(!(ten >= twenty));
   }
}

TEST_CASE("TreeNode Alterations")
{
   Tree<std::string>::Node node{ "Bar" };
   REQUIRE(node.GetData() == "Bar");

   SECTION("Altering Data")
   {
      auto& data = node.GetData();
      std::transform(std::begin(data), std::end(data), std::begin(data), ::toupper);

      REQUIRE(node.GetData() == "BAR");
   }
}

TEST_CASE("Prepending and Appending TreeNodes")
{
   Tree<int> tree{ 10 };

   const auto IsEachNodeValueLargerThanTheLast = [&]
   {
      int lastValue = -1;

      return std::all_of(std::begin(tree), std::end(tree),
         [&] (const auto& node)
      {
         const auto& data = node.GetData();
         const auto sanityCheck = data > lastValue;

         lastValue = data;

         return sanityCheck;
      });
   };

   SECTION("Prepending Nodes")
   {
      auto& head = *tree.GetHead();
      tree.AppendChild(1, head);
      tree.AppendChild(2, head);
      tree.AppendChild(3, head);
      tree.AppendChild(4, head);
      tree.AppendChild(5, head);
      tree.AppendChild(6, head);
      tree.AppendChild(7, head);
      tree.AppendChild(8, head);
      tree.AppendChild(9, head);

      const bool correctlyPrepended = IsEachNodeValueLargerThanTheLast();

      REQUIRE(correctlyPrepended);
      REQUIRE(tree.Size() == 10);
      REQUIRE(tree.GetHead()->GetChildCount() == 9);
   }

   SECTION("Appending Nodes")
   {
      auto& head = *tree.GetHead();
      tree.PrependChild(9, head);
      tree.PrependChild(8, head);
      tree.PrependChild(7, head);
      tree.PrependChild(6, head);
      tree.PrependChild(5, head);
      tree.PrependChild(4, head);
      tree.PrependChild(3, head);
      tree.PrependChild(2, head);
      tree.PrependChild(1, head);

      const bool correctlyPrepended = IsEachNodeValueLargerThanTheLast();

      REQUIRE(correctlyPrepended);
      REQUIRE(tree.Size() == 10);
      REQUIRE(tree.GetHead()->GetChildCount() == 9);
   }
}

TEST_CASE("Node Metadata")
{
   Tree<std::string> tree{ "F" };
   auto& nodeF = *tree.GetHead();
   auto& nodeB = tree.AppendChild("B", nodeF);
   auto& nodeA = tree.AppendChild("A", nodeB);
   auto& nodeD = tree.AppendChild("D", nodeB);
   auto& nodeC = tree.AppendChild("C", nodeD);
   auto& nodeE = tree.AppendChild("E", nodeD);
   auto& nodeG = tree.AppendChild("G", nodeF);
   auto& nodeI = tree.AppendChild("I", nodeG);
   auto& nodeH = tree.AppendChild("H", nodeI);

   SECTION("Tree Size")
   {
      REQUIRE(tree.Size() == 9);
   }

   SECTION("Node Depth")
   {
      REQUIRE(tree.Depth(nodeF) == 0);
      REQUIRE(tree.Depth(nodeA) == 2);
      REQUIRE(tree.Depth(nodeD) == 2);
      REQUIRE(tree.Depth(nodeC) == 3);
      REQUIRE(tree.Depth(nodeE) == 3);
      REQUIRE(tree.Depth(nodeG) == 1);
      REQUIRE(tree.Depth(nodeI) == 2);
      REQUIRE(tree.Depth(nodeH) == 3);
   }

   //SECTION("Descendent Count")
   //{
   //   REQUIRE(tree.GetHead()->GetFirstChild()->CountAllDescendants() == 4);
   //}
}

TEST_CASE("TreeNode::Iterators")
{
   const auto sharedNode = std::make_shared<Tree<std::string>::Node>("Test");
   const auto constItr = Tree<std::string>::PostOrderIterator{ *sharedNode };

   auto nonConstItr = Tree<std::string>::PostOrderIterator{ *sharedNode };

   SECTION("operator bool")
   {
      auto endItr = Tree<std::string>::PostOrderIterator{ };

      REQUIRE(constItr);
      REQUIRE(!endItr);
   }

   SECTION("operator*")
   {
      REQUIRE((*nonConstItr).GetData() == "Test");
   }

   SECTION("operator* const")
   {
      REQUIRE((*constItr).GetData() == "Test");
   }

   SECTION("operator&")
   {
      REQUIRE(&nonConstItr == sharedNode.get());
   }

   SECTION("operator& const")
   {
      REQUIRE(&constItr == sharedNode.get());
   }

   SECTION("operator->")
   {
      REQUIRE(nonConstItr->GetData() == "Test");
   }

   SECTION("operator-> const")
   {
      REQUIRE(constItr->GetData() == "Test");
   }

   SECTION("operator==")
   {
      const auto duplicate = Tree<std::string>::PostOrderIterator{ *sharedNode };

      REQUIRE(duplicate == constItr);
   }
}

TEST_CASE("Forward Pre- and Post-Order Traversal of Simple Binary Tree")
{
   Tree<std::string> tree{ "F" };
   auto& nodeF = *tree.GetHead();
   auto& nodeB = tree.AppendChild("B", nodeF);
   auto& nodeA = tree.AppendChild("A", nodeB);
   auto& nodeD = tree.AppendChild("D", nodeB);
   auto& nodeC = tree.AppendChild("C", nodeD);
   auto& nodeE = tree.AppendChild("E", nodeD);
   auto& nodeG = tree.AppendChild("G", nodeF);
   auto& nodeI = tree.AppendChild("I", nodeG);
   auto& nodeH = tree.AppendChild("H", nodeI);

   SECTION("Pre-order Traversal")
   {
      const std::vector<std::string> expectedTraversal =
         { "F", "B", "A", "D", "C", "E", "G", "I", "H" };

      int index = 0;

      bool traversalError = false;
      for (auto itr = tree.beginPreOrder(); itr != tree.endPreOrder(); ++itr)
      {
         const auto& data = itr->GetData();
         if (data != expectedTraversal[index++])
         {
            traversalError = true;
            break;
         }
      }

      REQUIRE(traversalError == false);
      REQUIRE(index == expectedTraversal.size());
   }

   SECTION("Post-order Traversal")
   {
      const std::vector<std::string> expectedTraversal =
         { "A", "C", "E", "D", "B", "H", "I", "G", "F" };

      int index = 0;

      bool traversalError = false;
      for (auto itr = tree.begin(); itr != tree.end(); ++itr)
      {
         const auto& data = itr->GetData();
         if (data != expectedTraversal[index++])
         {
            traversalError = true;
            break;
         }
      }

      REQUIRE(traversalError == false);
      REQUIRE(index == expectedTraversal.size());
   }
}

TEST_CASE("Partial Tree Iteration")
{
   Tree<std::string> tree{ "F" };
   auto& nodeF = *tree.GetHead();
   auto& nodeB = tree.AppendChild("B", nodeF);
   auto& nodeA = tree.AppendChild("A", nodeB);
   auto& nodeD = tree.AppendChild("D", nodeB);
   auto& nodeC = tree.AppendChild("C", nodeD);
   auto& nodeE = tree.AppendChild("E", nodeD);
   auto& nodeG = tree.AppendChild("G", nodeF);
   auto& nodeI = tree.AppendChild("I", nodeG);
   auto& nodeH = tree.AppendChild("H", nodeI);

   SECTION("Pre-Order Iteration")
   {
      const std::vector<std::string> expectedTraversal =
         { "B", "A", "D", "C", "E" };

      int index = 0;

      bool traversalError = false;

      auto& startingNode = tree.GetHead()->GetFirstChild();

      auto itr = Tree<std::string>::PreOrderIterator(*startingNode);
      const auto end = Tree<std::string>::PreOrderIterator{ };
      for (; itr != end; itr++) ///< Using the post-fix operator for more test coverage.
      {
         const auto& data = itr->GetData();
         if (data != expectedTraversal[index++])
         {
            traversalError = true;
            break;
         }
      }

      REQUIRE(traversalError == false);
      REQUIRE(index == expectedTraversal.size());
   }

   SECTION("Post-Order Iteration")
   {
      const std::vector<std::string> expectedTraversal =
         { "A", "C", "E", "D", "B" };

      int index = 0;

      bool traversalError = false;

      auto& startingNode = tree.GetHead()->GetFirstChild();

      auto itr = Tree<std::string>::PostOrderIterator(*startingNode);
      const auto end = Tree<std::string>::PostOrderIterator{ };
      for (; itr != end; itr++)  ///< Using the post-fix operator for more test coverage.
      {
         const auto& data = itr->GetData();
         if (data != expectedTraversal[index++])
         {
            traversalError = true;
            break;
         }
      }

      REQUIRE(traversalError == false);
      REQUIRE(index == expectedTraversal.size());
   }
}

TEST_CASE("Partial Tree Iteration Corner Cases")
{
   Tree<std::string> tree{ "F" };
   auto& nodeF = *tree.GetHead();
   auto& nodeB = tree.AppendChild("B", nodeF);
   auto& nodeA = tree.AppendChild("A", nodeB);
   auto& nodeD = tree.AppendChild("D", nodeB);
   auto& nodeC = tree.AppendChild("C", nodeD);
   auto& nodeE = tree.AppendChild("E", nodeD);
   auto& nodeG = tree.AppendChild("G", nodeF);
   auto& nodeI = tree.AppendChild("I", nodeG);
   auto& nodeH = tree.AppendChild("H", nodeI);

   SECTION("Pre-Order Iteration")
   {
      const std::vector<std::string> expectedTraversal = { "D", "C", "E" };

      int index = 0;

      bool traversalError = false;

      auto& startingNode = tree.GetHead()->GetFirstChild()->GetLastChild();

      auto itr = Tree<std::string>::PreOrderIterator{ *startingNode };
      const auto end = Tree<std::string>::PreOrderIterator{ };
      for (; itr != end; itr++) ///< Using the post-fix operator for more test coverage.
      {
         const auto& data = itr->GetData();
         if (data != expectedTraversal[index++])
         {
            traversalError = true;
            break;
         }
      }

      REQUIRE(traversalError == false);
      REQUIRE(index == expectedTraversal.size());
   }

   SECTION("Post-Order Iteration")
   {
      const std::vector<std::string> expectedTraversal = { "C", "E", "D" };

      int index = 0;

      bool traversalError = false;

      auto& startingNode = tree.GetHead()->GetFirstChild()->GetLastChild();

      auto itr = Tree<std::string>::PostOrderIterator{ *startingNode };
      const auto end = Tree<std::string>::PostOrderIterator{ };
      for (; itr != end; itr++) ///< Using the post-fix operator for more test coverage.
      {
         const auto& data = itr->GetData();
         if (data != expectedTraversal[index++])
         {
            traversalError = true;
            break;
         }
      }

      REQUIRE(traversalError == false);
      REQUIRE(index == expectedTraversal.size());
   }

   SECTION("Leaf Iteration")
   {
      const std::vector<std::string> expectedTraversal = { "C", "E" };

      int index = 0;

      bool traversalError = false;

      auto& startingNode = tree.GetHead()->GetFirstChild()->GetLastChild();

      auto itr = Tree<std::string>::LeafIterator{ *startingNode };
      const auto end = Tree<std::string>::LeafIterator{ };
      for (; itr != end; itr++) ///< Using the post-fix operator for more test coverage.
      {
         const auto& data = itr->GetData();
         if (data != expectedTraversal[index++])
         {
            traversalError = true;
            break;
         }
      }

      REQUIRE(traversalError == false);
      REQUIRE(index == expectedTraversal.size());
   }
}

TEST_CASE("STL Typedef Compliance")
{
   Tree<std::string> tree{ "F" };
   auto& nodeF = *tree.GetHead();
   auto& nodeB = tree.AppendChild("B", nodeF);
   auto& nodeA = tree.AppendChild("A", nodeB);
   auto& nodeD = tree.AppendChild("D", nodeB);
   auto& nodeC = tree.AppendChild("C", nodeD);
   auto& nodeE = tree.AppendChild("E", nodeD);
   auto& nodeG = tree.AppendChild("G", nodeF);
   auto& nodeI = tree.AppendChild("I", nodeG);
   auto& nodeH = tree.AppendChild("H", nodeI);

   SECTION("Standard Algorithms and Parameter Passing by reference")
   {
      const size_t count = std::count_if(std::begin(tree), std::end(tree),
         [] (Tree<std::string>::reference node)
      {
         return (node.GetData() == "C");
      });

      REQUIRE(count == 1);
   }

   SECTION("Standard Algorithms and Parameter Passing by const_reference")
   {
      const size_t count = std::count_if(std::begin(tree), std::end(tree),
         [] (Tree<std::string>::const_reference node)
      {
         return (node.GetData() == "D");
      });

      REQUIRE(count == 1);
   }
}

TEST_CASE("Leaf Iterator")
{
   Tree<std::string> tree{ "F" };
   auto& nodeF = *tree.GetHead();
   auto& nodeB = tree.AppendChild("B", nodeF);
   auto& nodeA = tree.AppendChild("A", nodeB);
   auto& nodeD = tree.AppendChild("D", nodeB);
   auto& nodeC = tree.AppendChild("C", nodeD);
   auto& nodeE = tree.AppendChild("E", nodeD);
   auto& nodeG = tree.AppendChild("G", nodeF);
   auto& nodeI = tree.AppendChild("I", nodeG);
   auto& nodeH = tree.AppendChild("H", nodeI);

   SECTION("Forward Traversal")
   {
      const std::vector<std::string> expectedTraversal = { "A", "C", "E", "H", };

      int index = 0;

      bool traversalError = false;

      auto itr = Tree<std::string>::LeafIterator(*tree.GetHead());
      const auto end = Tree<std::string>::LeafIterator{ };
      for (; itr != end; ++itr)
      {
         const auto& data = itr->GetData();
         if (data != expectedTraversal[index++])
         {
            traversalError = true;
            break;
         }
      }

      REQUIRE(traversalError == false);
      REQUIRE(index == expectedTraversal.size());
   }

   SECTION("Partial Tree Iteration")
   {
      const std::vector<std::string> expectedTraversal = { "A", "C", "E" };

      int index = 0;

      bool traversalError = false;

      auto itr = Tree<std::string>::LeafIterator(*tree.GetHead()->GetFirstChild());
      const auto end = Tree<std::string>::LeafIterator{ };
      for (; itr != end; ++itr)
      {
         const auto& data = itr->GetData();
         if (data != expectedTraversal[index++])
         {
            traversalError = true;
            break;
         }
      }

      REQUIRE(traversalError == false);
      REQUIRE(index == expectedTraversal.size());
   }

   SECTION("Tree<T>::beginLeaf and Tree<T>::endLeaf")
   {
      const std::vector<std::string> expectedTraversal = { "A", "C", "E", "H", };

      std::vector<std::string> output;
      std::transform(tree.beginLeaf(), tree.endLeaf(), std::back_inserter(output),
         [](Tree<std::string>::const_reference node)
      {
         return node.GetData();
      });

      REQUIRE(output.size() == expectedTraversal.size());
   }
}

TEST_CASE("Sibling Iterator")
{
   Tree<std::string> tree{ "X" };
   auto& head = *tree.GetHead();
   tree.AppendChild("B", head);
   tree.AppendChild("D", head);
   tree.AppendChild("A", head);
   tree.AppendChild("C", head);
   tree.AppendChild("F", head);
   tree.AppendChild("G", head);
   tree.AppendChild("E", head);
   tree.AppendChild("H", head);

   SECTION("Forward Traversal")
   {
      const std::vector<std::string> expectedTraversal =
         { "B", "D", "A", "C", "F", "G", "E", "H", };

      int index = 0;

      bool traversalError = false;

      const auto& startingNode = tree.GetHead()->GetFirstChild();

      auto itr = Tree<std::string>::SiblingIterator{ *startingNode };
      const auto end = Tree<std::string>::SiblingIterator{ };
      for (; itr != end; itr++)
      {
         const auto& data = itr->GetData();
         if (data != expectedTraversal[index++])
         {
            traversalError = true;
            break;
         }
      }

      REQUIRE(traversalError == false);
      REQUIRE(index == expectedTraversal.size());
   }
}

TEST_CASE("Sorting")
{
   SECTION("One Generation of Children")
   {
      Tree<std::string> tree{ "X" };
      auto& head = *tree.GetHead();
      tree.AppendChild("B", head);
      tree.AppendChild("D", head);
      tree.AppendChild("A", head);
      tree.AppendChild("C", head);
      tree.AppendChild("F", head);
      tree.AppendChild("G", head);
      tree.AppendChild("E", head);
      tree.AppendChild("H", head);

      const auto comparator =
         [](Tree<std::string>::const_reference lhs, Tree<std::string>::const_reference rhs)
      {
         return (lhs < rhs);
      };

      tree.SortChildren(head, comparator);

      const std::vector<std::string> expectedTraversal =
         { "A", "B", "C", "D", "E", "F", "G", "H", };

      int index = 0;

      bool traversalError = false;

      auto itr = Tree<std::string>::LeafIterator(*tree.GetHead());
      const auto end = Tree<std::string>::LeafIterator();
      for (; itr != end; itr++)  ///< Using the post-fix operator for more test coverage.
      {
         const auto& data = itr->GetData();
         if (data != expectedTraversal[index++])
         {
            traversalError = true;
            break;
         }
      }

      REQUIRE(traversalError == false);
      REQUIRE(index == expectedTraversal.size());
   }
   SECTION("An Entire Tree")
   {
      Tree<int> tree{ 999 };
      auto& head = *tree.GetHead();
      auto& firstChild = tree.AppendChild(634, head);
      tree.AppendChild(34, firstChild);
      tree.AppendChild(13, firstChild);
      tree.AppendChild(89, firstChild);
      tree.AppendChild(3, firstChild);
      tree.AppendChild(1, firstChild);
      tree.AppendChild(0, firstChild);
      tree.AppendChild(-5, firstChild);

      tree.AppendChild(375, head);
      tree.AppendChild(173, head);
      tree.AppendChild(128, head);

      const auto sizeBeforeSort = tree.Size();

      // Sort:
      std::for_each(std::begin(tree), std::end(tree),
         [&](auto& node)
      {
         tree.SortChildren(node,
            [] (auto& lhs, auto& rhs)
         {
            return lhs < rhs;
         });
      });

      const auto sizeAfterSort = tree.Size();

      bool sortingError = false;
      int lastItem = -999;

      // Verify:
      std::for_each(std::begin(tree), std::end(tree),
         [&](Tree<int>::const_reference node)
      {
         if (!node.HasChildren())
         {
            return;
         }

         auto child = node.GetFirstChild();
         while (child)
         {
            if (child->GetData() < lastItem)
            {
               sortingError = true;
               lastItem = child->GetData();
            }

            child = child->GetNextSibling();
         }
      });

      REQUIRE(sizeBeforeSort == sizeAfterSort);
      REQUIRE(sortingError == false);
   }
}

#if 0

TEST_CASE("TreeNode Copying")
{
   TreeNode<std::string> node{ "Node" };

   const auto copy = node;

   REQUIRE(node.GetData() == copy.GetData());
}

TEST_CASE("Tree Copying")
{
   Tree<std::string> tree{ "F" };

   auto& nodeF = *tree.GetHead();
   auto& nodeB = tree.AppendChild("B", nodeF);
   auto& nodeA = tree.AppendChild("A", nodeB);
   auto& nodeD = tree.AppendChild("D", nodeB);
   auto& nodeC = tree.AppendChild("C", nodeD);
   auto& nodeE = tree.AppendChild("E", nodeD);
   auto& nodeG = tree.AppendChild("G", nodeF);
   auto& nodeI = tree.AppendChild("I", nodeG);
   auto& nodeH = tree.AppendChild("H", nodeI);

   const auto copy = tree;

   REQUIRE(tree.Size() == copy.Size());

   SECTION("Pre-order Verification")
   {
      const std::vector<std::string> expectedTraversal =
      { "F", "B", "A", "D", "C", "E", "G", "I", "H" };

      int index = 0;

      bool traversalError = false;
      for (auto itr = copy.beginPreOrder(); itr != copy.endPreOrder(); ++itr)
      {
         const auto& data = itr->GetData();
         if (data != expectedTraversal[index++])
         {
            traversalError = true;
            break;
         }
      }

      REQUIRE(traversalError == false);
      REQUIRE(index == expectedTraversal.size());
   }
   SECTION("Post-order Verification")
   {
      const std::vector<std::string> expectedTraversal =
      { "A", "C", "E", "D", "B", "H", "I", "G", "F" };

      int index = 0;

      bool traversalError = false;
      for (auto itr = copy.begin(); itr != copy.end(); ++itr)
      {
         const auto& data = itr->GetData();
         if (data != expectedTraversal[index++])
         {
            traversalError = true;
            break;
         }
      }

      REQUIRE(traversalError == false);
      REQUIRE(index == expectedTraversal.size());
   }
}

#endif

namespace
{
   auto CONSTRUCTION_COUNT{ 0u };
   auto DESTRUCTION_COUNT{ 0u };
}

struct VerboseNode
{
   VerboseNode(const char* const data) :
      m_data(data)
   {
      CONSTRUCTION_COUNT++;
   }

   ~VerboseNode()
   {
      DESTRUCTION_COUNT++;
   }

   std::string m_data;
};

TEST_CASE("Tree and TreeNode Destruction")
{
   SECTION("Node Destruction Count")
   {
      CONSTRUCTION_COUNT = 0;

      {
         Tree<VerboseNode> tree{ "F" };
         auto& nodeF = *tree.GetHead();
         auto& nodeB = tree.AppendChild("B", nodeF);
         auto& nodeA = tree.AppendChild("A", nodeB);
         auto& nodeD = tree.AppendChild("D", nodeB);
         auto& nodeC = tree.AppendChild("C", nodeD);
         auto& nodeE = tree.AppendChild("E", nodeD);
         auto& nodeG = tree.AppendChild("G", nodeF);
         auto& nodeI = tree.AppendChild("I", nodeG);
         auto& nodeH = tree.AppendChild("H", nodeI);

         // Reset the destruction count, so that we don't accidentally count any destructor calls
         // that took place during the construction of the tree.
         DESTRUCTION_COUNT = 0;

         REQUIRE(tree.Size() == CONSTRUCTION_COUNT);
      }

      REQUIRE(CONSTRUCTION_COUNT == DESTRUCTION_COUNT);
   }
}

TEST_CASE("Selectively Delecting Nodes")
{
   const auto VerifyTraversal =
      [](const Tree<VerboseNode>& tree, const std::vector<std::string>& expected)
   {
      int index = 0;

      bool traversalError = false;
      for (auto itr = std::begin(tree); itr != std::end(tree); ++itr)
      {
         const auto& data = itr->GetData().m_data;
         if (data != expected[index++])
         {
            traversalError = true;
            break;
         }
      }

      REQUIRE(expected.size() == index);

      return !traversalError;
   };

   SECTION("Removing a Leaf Node Without Siblings")
   {
      CONSTRUCTION_COUNT = 0;
      size_t treeSize = 0;

      {
         Tree<VerboseNode> tree{ "F" };
         auto& nodeF = *tree.GetHead();
         auto& nodeB = tree.AppendChild("B", nodeF);
         auto& nodeA = tree.AppendChild("A", nodeB);
         auto& nodeD = tree.AppendChild("D", nodeB);
         auto& nodeC = tree.AppendChild("C", nodeD);
         auto& nodeE = tree.AppendChild("E", nodeD);
         auto& nodeG = tree.AppendChild("G", nodeF);
         auto& nodeI = tree.AppendChild("I", nodeG);
         auto& nodeH = tree.AppendChild("H", nodeI);

         treeSize = tree.Size();

         // Reset the destruction count, so that we don't accidentally count any destructor calls
         // that took place during the construction of the tree.
         DESTRUCTION_COUNT = 0;

         auto targetNode = tree.GetHead()->GetLastChild()->GetLastChild()->GetFirstChild();
         REQUIRE(targetNode != nullptr);
         REQUIRE(targetNode->GetData().m_data == "H");
         REQUIRE(targetNode->GetPreviousSibling() == nullptr);
         REQUIRE(targetNode->GetNextSibling() == nullptr);
         REQUIRE(targetNode->GetFirstChild() == nullptr);
         REQUIRE(targetNode->GetLastChild() == nullptr);

         const auto parentOfTarget = targetNode->GetParent();
         const auto parentsChildCount = parentOfTarget->GetChildCount();

         // Before removing the node from the tree, there should be 4 references to it:
         //    * Once in the underlying vector.
         //    * Once here as a local variable.
         //    * Once as the child-node of "D"
         //    * Once as the sibling-node of "C".
         REQUIRE(targetNode.use_count() == 4);

         targetNode->DetachFromTree();

         // After removing the node from the tree, there should be 2 references to it:
         //    * Once in the underlying vector.
         //    * Once here as a local variable.
         REQUIRE(targetNode.use_count() == 2);

         targetNode.~shared_ptr();

         // After destroying the local copy, the only remaining copy should be the one in the
         // underlying vector:
         REQUIRE(tree.GetNodesAsVector().size() > 0);
         REQUIRE(tree.GetNodesAsVector().back().use_count() == 1);

         const std::vector<std::string> expectedTraversal =
            { "A", "C", "E", "D", "B", "I", "G", "F" };

         const bool errorFree = VerifyTraversal(tree, expectedTraversal);
         REQUIRE(errorFree == true);
      }

      REQUIRE(DESTRUCTION_COUNT == treeSize);
   }

   SECTION("Removing a Leaf Node with A Left Sibling")
   {
      CONSTRUCTION_COUNT = 0;
      size_t treeSize = 0;

      {
         Tree<VerboseNode> tree{ "F" };
         auto& nodeF = *tree.GetHead();
         auto& nodeB = tree.AppendChild("B", nodeF);
         auto& nodeA = tree.AppendChild("A", nodeB);
         auto& nodeD = tree.AppendChild("D", nodeB);
         auto& nodeC = tree.AppendChild("C", nodeD);
         auto& nodeE = tree.AppendChild("E", nodeD);
         auto& nodeG = tree.AppendChild("G", nodeF);
         auto& nodeI = tree.AppendChild("I", nodeG);
         auto& nodeH = tree.AppendChild("H", nodeI);

         treeSize = tree.Size();

         // Reset the destruction count, so that we don't accidentally count any destructor calls
         // that took place during the construction of the tree.
         DESTRUCTION_COUNT = 0;

         auto targetNode = tree.GetHead()->GetFirstChild()->GetLastChild()->GetLastChild();
         REQUIRE(targetNode != nullptr);
         REQUIRE(targetNode->GetData().m_data == "E");
         REQUIRE(targetNode->GetPreviousSibling() != nullptr);
         REQUIRE(targetNode->GetNextSibling() == nullptr);
         REQUIRE(targetNode->GetFirstChild() == nullptr);
         REQUIRE(targetNode->GetLastChild() == nullptr);

         const auto parentOfTarget = targetNode->GetParent();
         const auto parentsChildCount = parentOfTarget->GetChildCount();

         // Before removing the node from the tree, there should be 4 references to it:
         //    * Once in the underlying vector.
         //    * Once here as a local variable.
         //    * Once as the child-node of "D"
         //    * Once as the sibling-node of "C".
         REQUIRE(targetNode.use_count() == 4);

         targetNode->DetachFromTree();

         // After removing the node from the tree, there should be 2 references to it:
         //    * Once in the underlying vector.
         //    * Once here as a local variable.
         REQUIRE(targetNode.use_count() == 2);

         targetNode.~shared_ptr();

         const auto& underlyingNodes = tree.GetNodesAsVector();

         auto itr = std::find_if(std::begin(underlyingNodes), std::end(underlyingNodes),
            [](const auto& node)
         {
            return node->GetData().m_data == "E";
         });

         if (itr != std::end(underlyingNodes))
         {
            // After removing the targeted node from the tree, it should have a reference count
            // of 1, since it should only exist in the underlying vector:
            REQUIRE(itr->use_count() == 1);
         }

         itr = std::find_if(std::begin(underlyingNodes), std::end(underlyingNodes),
            [](const auto& node)
         {
            return node->GetData().m_data == "C";
         });

         if (itr != std::end(underlyingNodes))
         {
            // After removing the node "E" from the tree, there should only be 3 references
            // to its sibling node "C":
            //    * Once in the underlying vector.
            //    * Once as the first child of "D"
            //    * Once as the last child of "D"
            REQUIRE(itr->use_count() == 3);
         }

         const std::vector<std::string> expectedTraversal =
            { "A", "C", "D", "B", "H", "I", "G", "F" };

         const bool errorFree = VerifyTraversal(tree, expectedTraversal);
         REQUIRE(errorFree == true);
      }

      REQUIRE(DESTRUCTION_COUNT == treeSize);
   }

#if 0

   SECTION("Removing a Leaf Node with A Right Sibling")
   {
      CONSTRUCTION_COUNT = 0;
      size_t treeSize = 0;

      {
         Tree<VerboseNode> tree{ "F" };
         auto& nodeF = *tree.GetHead();
         auto& nodeB = tree.AppendChild("B", nodeF);
         auto& nodeA = tree.AppendChild("A", nodeB);
         auto& nodeD = tree.AppendChild("D", nodeB);
         auto& nodeC = tree.AppendChild("C", nodeD);
         auto& nodeE = tree.AppendChild("E", nodeD);
         auto& nodeG = tree.AppendChild("G", nodeF);
         auto& nodeI = tree.AppendChild("I", nodeG);
         auto& nodeH = tree.AppendChild("H", nodeI);

         treeSize = tree.Size();

         // Reset the destruction count, so that we don't accidentally count any destructor calls
         // that took place during the construction of the tree.
         DESTRUCTION_COUNT = 0;

         const auto* targetNode = tree.GetHead()->GetFirstChild()->GetLastChild()->GetFirstChild();
         REQUIRE(targetNode != nullptr);
         REQUIRE(targetNode->GetData().m_data == "C");
         REQUIRE(targetNode->GetPreviousSibling() == nullptr);
         REQUIRE(targetNode->GetNextSibling() != nullptr);
         REQUIRE(targetNode->GetFirstChild() == nullptr);
         REQUIRE(targetNode->GetLastChild() == nullptr);

         const auto parentOfTarget = targetNode->GetParent();
         const auto parentsChildCount = parentOfTarget->GetChildCount();

         delete targetNode;

         REQUIRE(DESTRUCTION_COUNT == 1);
         REQUIRE(parentOfTarget->GetChildCount() == parentsChildCount - DESTRUCTION_COUNT);
         REQUIRE(tree.Size() == treeSize - DESTRUCTION_COUNT);

         const std::vector<std::string> expectedTraversal =
         { "A", "E", "D", "B", "H", "I", "G", "F" };

         const bool errorFree = VerifyTraversal(tree, expectedTraversal);
         REQUIRE(errorFree == true);
      }

      REQUIRE(DESTRUCTION_COUNT == treeSize);
   }

   SECTION("Removing a Leaf Node with Both Left and Right Siblings")
   {
      CONSTRUCTION_COUNT = 0;
      size_t treeSize = 0;

      {
         Tree<VerboseNode> tree{ "F" };
         auto& nodeF = *tree.GetHead();
         auto& nodeB = tree.AppendChild("B", nodeF);
         auto& nodeA = tree.AppendChild("A", nodeB);
         auto& nodeD = tree.AppendChild("D", nodeB);
         auto& nodeC = tree.AppendChild("C", nodeD);
         auto& nodeE = tree.AppendChild("E", nodeD);
         auto& nodeG = tree.AppendChild("G", nodeF);
         auto& nodeI = tree.AppendChild("I", nodeG);
         auto& nodeH = tree.AppendChild("H", nodeI);

         treeSize = tree.Size();

         // Reset the destruction count, so that we don't accidentally count any destructor calls
         // that took place during the construction of the tree.
         DESTRUCTION_COUNT = 0;

         const auto* targetNode =
            tree.GetHead()->GetFirstChild()->GetLastChild()->GetFirstChild()->GetNextSibling();
         REQUIRE(targetNode != nullptr);
         REQUIRE(targetNode->GetData().m_data == "X");
         REQUIRE(targetNode->GetPreviousSibling() != nullptr);
         REQUIRE(targetNode->GetNextSibling() != nullptr);
         REQUIRE(targetNode->GetFirstChild() == nullptr);
         REQUIRE(targetNode->GetLastChild() == nullptr);

         const auto parentOfTarget = targetNode->GetParent();
         const auto parentsChildCount = parentOfTarget->GetChildCount();

         delete targetNode;

         REQUIRE(DESTRUCTION_COUNT == 1);
         REQUIRE(parentOfTarget->GetChildCount() == parentsChildCount - DESTRUCTION_COUNT);
         REQUIRE(tree.Size() == treeSize - DESTRUCTION_COUNT);

         const std::vector<std::string> expectedTraversal =
         { "A", "C", "E", "D", "B", "H", "I", "G", "F" };

         const bool errorFree = VerifyTraversal(tree, expectedTraversal);
         REQUIRE(errorFree == true);
      }

      REQUIRE(DESTRUCTION_COUNT == treeSize);
   }

   SECTION("Removing a Node With a Left Sibling and Two Children")
   {
      CONSTRUCTION_COUNT = 0;
      size_t treeSize = 0;

      {
         Tree<VerboseNode> tree{ "F" };
         auto& nodeF = *tree.GetHead();
         auto& nodeB = tree.AppendChild("B", nodeF);
         auto& nodeA = tree.AppendChild("A", nodeB);
         auto& nodeD = tree.AppendChild("D", nodeB);
         auto& nodeC = tree.AppendChild("C", nodeD);
         auto& nodeE = tree.AppendChild("E", nodeD);
         auto& nodeG = tree.AppendChild("G", nodeF);
         auto& nodeI = tree.AppendChild("I", nodeG);
         auto& nodeH = tree.AppendChild("H", nodeI);

         treeSize = tree.Size();

         // Reset the destruction count, so that we don't accidentally count any destructor calls
         // that took place during the construction of the tree.
         DESTRUCTION_COUNT = 0;

         const auto* targetNode = tree.GetHead()->GetFirstChild()->GetLastChild();
         REQUIRE(targetNode != nullptr);
         REQUIRE(targetNode->GetData().m_data == "D");
         REQUIRE(targetNode->GetPreviousSibling() != nullptr);
         REQUIRE(targetNode->GetNextSibling() == nullptr);
         REQUIRE(targetNode->GetFirstChild() != nullptr);
         REQUIRE(targetNode->GetFirstChild() != targetNode->GetLastChild());

         const auto parentOfTarget = targetNode->GetParent();
         const auto parentsChildCount = parentOfTarget->GetChildCount();

         delete targetNode;

         REQUIRE(DESTRUCTION_COUNT == 3);
         REQUIRE(parentOfTarget->GetChildCount() == parentsChildCount - 1);
         REQUIRE(tree.Size() == treeSize - DESTRUCTION_COUNT);

         const std::vector<std::string> expectedTraversal =
         { "A", "B", "H", "I", "G", "F" };

         const bool errorFree = VerifyTraversal(tree, expectedTraversal);
         REQUIRE(errorFree == true);
      }

      REQUIRE(DESTRUCTION_COUNT == treeSize);
   }

   SECTION("Deleting a Node by Calling DeleteFromTree()")
   {
      CONSTRUCTION_COUNT = 0;
      size_t treeSize = 0;

      {
         Tree<VerboseNode> tree{ "F" };
         auto& nodeF = *tree.GetHead();
         auto& nodeB = tree.AppendChild("B", nodeF);
         auto& nodeA = tree.AppendChild("A", nodeB);
         auto& nodeD = tree.AppendChild("D", nodeB);
         auto& nodeC = tree.AppendChild("C", nodeD);
         auto& nodeE = tree.AppendChild("E", nodeD);
         auto& nodeG = tree.AppendChild("G", nodeF);
         auto& nodeI = tree.AppendChild("I", nodeG);
         auto& nodeH = tree.AppendChild("H", nodeI);

         treeSize = tree.Size();

         // Reset the destruction count, so that we don't accidentally count any destructor calls
         // that took place during the construction of the tree.
         DESTRUCTION_COUNT = 0;

         auto* targetNode = tree.GetHead()->GetFirstChild()->GetLastChild();
         REQUIRE(targetNode != nullptr);
         REQUIRE(targetNode->GetData().m_data == "D");
         REQUIRE(targetNode->GetPreviousSibling() != nullptr);
         REQUIRE(targetNode->GetNextSibling() == nullptr);
         REQUIRE(targetNode->GetFirstChild() != nullptr);
         REQUIRE(targetNode->GetFirstChild() != targetNode->GetLastChild());

         const auto parentOfTarget = targetNode->GetParent();
         const auto parentsChildCount = parentOfTarget->GetChildCount();

         targetNode->DeleteFromTree();

         REQUIRE(DESTRUCTION_COUNT == 3);
         REQUIRE(parentOfTarget->GetChildCount() == parentsChildCount - 1);
         REQUIRE(tree.Size() == treeSize - DESTRUCTION_COUNT);

         const std::vector<std::string> expectedTraversal =
         { "A", "B", "H", "I", "G", "F" };

         const bool errorFree = VerifyTraversal(tree, expectedTraversal);
         REQUIRE(errorFree == true);
      }

      REQUIRE(DESTRUCTION_COUNT == treeSize);
   }

   SECTION("Deleting Multiple Nodes from Tree")
   {
      CONSTRUCTION_COUNT = 0;
      size_t treeSize = 0;

      {
         Tree<VerboseNode> tree{ "F" };
         auto& nodeF = *tree.GetHead();
         auto& nodeB = tree.AppendChild("B", nodeF);
         auto& nodeA = tree.AppendChild("Delete Me", nodeB);
         auto& nodeD = tree.AppendChild("D", nodeB);
         auto& nodeC = tree.AppendChild("Delete Me", nodeD);
         auto& nodeE = tree.AppendChild("Delete Me", nodeD);
         auto& nodeG = tree.AppendChild("G", nodeF);
         auto& nodeI = tree.AppendChild("I", nodeG);
         auto& nodeH = tree.AppendChild("Delete Me", nodeI);

         treeSize = tree.Size();

         // Reset the destruction count, so that we don't accidentally count any destructor calls
         // that took place during the construction of the tree.
         DESTRUCTION_COUNT = 0;

         std::vector<Tree<VerboseNode>::Node*> toBeDeleted;

         for (auto&& node : tree)
         {
            if (node.GetData().m_data == "Delete Me")
            {
               toBeDeleted.emplace_back(&node);
            }
         }

         const size_t numberOfNodesToDelete = toBeDeleted.size();

         for (auto* node : toBeDeleted)
         {
            node->DeleteFromTree();
         }

         const std::vector<std::string> expectedTraversal = { "D", "B", "I", "G", "F" };
         const bool errorFree = VerifyTraversal(tree, expectedTraversal);
         REQUIRE(errorFree == true);
         REQUIRE(DESTRUCTION_COUNT == numberOfNodesToDelete);
      }

      REQUIRE(DESTRUCTION_COUNT == treeSize);
   }
#endif

}
