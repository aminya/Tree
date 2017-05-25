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
         [&] (Tree<int>::const_reference node) noexcept
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
      auto& root = *tree.GetRoot();

      root.AppendChild("D")->AppendChild("E");
      root.GetFirstChild()->PrependChild("C");
      root.PrependChild("A");

      tree.OptimizeMemoryLayoutFor<PreOrderTraversal>();

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
   auto& root = *tree.GetRoot();

   root.AppendChild("B")->AppendChild("A");
   root.AppendChild("G")->AppendChild("I")->AppendChild("H");
   root.GetFirstChild()->AppendChild("D")->AppendChild("C");
   root.GetFirstChild()->GetLastChild()->AppendChild("E");

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

         auto* doomedNode = root.GetFirstChild()->GetNextSibling()->GetNextSibling();
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
         auto& root = *tree.GetRoot();

         root.AppendChild("B")->AppendChild("A");
         root.AppendChild("G")->AppendChild("I")->AppendChild("H");
         root.GetFirstChild()->AppendChild("D")->AppendChild("C");
         root.GetFirstChild()->GetLastChild()->AppendChild("E");

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
         auto& root = *tree.GetRoot();

         root.AppendChild("B")->AppendChild("A");
         root.AppendChild("G")->AppendChild("I")->AppendChild("H");
         root.GetFirstChild()->AppendChild("D")->AppendChild("C");
         root.GetFirstChild()->GetLastChild()->AppendChild("E");

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
         auto& root = *tree.GetRoot();

         root.AppendChild("B")->AppendChild("A");
         root.AppendChild("G")->AppendChild("I")->AppendChild("H");
         root.GetFirstChild()->AppendChild("D")->AppendChild("C");
         root.GetFirstChild()->GetLastChild()->AppendChild("E");

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
         auto& root = *tree.GetRoot();

         root.AppendChild("B")->AppendChild("A");
         root.AppendChild("G")->AppendChild("I")->AppendChild("H");
         root.GetFirstChild()->AppendChild("D")->AppendChild("C");
         root.GetFirstChild()->GetLastChild()->AppendChild("X");
         root.GetFirstChild()->GetLastChild()->AppendChild("E");

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
         auto& root = *tree.GetRoot();

         root.AppendChild("B")->AppendChild("A");
         root.AppendChild("G")->AppendChild("I")->AppendChild("H");
         root.GetFirstChild()->AppendChild("D")->AppendChild("C");
         root.GetFirstChild()->GetLastChild()->AppendChild("E");

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
         auto& root = *tree.GetRoot();

         root.AppendChild("B")->AppendChild("A");
         root.AppendChild("G")->AppendChild("Delete Me (I)")->AppendChild("H");
         root.GetFirstChild()->AppendChild("Delete Me (D)")->AppendChild("C");
         root.GetFirstChild()->GetLastChild()->AppendChild("Delete Me (E)");

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
         REQUIRE(root.CountAllDescendants() == expected.size() - rootNode);
      }

      REQUIRE(DESTRUCTION_COUNT == treeSize);
   }
}

TEST_CASE("Sorting")
{
   SECTION("Immediate Children")
   {
      Tree<std::string> tree{ "X" };
      auto& root = *tree.GetRoot();

      root.AppendChild("B");
      root.AppendChild("D");
      root.AppendChild("A");
      root.AppendChild("C");
      root.AppendChild("F");
      root.AppendChild("G");
      root.AppendChild("E");
      root.AppendChild("H");

      root.SortChildren([] (const auto& lhs, const auto& rhs) noexcept { return lhs < rhs; });

      const std::vector<std::string> expected = { "A", "B", "C", "D", "E", "F", "G", "H", "X" };

      std::vector<std::string> actual;
      std::transform(std::begin(tree), std::end(tree), std::back_inserter(actual),
         [] (const auto& node) noexcept { return node.GetData(); });

      VerifyTraversal(expected, actual);
   }

   SECTION("A Larger Tree")
   {
      Tree<int> tree{ 666 };
      auto& root = *tree.GetRoot();
      root.AppendChild(37);

      auto* firstChild = root.GetFirstChild();
      firstChild->AppendChild(6);
      firstChild->AppendChild(8);
      firstChild->AppendChild(2);
      firstChild->AppendChild(3);
      firstChild->AppendChild(1);
      firstChild->AppendChild(0);
      firstChild->AppendChild(-5);

      root.AppendChild(48);
      root.AppendChild(17);
      root.AppendChild(12);

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
