// B+_Tree.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

//узел
template <typename Key, typename Value>
class BPlusNode {
public:
    bool is_leaf;
    int key_num;
    vector<Key> keys;
    BPlusNode* parent;
    vector<BPlusNode*> children;
    vector<Value> values; 
    BPlusNode* left;
    BPlusNode* right;

    BPlusNode(int t, bool leaf = true)
        : is_leaf(leaf), key_num(0), parent(nullptr), left(nullptr), right(nullptr) {
        keys.resize(2 * t - 1);
        children.resize(2 * t, nullptr);
        if (leaf) {
            values.resize(2 * t - 1);
        }
    }

    ~BPlusNode() {
        if (!is_leaf) {
            for (auto child : children) {
                if (child) delete child;
            }
        }
    }
};

template <typename Key, typename Value>
class BPlusTree {
private:
    int t;//мин степент
    BPlusNode<Key, Value>* root;

    //нахождение листа
    BPlusNode<Key, Value>* find_leaf(Key key) {
        BPlusNode<Key, Value>* current = root;
        while (!current->is_leaf) {
            int i = 0;
            while (i < current->key_num && key >= current->keys[i]) {
                i++;
            }
            current = current->children[i];
        }
        return current;
    }
    //разделение
    void split(BPlusNode<Key, Value>* node) {
        BPlusNode<Key, Value>* new_node = new BPlusNode<Key, Value>(t, node->is_leaf);
        int mid = t - 1;
        Key mid_key = node->keys[mid];

        for (int i = 0; i < t - 1; ++i) {
            new_node->keys[i] = node->keys[mid + 1 + i];
            if (node->is_leaf) {
                new_node->values[i] = node->values[mid + 1 + i];
            }
        }
        new_node->key_num = t - 1;

        if (!node->is_leaf) {
            for (int i = 0; i < t; ++i) {
                new_node->children[i] = node->children[mid + 1 + i];
                if (new_node->children[i]) {
                    new_node->children[i]->parent = new_node;
                }
            }
        }
        else {
            new_node->keys.insert(new_node->keys.begin(), node->keys[mid]);
            new_node->values.insert(new_node->values.begin(), node->values[mid]);
            new_node->key_num = t;
        }

        
        if (node->is_leaf) {
            new_node->right = node->right;
            if (node->right) {
                node->right->left = new_node;
            }
            node->right = new_node;
            new_node->left = node;
        }

        
        node->key_num = mid;

        
        if (node == root) {
            BPlusNode<Key, Value>* new_root = new BPlusNode<Key, Value>(t, false);
            new_root->keys[0] = mid_key;
            new_root->children[0] = node;
            new_root->children[1] = new_node;
            new_root->key_num = 1;
            node->parent = new_root;
            new_node->parent = new_root;
            root = new_root;
        }
        else {
            BPlusNode<Key, Value>* parent = node->parent;
            int pos = 0;
            while (pos < parent->key_num && mid_key > parent->keys[pos]) {
                pos++;
            }

   
            for (int i = parent->key_num; i > pos; --i) {
                parent->keys[i] = parent->keys[i - 1];
            }
            for (int i = parent->key_num + 1; i > pos + 1; --i) {
                parent->children[i] = parent->children[i - 1];
            }

            parent->keys[pos] = mid_key;
            parent->children[pos + 1] = new_node;
            parent->key_num++;
            new_node->parent = parent;

            if (parent->key_num == 2 * t - 1) {
                split(parent);
            }
        }
    }

    void borrow_from_left(BPlusNode<Key, Value>* node, BPlusNode<Key, Value>* left_sibling) {
        if (node->is_leaf) {

            node->keys.insert(node->keys.begin(), left_sibling->keys.back());
            node->values.insert(node->values.begin(), left_sibling->values.back());
            left_sibling->keys.pop_back();
            left_sibling->values.pop_back();
        }
        else {
            node->keys.insert(node->keys.begin(), node->parent->keys.back());
            node->children.insert(node->children.begin(), left_sibling->children.back());
            node->parent->keys.back() = left_sibling->keys.back();
            left_sibling->keys.pop_back();
            left_sibling->children.pop_back();
        }
        node->key_num++;
        left_sibling->key_num--;
    }

    void borrow_from_right(BPlusNode<Key, Value>* node, BPlusNode<Key, Value>* right_sibling) {

        if (node->is_leaf) {
            
            node->keys.push_back(right_sibling->keys.front());
            node->values.push_back(right_sibling->values.front());
            right_sibling->keys.erase(right_sibling->keys.begin());
            right_sibling->values.erase(right_sibling->values.begin());
        }
        else {
            
            node->keys.push_back(node->parent->keys.front());
            node->children.push_back(right_sibling->children.front());
            node->parent->keys.front() = right_sibling->keys.front();
            right_sibling->keys.erase(right_sibling->keys.begin());
            right_sibling->children.erase(right_sibling->children.begin());
        }
        node->key_num++;
        right_sibling->key_num--;
    }

    void merge(BPlusNode<Key, Value>* node, BPlusNode<Key, Value>* sibling) {
        if (node->is_leaf) {
            
            for (int i = 0; i < sibling->key_num; ++i) {
                node->keys.push_back(sibling->keys[i]);
                node->values.push_back(sibling->values[i]);
            }
            node->key_num += sibling->key_num;
            node->right = sibling->right;
            if (sibling->right) {
                sibling->right->left = node;
            }
        }
        else {
          
            node->keys.push_back(node->parent->keys.front());
            for (int i = 0; i < sibling->key_num; ++i) {
                node->keys.push_back(sibling->keys[i]);
                node->children.push_back(sibling->children[i]);
            }
            node->children.push_back(sibling->children.back());
            node->key_num += sibling->key_num + 1;
        }

        
        Key separator = node->is_leaf ? sibling->keys[0] : node->parent->keys.front();
        BPlusNode<Key, Value>* parent = node->parent;
        int pos = 0;
        while (pos < parent->key_num && parent->keys[pos] != separator) {
            pos++;
        }

        for (int i = pos; i < parent->key_num - 1; ++i) {
            parent->keys[i] = parent->keys[i + 1];
        }
        for (int i = pos + 1; i < parent->key_num; ++i) {
            parent->children[i] = parent->children[i + 1];
        }
        parent->key_num--;

        delete sibling;

        if (parent == root && parent->key_num == 0) {
            root = node;
            node->parent = nullptr;
            delete parent;
        }
        else if (parent->key_num < t - 1) {
            rebalance(parent);
        }
    }

    void rebalance(BPlusNode<Key, Value>* node) {
        if (node == root || node->key_num >= t - 1) return;

        BPlusNode<Key, Value>* parent = node->parent;
        int pos = 0;
        while (parent->children[pos] != node) pos++;

  
        if (pos > 0) {
            BPlusNode<Key, Value>* left_sibling = parent->children[pos - 1];
            if (left_sibling->key_num > t - 1) {
                borrow_from_left(node, left_sibling);
                return;
            }
        }

   
        if (pos < parent->key_num) {
            BPlusNode<Key, Value>* right_sibling = parent->children[pos + 1];
            if (right_sibling->key_num > t - 1) {
                borrow_from_right(node, right_sibling);
                return;
            }
        }

        
        if (pos > 0) {
            merge(parent->children[pos - 1], node);
        }
        else {
            merge(node, parent->children[pos + 1]);
        }
    }

public:
    BPlusTree(int degree) : t(degree), root(nullptr) {}

    ~BPlusTree() {
        if (root) delete root;
    }

    
    void insert(Key key, Value value) {
        if (!root) {
            root = new BPlusNode<Key, Value>(t);
            root->keys[0] = key;
            root->values[0] = value;
            root->key_num = 1;
            return;
        }

        BPlusNode<Key, Value>* leaf = find_leaf(key);

        
        for (int i = 0; i < leaf->key_num; ++i) {
            if (leaf->keys[i] == key) {
                leaf->values[i] = value; 
                return;
            }
        }

        
        int pos = 0;
        while (pos < leaf->key_num && leaf->keys[pos] < key) {
            pos++;
        }

        for (int i = leaf->key_num; i > pos; --i) {
            leaf->keys[i] = leaf->keys[i - 1];
            leaf->values[i] = leaf->values[i - 1];
        }

        leaf->keys[pos] = key;
        leaf->values[pos] = value;
        leaf->key_num++;

        if (leaf->key_num == 2 * t - 1) {
            split(leaf);
        }
    }

    
    Value* search(Key key) {
        if (!root) return nullptr;

        BPlusNode<Key, Value>* leaf = find_leaf(key);
        for (int i = 0; i < leaf->key_num; ++i) {
            if (leaf->keys[i] == key) {
                return &leaf->values[i];
            }
        }
        return nullptr;
    }

    
    bool remove(Key key) {
        if (!root) return false;

        BPlusNode<Key, Value>* leaf = find_leaf(key);
        int pos = -1;
        for (int i = 0; i < leaf->key_num; ++i) {
            if (leaf->keys[i] == key) {
                pos = i;
                break;
            }
        }

        if (pos == -1) return false;

        
        for (int i = pos; i < leaf->key_num - 1; ++i) {
            leaf->keys[i] = leaf->keys[i + 1];
            leaf->values[i] = leaf->values[i + 1];
        }
        leaf->key_num--;

        if (leaf == root) {
            if (leaf->key_num == 0) {
                delete root;
                root = nullptr;
            }
            return true;
        }

        if (leaf->key_num < t - 1) {
            rebalance(leaf);
        }

        return true;
    }

    
};

int main() {
    BPlusTree<int, string> tree(3); 

    
    tree.insert(10, "Value10");
    tree.insert(20, "Value20");
    tree.insert(5, "Value5");
    tree.insert(15, "Value15");
    tree.insert(25, "Value25");
    tree.insert(30, "Value30");
    tree.insert(35, "Value35");
    tree.insert(40, "Value40");

    
    string* value = tree.search(15);
    if (value) {
        cout << "Found 15: " << *value << endl;
    }
    else {
        cout << "15 not found" << endl;
    }

    
    tree.remove(15);
    tree.remove(25);

    return 0;
}



