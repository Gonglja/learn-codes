#include <iostream>
#include <vector>
#include <queue>
#include "treenode.h"

using namespace std;

template<typename T>
void printVec(const T& elms) {
  for (const auto &elm : elms)
    cout << elm << " ";
  cout << endl;
}

class Preorder {
 public:
  Preorder(TreeNode* root) {
    traversal(root);
    cout << "Preorder: ";
    printVec(res);
  }

  void traversal(TreeNode* root) {
    if (root == nullptr)
      return;
    // 前序
    res.push_back(root->val);
    traversal(root->left);
    traversal(root->right);
  }

  vector<int> res;
};

class Midorder {
 public:
  Midorder(TreeNode* root) {
    traversal(root);
    cout << "Middleorder: ";
    printVec(res);
  }

  void traversal(TreeNode* root) {
    if (root == nullptr)
      return;
    traversal(root->left);
    // 中序
    res.push_back(root->val);
    traversal(root->right);
  }
  vector<int> res;

};

class Postorder {
 public:
  Postorder(TreeNode* root) {
    traversal(root);
    cout << "Postorder: ";
    printVec(res);
  }
  void traversal(TreeNode* root) {
    if (root == nullptr)
      return;
    traversal(root->left);
    traversal(root->right);
    // 后序
    res.push_back(root->val);
  }
  vector<int> res;

};

class Levelorder {
 public:
  Levelorder(TreeNode* root) {
    traversal(root);
    cout << "Levelorder: ";
    printVec(res);
  }

  void traversal(TreeNode* root) {
    queue<TreeNode*> q;
    // 保存首个结点
    if (root) {
      res.push_back(root->val);
      q.push(root);
    }
    // 如果队列不为空，
    // 首先判断左子树是否存在，若存在则将其加到队列中，
    // 在判断右子树是否存在，若存在则将其加到队列中，
    // 将当前结点pop出来
    while (!q.empty()) {
      TreeNode* curr = q.front();
      if (curr->left) {
        q.push(curr->left);
        res.push_back(curr->left->val);
      }

      if (curr->right) {
        q.push(curr->right);
        res.push_back(curr->right->val);
      }
      q.pop();
    }
  }
  vector<int> res;
};

TreeNode* buildTree() {
  TreeNode *dummy;
  dummy = new TreeNode(1);
  dummy->left = new TreeNode(2);
  dummy->right = new TreeNode(3);
  TreeNode* curr = dummy->left;
  curr->left = new TreeNode(4);
  curr->right = new TreeNode(5);
  curr = curr->right;
  curr->left = new TreeNode(7);
  curr->right = new TreeNode(8);
  curr = dummy->right;
  curr->right = new TreeNode(6);
  return dummy;
}

void destroyTree(TreeNode* root) {
  if (root == nullptr)
    return;
  if (root->left)
    destroyTree(root->left);
  if (root->right)
    destroyTree(root->right);
}


int main() {
  TreeNode* root;
  root = buildTree();
  Preorder preorder(root);
  Midorder midorder(root);
  Postorder postorder(root);
  Levelorder levelorder(root);
  destroyTree(root);
  return 0;
}
