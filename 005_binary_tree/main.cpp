#include <iostream>
#include <vector>
#include <queue>
#include <stack>

#include "treenode.h"

using namespace std;

template<typename T>
void printVec(const T &elms) {
  for (const auto &elm : elms)
    cout << elm << " ";
  cout << endl;
}

class Preorder {
 public:
  Preorder(TreeNode *root) {
    traversal_R(root);
    cout << "Preorder_R: ";
    printVec(res);
    res.clear();
    traversal_I1(root);
    cout << "Preorder_I: ";
    printVec(res);
  }
  // 递归
  void traversal_R(TreeNode *root) {
    if (root == nullptr)
      return;
    // 前序
    res.push_back(root->val);
    traversal_R(root->left);
    traversal_R(root->right);
  }
  // 遍历
  void traversal_I(TreeNode *root) {
    stack<TreeNode *> s;
    if (root == nullptr)
      return;
    s.push(root);

    while (!s.empty()) {
      // 一直沿着左子树结点访问，直到遇到没有分支的结点；沿途分支遇到后立即访问
      TreeNode *curr = root;
      while (curr) {
        // 访问数据
        res.push_back(curr->val);

        // 如果存在右子树，将右节结点暂存
        if (curr->right)
          s.push(curr->right);

        // 左结点更进一步
        curr = curr->left;
      }
      if (s.empty())
        break;
      root = s.top(); // 更新根
      s.pop();
    }

  }

  void traversal_I1(TreeNode *root) {
    stack<TreeNode *> s;
    TreeNode *curr = root;
    // 当前根不为空，或者 栈不为空
    while (curr != nullptr || !s.empty()) {
      if (curr != nullptr) {  // 如果根不为空，则直接访问
        res.push_back(curr->val);
        s.push(curr);
        curr = curr->left;
      } else { // if (curr == nullptr && !s.empty())
        TreeNode *tmp = s.top();s.pop();
        curr = tmp->right;
      }
    }
  }


  vector<int> res;
};

class Midorder {
 public:
  Midorder(TreeNode *root) {
    traversal_R(root);
    cout << "Middleorder_R: ";
    printVec(res);
    res.clear();

    traversal_I(root);
    cout << "Middleorder_I: ";
    printVec(res);

  }

  void traversal_R(TreeNode *root) {
    if (root == nullptr)
      return;
    traversal_R(root->left);
    // 中序
    res.push_back(root->val);
    traversal_R(root->right);
  }

  void traversal_I (TreeNode *root) {
    stack<TreeNode *> s;
    TreeNode *curr = root;
    // 中序访问：左根右
    while (curr != nullptr || !s.empty()) {
      if (curr != nullptr) {
        s.push(curr);
        curr = curr->left;
      } else {
        TreeNode *tmp = s.top();s.pop();
        res.push_back(tmp->val);
        curr = tmp->right;
      }
    }
  }
  vector<int> res;

};

class Postorder {
 public:
  Postorder(TreeNode *root) {
    traversal_R(root);
    cout << "Postorder_R: ";
    printVec(res);
    res.clear();

    traversal_I(root);
    cout << "Postorder_I: ";
    printVec(res);
  }
  void traversal_R(TreeNode *root) {
    if (root == nullptr)
      return;
    traversal_R(root->left);
    traversal_R(root->right);
    // 后序
    res.push_back(root->val);
  }

  void traversal_I (TreeNode *root) {
    // TODO
  }
  vector<int> res;

};

class Levelorder {
 public:
  Levelorder(TreeNode *root) {
    traversal(root);
    cout << "Levelorder: ";
    printVec(res);
  }

  void traversal(TreeNode *root) {
    queue<TreeNode *> q;
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
      TreeNode *curr = q.front();
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

TreeNode *buildTree() {
  TreeNode *dummy;
  dummy = new TreeNode(1);
  dummy->left = new TreeNode(2);
  dummy->right = new TreeNode(3);
  TreeNode *curr = dummy->left;
  curr->left = new TreeNode(4);
  curr->right = new TreeNode(5);
  curr = curr->right;
  curr->left = new TreeNode(7);
  curr->right = new TreeNode(8);
  curr = dummy->right;
  curr->right = new TreeNode(6);
  return dummy;
}

void destroyTree(TreeNode *root) {
  if (root == nullptr)
    return;
  if (root->left)
    destroyTree(root->left);
  if (root->right)
    destroyTree(root->right);
}

int main() {
  TreeNode *root;
  root = buildTree();
  Preorder preorder(root);
  Midorder midorder(root);
  Postorder postorder(root);
  Levelorder levelorder(root);
  destroyTree(root);
  return 0;
}
