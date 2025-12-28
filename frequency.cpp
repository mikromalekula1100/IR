#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sqlite3.h>
#include <fstream>
#include <algorithm>

std::map<std::string, int> term_freq;

std::string remove_html_tags(const std::string& html) {
    std::string text = "";
    bool in_tag = false;
    for (char c : html) {
        if (c == '<') { in_tag = true; text += ' '; continue; }
        if (c == '>') { in_tag = false; continue; }
        if (!in_tag) text += c;
    }
    return text;
}

std::vector<std::string> tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::string current_token = "";
    for (size_t i = 0; i < text.length(); ++i) {
        unsigned char c = text[i];
        if (c <= 32 || c == '(' || c == ')' || c == '[' || c == ']' || 
            c == ',' || c == '.' || c == '!' || c == '?' || c == ':' || 
            c == ';' || c == '"' || c == '\'' || c == '-') {
            if (current_token.length() > 0) {
                tokens.push_back(current_token);
                current_token = "";
            }
        } else {
            if (c >= 'A' && c <= 'Z') c = c + 32;
            current_token += c;
        }
    }
    if (current_token.length() > 0) tokens.push_back(current_token);
    return tokens;
}

int callback(void* NotUsed, int argc, char** argv, char** azColName) {
    if (argv[0]) {
        std::string text = remove_html_tags(argv[0]);
        std::vector<std::string> tokens = tokenize(text);
        
        for (const auto& t : tokens) {
            term_freq[t]++;
        }
    }
    return 0;
}

int main() {
    sqlite3* db;
    if (sqlite3_open("crawler_data.db", &db)) {
        std::cerr << "Error db\n"; return 1;
    }

    std::cout << "Counting term frequencies (this may take a minute)...\n";
    char* zErrMsg = 0;
    sqlite3_exec(db, "SELECT raw_html FROM documents LIMIT 10000", callback, 0, &zErrMsg);
    sqlite3_close(db);

    std::cout << "Saving to csv...\n";
    std::ofstream out("freq_data.csv");
    out << "word,count\n";
    
    std::vector<std::pair<std::string, int>> sorted_terms(term_freq.begin(), term_freq.end());
    
    std::sort(sorted_terms.begin(), sorted_terms.end(), 
        [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
            return a.second > b.second;
        }
    );

    int rank = 1;
    for (const auto& p : sorted_terms) {
        out << rank << "," << p.second << "," << p.first << "\n";
        rank++;
    }
    out.close();
    std::cout << "Done! Saved to freq_data.csv\n";
    std::cout << "Top word: " << sorted_terms[0].first << " (" << sorted_terms[0].second << ")\n";

    return 0;
}
