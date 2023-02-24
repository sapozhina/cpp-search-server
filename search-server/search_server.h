#pragma once
#include "document.h"
#include "string_processing.h"
#include <algorithm>
#include <tuple>
#include <map>
#include <set>
#include <cmath>
#include <stdexcept>

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double ACCEPTED_RELEVANCE_DIFFERENCE = 1e-6;

class SearchServer {

public:
   
    SearchServer()=default;
    SearchServer(const std::string& stop_words);
    template <typename Cont>
    SearchServer(const Cont& stop_words);
    void SetStopWords(const std::string& text);
    void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings);
    template <typename Status_check>  
    std::vector<Document> FindTopDocuments(const std::string& raw_query, Status_check status_check) const;
    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus status) const;
    std::vector<Document> FindTopDocuments(const std::string& raw_query) const;
    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const;
    int GetDocumentId(int index) const;
    int GetDocumentCount() const;

private:
    
    static bool IsValidWord(const std::string& word);
  
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    struct QueryWord {
        std::string data;
        bool is_minus;
        bool is_stop;
    };
    
    struct Query {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };

    std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::map<int, DocumentData> documents_;
    std::vector<int> docIDs_;
    bool IsStopWord(const std::string& word) const;
    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;
    static int ComputeAverageRating(const std::vector<int>& ratings);
    QueryWord ParseQueryWord(std::string text) const;
    Query ParseQuery(const std::string& text) const;
    // Existence required
    double ComputeWordInverseDocumentFreq(const std::string& word) const;
    template <typename Status_check>
    std::vector<Document> FindAllDocuments(const Query& query, Status_check status_check) const;
};


template <typename Cont>
SearchServer::SearchServer(const Cont& stop_words) {
    for (const auto word: stop_words) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument("стоп-слова не должны содержать недопустимые символы"s);
            } 
        stop_words_.insert(word);    
    }
}

template <typename Status_check>  
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, Status_check status_check) const {
    const Query query = ParseQuery(raw_query);
        
    auto matched_documents = FindAllDocuments(query,  status_check);

    sort(matched_documents.begin(), matched_documents.end(),
        [](const Document& lhs, const Document& rhs) {
            if (std::abs(lhs.relevance - rhs.relevance) < ACCEPTED_RELEVANCE_DIFFERENCE) {
                return lhs.rating > rhs.rating;
            } 
            else {
                return lhs.relevance > rhs.relevance;
            }
        });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;
     
}

template <typename Status_check>
    std::vector<Document> SearchServer::FindAllDocuments(const Query& query, Status_check status_check) const {
        std::map<int, double> document_to_relevance;
        for (const std::string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
               for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                if (status_check (document_id, documents_.at(document_id).status,documents_.at(document_id).rating)) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }    
        

        for (const std::string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        std::vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back(
                {document_id, relevance, documents_.at(document_id).rating});
        }
        return matched_documents;
                                   
}