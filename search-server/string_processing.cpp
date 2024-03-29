#include "string_processing.h"

std::vector<std::string> SplitIntoWords(const std::string& text) {
    std::vector<std::string> words;
    std::string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
    return words;
}

std::vector<std::string_view> SplitIntoWords(std::string_view text){
    std::vector<std::string_view> words;
    auto pos = text.find_first_not_of(" ");
    const auto pos_end = text.npos;
    text.remove_prefix(std::min((text.size()), pos));

    while (!text.empty()) {
        auto space = text.find(' ');
        words.push_back(space == pos_end ? text.substr(0) : text.substr(0, space));
        pos = text.find_first_not_of(" ", space);
        text.remove_prefix(std::min(text.size(), pos));
    }

    return words;
}