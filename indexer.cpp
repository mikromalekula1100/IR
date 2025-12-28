#include <iostream>
#include <string>
#include <vector> 
#include <sqlite3.h>
#include <cstring>
#include <fstream>
#include <algorithm>

const int HASH_TABLE_SIZE = 50021; 

struct DocNode {
    int doc_id;
    DocNode* next;
};

struct TermNode {
    char* term;           
    int doc_count;        
    DocNode* head;        
    DocNode* tail;        
    TermNode* next;       
};

TermNode* hashTable[HASH_TABLE_SIZE];

unsigned long hash_func(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash % HASH_TABLE_SIZE;
}

bool ends_with(const std::string& value, const std::string& suffix) {
    if (suffix.size() > value.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), value.rbegin());
}

std::string stem_word(std::string word) {
    if (word.length() <= 3) return word;
    
    const char* suffixes[] = { 
        "ational", "tional", "ement", "ing", "ed", "ly", "es", "s", "ion", // En
        "ыми", "ого", "ему", "ая", "ый", "ий", "а", "о", "ы", "и", "е", "у" // Ru
    };
    
    for (const char* txt : suffixes) {
        std::string suf = txt;
        if (ends_with(word, suf)) {
            if (word.length() - suf.length() > 3) {
                 return word.substr(0, word.length() - suf.length());
            }
        }
    }
    return word;
}

void parse_and_index(int doc_id, const std::string& html) {
    std::string current_token = "";
    bool in_tag = false;

    for (size_t i = 0; i < html.length(); ++i) {
        char c = html[i];
        if (c == '<') { in_tag = true; continue; }
        if (c == '>') { in_tag = false; continue; }
        if (in_tag) continue;

        bool is_separator = (unsigned char)c <= 32 || strchr(".,!?:;\"'()[]-", c);

        if (is_separator) {
            if (!current_token.empty()) {
                std::string stemmed = stem_word(current_token);
                
                const char* term_str = stemmed.c_str();
                unsigned long idx = hash_func(term_str);
                
                TermNode* node = hashTable[idx];
                TermNode* target = nullptr;

                while (node != nullptr) {
                    if (strcmp(node->term, term_str) == 0) {
                        target = node;
                        break;
                    }
                    node = node->next;
                }

                if (target == nullptr) {
                    target = new TermNode;
                    target->term = strdup(term_str);
                    target->doc_count = 0;
                    target->head = nullptr;
                    target->tail = nullptr;
                    target->next = hashTable[idx]; // Вставка в начало списка
                    hashTable[idx] = target;
                }

                if (target->tail == nullptr || target->tail->doc_id != doc_id) {
                    DocNode* dn = new DocNode;
                    dn->doc_id = doc_id;
                    dn->next = nullptr;
                    
                    if (target->head == nullptr) target->head = dn;
                    else target->tail->next = dn;
                    
                    target->tail = dn;
                    target->doc_count++;
                }
                
                current_token = "";
            }
        } else {
            if (c >= 'A' && c <= 'Z') c += 32;
            current_token += c;
        }
    }
}

int process_doc(void* file_ptr, int argc, char** argv, char** azColName) {
    if (argv[0] && argv[1] && argv[2]) {
        int id = std::stoi(argv[0]);
        std::string url = argv[1];
        std::string text = argv[2];

        std::ofstream* docs_out = (std::ofstream*)file_ptr;
        size_t url_len = url.size();
        docs_out->write((char*)&id, sizeof(int));
        docs_out->write((char*)&url_len, sizeof(size_t));
        docs_out->write(url.c_str(), url_len);

        parse_and_index(id, text);
        
        if (id % 1000 == 0) std::cout << "Indexed doc ID: " << id << "\r" << std::flush;
    }
    return 0;
}

int main() {
    for(int i=0; i<HASH_TABLE_SIZE; ++i) hashTable[i] = nullptr;

    std::cout << "--- STARTING INDEXING ---" << std::endl;

    std::ofstream docs_file("docs.bin", std::ios::binary);
    if (!docs_file) { std::cerr << "Error creating docs.bin\n"; return 1; }

    sqlite3* db;
    if (sqlite3_open("crawler_data.db", &db)) {
        std::cerr << "Error opening DB\n"; return 1;
    }

    char* err = 0;
    sqlite3_exec(db, "SELECT id, url, raw_html FROM documents", process_doc, &docs_file, &err);
    
    sqlite3_close(db);
    docs_file.close();

    std::cout << "\nIndexing complete. Writing binary index..." << std::endl;

    std::ofstream idx_file("index.bin", std::ios::binary);
    
    int total_terms = 0;
    
    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
        TermNode* node = hashTable[i];
        while (node != nullptr) {
            // Пишем длину слова
            int term_len = strlen(node->term);
            idx_file.write((char*)&term_len, sizeof(int));
            
            // Пишем слово
            idx_file.write(node->term, term_len);
            
            // Пишем количество документов
            idx_file.write((char*)&node->doc_count, sizeof(int));
            
            // Пишем список ID
            DocNode* curr = node->head;
            while (curr != nullptr) {
                idx_file.write((char*)&curr->doc_id, sizeof(int));
                curr = curr->next;
            }

            total_terms++;
            node = node->next;
        }
    }
    
    idx_file.close();

    std::cout << "--- DONE ---" << std::endl;
    std::cout << "Total Terms in Index: " << total_terms << std::endl;
    std::cout << "Files created: index.bin, docs.bin" << std::endl;

    return 0;
}
