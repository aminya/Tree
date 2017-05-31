#include <algorithm>
#include <iostream>
#include <string>

#include "../Tree/FlatTree.hpp"

#include "DriveScanner.h"
#include "Stopwatch.hpp"

int main()
{
   std::cout.imbue(std::locale{ "" });
   std::cout << "Scanning Drive to Create a Large Tree...\n" << std::endl;

   DriveScanner scanner{ std::experimental::filesystem::path{ "C:\\" } };
   scanner.Start();

   std::cout << "\n";

   { // Pre-Order Traversal:
      std::uintmax_t treeSize{ 0 };
      std::uintmax_t totalBytes{ 0 };

      const auto traversalTime = Stopwatch<std::chrono::milliseconds>([&]() noexcept
      {
         std::for_each(
            scanner.m_theTree->beginPreOrder(),
            scanner.m_theTree->endPreOrder(),
            [&] (const auto& node) noexcept
         {
            treeSize += 1;
            totalBytes += node.GetData().size;
         });
      }).GetElapsedTime();

      std::cout << totalBytes << " bytes on disk!\n";

      std::cout
         << "Pre-order-traversed " << treeSize << " nodes in "
         << traversalTime.count() << " milliseconds (or "
         << treeSize / traversalTime.count() << " nodes/ms)." << std::endl;
   }

   { // Post-Order Traversal:
      std::uintmax_t treeSize{ 0 };
      std::uintmax_t totalBytes{ 0 };

      const auto traversalTime = Stopwatch<std::chrono::milliseconds>([&] () noexcept
      {
         std::for_each(
            std::begin(*scanner.m_theTree),
            std::end(*scanner.m_theTree),
            [&] (const auto& node) noexcept
         {
            treeSize += 1;
            totalBytes += node.GetData().size;
         });
      }).GetElapsedTime();

      std::cout << totalBytes << " bytes on disk!\n";

      std::cout
         << "Post-order-traversed " << treeSize << " nodes in "
         << traversalTime.count() << " milliseconds (or "
         << treeSize / traversalTime.count() << " nodes/ms)." << std::endl;
   }

   std::cout << "\n";

   Stopwatch<std::chrono::milliseconds>([&]()
   {
      scanner.m_theTree->OptimizeMemoryLayoutFor<PreOrderTraversal>();
   }, "Optimized Layout in ");

   std::cout << "\n";

   { // Pre-Order Traversal:
      std::uintmax_t treeSize{ 0 };
      std::uintmax_t totalBytes{ 0 };

      const auto traversalTime = Stopwatch<std::chrono::milliseconds>([&]() noexcept
      {
         std::for_each(
            scanner.m_theTree->beginPreOrder(),
            scanner.m_theTree->endPreOrder(),
            [&] (const auto& node) noexcept
         {
            treeSize += 1;
            totalBytes += node.GetData().size;
         });
      }).GetElapsedTime();

      std::cout << totalBytes << " bytes on disk!\n";

      std::cout
         << "Pre-order-traversed " << treeSize << " nodes in "
         << traversalTime.count() << " milliseconds (or "
         << treeSize / traversalTime.count() << " nodes/ms)." << std::endl;
   }

   { // Post-Order Traversal:
      std::uintmax_t treeSize{ 0 };
      std::uintmax_t totalBytes{ 0 };

      const auto traversalTime = Stopwatch<std::chrono::milliseconds>([&]() noexcept
      {
         std::for_each(
            std::begin(*scanner.m_theTree),
            std::end(*scanner.m_theTree),
            [&] (const auto& node) noexcept
         {
            treeSize += 1;
            totalBytes += node.GetData().size;
         });
      }).GetElapsedTime();

      std::cout << totalBytes << " bytes on disk!\n";

      std::cout
         << "Post-order-traversed " << treeSize << " nodes in "
         << traversalTime.count() << " milliseconds (or "
         << treeSize / traversalTime.count() << " nodes/ms)." << std::endl;
   }

   std::cout << std::endl;

   return 0;
}
