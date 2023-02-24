#pragma once
#include "search_server.h"
#include <deque>

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server)
    :server(search_server)
     {
       /* server = search_server;*/ // напишите реализацию
    }
    // сделаем "обёртки" для всех методов поиска, чтобы сохранять результаты для нашей статистики
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        auto result =  server.FindTopDocuments(raw_query,  document_predicate);
        if (time < min_in_day_ ) {
            time++;
            requests_.push_back({result.size(), result.empty()});
            if (result.empty()) {
                no_results_++;
                }
        }
        else {
            if  (requests_.front().IsEmpty) {
                no_results_--;
                }
            requests_.pop_front();
            requests_.push_back({result.size(), result.empty()});
            if (result.empty()) {
                no_results_++;
                }

        }
        
        return result;// напишите реализацию
    }
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    std::vector<Document> AddFindRequest(const std::string& raw_query);
    int GetNoResultRequests() const;

private:
    struct QueryResult {
        size_t number_of_documents;
        bool IsEmpty;// определите, что должно быть в структуре
    };
    std::deque<QueryResult> requests_;
    int no_results_ = 0;
    const static int min_in_day_ = 1440;
    int time = 0;
    const SearchServer& server;// возможно, здесь вам понадобится что-то ещё
}; 


