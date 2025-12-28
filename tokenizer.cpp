#include <iostream>
#include <string>
#include <vector>
#include <sqlite3.h>
#include <chrono>
#include <cctype>

size_t total_tokens = 0;
size_t total_token_length = 0;
size_t total_text_size = 0;

std::string remove_html_tags(const std::string& html) {
    std::string text = "";
    bool in_tag = false;
    for (char c : html) {
        if (c == '<') {
            in_tag = true;
            text += ' ';
            continue;
        }
        if (c == '>') {
            in_tag = false;
            continue;
        }
        if (!in_tag) {
            text += c;
        }
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
        std::string html = argv[0]; // raw_html
        
        std::string text = remove_html_tags(html);
        total_text_size += text.length();

        std::vector<std::string> tokens = tokenize(text);
        
        total_tokens += tokens.size();
        for (const auto& t : tokens) {
            total_token_length += t.length();
        }
    }
    return 0;
}

int main() {
    sqlite3* db;
    char* zErrMsg = 0;
    int rc;

    rc = sqlite3_open("crawler_data.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << "\n";
        return 0;
    }

    std::cout << "Starting tokenization on collected data...\n";
    auto start = std::chrono::high_resolution_clock::now();

    const char* sql = "SELECT raw_html FROM documents";
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << zErrMsg << "\n";
        sqlite3_free(zErrMsg);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    
    sqlite3_close(db);

    double time_sec = diff.count();
    double size_kb = total_text_size / 1024.0;

    std::cout << "\n--- RESULTS ---\n";
    std::cout << "Total Documents processed: (see SQL count)\n"; 
    std::cout << "Time: " << time_sec << " sec\n";
    std::cout << "Clean Text Size: " << size_kb << " KB\n";
    std::cout << "Total Tokens: " << total_tokens << "\n";
    if (total_tokens > 0)
        std::cout << "Avg Token Length: " << (double)total_token_length / total_tokens << "\n";
    std::cout << "Speed: " << size_kb / time_sec << " KB/sec\n";

    return 0;
}
