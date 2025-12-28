#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <map>
#include <sstream>
#include <stack>
#include <set>

std::map<std::string, std::vector<int>> search_index;
std::map<int, std::string> doc_urls;
std::vector<int> all_doc_ids; 

bool ends_with(const std::string& value, const std::string& suffix) {
    if (suffix.size() > value.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), value.rbegin());
}

std::string stem_word(std::string word) {
    if (word.length() <= 3) return word;
    const char* suffixes[] = { 
        "ational", "tional", "ement", "ing", "ed", "ly", "es", "s", "ion",
        "ыми", "ого", "ему", "ая", "ый", "ий", "а", "о", "ы", "и", "е", "у"
    };
    for (const char* txt : suffixes) {
        std::string suf = txt;
        if (ends_with(word, suf)) {
            if (word.length() - suf.length() > 3) return word.substr(0, word.length() - suf.length());
        }
    }
    return word;
}

void load_index() {
    std::ifstream docs_file("docs.bin", std::ios::binary);
    if (docs_file) {
        while (docs_file.peek() != EOF) {
            int id;
            size_t len;
            docs_file.read((char*)&id, sizeof(int));
            docs_file.read((char*)&len, sizeof(size_t));
            std::string url(len, ' ');
            docs_file.read(&url[0], len);
            doc_urls[id] = url;
            all_doc_ids.push_back(id);
        }
    }
    std::sort(all_doc_ids.begin(), all_doc_ids.end());

    std::ifstream idx_file("index.bin", std::ios::binary);
    if (idx_file) {
        while (idx_file.peek() != EOF) {
            int term_len;
            idx_file.read((char*)&term_len, sizeof(int));
            if (idx_file.eof()) break;
            
            std::string term(term_len, ' ');
            idx_file.read(&term[0], term_len);
            
            int doc_count;
            idx_file.read((char*)&doc_count, sizeof(int));
            
            std::vector<int> docs(doc_count);
            for(int i=0; i<doc_count; ++i) {
                idx_file.read((char*)&docs[i], sizeof(int));
            }
            std::sort(docs.begin(), docs.end());
            
            // ИСПОЛЬЗУЕМ НОВОЕ ИМЯ
            search_index[term] = docs;
        }
    }
}


std::vector<int> op_and(const std::vector<int>& a, const std::vector<int>& b) {
    std::vector<int> res;
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(res));
    return res;
}

std::vector<int> op_or(const std::vector<int>& a, const std::vector<int>& b) {
    std::vector<int> res;
    std::set_union(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(res));
    return res;
}

std::vector<int> op_not(const std::vector<int>& a) {
    std::vector<int> res;
    std::set_difference(all_doc_ids.begin(), all_doc_ids.end(), a.begin(), a.end(), std::back_inserter(res));
    return res;
}

std::vector<int> get_docs(std::string term) {
    std::string clean_term = "";
    for(char c : term) {
        if (!strchr("()!&| ", c)) { 
            if(c >= 'A' && c <= 'Z') c += 32;
            clean_term += c;
        }
    }
    clean_term = stem_word(clean_term);
    
    if (search_index.count(clean_term)) return search_index[clean_term];
    return {}; 
}

std::vector<int> evaluate_query(std::string query) {
    if (query.find(' ') == std::string::npos && query.find('&') == std::string::npos && query.find('|') == std::string::npos) {
        return get_docs(query);
    }
    
    std::stringstream ss(query);
    std::string item;
    std::vector<std::string> tokens;
    while(ss >> item) tokens.push_back(item);

    std::vector<int> result;
    if (tokens.empty()) return {};

    result = get_docs(tokens[0]);

    for (size_t i = 1; i < tokens.size(); ++i) {
        std::string token = tokens[i];
        
        if (token == "&&" || token == "AND") {
            if (i+1 < tokens.size()) {
                std::vector<int> right = get_docs(tokens[i+1]);
                result = op_and(result, right);
                i++; 
            }
        }
        else if (token == "||" || token == "OR") {
             if (i+1 < tokens.size()) {
                std::vector<int> right = get_docs(tokens[i+1]);
                result = op_or(result, right);
                i++;
            }
        }
        else if (token[0] == '!') {
             std::string word = token.substr(1);
             std::vector<int> right = get_docs(word);
             std::vector<int> not_right = op_not(right);
             result = op_and(result, not_right);
        }
        else {
            std::vector<int> right = get_docs(token);
            result = op_and(result, right);
        }
    }
    return result;
}

int main(int argc, char* argv[]) {
    load_index();

    if (argc > 1) {
        std::string query = "";
        for(int i=1; i<argc; ++i) query += std::string(argv[i]) + " ";
        std::vector<int> results = evaluate_query(query);
        for(int id : results) {
            std::cout << doc_urls[id] << std::endl;
        }
        return 0;
    }

    std::string q;
    std::cout << "Index loaded. Terms: " << search_index.size() << ". Docs: " << doc_urls.size() << std::endl;
    while (true) {
        std::cout << "\nEnter query (or 'exit'): ";
        std::getline(std::cin, q);
        if (q == "exit") break;
        
        std::vector<int> results = evaluate_query(q);
        
        std::cout << "Found " << results.size() << " documents:\n";
        int limit = 0;
        for (int id : results) {
            std::cout << "[" << id << "] " << doc_urls[id] << std::endl;
            if (++limit >= 10) {
                std::cout << "... more results hidden ..." << std::endl;
                break;
            }
        }
    }

    return 0;
}
