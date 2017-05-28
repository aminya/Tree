#pragma once

#include <experimental/filesystem>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

#include "../Tree/Tree.hpp"

#include "FileInfo.hpp"

/**
* @brief The NodeAndPath struct
*/
struct TreeAndPath
{
   std::unique_ptr<Tree<FileInfo>> tree;
   std::experimental::filesystem::path path;

   TreeAndPath(
      decltype(tree) tree,
      decltype(path) path)
      :
      tree{ std::move(tree) },
      path{ std::move(path) }
   {
   }

   TreeAndPath() = default;
};

template<typename Type>
class ThreadSafeQueue;

/**
* @brief The DriveScanner class
*/
class DriveScanner
{
public:

   static constexpr std::uintmax_t SIZE_UNDEFINED{ 0 };

   explicit DriveScanner(const std::experimental::filesystem::path& path);

   /**
   * @brief Kicks off the drive scanning process.
   */
   void Start();

   std::unique_ptr<Tree<FileInfo>> m_theTree{ nullptr };

private:

   void ProcessQueue(
      ThreadSafeQueue<TreeAndPath>& taskQueue,
      ThreadSafeQueue<TreeAndPath>& resultsQueue) noexcept;

   /**
   * @brief Helper function to process a single file.
   *
   * @note This function assumes the path is valid and accessible.
   *
   * @param[in] path            The location on disk to scan.
   * @param[in] fileNode        The Node in Tree to append newly discoved files to.
   */
   void ProcessFile(
      const std::experimental::filesystem::path& path,
      Tree<FileInfo>::Node& treeNode) noexcept;

   /**
   * @brief Performs a recursive depth-first exploration of the file system.
   *
   * @param[in] path            The location on disk to scan.
   * @param[in] fileNode        The Node in Tree to append newly discoved files to.
   */
   void ProcessDirectory(
      const std::experimental::filesystem::path& path,
      Tree<FileInfo>::Node& fileNode) noexcept;

   /**
   * @brief Helper function to facilitate exception-free iteration over a directory.
   *
   * @param[in] itr             Reference to the directory to iterate over.
   * @param[in] treeNode        The Node to append the contents of the directory to.
   */
   void IterateOverDirectoryAndScan(
      std::experimental::filesystem::directory_iterator& itr,
      Tree<FileInfo>::Node& treeNode) noexcept;

   std::unique_ptr<Tree<FileInfo>> CreateTreeAndRootNode();

   const std::experimental::filesystem::path m_rootPath;
};
