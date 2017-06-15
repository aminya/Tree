#include <algorithm>
#include <iostream>
#include <string>

#include "../Tree/FlatTree.hpp"

#include "DriveScanner.h"
#include "Stopwatch.hpp"

#include <numeric>

namespace
{
   template<
      typename ChronoType,
      typename LambdaType
   >
   auto RunTrials(LambdaType&& lambda)
   {
      std::vector<ChronoType> elapsedTimes;
      elapsedTimes.reserve(1'000);

      for (auto index{ 0u }; index < elapsedTimes.capacity(); ++index)
      {
         const auto clock = Stopwatch<ChronoType>([&]() noexcept { lambda(); });
         elapsedTimes.emplace_back(clock.GetElapsedTime());
      }

      const auto sum = std::accumulate(std::begin(elapsedTimes), std::end(elapsedTimes), 0ull,
         [](auto total, auto current) noexcept { return total + current.count(); });

      return sum / elapsedTimes.size();
   }
}

int main()
{
   using ChronoType = std::chrono::microseconds;

   std::cout.imbue(std::locale{ "" });
   std::cout << "Scanning Drive to Create a Large Tree...\n" << std::endl;

   DriveScanner scanner{ std::experimental::filesystem::path{ "C:\\" } };
   scanner.Start();

   std::cout << "\n";

   const auto preOrderTraversal = [&]() noexcept
   {
      std::uintmax_t treeSize{ 0 };
      std::uintmax_t totalBytes{ 0 };

      std::for_each(
         scanner.m_theTree->beginPreOrder(),
         scanner.m_theTree->endPreOrder(),
         [&](const auto& node) noexcept
      {
         treeSize += 1;

         if (node.GetData().type == FileType::REGULAR)
         {
            totalBytes += node.GetData().size;
         }
      });
   };

   const auto postOrderTraversal = [&]() noexcept
   {
      std::uintmax_t treeSize{ 0 };
      std::uintmax_t totalBytes{ 0 };

      std::for_each(
         std::begin(*scanner.m_theTree),
         std::end(*scanner.m_theTree),
         [&](const auto& node) noexcept
      {
         treeSize += 1;

         if (node.GetData().type == FileType::REGULAR)
         {
            totalBytes += node.GetData().size;
         }
      });
   };

   const auto averagePreOrderTimeWithoutOptimization = RunTrials<ChronoType>(preOrderTraversal);

   std::cout
      << "Average Pre-Order Before Optimization: " << averagePreOrderTimeWithoutOptimization
      << " " << StopwatchInternals::TypeName<ChronoType>::value << "\n";

   const auto averagePostOrderTimeWithoutOptimization = RunTrials<ChronoType>(postOrderTraversal);

   std::cout
      << "Average Post-Order Before Optimization: " << averagePostOrderTimeWithoutOptimization
      << " " << StopwatchInternals::TypeName<ChronoType>::value << "\n";

   std::cout << "\n";

   Stopwatch<ChronoType>([&]() noexcept
   {
      scanner.m_theTree->OptimizeMemoryLayoutFor<PostOrderTraversal>();
   }, "Optimized Layout in ");

   std::cout << "\n";

   const auto averagePreOrderTimeAfterOptimization = RunTrials<ChronoType>(preOrderTraversal);

   std::cout
      << "Average Pre-Order After Optimization: " << averagePreOrderTimeAfterOptimization
      << " " << StopwatchInternals::TypeName<ChronoType>::value << "\n";

   const auto averagePostOrderTimeAfterOptimization = RunTrials<ChronoType>(postOrderTraversal);

   std::cout
      << "Average Post-Order After Optimization: " << averagePostOrderTimeAfterOptimization
      << " " << StopwatchInternals::TypeName<ChronoType>::value << "\n";

   std::cout << "\n";

#if 0
   { // Post-Order Traversal:
      std::uintmax_t treeSize{ 0 };
      std::uintmax_t totalBytes{ 0 };

      const auto traversalTime = Stopwatch<ChronoType>([&]() noexcept
      {
         const auto& underlyingVector = scanner.m_theTree->GetDataAsVector();

         std::for_each(
            std::begin(underlyingVector),
            std::end(underlyingVector),
            [&] (const auto& file) noexcept
         {
            treeSize += 1;

            if (file.type == FileType::REGULAR)
            {
               totalBytes += file.size;
            }
         });
      });

      if (traversalTime.GetElapsedTime().count() > 0)
      {
         std::cout
            << "Linear traversal " << treeSize << " nodes in "
            << traversalTime.GetElapsedTime().count() << " "
            << traversalTime.GetUnitsAsCharacterArray() << " (or "
            << treeSize / traversalTime.GetElapsedTime().count() << " nodes/unit of time)."
            << std::endl;
      }
      else
      {
         std::cout << "Too fast to register!" << std::endl;
      }
   }
#endif

   std::cout << std::flush;

   return 0;
}
