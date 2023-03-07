#include "test_framework.h"

void AssertImpl(bool value, const std::string& expr_str, const std::string& file, const std::string& func, unsigned line,  const std::string& hint) {
    if (!value) {
        std::cout << file << "("s << line << "): "s << func << ": "s;
        std::cout << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            std::cout << " Hint: "s << hint;
        }
        std::cout << std::endl;
        abort();
    }
}

std::ostream& operator<<(std::ostream& out, const DocumentStatus& x) {
 switch (x){
    case DocumentStatus::ACTUAL:
    out<<"ACTUAL"s;
    break;
    case DocumentStatus::BANNED:
    out<<"BANNED"s;
    break;
    case DocumentStatus::IRRELEVANT:
    out<<"IRRELEVANT"s;
    break;
    case DocumentStatus::REMOVED:
    out<<"REMOVED"s;
    break;
 }

    return out;
}