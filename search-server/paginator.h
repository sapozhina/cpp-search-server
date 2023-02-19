#pragma once

#include "document.h"
#include <iostream>

template <typename Iterator>
class IteratorRange {

    Iterator page_begin;
    Iterator page_end;
    public:
    IteratorRange (Iterator range_begin, Iterator range_end) {
    page_begin = range_begin;
    page_end = range_end ;
    }

    Iterator begin() const{
    return page_begin;
    }
    Iterator end() const{
    return page_end;
    }
    size_t size() const{
    return page_end - page_begin;
    }    
}; 


template <typename Iterator>
class Paginator {
    vector<IteratorRange<Iterator>> pages; 
     
    public:
    Paginator (Iterator begin, Iterator end, size_t page_size) {
        
       for ( ; begin < end  ; begin += page_size) {
       if (end - begin < page_size) {
       pages.push_back(IteratorRange(begin, end));
       }
       else {
       pages.push_back(IteratorRange(begin,  begin + page_size));
       }
         
       } 
       
    }
    auto begin() const{
    return pages.begin();
    }
    auto end() const{
    return pages.end();
    }
    size_t size() const{
    return pages.size();
    }     

}; 

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}

std::ostream& std::operator<<(ostream& os, const Document& document) {
    os << "{ document_id = "s<< document.id<< ", relevance = "s<< document.relevance << ", rating = "s<< document.rating<< " }"s;
    return os;
}

template <typename Iterator>
std::ostream& std::operator<<(ostream& os, const IteratorRange<Iterator> &range) {
    Iterator begin =  range.begin();
    for ( ; begin < range.end(); begin++) {
        os<< *begin;
    }
    return os;
}