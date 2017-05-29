#include <algorithm>
#include <iostream>
#include <string>

#include "../Tree/VectorTree.hpp"

#include "DriveScanner.h"
#include "Stopwatch.hpp"

int main()
{
   std::cout.imbue(std::locale{ "" });
   std::cout << "Scanning Drive to Create a Large Tree...\n" << std::endl;

   DriveScanner scanner{ std::experimental::filesystem::path{ "C:\\" } };
   scanner.Start();

   std::uintmax_t treeSize{ 0 };

   { // Pre-Order Traversal:
      const auto traversalTime = Stopwatch<std::chrono::milliseconds>([&] () noexcept
      {
         treeSize = std::count_if(
            scanner.m_theTree->beginPreOrder(),
            scanner.m_theTree->endPreOrder(),
            [] (const auto&) noexcept
         {
            return true;
         });
      }).GetElapsedTime();

      std::cout
         << "\nPre-order-traversed " << treeSize << " nodes in "
         << traversalTime.count() << " milliseconds (or "
         << treeSize / traversalTime.count() << " nodes/ms)." << std::endl;
   }

   { // Post-Order Traversal:
      const auto traversalTime = Stopwatch<std::chrono::milliseconds>([&] () noexcept
      {
         treeSize = std::count_if(
            std::begin(*scanner.m_theTree),
            std::end(*scanner.m_theTree),
            [] (const auto&) noexcept
         {
            return true;
         });
      }).GetElapsedTime();

      std::cout
         << "\nPost-order-traversed " << treeSize << " nodes in "
         << traversalTime.count() << " milliseconds (or "
         << treeSize / traversalTime.count() << " nodes/ms)." << std::endl;
   }

#if 0
   Stopwatch<std::chrono::milliseconds>([&]()
   {
      scanner.m_theTree->OptimizeMemoryLayoutFor<PreOrderTraversal>();
   }, "Optimized Layout in ");

   { // Pre-Order Traversal:
      const auto traversalTime = Stopwatch<std::chrono::milliseconds>([&]() noexcept
      {
         treeSize = std::count_if(
            scanner.m_theTree->beginPreOrder(),
            scanner.m_theTree->endPreOrder(),
            [](const auto&) noexcept
         {
            return true;
         });
      }).GetElapsedTime();

      std::cout
         << "\nPre-order-traversed " << treeSize << " nodes in "
         << traversalTime.count() << " milliseconds (or "
         << treeSize / traversalTime.count() << " nodes/ms)." << std::endl;
   }

   { // Post-Order Traversal:
      const auto traversalTime = Stopwatch<std::chrono::milliseconds>([&]() noexcept
      {
         treeSize = std::count_if(
            std::begin(*scanner.m_theTree),
            std::end(*scanner.m_theTree),
            [](const auto&) noexcept
         {
            return true;
         });
      }).GetElapsedTime();

      std::cout
         << "\nPost-order-traversed " << treeSize << " nodes in "
         << traversalTime.count() << " milliseconds (or "
         << treeSize / traversalTime.count() << " nodes/ms)." << std::endl;
   }
#endif

   std::cout << std::endl;

   return 0;
}
