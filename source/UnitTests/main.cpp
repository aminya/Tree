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
   * @param[in] expected            The expected sequence.
   * @param[in] actual              The actual sequence.
   *
   * @note The `expected` vector has to be smaller than the `actual` vector that it is compared
   * against.
   */
   template<typename DataType>
   void SubsetEquals(
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
   *
   * @param[in] expected                  The expected sequence.
   * @param[in] actual                    The actual sequence.
   * @param[in] performSubsetComparision  Forces the consumer to explicitly acknowledge that he
   *                                      or she wants to allow subset comparison.
   */
   template<typename DataType>
   void VerifyTraversal(
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

      THEN("The root node should be properly initialized")
      {
         REQUIRE(tree.Size() == 1);

         REQUIRE(tree.GetRoot()->GetData() == 10);

         REQUIRE(!tree.GetRoot()->GetParent());
         REQUIRE(!tree.GetRoot()->GetFirstChild());
         REQUIRE(!tree.GetRoot()->GetLastChild());
         REQUIRE(!tree.GetRoot()->GetNextSibling());
         REQUIRE(!tree.GetRoot()->GetPreviousSibling());
      }

      WHEN("A single node is appended to the root node")
      {
         tree.GetRoot()->AppendChild(30);

         THEN("The node should be properly integrated into the tree")
         {
            REQUIRE(tree.Size() == 2);

            REQUIRE(tree.GetRoot()->GetFirstChild()->GetParent() == tree.GetRoot());
            REQUIRE(tree.GetRoot()->GetLastChild()->GetParent() == tree.GetRoot());

            REQUIRE(tree.GetRoot()->GetFirstChild()->GetData() == 30);
            REQUIRE(tree.GetRoot()->GetLastChild()->GetData() == 30);
            REQUIRE(tree.GetRoot()->GetLastChild() == tree.GetRoot()->GetFirstChild());
         }

         AND_WHEN("A second node is appended to the root node")
         {
            tree.GetRoot()->AppendChild(40);

            THEN("That node should be properly integrated into the tree")
            {
               REQUIRE(tree.Size() == 3);

               REQUIRE(tree.GetRoot()->GetFirstChild()->GetData() == 30);
               REQUIRE(tree.GetRoot()->GetFirstChild()->GetNextSibling() == tree.GetRoot()->GetLastChild());

               REQUIRE(tree.GetRoot()->GetLastChild()->GetData() == 40);
               REQUIRE(tree.GetRoot()->GetLastChild()->GetPreviousSibling() == tree.GetRoot()->GetFirstChild());

               REQUIRE(tree.GetRoot()->GetLastChild()->GetParent() == tree.GetRoot());
               REQUIRE(tree.GetRoot()->GetFirstChild()->GetParent() == tree.GetRoot()->GetLastChild()->GetParent());

               REQUIRE(tree.GetRoot()->GetFirstChild()->GetNextSibling()->GetData() == 40);
               REQUIRE(tree.GetRoot()->GetLastChild()->GetPreviousSibling()->GetData() == 30);
            }

            AND_WHEN("A third node is prepended to the root node")
            {
               tree.GetRoot()->PrependChild(20);

               THEN("That node should be properly integrated into the tree")
               {
                  REQUIRE(tree.Size() == 4);

                  REQUIRE(tree.GetRoot()->GetFirstChild()->GetParent() == tree.GetRoot());

                  REQUIRE(tree.GetRoot()->GetFirstChild()->GetData() == 20);
                  REQUIRE(tree.GetRoot()->GetFirstChild()->GetNextSibling()->GetData() == 30);
                  REQUIRE(tree.GetRoot()->GetFirstChild()->GetNextSibling()->GetNextSibling()->GetData() == 40);

                  REQUIRE(tree.GetRoot()->GetLastChild()->GetData() == 40);
                  REQUIRE(tree.GetRoot()->GetLastChild()->GetPreviousSibling()->GetData() == 30);
                  REQUIRE(tree.GetRoot()->GetLastChild()->GetPreviousSibling()->GetPreviousSibling()->GetData() == 20);
               }
            }
         }
      }
   }
}

TEST_CASE("Node Comparison Operations")
{
   Tree<int> tree{ 10 };

   SECTION("Equality")
   {
      REQUIRE(tree.GetRoot() == tree.GetRoot());
   }
}

TEST_CASE("Node Alterations")
{
   Tree<std::string> tree{ "caps" };

   SECTION("Altering Data")
   {
      auto& data = tree.GetRoot()->GetData();
      std::transform(std::begin(data), std::end(data), std::begin(data), ::toupper);

      REQUIRE(tree.GetRoot()->GetData() == "CAPS");
   }
}

TEST_CASE("Prepending and Appending Nodes")
{
   Tree<int> tree{ 10 };

   const auto IsEachNodeValueLargerThanTheLast = [&] () noexcept
   {
      int lastValue = -1;

      return std::all_of(std::begin(tree), std::end(tree),
         [&] (Tree<int>::const_reference node) noexcept
      {
         const auto& data = node.GetData();
         return data > lastValue;
      });
   };

   SECTION("Prepending Nodes")
   {
      REQUIRE(tree.GetRoot()->GetChildCount() == 0);
      REQUIRE(tree.GetRoot()->CountAllDescendants() == 0);

      tree.GetRoot()->AppendChild(1);
      tree.GetRoot()->AppendChild(2);
      tree.GetRoot()->AppendChild(3);
      tree.GetRoot()->AppendChild(4);
      tree.GetRoot()->AppendChild(5);
      tree.GetRoot()->AppendChild(6);
      tree.GetRoot()->AppendChild(7);
      tree.GetRoot()->AppendChild(8);
      tree.GetRoot()->AppendChild(9);

      const bool correctlyPrepended = IsEachNodeValueLargerThanTheLast();
      REQUIRE(correctlyPrepended);

      REQUIRE(tree.GetRoot()->GetChildCount() == 9);
      REQUIRE(tree.GetRoot()->CountAllDescendants() == 9);
   }

   SECTION("Appending Nodes")
   {
      REQUIRE(tree.GetRoot()->GetChildCount() == 0);
      REQUIRE(tree.GetRoot()->CountAllDescendants() == 0);

      tree.GetRoot()->PrependChild(9);
      tree.GetRoot()->PrependChild(8);
      tree.GetRoot()->PrependChild(7);
      tree.GetRoot()->PrependChild(6);
      tree.GetRoot()->PrependChild(5);
      tree.GetRoot()->PrependChild(4);
      tree.GetRoot()->PrependChild(3);
      tree.GetRoot()->PrependChild(2);
      tree.GetRoot()->PrependChild(1);

      const bool correctlyAppended = IsEachNodeValueLargerThanTheLast();
      REQUIRE(correctlyAppended);

      REQUIRE(tree.GetRoot()->GetChildCount() == 9);
      REQUIRE(tree.GetRoot()->CountAllDescendants() == 9);
   }
}

TEST_CASE("Node Counting")
{
   Tree<std::string> tree{ "F" };

   tree.GetRoot()->AppendChild("B")->AppendChild("A");
   tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
   tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");
   tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");

   SECTION("Tree Size")
   {
      REQUIRE(tree.Size() == 9);
   }

   SECTION("Node Depth")
   {
      REQUIRE(tree.GetRoot()->Depth() == 0);
      REQUIRE(tree.GetRoot()->GetFirstChild()->Depth() == 1);
      REQUIRE(tree.GetRoot()->GetFirstChild()->GetFirstChild()->Depth() == 2);
   }

   SECTION("Descendent Count")
   {
      REQUIRE(tree.GetRoot()->GetFirstChild()->CountAllDescendants() == 4);
   }
}

TEST_CASE("Forward Pre- and Post-Order Traversal of Simple Binary Tree")
{
   Tree<std::string> tree{ "F" };

   tree.GetRoot()->AppendChild("B")->AppendChild("A");
   tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
   tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");
   tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");

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

   tree.GetRoot()->AppendChild("B")->AppendChild("A");
   tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
   tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");
   tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");

   SECTION("Pre-Order Iteration")
   {
      const std::vector<std::string> expected = { "B", "A", "D", "C", "E" };

      const auto begin = Tree<std::string>::PreOrderIterator{ tree.GetRoot()->GetFirstChild() };
      const auto end = Tree<std::string>::PreOrderIterator{ };

      std::vector<std::string> actual;
      std::transform(begin, end, std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }

   SECTION("Post-Order Iteration")
   {
      const std::vector<std::string> expected = { "A", "C", "E", "D", "B" };

      const auto begin = Tree<std::string>::PostOrderIterator{ tree.GetRoot()->GetFirstChild() };
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

   tree.GetRoot()->AppendChild("B")->AppendChild("A");
   tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
   tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");
   tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");

   SECTION("Pre-Order Iteration")
   {
      const std::vector<std::string> expected = { "D", "C", "E" };

      const auto* startingNode = tree.GetRoot()->GetFirstChild()->GetLastChild();

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

      const auto* startingNode = tree.GetRoot()->GetFirstChild()->GetLastChild();

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

      const auto* startingNode = tree.GetRoot()->GetFirstChild()->GetLastChild();

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

   tree.GetRoot()->AppendChild("B")->AppendChild("A");
   tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
   tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");
   tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");

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

   tree.GetRoot()->AppendChild("B")->AppendChild("A");
   tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
   tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");
   tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");

   SECTION("Forward Traversal")
   {
      const std::vector<std::string> expected = { "A", "C", "E", "H", };

      const auto begin = Tree<std::string>::LeafIterator{ tree.GetRoot() };
      const auto end = Tree<std::string>::LeafIterator{ };

      std::vector<std::string> actual;
      std::transform(begin, end, std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }

   SECTION("Partial Tree Iteration")
   {
      const std::vector<std::string> expected = { "A", "C", "E" };

      const auto begin = Tree<std::string>::LeafIterator{ tree.GetRoot()->GetFirstChild() };
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

   tree.GetRoot()->AppendChild("B");
   tree.GetRoot()->AppendChild("D");
   tree.GetRoot()->AppendChild("A");
   tree.GetRoot()->AppendChild("C");
   tree.GetRoot()->AppendChild("F");
   tree.GetRoot()->AppendChild("G");
   tree.GetRoot()->AppendChild("E");
   tree.GetRoot()->AppendChild("H");

   SECTION("Forward Traversal")
   {
      const std::vector<std::string> expected = { "B", "D", "A", "C", "F", "G", "E", "H", };

      const auto begin = decltype(tree)::SiblingIterator{ tree.GetRoot()->GetFirstChild() };
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

      tree.GetRoot()->PrependChild("E");
      tree.GetRoot()->PrependChild("D");
      tree.GetRoot()->PrependChild("C");
      tree.GetRoot()->PrependChild("B");
      tree.GetRoot()->PrependChild("A");

      tree.OptimizeMemoryLayoutFor<LeafTraversal>();

      const auto& actual = tree.GetDataAsVector();
      const std::vector<std::string> expected = { "A", "B", "C", "D", "E" };

      constexpr auto compareSubset{ true };
      VerifyTraversal(expected, actual, compareSubset);

      REQUIRE(tree.GetRoot()->GetData() == "F");
   }

   SECTION("Pre-Order Iteration")
   {
      Tree<std::string> tree{ "B" };

      tree.GetRoot()->AppendChild("D")->AppendChild("E");
      tree.GetRoot()->GetFirstChild()->PrependChild("C");
      tree.GetRoot()->PrependChild("A");

      tree.OptimizeMemoryLayoutFor<PreOrderTraversal>();

      const auto& actual = tree.GetDataAsVector();
      const std::vector<std::string> expected = { "B", "A", "D", "C", "E" };

      VerifyTraversal(expected, actual);

      REQUIRE(tree.GetRoot()->GetData() == "B");
   }

   SECTION("Post-Order Iteration")
   {
      Tree<std::string> tree{ "B" };

      tree.GetRoot()->AppendChild("D")->AppendChild("E");
      tree.GetRoot()->GetFirstChild()->PrependChild("C");
      tree.GetRoot()->PrependChild("A");

      tree.OptimizeMemoryLayoutFor<PostOrderTraversal>();

      const auto& actual = tree.GetDataAsVector();
      const std::vector<std::string> expected = { "A", "C", "E", "D", "B" };

      VerifyTraversal(expected, actual);

      REQUIRE(tree.GetRoot()->GetData() == "B");
   }
}

TEST_CASE("More Complex Memory Layout Optimization")
{
   Tree<std::string> tree{ "F" };

   tree.GetRoot()->AppendChild("B")->AppendChild("A");
   tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");
   tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
   tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");

   SECTION("Pre-Order Iteration")
   {
      tree.OptimizeMemoryLayoutFor<PreOrderTraversal>();

      const auto& actual = tree.GetDataAsVector();
      const std::vector<std::string> expected =  { "F", "B", "A", "D", "C", "E", "G", "I", "H" };

      VerifyTraversal(expected, actual);

      REQUIRE(tree.GetRoot()->GetData() == "F");
   }

   SECTION("Post-Order Iteration")
   {
      tree.OptimizeMemoryLayoutFor<PostOrderTraversal>();

      const auto& actual = tree.GetDataAsVector();
      const std::vector<std::string> expected = { "A", "C", "E", "D", "B", "H", "I", "G", "F" };

      VerifyTraversal(expected, actual);

      REQUIRE(tree.GetRoot()->GetData() == "F");
   }

   SECTION("Leaf Iteration")
   {
      tree.OptimizeMemoryLayoutFor<LeafTraversal>();
      const auto& actual = tree.GetDataAsVector();

      const std::vector<std::string> expected = { "A", "C", "E", "H" };

      constexpr auto compareSubset{ true };
      VerifyTraversal(expected, actual, compareSubset);

      REQUIRE(tree.GetRoot()->GetData() == "F");
   }

   SECTION("Cycling Through Various Layouts")
   {
      tree.OptimizeMemoryLayoutFor<LeafTraversal>();
      tree.OptimizeMemoryLayoutFor<PostOrderTraversal>();
      tree.OptimizeMemoryLayoutFor<LeafTraversal>();
      tree.OptimizeMemoryLayoutFor<PreOrderTraversal>();
      tree.OptimizeMemoryLayoutFor<PostOrderTraversal>();
      tree.OptimizeMemoryLayoutFor<PreOrderTraversal>();

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
      m_data{ data }
   {
      ++CONSTRUCTION_COUNT;
   }

   ~VerboseNode()
   {
      ++DESTRUCTION_COUNT;
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

         tree.GetRoot()->AppendChild("B")->AppendChild("A");
         tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");
         tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
         tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");

         DESTRUCTION_COUNT = 0u;

         REQUIRE(tree.Size() == CONSTRUCTION_COUNT);
      }

      REQUIRE(CONSTRUCTION_COUNT == DESTRUCTION_COUNT);
   }
}

TEST_CASE("Selectively Delecting Nodes")
{
   CONSTRUCTION_COUNT = 0u;

   SECTION("Removing One of Many Siblings")
   {
      {
         Tree<VerboseNode> tree{ "0" };

         tree.GetRoot()->AppendChild("1");
         tree.GetRoot()->AppendChild("2");
         tree.GetRoot()->AppendChild("3");
         tree.GetRoot()->AppendChild("4");
         tree.GetRoot()->AppendChild("5");
         tree.GetRoot()->AppendChild("6");
         tree.GetRoot()->AppendChild("7");
         tree.GetRoot()->AppendChild("8");
         tree.GetRoot()->AppendChild("9");

         DESTRUCTION_COUNT = 0u;

         auto* doomedNode = tree.GetRoot()->GetFirstChild()->GetNextSibling()->GetNextSibling();
         REQUIRE(doomedNode->GetData().m_data == "3");

         doomedNode->Detach();

         const std::vector<std::string> expected = { "1", "2", "4", "5", "6", "7", "8", "9", "0" };

         std::vector<std::string> actual;
         std::transform(std::begin(tree), std::end(tree), std::back_inserter(actual),
            [] (const auto& node) noexcept { return node.GetData().m_data; });

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

         tree.GetRoot()->AppendChild("B")->AppendChild("A");
         tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");
         tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
         tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");

         treeSize = tree.Size();

         DESTRUCTION_COUNT = 0u;

         auto* doomedNode = tree.GetRoot()->GetLastChild()->GetLastChild()->GetFirstChild();
         REQUIRE(doomedNode != nullptr);
         REQUIRE(doomedNode->GetData().m_data == "H");
         REQUIRE(doomedNode->GetPreviousSibling() == nullptr);
         REQUIRE(doomedNode->GetNextSibling() == nullptr);
         REQUIRE(doomedNode->GetFirstChild() == nullptr);
         REQUIRE(doomedNode->GetLastChild() == nullptr);

         const auto numberOfDetachedNodes = doomedNode->Detach();
         REQUIRE(numberOfDetachedNodes == 1);

         const std::vector<std::string> expected = { "A", "C", "E", "D", "B", "I", "G", "F" };

         std::vector<std::string> actual;
         std::transform(std::begin(tree), std::end(tree), std::back_inserter(actual),
            [] (const auto& node) noexcept { return node.GetData().m_data; });

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

         tree.GetRoot()->AppendChild("B")->AppendChild("A");
         tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");
         tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
         tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");

         treeSize = tree.Size();

         DESTRUCTION_COUNT = 0;

         auto doomedNode = tree.GetRoot()->GetFirstChild()->GetLastChild()->GetLastChild();
         REQUIRE(doomedNode != nullptr);
         REQUIRE(doomedNode->GetData().m_data == "E");
         REQUIRE(doomedNode->GetPreviousSibling() != nullptr);
         REQUIRE(doomedNode->GetNextSibling() == nullptr);
         REQUIRE(doomedNode->GetFirstChild() == nullptr);
         REQUIRE(doomedNode->GetLastChild() == nullptr);

         const auto numberOfDetachedNodes = doomedNode->Detach();
         REQUIRE(numberOfDetachedNodes == 1);

         const std::vector<std::string> expected = { "A", "C", "D", "B", "H", "I", "G", "F" };

         std::vector<std::string> actual;
         std::transform(std::begin(tree), std::end(tree), std::back_inserter(actual),
            [] (const auto& node) noexcept { return node.GetData().m_data; });

         VerifyTraversal(expected, actual);
      }

      REQUIRE(DESTRUCTION_COUNT == treeSize);
   }

   SECTION("Removing a Leaf Node with A Right Sibling")
   {
      CONSTRUCTION_COUNT = 0;
      size_t treeSize = 0;

      {
         Tree<VerboseNode> tree{ "F" };

         tree.GetRoot()->AppendChild("B")->AppendChild("A");
         tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");
         tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
         tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");

         treeSize = tree.Size();

         DESTRUCTION_COUNT = 0;

         auto* doomedNode = tree.GetRoot()->GetFirstChild()->GetLastChild()->GetFirstChild();
         REQUIRE(doomedNode != nullptr);
         REQUIRE(doomedNode->GetData().m_data == "C");
         REQUIRE(doomedNode->GetPreviousSibling() == nullptr);
         REQUIRE(doomedNode->GetNextSibling() != nullptr);
         REQUIRE(doomedNode->GetFirstChild() == nullptr);
         REQUIRE(doomedNode->GetLastChild() == nullptr);

         const auto numberOfDetachedNodes = doomedNode->Detach();
         REQUIRE(numberOfDetachedNodes == 1);

         const std::vector<std::string> expected = { "A", "E", "D", "B", "H", "I", "G", "F" };

         std::vector<std::string> actual;
         std::transform(std::begin(tree), std::end(tree), std::back_inserter(actual),
            [] (const auto& node) noexcept { return node.GetData().m_data; });

         VerifyTraversal(expected, actual);
      }

      REQUIRE(DESTRUCTION_COUNT == treeSize);
   }

   SECTION("Removing a Leaf Node with Both Left and Right Siblings")
   {
      CONSTRUCTION_COUNT = 0;
      size_t treeSize = 0;

      {
         Tree<VerboseNode> tree{ "F" };

         tree.GetRoot()->AppendChild("B")->AppendChild("A");
         tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");
         tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
         tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("X");
         tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");

         treeSize = tree.Size();

         DESTRUCTION_COUNT = 0;

         auto* doomedNode =
            tree.GetRoot()->GetFirstChild()->GetLastChild()->GetFirstChild()->GetNextSibling();

         REQUIRE(doomedNode != nullptr);
         REQUIRE(doomedNode->GetData().m_data == "X");
         REQUIRE(doomedNode->GetPreviousSibling() != nullptr);
         REQUIRE(doomedNode->GetNextSibling() != nullptr);
         REQUIRE(doomedNode->GetFirstChild() == nullptr);
         REQUIRE(doomedNode->GetLastChild() == nullptr);

         const auto parentOfTarget = doomedNode->GetParent();
         const auto parentsChildCount = parentOfTarget->GetChildCount();

         const auto numberOfDetachedNodes = doomedNode->Detach();
         REQUIRE(numberOfDetachedNodes == 1);

         const std::vector<std::string> expected = { "A", "C", "E", "D", "B", "H", "I", "G", "F" };

         std::vector<std::string> actual;
         std::transform(std::begin(tree), std::end(tree), std::back_inserter(actual),
            [] (const auto& node) noexcept { return node.GetData().m_data; });

         VerifyTraversal(expected, actual);
      }

      REQUIRE(DESTRUCTION_COUNT == treeSize);
   }

   SECTION("Removing a Node With a Left Sibling and Two Children")
   {
      CONSTRUCTION_COUNT = 0;
      size_t treeSize = 0;

      {
         Tree<VerboseNode> tree{ "F" };

         tree.GetRoot()->AppendChild("B")->AppendChild("A");
         tree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");
         tree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
         tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");

         treeSize = tree.Size();

         DESTRUCTION_COUNT = 0;

         auto* doomedNode = tree.GetRoot()->GetFirstChild()->GetLastChild();
         REQUIRE(doomedNode != nullptr);
         REQUIRE(doomedNode->GetData().m_data == "D");
         REQUIRE(doomedNode->GetPreviousSibling() != nullptr);
         REQUIRE(doomedNode->GetNextSibling() == nullptr);
         REQUIRE(doomedNode->GetFirstChild() != nullptr);
         REQUIRE(doomedNode->GetFirstChild() != doomedNode->GetLastChild());

         const auto numberOfDetachedNodes = doomedNode->Detach();
         REQUIRE(numberOfDetachedNodes == 3);

         const std::vector<std::string> expected = { "A", "B", "H", "I", "G", "F" };

         std::vector<std::string> actual;
         std::transform(std::begin(tree), std::end(tree), std::back_inserter(actual),
            [] (const auto& node) noexcept { return node.GetData().m_data; });

         VerifyTraversal(expected, actual);
      }

      REQUIRE(DESTRUCTION_COUNT == treeSize);
   }

   SECTION("Deleting Multiple Nodes from Tree")
   {
      CONSTRUCTION_COUNT = 0;
      size_t treeSize = 0;

      {
         Tree<VerboseNode> tree{ "F" };

         tree.GetRoot()->AppendChild("B")->AppendChild("A");
         tree.GetRoot()->AppendChild("G")->AppendChild("Delete Me (I)")->AppendChild("H");
         tree.GetRoot()->GetFirstChild()->AppendChild("Delete Me (D)")->AppendChild("C");
         tree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("Delete Me (E)");

         treeSize = tree.Size();

         DESTRUCTION_COUNT = 0;

         const auto numberOfDetachedNodes =
            Tree<VerboseNode>::DetachNodeIf(std::begin(tree), std::end(tree),
            [] (const auto& node)
         {
            return node.GetData().m_data.find("Delete Me") != std::string::npos;
         });

         REQUIRE(numberOfDetachedNodes == 5);

         const std::vector<std::string> expected = { "A", "B", "G", "F" };

         std::vector<std::string> actual;
         std::transform(std::begin(tree), std::end(tree), std::back_inserter(actual),
            [] (const auto& node) noexcept { return node.GetData().m_data; });

         VerifyTraversal(expected, actual);

         constexpr auto rootNode{ 1 };
         REQUIRE(tree.GetRoot()->CountAllDescendants() == expected.size() - rootNode);
      }

      REQUIRE(DESTRUCTION_COUNT == treeSize);
   }
}

TEST_CASE("Sorting")
{
   SECTION("Immediate Children")
   {
      Tree<std::string> tree{ "X" };

      tree.GetRoot()->AppendChild("B");
      tree.GetRoot()->AppendChild("D");
      tree.GetRoot()->AppendChild("A");
      tree.GetRoot()->AppendChild("C");
      tree.GetRoot()->AppendChild("F");
      tree.GetRoot()->AppendChild("G");
      tree.GetRoot()->AppendChild("E");
      tree.GetRoot()->AppendChild("H");

      tree.GetRoot()->SortChildren([] (const auto& lhs, const auto& rhs) noexcept { return lhs < rhs; });

      const std::vector<std::string> expected = { "A", "B", "C", "D", "E", "F", "G", "H", "X" };

      std::vector<std::string> actual;
      std::transform(std::begin(tree), std::end(tree), std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }

   SECTION("A Larger Tree")
   {
      Tree<int> tree{ 666 };
      tree.GetRoot()->AppendChild(37);

      tree.GetRoot()->GetFirstChild()->AppendChild(6);
      tree.GetRoot()->GetFirstChild()->AppendChild(8);
      tree.GetRoot()->GetFirstChild()->AppendChild(2);
      tree.GetRoot()->GetFirstChild()->AppendChild(3);
      tree.GetRoot()->GetFirstChild()->AppendChild(1);
      tree.GetRoot()->GetFirstChild()->AppendChild(0);
      tree.GetRoot()->GetFirstChild()->AppendChild(-5);

      tree.GetRoot()->AppendChild(48);
      tree.GetRoot()->AppendChild(17);
      tree.GetRoot()->AppendChild(12);

      const auto sizeBeforeSort = tree.Size();

      std::for_each(std::begin(tree), std::end(tree),
         [] (auto& node) noexcept
      {
         node.SortChildren([] (auto& lhs, auto& rhs) noexcept { return lhs < rhs; });
      });

      const auto sizeAfterSort = tree.Size();

      REQUIRE(sizeBeforeSort == sizeAfterSort);

      const std::vector<int> expected = { 12, 17, -5, 0, 1, 2, 3, 6, 8, 37, 48, 666 };

      std::vector<int> actual;
      std::transform(std::begin(tree), std::end(tree), std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }
}

TEST_CASE("Copying Nodes from One Tree to Another")
{
   Tree<std::string> numberTree{ "6" };

   numberTree.GetRoot()->AppendChild("2")->AppendChild("1");
   numberTree.GetRoot()->GetFirstChild()->AppendChild("4")->AppendChild("3");
   numberTree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("5");
   numberTree.GetRoot()->AppendChild("7")->AppendChild("8")->AppendChild("9");

   Tree<std::string> letterTree{ "F" };

   letterTree.GetRoot()->AppendChild("B")->AppendChild("A");
   letterTree.GetRoot()->GetFirstChild()->AppendChild("D")->AppendChild("C");
   letterTree.GetRoot()->GetFirstChild()->GetLastChild()->AppendChild("E");
   letterTree.GetRoot()->AppendChild("G")->AppendChild("I")->AppendChild("H");

   SECTION("Appending Two Trees to a Third")
   {
      Tree<std::string> masterTree{ "master" };

      masterTree.GetRoot()->AppendChild(*letterTree.GetRoot());
      masterTree.GetRoot()->AppendChild(*numberTree.GetRoot());

      const std::vector<std::string> expected
      {
         "A", "C", "E", "D", "B", "H", "I", "G", "F",
         "1", "3", "5", "4", "2", "9", "8", "7", "6",
         "master"
      };

      std::vector<std::string> actual;
      std::transform(std::begin(masterTree), std::end(masterTree), std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }

   SECTION("Prepending Two Trees to a Third")
   {
      Tree<std::string> masterTree{ "master" };

      masterTree.GetRoot()->PrependChild(*letterTree.GetRoot());
      masterTree.GetRoot()->PrependChild(*numberTree.GetRoot());

      const std::vector<std::string> expected
      {
         "1", "3", "5", "4", "2", "9", "8", "7", "6",
         "A", "C", "E", "D", "B", "H", "I", "G", "F",
         "master"
      };

      std::vector<std::string> actual;
      std::transform(std::begin(masterTree), std::end(masterTree), std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }
}
