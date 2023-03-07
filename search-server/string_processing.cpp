#include "string_processing.h"
#include <stdexcept>



std::vector<std::string> SplitIntoWords(const std::string& text) {
    std::vector<std::string> words;
    std::string word;
    for (const char c : text) {
        if  (c >= '\0' && c < ' ') {throw std::invalid_argument("попытка добавить текст документа или поисковый запрос с запрещенным(и) символом(симловами)"s);}
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