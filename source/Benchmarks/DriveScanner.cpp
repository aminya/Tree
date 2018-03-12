#include "DriveScanner.h"

#include "IgnoreUnused.hpp"
#include "Stopwatch.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <mutex>
#include <system_error>
#include <thread>
#include <vector>

#include <boost/asio/post.hpp>

#ifdef WIN32
   #include <windows.h>
   #include <fileapi.h>
#endif

namespace
{
   std::mutex streamMutex;

   /**
   * @brief Use the `FindFirstFileW(...)` function to retrieve the file size.
   *
   * The `std::experimental::filesystem::file_size(...)` function uses a different native function
   * to get at the file size for a given file, and this function (while probably faster than
   * `FindFirstFileW(...)`) has a tendency to throw. If such exceptional behaviour were to occur,
   * then this function can be used to hopefully still get at the file size.
   *
   * @param path[in]               The path to the troublesome file.
   *
   * @returns The size of the file if it's accessible, and zero otherwise.
   */
   std::uintmax_t GetFileSizeUsingWinAPI(const std::experimental::filesystem::path& path)
   {
      std::uintmax_t fileSize{ 0 };

#ifdef WIN32
      WIN32_FIND_DATA fileData;
      const HANDLE fileHandle = FindFirstFileW(path.wstring().data(), &fileData);
      if (fileHandle == INVALID_HANDLE_VALUE)
      {
         return 0;
      }

      const auto highWord = static_cast<std::uintmax_t>(fileData.nFileSizeHigh);
      fileSize = (highWord << sizeof(fileData.nFileSizeLow) * 8) | fileData.nFileSizeLow;
#endif

      return fileSize;
   }

   /**
   * @brief Helper function to safely wrap the retrieval of a file's size.
   *
   * @param path[in]               The path to the file.
   *
   * @return The size of the file if it's accessible, and zero otherwise.
   */
   std::uintmax_t ComputeFileSize(const std::experimental::filesystem::path& path) noexcept
   {
      try
      {
         assert(!std::experimental::filesystem::is_directory(path));

         return std::experimental::filesystem::file_size(path);
      }
      catch (...)
      {
         std::lock_guard<std::mutex> lock{ streamMutex };
         IgnoreUnused(lock);

         std::wcout << "Falling back on the Win API for: \"" << path.wstring() << "\"" << std::endl;
         return GetFileSizeUsingWinAPI(path);
      }
   }

   /**
   * @brief Removes nodes whose corresponding file or directory size is zero. This is often
   * necessary because a directory may contain only a single other directory within it that is
   * empty. In such a case, the outer directory has a size of zero, but
   * std::experimental::filesystem::is_empty will still have reported this directory as being
   * non-empty.
   *
   * @param[in, out] tree           The tree to be pruned.
   */
   void PruneEmptyFilesAndDirectories(Tree<FileInfo>& tree)
   {
      std::vector<Tree<FileInfo>::Node*> toBeDeleted;

      for (auto&& node : tree)
      {
         if (node->size == 0)
         {
            toBeDeleted.emplace_back(&node);
         }
      }

      const auto nodesRemoved = toBeDeleted.size();

      for (auto* node : toBeDeleted)
      {
         node->DeleteFromTree();
      }

      std::cout << "Number of Sizeless Files Removed: " << nodesRemoved << std::endl;
   }

   /**
   * @brief Performs a post-processing step that iterates through the tree and computes the size
   * of all directories.
   *
   * @param[in, out] tree          The tree whose nodes need their directory sizes computed.
   */
   void ComputeDirectorySizes(Tree<FileInfo>& tree)
   {
      for (auto&& node : tree)
      {
         Tree<FileInfo>::Node* parent = node.GetParent();
         if (!parent)
         {
            return;
         }

         FileInfo& parentInfo = parent->GetData();
         if (parentInfo.type == FileType::DIRECTORY)
         {
            parentInfo.size += node->size;
         }
      }
   }

   /**
   * @brief Contructs the root node for the file tree.
   *
   * @param[in] path                The path to the directory that should constitute the root node.
   */
   std::shared_ptr<Tree<FileInfo>> CreateTreeAndRootNode(const std::experimental::filesystem::path& path)
   {
      if (!std::experimental::filesystem::is_directory(path))
      {
         return nullptr;
      }

      FileInfo fileInfo
      {
         path.wstring(),
         /* extension = */ L"",
         DriveScanner::SIZE_UNDEFINED,
         FileType::DIRECTORY
      };

      return std::make_shared<Tree<FileInfo>>(Tree<FileInfo>(std::move(fileInfo)));
   }
}

DriveScanner::DriveScanner(const std::experimental::filesystem::path& path) :
   m_rootPath{ path },
   m_fileTree{ CreateTreeAndRootNode(path) }
{
}

void DriveScanner::ProcessFile(
   const std::experimental::filesystem::path& path,
   Tree<FileInfo>::Node& node) noexcept
{
   const auto fileSize = ComputeFileSize(path);
   if (fileSize == 0u)
   {
      return;
   }

   FileInfo fileInfo
   {
      path.filename().stem().wstring(),
      path.filename().extension().wstring(),
      fileSize,
      FileType::REGULAR
   };

   const std::lock_guard<decltype(m_mutex)> lock{ m_mutex };
   node.AppendChild(std::move(fileInfo));
}

void DriveScanner::ProcessDirectory(
   const std::experimental::filesystem::path& path,
   Tree<FileInfo>::Node& node) noexcept
{
   bool isRegularFile = false;
   try
   {
      // In certain cases, this function can, apparently, raise exceptions, although it
      // isn't entirely clear to me what circumstances need to exist for this to occur:
      isRegularFile = std::experimental::filesystem::is_regular_file(path);
   }
   catch (...)
   {
      return;
   }

   if (isRegularFile)
   {
      ProcessFile(path, node);
   }
   else if (std::experimental::filesystem::is_directory(path)
      && !std::experimental::filesystem::is_symlink(path))
   {
      try
      {
         // In some edge-cases, the Windows operating system doesn't allow anyone to access certain
         // directories, and attempts to do so will result in exceptional behaviour---pun intended.
         // In order to deal with these rare cases, we'll need to rely on a try-catch to keep going.
         // One example of a problematic directory in Windows 7 is: "C:\System Volume Information".
         if (std::experimental::filesystem::is_empty(path))
         {
            return;
         }
      }
      catch (...)
      {
         return;
      }

      FileInfo directoryInfo
      {
         path.filename().wstring(),
         /* extension = */ L"",
         DriveScanner::SIZE_UNDEFINED,
         FileType::DIRECTORY
      };

      const std::lock_guard<decltype(m_mutex)> lock{ m_mutex };
      auto* const lastChild = node.AppendChild(std::move(directoryInfo));

      auto itr = std::experimental::filesystem::directory_iterator{ path };
      AddDirectoriesToQueue(itr, *lastChild);
   }
}

void DriveScanner::AddDirectoriesToQueue(
   std::experimental::filesystem::directory_iterator& itr,
   Tree<FileInfo>::Node& node) noexcept
{
   const auto end = std::experimental::filesystem::directory_iterator{ };
   while (itr != end)
   {
      boost::asio::post(m_threadPool, [&, path = itr->path()] () noexcept
      {
         ProcessDirectory(path, node);
      });

      ++itr;
   }
}

void DriveScanner::Start()
{
   Stopwatch<std::chrono::seconds>([&] () noexcept
   {
      boost::asio::post(m_threadPool, [&] () noexcept
      {
         auto itr = std::experimental::filesystem::directory_iterator{ m_rootPath };
         AddDirectoriesToQueue(itr, *m_fileTree->GetRoot());
      });

      m_threadPool.join();
   }, "Scanned Drive in ");

   const auto treeSize = m_fileTree->Size();

   ComputeDirectorySizes(*m_fileTree);
   PruneEmptyFilesAndDirectories(*m_fileTree);
}

std::shared_ptr<Tree<FileInfo>> DriveScanner::GetTree()
{
   return m_fileTree;
}
