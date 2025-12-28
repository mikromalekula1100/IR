#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

bool ends_with(const std::string& value, const std::string& suffix) {
    if (suffix.size() > value.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), value.rbegin());
}

std::string stem_word(std::string word) {
    if (word.length() <= 3) return word;

    std::vector<std::string> ru_suffixes = {
        "ыми", "ого", "ему", "ая", "ый", "ий", 
        "а", "о", "ы", "и", "е", "у" 
    };

    for (const auto& suf : ru_suffixes) {
        if (ends_with(word, suf)) {
            if (word.length() - suf.length() > 3) {
                 return word.substr(0, word.length() - suf.length());
            }
        }
    }

    std::vector<std::string> en_suffixes = {
        "ational", "tional", "ement", "ing", "ed", "ly", "es", "s", "ion"
    };

    for (const auto& suf : en_suffixes) {
        if (ends_with(word, suf)) {
             if (word.length() - suf.length() >= 3) {
                 return word.substr(0, word.length() - suf.length());
            }
        }
    }

    return word;
}

int main() {
    std::vector<std::string> tests = {
        "computers", "computing", "computed", 
        "systems", "systematic",
        "красный", "красного", "красному", 
        "делала", "делали", "программирование"
    };

    std::cout << "--- TEST STEMMING ---\n";
    for (const auto& w : tests) {
        std::cout << w << " -> " << stem_word(w) << std::endl;
    }
    return 0;
}
