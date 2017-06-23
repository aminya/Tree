#include "DriveScanner.h"

#include "IgnoreUnused.hpp"
#include "Stopwatch.hpp"
#include "ThreadSafeQueue.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <mutex>
#include <system_error>
#include <thread>
#include <vector>

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

      const size_t nodesRemoved = toBeDeleted.size();

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
   * @brief Creates two vectors full of tasks in need of processing.
   *
   * @param[in] path               The initial enty path at which to start the scan.
   *
   * @returns A pair of vectors containing partitioned, scannable files. The first element in the
   * pair contains the directories, and the second element contains the regular files.
   */
   std::pair<std::vector<NodeAndPath>, std::vector<NodeAndPath>>
      CreateTaskItems(const std::experimental::filesystem::path& entryPath)
   {
      std::error_code errorCode;
      auto itr = std::experimental::filesystem::directory_iterator{ entryPath, errorCode };
      if (errorCode)
      {
         std::cout << "Could not create directory iterator.\n";
         return { };
      }

      std::vector<NodeAndPath> directoriesToProcess;
      std::vector<NodeAndPath> filesToProcess;

      const auto end = std::experimental::filesystem::directory_iterator{ };
      while (itr != end)
      {
         const auto path = itr->path();

         const auto fileType = std::experimental::filesystem::is_directory(path)
            ? FileType::DIRECTORY
            : FileType::REGULAR;

         constexpr std::uintmax_t fileSizeToBeComputedLater{ 0 };

         const FileInfo fileInfo
         {
            path.filename().wstring(),
            path.extension().wstring(),
            fileSizeToBeComputedLater,
            fileType
         };

         NodeAndPath nodeAndPath
         {
            std::make_unique<Tree<FileInfo>::Node>(std::move(fileInfo)),
            std::move(path)
         };

         if (fileType == FileType::DIRECTORY)
         {
            directoriesToProcess.emplace_back(std::move(nodeAndPath));
         }
         else if (fileType == FileType::REGULAR)
         {
            filesToProcess.emplace_back(std::move(nodeAndPath));
         }

         ++itr;
      }

      return std::make_pair(std::move(directoriesToProcess), std::move(filesToProcess));
   }

   /**
   * @brief Puts all the scanning result pieces back together again...
   *
   * @param[in] queue              A queue containing the results of the scanning tasks.
   * @param[out] fileTree          The tree into which the scan results should be inserted.
   */
   void BuildFinalTree(
      ThreadSafeQueue<NodeAndPath>& queue,
      Tree<FileInfo>& fileTree)
   {
      while (!queue.IsEmpty())
      {
         NodeAndPath nodeAndPath{};
         const auto successfullyPopped = queue.TryPop(nodeAndPath);
         if (!successfullyPopped)
         {
            assert(false);
            break;
         }

         fileTree.GetRoot()->AppendChild(*nodeAndPath.node);
         nodeAndPath.node.release();
      }
   }
}

DriveScanner::DriveScanner(const std::experimental::filesystem::path& path) :
   m_rootPath{ path }
{
}

std::shared_ptr<Tree<FileInfo>> DriveScanner::CreateTreeAndRootNode()
{
   assert(std::experimental::filesystem::is_directory(m_rootPath));
   if (!std::experimental::filesystem::is_directory(m_rootPath))
   {
      return nullptr;
   }

   const FileInfo fileInfo
   {
      m_rootPath.wstring(),
      /* extension = */ L"",
      DriveScanner::SIZE_UNDEFINED,
      FileType::DIRECTORY
   };

   return std::make_shared<Tree<FileInfo>>(Tree<FileInfo>(fileInfo));
}

void DriveScanner::ProcessFile(
   const std::experimental::filesystem::path& path,
   Tree<FileInfo>::Node& Node) noexcept
{
   std::uintmax_t fileSize = ComputeFileSize(path);

   if (fileSize == 0)
   {
      return;
   }

   const FileInfo fileInfo
   {
      path.filename().stem().wstring(),
      path.filename().extension().wstring(),
      fileSize,
      FileType::REGULAR
   };

   Node.AppendChild(fileInfo);
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

      const FileInfo directoryInfo
      {
         path.filename().wstring(),
         /* extension = */ L"",
         DriveScanner::SIZE_UNDEFINED,
         FileType::DIRECTORY
      };

      node.AppendChild(directoryInfo);

      auto itr = std::experimental::filesystem::directory_iterator{ path };
      IterateOverDirectoryAndScan(itr, *node.GetLastChild());
   }
}

void DriveScanner::IterateOverDirectoryAndScan(
   std::experimental::filesystem::directory_iterator& itr,
   Tree<FileInfo>::Node& node) noexcept
{
   const auto end = std::experimental::filesystem::directory_iterator{ };
   while (itr != end)
   {
      ProcessDirectory(itr->path(), node);

      ++itr;
   }
}

void DriveScanner::ProcessQueue(
   ThreadSafeQueue<NodeAndPath>& taskQueue,
   ThreadSafeQueue<NodeAndPath>& resultsQueue) noexcept
{
   while (!taskQueue.IsEmpty())
   {
      NodeAndPath nodeAndPath{ };
      const auto successfullyPopped = taskQueue.TryPop(nodeAndPath);
      if (!successfullyPopped)
      {
         assert(false);
         break;
      }

      IterateOverDirectoryAndScan(
         std::experimental::filesystem::directory_iterator{ nodeAndPath.path },
         *nodeAndPath.node);

      {
         std::lock_guard<std::mutex> lock{ streamMutex };
         IgnoreUnused(lock);

         std::wcout << "Finished scanning: \"" << nodeAndPath.path.wstring() << "\"" << std::endl;
      }

      resultsQueue.Emplace(std::move(nodeAndPath));
   }

   std::lock_guard<std::mutex> lock{ streamMutex };
   IgnoreUnused(lock);

   std::cout << "Thread " << std::this_thread::get_id() << " has finished..." << std::endl;
}

void DriveScanner::Start()
{
   m_fileTree = CreateTreeAndRootNode();
   if (!m_fileTree)
   {
      return;
   }

   Stopwatch<std::chrono::seconds>([&] () noexcept
   {
      std::pair<std::vector<NodeAndPath>, std::vector<NodeAndPath>> directoriesAndFiles =
         CreateTaskItems(m_rootPath);

      ThreadSafeQueue<NodeAndPath> resultQueue;
      ThreadSafeQueue<NodeAndPath> taskQueue;

      for (auto&& directory : directoriesAndFiles.first)
      {
         taskQueue.Emplace(std::move(directory));
      }

      std::vector<std::thread> scanningThreads;

      const auto numberOfThreads = (std::min)(std::thread::hardware_concurrency(), 4u);

      for (auto i{ 0u }; i < numberOfThreads; ++i)
      {
         scanningThreads.emplace_back(std::thread{ [&] () noexcept { ProcessQueue(taskQueue, resultQueue); } });
      }

      for (auto&& file : directoriesAndFiles.second)
      {
         ProcessFile(file.path, *m_fileTree->GetRoot());
      }

      for (auto&& thread : scanningThreads)
      {
         thread.join();
      }

      BuildFinalTree(resultQueue, *m_fileTree);
   }, "Scanned Drive in ");

   ComputeDirectorySizes(*m_fileTree);
   PruneEmptyFilesAndDirectories(*m_fileTree);
}
