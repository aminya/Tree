# Tree: An STL Compatible N-ary Tree

The project contained in this repository represents an STL compatible, templated tree data structure. Each node in this tree can have any number of sibling and child nodes, but only one parent. By providing the necessary functions and typedefs, this library aims to be fully compatible with all iterator based STL algorithms. In order to provide this compatible, there are several iterator classes available to traverse the tree: a pre-order iterator, a post-order iterator, a leaf iterator, and a sibling iterator. Please note that an in-order iterator has not been provided because there is no intuitive way to traverse an N-ary tree in an in-order fashion for any N greater than two.

# Usage Examples

The following is an example of how one might construct a simple, binary tree:

```
Tree<std::string> tree{ "Head" };
tree.GetHead()->AppendChild("Left Child");
tree.GetHead()->AppendChild("Right Child");
tree.GetHead()->GetFirstChild()->AppendChild("First Grandchild of Left Child");
tree.GetHead()->GetFirstChild()->AppendChild("Second Grandchild of Left Child");
```
