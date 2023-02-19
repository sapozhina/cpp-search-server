#pragma once
#include <iostream>
#include <utility>
#include <string>
#include <vector>
#include <map>


template < typename key,  typename value > 
std::ostream& operator<<(std::ostream& out, const std::pair<key,value>& p1) {
     
    out<<p1.first; 
    out<< ": "s;
    out<<p1.second; 
     
    return out;
}


template <typename Cont>
void Print(std::ostream& out, const Cont& x){
    bool a=1;
    for (const auto& x1: x) {
        if (a) {
         out<<x1;
         a=0;
        }
        else {
            out<<", "s<<x1;
        }
    }
 
}



template <typename Element1>
std::ostream& operator<<(std::ostream& out, const std::vector<Element1>& x) {
 out<<"["s; 
   Print(out,  x) ;
     out<<"]"s;
    return out;
}

template <typename Element2>
std::ostream& operator<<(std::ostream& out, const std::set<Element2>& x) {
 out<<"{"s; 
   Print(out,  x) ;
     out<<"}"s;
    return out;
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

template < typename key,  typename value>
std::ostream& operator<<(std::ostream& out, const std::map <key, value>& x) {
 out<<"{"s; 
   Print(out,  x) ;
     out<<"}"s;
    return out;
}

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const std::string& t_str, const std::string& u_str, const std::string& file,
                     const std::string& func, unsigned line, const std::string& hint) {
    if (t != u) {
        std::cout << boolalpha;
        std::cout << file << "("s << line << "): "s << func << ": "s;
        std::cout << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        std::cout << t << " != "s << u << "."s;
        if (!hint.empty()) {
            std::cout << " Hint: "s << hint;
        }
        std::cout << std::endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const std::string& expr_str, const std::string& file, const std::string& func, unsigned line,
                const std::string& hint) {
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

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))


template <typename TestFunc >
void RunTestImpl(TestFunc testfunc, const std::string& testfunc_str) {
   testfunc();
   std::cerr<< testfunc_str<<" OK"s<< std::endl;  
}

#define RUN_TEST(func) RunTestImpl ((func), #func)