#pragma once

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "Catch.hpp"

#include "../Tree/Tree.hpp"

#include <algorithm>
#include <vector>

namespace
{
   /**
   * @brief Allows for the comparison of vectors of unequal length.
   *
   * The `expected` vector has to be smaller than the `actual` vector that it is compared against.
   */
   template<typename DataType>
   auto SubsetEquals(
      const std::vector<DataType>& expected,
      const std::vector<DataType>& actual)
   {
      bool isCorrect{ false };

      const auto expectedSize = expected.size();
      const auto actualSize = actual.size();
      if (expectedSize < actualSize)
      {
         const auto delta = static_cast<int>(actualSize - expectedSize);

         auto actualEnd = std::end(actual);
         std::advance(actualEnd, -delta);

         isCorrect = std::equal(
            std::begin(actual), actualEnd,
            std::begin(expected), std::end(expected));
      }

      REQUIRE(isCorrect == true);
   }

   /**
   * @brief Compares two vectors, ensuring that both contain the same data.
   */
   template<typename DataType>
   auto VerifyTraversal(
      const std::vector<DataType>& expected,
      const std::vector<DataType>& actual,
      bool performSubsetComparision = false)
   {
      if (performSubsetComparision)
      {
         SubsetEquals(expected, actual);
         return;
      }

      REQUIRE(expected.size() == actual.size());

      const auto isCorrect = std::equal(
         std::begin(actual), std::end(actual),
         std::begin(expected), std::end(expected));

      REQUIRE(isCorrect == true);
   }
}

SCENARIO("Building a Basic Tree of Depth One")
{
   GIVEN("A Tree With a Root Node")
   {
      Tree<int> tree{ 10 };
      auto& root = *tree.GetRoot();

      THEN("The root node should be properly initialized")
      {
         REQUIRE(tree.Size() == 1);

         REQUIRE(root.GetData() == 10);

         REQUIRE(!root.GetParent());
         REQUIRE(!root.GetFirstChild());
         REQUIRE(!root.GetLastChild());
         REQUIRE(!root.GetNextSibling());
         REQUIRE(!root.GetPreviousSibling());
      }

      WHEN("A single node is appended to the root node")
      {
         root.AppendChild(30);

         THEN("The node should be properly integrated into the tree")
         {
            REQUIRE(tree.Size() == 2);

            REQUIRE(*root.GetFirstChild()->GetParent() == root);
            REQUIRE(*root.GetLastChild()->GetParent() == root);

            REQUIRE(root.GetFirstChild()->GetData() == 30);
            REQUIRE(root.GetLastChild()->GetData() == 30);
            REQUIRE(root.GetLastChild() == root.GetFirstChild());
         }

         AND_WHEN("A second node is appended to the root node")
         {
            root.AppendChild(40);

            THEN("That node should be properly integrated into the tree")
            {
               REQUIRE(tree.Size() == 3);

               REQUIRE(root.GetFirstChild()->GetData() == 30);
               REQUIRE(root.GetFirstChild()->GetNextSibling() == root.GetLastChild());

               REQUIRE(root.GetLastChild()->GetData() == 40);
               REQUIRE(root.GetLastChild()->GetPreviousSibling() == root.GetFirstChild());

               REQUIRE(*root.GetLastChild()->GetParent() == root);
               REQUIRE(root.GetFirstChild()->GetParent() == root.GetLastChild()->GetParent());

               REQUIRE(root.GetFirstChild()->GetNextSibling()->GetData() == 40);
               REQUIRE(root.GetLastChild()->GetPreviousSibling()->GetData() == 30);
            }

            AND_WHEN("A third node is prepended to the root node")
            {
               root.PrependChild(20);

               THEN("That node should be properly integrated into the tree")
               {
                  REQUIRE(tree.Size() == 4);

                  REQUIRE(*root.GetFirstChild()->GetParent() == root);

                  REQUIRE(root.GetFirstChild()->GetData() == 20);
                  REQUIRE(root.GetFirstChild()->GetNextSibling()->GetData() == 30);
                  REQUIRE(root.GetFirstChild()->GetNextSibling()->GetNextSibling()->GetData() == 40);

                  REQUIRE(root.GetLastChild()->GetData() == 40);
                  REQUIRE(root.GetLastChild()->GetPreviousSibling()->GetData() == 30);
                  REQUIRE(root.GetLastChild()->GetPreviousSibling()->GetPreviousSibling()->GetData() == 20);
               }
            }
         }
      }
   }
}

TEST_CASE("Node Comparison Operations")
{
   Tree<int> tree{ 10 };
   auto& root = *tree.GetRoot();

   SECTION("Equality")
   {
      REQUIRE(root == root);
   }
}

TEST_CASE("Node Alterations")
{
   Tree<std::string> tree{ "caps" };
   auto& root = *tree.GetRoot();

   SECTION("Altering Data")
   {
      auto& data = root.GetData();
      std::transform(std::begin(data), std::end(data), std::begin(data), ::toupper);

      REQUIRE(root.GetData() == "CAPS");
   }
}

TEST_CASE("Prepending and Appending TreeNodes")
{
   Tree<int> tree{ 10 };
   auto& root = *tree.GetRoot();

   const auto IsEachNodeValueLargerThanTheLast = [&] () noexcept
   {
      int lastValue = -1;

      return std::all_of(std::begin(tree), std::end(tree),
         [&](Tree<int>::const_reference node) noexcept
      {
         const auto& data = node.GetData();
         return data > lastValue;
      });
   };

   SECTION("Prepending Nodes")
   {
      REQUIRE(root.GetChildCount() == 0);
      REQUIRE(root.CountAllDescendants() == 0);

      root.AppendChild(1);
      root.AppendChild(2);
      root.AppendChild(3);
      root.AppendChild(4);
      root.AppendChild(5);
      root.AppendChild(6);
      root.AppendChild(7);
      root.AppendChild(8);
      root.AppendChild(9);

      const bool correctlyPrepended = IsEachNodeValueLargerThanTheLast();
      REQUIRE(correctlyPrepended);

      REQUIRE(root.GetChildCount() == 9);
      REQUIRE(root.CountAllDescendants() == 9);
   }

   SECTION("Appending Nodes")
   {
      REQUIRE(root.GetChildCount() == 0);
      REQUIRE(root.CountAllDescendants() == 0);

      root.PrependChild(9);
      root.PrependChild(8);
      root.PrependChild(7);
      root.PrependChild(6);
      root.PrependChild(5);
      root.PrependChild(4);
      root.PrependChild(3);
      root.PrependChild(2);
      root.PrependChild(1);

      const bool correctlyAppended = IsEachNodeValueLargerThanTheLast();
      REQUIRE(correctlyAppended);

      REQUIRE(root.GetChildCount() == 9);
      REQUIRE(root.CountAllDescendants() == 9);
   }
}

TEST_CASE("Node Counting")
{
   Tree<std::string> tree{ "F" };
   auto& root = *tree.GetRoot();

   root.AppendChild("B")->AppendChild("A");
   root.GetFirstChild()->AppendChild("D")->AppendChild("C");
   root.GetFirstChild()->GetLastChild()->AppendChild("E");
   root.AppendChild("G")->AppendChild("I")->AppendChild("H");

   SECTION("Tree Size")
   {
      REQUIRE(tree.Size() == 9);
   }

   SECTION("Node Depth")
   {
      REQUIRE(root.Depth() == 0);
      REQUIRE(root.GetFirstChild()->Depth() == 1);
      REQUIRE(root.GetFirstChild()->GetFirstChild()->Depth() == 2);
   }

   SECTION("Descendent Count")
   {
      REQUIRE(root.GetFirstChild()->CountAllDescendants() == 4);
   }
}

TEST_CASE("Forward Pre- and Post-Order Traversal of Simple Binary Tree")
{
   Tree<std::string> tree{ "F" };
   auto& root = *tree.GetRoot();

   root.AppendChild("B")->AppendChild("A");
   root.GetFirstChild()->AppendChild("D")->AppendChild("C");
   root.GetFirstChild()->GetLastChild()->AppendChild("E");
   root.AppendChild("G")->AppendChild("I")->AppendChild("H");

   SECTION("Pre-order Traversal")
   {
      const std::vector<std::string> expected = { "F", "B", "A", "D", "C", "E", "G", "I", "H" };

      std::vector<std::string> actual;
      std::transform(tree.beginPreOrder(), tree.endPreOrder(), std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }

   SECTION("Post-order Traversal")
   {
      const std::vector<std::string> expected = { "A", "C", "E", "D", "B", "H", "I", "G", "F" };

      std::vector<std::string> actual;
      std::transform(std::begin(tree), std::end(tree), std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }
}

TEST_CASE("Partial Tree Iteration")
{
   Tree<std::string> tree{ "F" };
   auto& root = *tree.GetRoot();

   root.AppendChild("B")->AppendChild("A");
   root.GetFirstChild()->AppendChild("D")->AppendChild("C");
   root.GetFirstChild()->GetLastChild()->AppendChild("E");
   root.AppendChild("G")->AppendChild("I")->AppendChild("H");

   SECTION("Pre-Order Iteration")
   {
      const std::vector<std::string> expected = { "B", "A", "D", "C", "E" };

      const auto begin = Tree<std::string>::PreOrderIterator{ root.GetFirstChild() };
      const auto end = Tree<std::string>::PreOrderIterator{ };

      std::vector<std::string> actual;
      std::transform(begin, end, std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }

   SECTION("Post-Order Iteration")
   {
      const std::vector<std::string> expected = { "A", "C", "E", "D", "B" };

      const auto begin = Tree<std::string>::PostOrderIterator{ root.GetFirstChild() };
      const auto end = Tree<std::string>::PostOrderIterator{ };

      std::vector<std::string> actual;
      std::transform(begin, end, std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }
}

TEST_CASE("Partial Tree Iteration Corner Cases")
{
   Tree<std::string> tree{ "F" };
   auto& root = *tree.GetRoot();

   root.AppendChild("B")->AppendChild("A");
   root.GetFirstChild()->AppendChild("D")->AppendChild("C");
   root.GetFirstChild()->GetLastChild()->AppendChild("E");
   root.AppendChild("G")->AppendChild("I")->AppendChild("H");

   SECTION("Pre-Order Iteration")
   {
      const std::vector<std::string> expected = { "D", "C", "E" };

      const auto* startingNode = root.GetFirstChild()->GetLastChild();

      const auto begin = Tree<std::string>::PreOrderIterator{ startingNode };
      const auto end = Tree<std::string>::PreOrderIterator{ };

      std::vector<std::string> actual;
      std::transform(begin, end, std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }

   SECTION("Post-Order Iteration")
   {
      const std::vector<std::string> expected = { "C", "E", "D" };

      const auto* startingNode = root.GetFirstChild()->GetLastChild();

      const auto begin = Tree<std::string>::PostOrderIterator{ startingNode };
      const auto end = Tree<std::string>::PostOrderIterator{ };

      std::vector<std::string> actual;
      std::transform(begin, end, std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }

   SECTION("Leaf Iteration")
   {
      const std::vector<std::string> expected = { "C", "E" };

      const auto* startingNode = root.GetFirstChild()->GetLastChild();

      const auto begin = Tree<std::string>::LeafIterator{ startingNode };
      const auto end = Tree<std::string>::LeafIterator{ };

      std::vector<std::string> actual;
      std::transform(begin, end, std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }
}

TEST_CASE("STL Typedef Compliance")
{
   Tree<std::string> tree{ "F" };
   auto& root = *tree.GetRoot();

   root.AppendChild("B")->AppendChild("A");
   root.GetFirstChild()->AppendChild("D")->AppendChild("C");
   root.GetFirstChild()->GetLastChild()->AppendChild("E");
   root.AppendChild("G")->AppendChild("I")->AppendChild("H");

   SECTION("Standard Algorithms and Parameter Passing by value_type")
   {
      const auto count = std::count_if(std::begin(tree), std::end(tree),
         [] (Tree<std::string>::value_type node)
      {
         return (node.GetData() == "A");
      });

      REQUIRE(count == 1);
   }

   SECTION("Standard Algorithms and Parameter Passing by reference")
   {
      const auto count = std::count_if(std::begin(tree), std::end(tree),
         [] (Tree<std::string>::reference node)
      {
         return (node.GetData() == "C");
      });

      REQUIRE(count == 1);
   }

   SECTION("Standard Algorithms and Parameter Passing by const_reference")
   {
      const auto count = std::count_if(std::begin(tree), std::end(tree),
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
   auto& root = *tree.GetRoot();

   root.AppendChild("B")->AppendChild("A");
   root.GetFirstChild()->AppendChild("D")->AppendChild("C");
   root.GetFirstChild()->GetLastChild()->AppendChild("E");
   root.AppendChild("G")->AppendChild("I")->AppendChild("H");

   SECTION("Forward Traversal")
   {
      const std::vector<std::string> expected = { "A", "C", "E", "H", };

      const auto begin = Tree<std::string>::LeafIterator{ &root };
      const auto end = Tree<std::string>::LeafIterator{ };

      std::vector<std::string> actual;
      std::transform(begin, end, std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }

   SECTION("Partial Tree Iteration")
   {
      const std::vector<std::string> expected = { "A", "C", "E" };

      const auto begin = Tree<std::string>::LeafIterator{ root.GetFirstChild() };
      const auto end = Tree<std::string>::LeafIterator{ };

      std::vector<std::string> actual;
      std::transform(begin, end, std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }

   SECTION("Tree<T>::beginLeaf and Tree<T>::endLeaf")
   {
      const std::vector<std::string> expected = { "A", "C", "E", "H", };

      std::vector<std::string> actual;
      std::transform(tree.beginLeaf(), tree.endLeaf(), std::back_inserter(actual),
         [] (Tree<std::string>::const_reference node) { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }
}

TEST_CASE("Sibling Iterator")
{
   Tree<std::string> tree{ "IDK" };
   auto& root = *tree.GetRoot();

   root.AppendChild("B");
   root.AppendChild("D");
   root.AppendChild("A");
   root.AppendChild("C");
   root.AppendChild("F");
   root.AppendChild("G");
   root.AppendChild("E");
   root.AppendChild("H");

   SECTION("Forward Traversal")
   {
      const std::vector<std::string> expected = { "B", "D", "A", "C", "F", "G", "E", "H", };

      const auto begin = decltype(tree)::SiblingIterator{ root.GetFirstChild() };
      const auto end = decltype(tree)::SiblingIterator{ };

      std::vector<std::string> actual;
      std::transform(begin, end, std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }
}

TEST_CASE("Simple Memory Layout Optimization")
{
   SECTION("Leaf Iteration")
   {
      Tree<std::string> tree{ "F" };
      auto& root = *tree.GetRoot();

      root.PrependChild("E");
      root.PrependChild("D");
      root.PrependChild("C");
      root.PrependChild("B");
      root.PrependChild("A");

      tree.OptimizeMemoryLayoutFor<decltype(tree)::LeafIterator>();

      const auto& actual = tree.GetDataAsVector();
      const std::vector<std::string> expected = { "A", "B", "C", "D", "E" };

      constexpr auto compareSubset{ true };
      VerifyTraversal(expected, actual, compareSubset);

      REQUIRE(tree.GetRoot()->GetData() == "F");
   }

   SECTION("Pre-Order Iteration")
   {
      Tree<std::string> tree{ "B" };
      auto& root = *tree.GetRoot();

      root.AppendChild("D")->AppendChild("E");
      root.GetFirstChild()->PrependChild("C");
      root.PrependChild("A");

      tree.OptimizeMemoryLayoutFor<decltype(tree)::PreOrderIterator>();
      
      const auto& actual = tree.GetDataAsVector();
      const std::vector<std::string> expected = { "B", "A", "D", "C", "E" };

      VerifyTraversal(expected, actual);

      REQUIRE(tree.GetRoot()->GetData() == "B");
   }

   SECTION("Post-Order Iteration")
   {
      Tree<std::string> tree{ "B" };
      auto& root = *tree.GetRoot();

      root.AppendChild("D")->AppendChild("E");
      root.GetFirstChild()->PrependChild("C");
      root.PrependChild("A");

      tree.OptimizeMemoryLayoutFor<decltype(tree)::PostOrderIterator>();
      
      const auto& actual = tree.GetDataAsVector();
      const std::vector<std::string> expected = { "A", "C", "E", "D", "B" };

      VerifyTraversal(expected, actual);

      REQUIRE(tree.GetRoot()->GetData() == "B");
   }
}

TEST_CASE("More Complex Memory Layout Optimization")
{
   Tree<std::string> tree{ "F" };
   auto& root = *tree.GetRoot();

   root.AppendChild("B")->AppendChild("A");
   root.AppendChild("G")->AppendChild("I")->AppendChild("H");
   root.GetFirstChild()->AppendChild("D")->AppendChild("C");
   root.GetFirstChild()->GetLastChild()->AppendChild("E");

   SECTION("Pre-Order Iteration")
   {
      using IteratorType = decltype(tree)::PreOrderIterator;

      tree.OptimizeMemoryLayoutFor<IteratorType>();

      const auto& actual = tree.GetDataAsVector();
      const std::vector<std::string> expected =  { "F", "B", "A", "D", "C", "E", "G", "I", "H" };

      VerifyTraversal(expected, actual);
      
      REQUIRE(tree.GetRoot()->GetData() == "F");
   }

   SECTION("Post-Order Iteration")
   {
      using IteratorType = decltype(tree)::PostOrderIterator;

      tree.OptimizeMemoryLayoutFor<IteratorType>();

      const auto& actual = tree.GetDataAsVector();
      const std::vector<std::string> expected = { "A", "C", "E", "D", "B", "H", "I", "G", "F" };

      VerifyTraversal(expected, actual);

      REQUIRE(tree.GetRoot()->GetData() == "F");
   }

   SECTION("Leaf Iteration")
   {
      using IteratorType = decltype(tree)::LeafIterator;

      tree.OptimizeMemoryLayoutFor<IteratorType>();
      const auto& actual = tree.GetDataAsVector();

      const std::vector<std::string> expected = { "A", "C", "E", "H" };

      constexpr auto compareSubset{ true };
      VerifyTraversal(expected, actual, compareSubset);

      REQUIRE(tree.GetRoot()->GetData() == "F");
   }

   SECTION("Cycling Through Layouts")
   {
      tree.OptimizeMemoryLayoutFor<decltype(tree)::LeafIterator>();
      tree.OptimizeMemoryLayoutFor<decltype(tree)::PostOrderIterator>();
      tree.OptimizeMemoryLayoutFor<decltype(tree)::LeafIterator>();
      tree.OptimizeMemoryLayoutFor<decltype(tree)::PreOrderIterator>();

      const auto& actual = tree.GetDataAsVector();
      const std::vector<std::string> expected = { "F", "B", "A", "D", "C", "E", "G", "I", "H" };

      VerifyTraversal(expected, actual);

      REQUIRE(tree.GetRoot()->GetData() == "F");
   }
}

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

TEST_CASE("Tree and Node Destruction")
{
   SECTION("Node Destruction Count")
   {
      CONSTRUCTION_COUNT = 0u;

      {
         Tree<VerboseNode> tree{ "F" };
         auto& root = *tree.GetRoot();

         root.AppendChild("B")->AppendChild("A");
         root.AppendChild("G")->AppendChild("I")->AppendChild("H");
         root.GetFirstChild()->AppendChild("D")->AppendChild("C");
         root.GetFirstChild()->GetLastChild()->AppendChild("E");

         DESTRUCTION_COUNT = 0u;

         REQUIRE(tree.Size() == CONSTRUCTION_COUNT);
      }

      REQUIRE(CONSTRUCTION_COUNT == DESTRUCTION_COUNT);
   }
}

#if 0

TEST_CASE("Selectively Delecting Nodes")
{
   CONSTRUCTION_COUNT = 0u;

   SECTION("Removing One of Many Siblings")
   {
      {
         Tree<VerboseNode> tree{ "0" };
         auto& root = *tree.GetRoot();

         root.AppendChild("1");
         root.AppendChild("2");
         root.AppendChild("3");
         root.AppendChild("4");
         root.AppendChild("5");
         root.AppendChild("6");
         root.AppendChild("7");
         root.AppendChild("8");
         root.AppendChild("9");

         DESTRUCTION_COUNT = 0u;

         auto* doomedNode = tree.GetHead()->GetFirstChild()->GetNextSibling()->GetNextSibling();
         REQUIRE(doomedNode->GetData().m_data == "3");

         //tree.Detach(doomedNode);
         doomedNode->Detach();

         const std::vector<std::string> expected = { "1", "2", "4", "5", "6", "7", "8", "9", "0" };

         std::vector<std::string> actual;
         std::transform(std::begin(tree), std::end(end), std::back_inserter(actual),
            [] (const auto& node) noexcept { return node.GetData(); });

         VerifyTraversal(expected, actual);

         REQUIRE(tree.Size() == CONSTRUCTION_COUNT);
      }

      REQUIRE(CONSTRUCTION_COUNT == DESTRUCTION_COUNT);
   }

   SECTION("Removing a Leaf Node Without Siblings")
   {
      std::size_t treeSize = 0u;

      {
         Tree<VerboseNode> tree{ "F" };
         auto& root = *tree.GetRoot();

         root.AppendChild("B")->AppendChild("A");
         root.AppendChild("G")->AppendChild("I")->AppendChild("H");
         root.GetFirstChild()->AppendChild("D")->AppendChild("C");
         root.GetFirstChild()->GetLastChild()->AppendChild("E");

         treeSize = tree.Size();

         // Reset the destruction count, so that we don't accidentally count any destructor calls
         // that took place during the construction of the tree.
         DESTRUCTION_COUNT = 0u;

         auto* doomedNode = tree.GetHead()->GetLastChild()->GetLastChild()->GetFirstChild();
         REQUIRE(doomedNode != nullptr);
         REQUIRE(doomedNode->GetData().m_data == "H");
         REQUIRE(doomedNode->GetPreviousSibling() == nullptr);
         REQUIRE(doomedNode->GetNextSibling() == nullptr);
         REQUIRE(doomedNode->GetFirstChild() == nullptr);
         REQUIRE(doomedNode->GetLastChild() == nullptr);

         const auto parentOfTarget = doomedNode->GetParent();
         const auto parentsChildCount = doomedNode->GetChildCount();

         //tree.Detach(doomedNode);
         doomedNode->Detach();

         // After destroying the local copy, the only remaining copy should be the one in the
         // underlying vector:
         const auto& underlyingVector = tree.GetNodesAsVector();
         REQUIRE(underlyingVector.size() > 0);
         REQUIRE(underlyingVector.back().use_count() == 1);

         const std::vector<std::string> expected = { "A", "C", "E", "D", "B", "I", "G", "F" };

         std::vector<std::string> actual;
         std::transform(std::begin(tree), std::end(end), std::back_inserter(actual),
            [] (const auto& node) noexcept { return node.GetData(); });

         VerifyTraversal(expected, actual);
      }

      REQUIRE(DESTRUCTION_COUNT == treeSize);
   }

   SECTION("Removing a Leaf Node with A Left Sibling")
   {
      CONSTRUCTION_COUNT = 0u;
      std::size_t treeSize = 0u;

      {
         Tree<VerboseNode> tree{ "F" };
         auto& root = *tree.GetRoot();

         root.AppendChild("B")->AppendChild("A");
         root.AppendChild("G")->AppendChild("I")->AppendChild("H");
         root.GetFirstChild()->AppendChild("D")->AppendChild("C");
         root.GetFirstChild()->GetLastChild()->AppendChild("E");

         treeSize = tree.Size();

         // Reset the destruction count, so that we don't accidentally count any destructor calls
         // that took place during the construction of the tree.
         DESTRUCTION_COUNT = 0;

         auto doomedNode = tree.GetHead()->GetFirstChild()->GetLastChild()->GetLastChild();
         REQUIRE(doomedNode != nullptr);
         REQUIRE(doomedNode->GetData().m_data == "E");
         REQUIRE(doomedNode->GetPreviousSibling() != nullptr);
         REQUIRE(doomedNode->GetNextSibling() == nullptr);
         REQUIRE(doomedNode->GetFirstChild() == nullptr);
         REQUIRE(doomedNode->GetLastChild() == nullptr);

         const auto parentOfTarget = doomedNode->GetParent();
         const auto parentsChildCount = parentOfTarget->GetChildCount();

         // Before removing the node from the tree, there should be 4 references to it:
         //    * Once in the underlying vector.
         //    * Once here as a local variable.
         //    * Once as the child-node of "D"
         //    * Once as the sibling-node of "C".
         REQUIRE(doomedNode.use_count() == 4);

         tree.Detach(doomedNode);

         // After removing the node from the tree, there should be 2 references to it:
         //    * Once in the underlying vector.
         //    * Once here as a local variable.
         REQUIRE(doomedNode.use_count() == 2);

         doomedNode.~shared_ptr();

         const auto& underlyingVector = tree.GetNodesAsVector();

         auto itr = std::find_if(std::begin(underlyingVector), std::end(underlyingVector),
            [](const auto& node) noexcept
         {
            return node->GetData().m_data == "E";
         });

         if (itr != std::end(underlyingVector))
         {
            // After removing the targeted node from the tree, it should have a reference count
            // of 1, since it should only exist in the underlying vector:
            REQUIRE(itr->use_count() == 1);
         }

         itr = std::find_if(std::begin(underlyingVector), std::end(underlyingVector),
            [](const auto& node) noexcept
         {
            return node->GetData().m_data == "C";
         });

         if (itr != std::end(underlyingVector))
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

   SECTION("Removing a Leaf Node with A Right Sibling")
   {
      CONSTRUCTION_COUNT = 0;
      size_t treeSize = 0;

      {
         Tree<VerboseNode> tree{ "F" };
         auto& root = *tree.GetRoot();

         root.AppendChild("B")->AppendChild("A");
         root.AppendChild("G")->AppendChild("I")->AppendChild("H");
         root.GetFirstChild()->AppendChild("D")->AppendChild("C");
         root.GetFirstChild()->GetLastChild()->AppendChild("E");

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
         auto& root = *tree.GetRoot();

         root.AppendChild("B")->AppendChild("A");
         root.AppendChild("G")->AppendChild("I")->AppendChild("H");
         root.GetFirstChild()->AppendChild("D")->AppendChild("C");
         root.GetFirstChild()->GetLastChild()->AppendChild("E");

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
         auto& root = *tree.GetRoot();

         root.AppendChild("B")->AppendChild("A");
         root.AppendChild("G")->AppendChild("I")->AppendChild("H");
         root.GetFirstChild()->AppendChild("D")->AppendChild("C");
         root.GetFirstChild()->GetLastChild()->AppendChild("E");

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
         auto& root = *tree.GetRoot();

         root.AppendChild("B")->AppendChild("A");
         root.AppendChild("G")->AppendChild("I")->AppendChild("H");
         root.GetFirstChild()->AppendChild("D")->AppendChild("C");
         root.GetFirstChild()->GetLastChild()->AppendChild("E");

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
         [](Tree<std::string>::const_reference lhs, Tree<std::string>::const_reference rhs) noexcept
      {
         return (lhs < rhs);
      };

      tree.SortChildren(head, comparator);

      const std::vector<std::string> expectedTraversal =
      { "A", "B", "C", "D", "E", "F", "G", "H", };

      int index = 0;

      bool traversalError = false;

      auto itr = Tree<std::string>::LeafIterator(*tree.GetHead());
      const auto end = Tree<std::string>::LeafIterator{};
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
         [&](auto& node) noexcept
      {
         tree.SortChildren(node,
            [](auto& lhs, auto& rhs) noexcept
         {
            return lhs < rhs;
         });
      });

      const auto sizeAfterSort = tree.Size();

      bool sortingError = false;
      int lastItem = -999;

      // Verify:
      std::for_each(std::begin(tree), std::end(tree),
         [&](Tree<int>::const_reference node) noexcept
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

#endif