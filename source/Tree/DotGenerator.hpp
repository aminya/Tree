#pragma once

#include <algorithm>
#include <codecvt>
#include <iostream>
#include <fstream>
#include <locale>
#include <sstream>
#include <string>

#include "Tree.hpp"

template<typename DataType>
std::ostream& operator<<(std::ostream& stream, const TreeNode<DataType>& node)
{
   std::static_assert(false, "You'll need to provide your own overload.");
}

template<>
std::ostream& operator<<(std::ostream& stream, const TreeNode<int>& node)
{
   stream << node.GetData();
   return stream;
}

template<>
std::ostream& operator<<(std::ostream& stream, const TreeNode<float>& node)
{
   stream << node.GetData();
   return stream;
}

template<>
std::ostream& operator<<(std::ostream& stream, const TreeNode<double>& node)
{
   stream << node.GetData();
   return stream;
}

template<>
std::ostream& operator<<(std::ostream& stream, const TreeNode<short>& node)
{
   stream << node.GetData();
   return stream;
}

template<>
std::ostream& operator<<(std::ostream& stream, const TreeNode<char>& node)
{
   stream << node.GetData();
   return stream;
}

template<>
std::ostream& operator<<(std::ostream& stream, const TreeNode<wchar_t>& node)
{
   stream << node.GetData();
   return stream;
}

template<>
std::ostream& operator<<(std::ostream& stream, const TreeNode<std::string>& node)
{
   stream << node.GetData();
   return stream;
}

template<>
std::ostream& operator<<(std::ostream& stream, const TreeNode<std::wstring>& node)
{
   using WideToNarrowConverterType = std::codecvt_utf8<wchar_t>;
   thread_local static std::wstring_convert<WideToNarrowConverterType, wchar_t> converter;

   stream << converter.to_bytes(node.GetData());
   return stream;
}

namespace DotGenerator
{
   template<typename NodeType>
   void OutputToFile(const Tree<NodeType>& tree, const std::string& fileName)
   {
      std::stringstream graphStream;

      graphStream
         << "digraph {" << std::endl
         << "   rankdir = TB;" << std::endl
         << "   edge [arrowsize=0.4, fontsize=10]" << std::endl;

      const TreeNode<NodeType>* head = tree.GetHead();

      graphStream
         << std::endl
         << "   // Node Declarations:"
         << std::endl;

      std::for_each(
         Tree<NodeType>::PreOrderIterator{ head },
         Tree<NodeType>::PreOrderIterator{ },
         [&](Tree<NodeType>::const_reference node)
      {
         const auto nodeLabel = std::to_string(reinterpret_cast<size_t>(&node));

         graphStream
            << "   "
            << nodeLabel
            << " [label = \"" << node << "\"]"
            << std::endl;
      });

      graphStream
         << std::endl
         << "   // Edge Declarations:"
         << std::endl;

      std::for_each(
         Tree<NodeType>::PreOrderIterator{ head },
         Tree<NodeType>::PreOrderIterator{ },
         [&](Tree<NodeType>::const_reference node)
      {
         const auto* parent = node.GetParent();
         if (!parent)
         {
            return;
         }

         const auto parentLabel = std::to_string(reinterpret_cast<size_t>(parent));
         const auto childLabel =  std::to_string(reinterpret_cast<size_t>(&node));

         graphStream
            << "   "
            << parentLabel << " -> " << childLabel
            << std::endl;
      });

      graphStream << "}";

      std::ofstream outputFile
      {
         fileName,
         std::ofstream::out
      };

      outputFile << graphStream.rdbuf();
   }
};