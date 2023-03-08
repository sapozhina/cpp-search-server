#include "request_queue.h"


std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
    auto result =  server.FindTopDocuments(raw_query,  status);
    if (time < min_in_day_) {
        time++;
        requests_.push_back({result.size(), result.empty()});
        if (result.empty()) {
            no_results_++;
        }
    }
    else {
        if (requests_.front().IsEmpty) {
            no_results_--;
        }
        requests_.pop_front();
        requests_.push_back({result.size(), result.empty()});
        if (result.empty()) {
            no_results_++;
        }
    }
    return result; // напишите реализацию
}
    
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
    auto result =  server.FindTopDocuments(raw_query);
    if (time < min_in_day_) {
        time++;
        requests_.push_back({result.size(), result.empty()});
        if (result.empty()) {
            no_results_++;
        }
    }
    else {
        if(requests_.front().IsEmpty) {
            no_results_--;
        }
        requests_.pop_front();
        requests_.push_back({result.size(), result.empty()});
        if(result.empty()) {
            no_results_++;
        }
    }
    return result; // напишите реализацию
}

int RequestQueue::GetNoResultRequests() const {
    return  no_results_;// напишите реализацию
}
 

