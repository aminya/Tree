#pragma once

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "Catch.hpp"

#include "../Tree/Tree.hpp"

SCENARIO("Adding nodes to the tree", "[treeConstruction]")
{
   GIVEN("a tree with only a head node")
   {
      Tree<std::string> tree{ "Head" };

      REQUIRE(tree.GetHead() != nullptr);
      REQUIRE(tree.GetHead()->GetData() == "Head");

      WHEN("a single child node is added to the head node")
      {
         std::string firstChildLabel{ "First Child" };
         tree.GetHead()->AppendChild(firstChildLabel);

         THEN("that child is reachable from the head node")
         {
            REQUIRE(tree.GetHead()->GetChildCount() == 1);
            REQUIRE(tree.GetHead()->GetFirstChild() != nullptr);
            REQUIRE(tree.GetHead()->GetLastChild() != nullptr);
            REQUIRE(tree.GetHead()->GetFirstChild() == tree.GetHead()->GetLastChild());
            REQUIRE(tree.GetHead()->GetFirstChild()->GetData() == firstChildLabel);
         }

         WHEN("a sibling is added to the head node's only child")
         {
            std::string secondChildLabel{ "Second Child" };
            tree.GetHead()->AppendChild(secondChildLabel);

            THEN("both the first child and second child are properly reachable")
            {
               REQUIRE(tree.GetHead()->GetChildCount() == 2);
               REQUIRE(tree.GetHead()->GetFirstChild() != nullptr);
               REQUIRE(tree.GetHead()->GetLastChild() != nullptr);
               REQUIRE(tree.GetHead()->GetFirstChild() != tree.GetHead()->GetLastChild());
               REQUIRE(tree.GetHead()->GetFirstChild()->GetData() == firstChildLabel);
               REQUIRE(tree.GetHead()->GetLastChild()->GetData() == secondChildLabel);
               REQUIRE(tree.GetHead()->GetFirstChild()->GetNextSibling() == tree.GetHead()->GetLastChild());
            }
         }
      }
   }
}
