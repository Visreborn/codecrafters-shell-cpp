#pragma once
#include<unordered_map>
#include<vector>
#include<string>
#include<algorithm>

struct Node {
    int endCount;
    int prefixCount;    
    std :: unordered_map<char, Node*> child;

    // constructor
    Node() {
        endCount = 0;
        prefixCount = 0;
    }

    //Deconstructor
    ~Node() {   
        for(auto &[_, v] : child) {
            delete v;
        }
    }
};

class Trie {
    private:
        Node *root;

        void dfs(Node *cur_node, std :: string word, std :: vector<std :: string> &ans) {
            if(cur_node -> endCount > 0) {
                ans.push_back(word);
            }

            for(auto &[ch, node] : cur_node -> child) {                    
                dfs(node, word + ch, ans);
            }
        }
    
    public:
        // constructor
        Trie() {
            root = new Node();
        }

        ~Trie() {
            delete root;
        }

        void insert(const std :: string &full_word) {
            Node *cur_node = root;
            
            for(int i = 0; i < full_word.size(); i ++) {
                if(cur_node -> child.find(full_word[i]) == cur_node -> child.end()) {
                    cur_node -> child[full_word[i]] = new Node();
                }

                cur_node = cur_node -> child[full_word[i]];
                cur_node -> prefixCount ++;
            }

            cur_node -> endCount ++;
        }

        std :: string get_longest_common_prefix(const std :: string &prefix) {
            Node *cur_node = root;
            std :: string ans = prefix;

            for(auto &ch : prefix) {
                if(cur_node -> child.find(ch) == cur_node -> child.end()) {
                    // we could not add more to the auto completion
                    return ans;
                }

                cur_node = cur_node -> child[ch];
            }

            while(cur_node -> child.size() == 1 && cur_node -> endCount == 0) {
                // there is only one branch
                auto only_branch = cur_node -> child.begin();

                // we add more as the auto completion continue
                ans += only_branch -> first;

                cur_node = only_branch -> second;
            }

            return ans;
        }

        std :: vector<std :: string> get_all_words(const std :: string &prefix) {
            std :: vector<std :: string> ans;

            Node *cur_node = root;

            for(auto &ch : prefix) {
                if(cur_node -> child.find(ch) == cur_node -> child.end()) {
                    return ans;
                }

                cur_node = cur_node -> child[ch];
            }

            dfs(cur_node, prefix, ans);
            std :: sort(ans.begin(), ans.end());

            return ans;
        }
};