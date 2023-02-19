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
    SearchServer(const Cont& stop_words) {
        for (const auto word: stop_words) {
             if (!IsValidWord(word)) {throw std::invalid_argument("стоп-слова не должны содержать недопустимые символы"s);} 
            stop_words_.insert(word);    
        }
    }
     
     

   


   void SetStopWords(const std::string& text)   ;

    void AddDocument(int document_id, const std::string& document, DocumentStatus status,
                     const std::vector<int>& ratings)  ;
  
    
     template <typename Status_check>  
    std::vector<Document> FindTopDocuments(const std::string& raw_query, Status_check status_check) const {
        const Query query = ParseQuery(raw_query);
        
        auto matched_documents = FindAllDocuments(query,  status_check);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 if (std::abs(lhs.relevance - rhs.relevance) < ACCEPTED_RELEVANCE_DIFFERENCE) {
                     return lhs.rating > rhs.rating;
                 } else {
                     return lhs.relevance > rhs.relevance;
                 }
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
     
}
    
     std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus status) const;
    
    std::vector<Document> FindTopDocuments(const std::string& raw_query) const;

    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query,
                                                        int document_id) const ;
    
    
    int GetDocumentId(int index) const;

    int GetDocumentCount() const;

    static bool IsValidWord(const std::string& word) {
        // A valid word must not contain special characters
        return std::none_of(word.begin(), word.end(), [](char c) {
            return c >= '\0' && c < ' ';
        });
    }
private:

  
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::map<int, DocumentData> documents_;
    std::vector<int> docIDs_;

    bool IsStopWord(const std::string& word) const {
        return stop_words_.count(word) > 0;
    }

   std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const {
        std::vector<std::string> words;
        for (const std::string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    static int ComputeAverageRating(const std::vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = 0;
        for (const int rating : ratings) {
            rating_sum += rating;
        }
        return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord {
        std::string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(std::string text) const {
        bool is_minus = false;
        // Word shouldn't be empty
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
            if(text.empty()) {throw std::invalid_argument("Отсутствие текста после символа «минус» в поисковом запросе"s);}
            if(text[0]=='-') {throw std::invalid_argument("Наличие более чем одного минуса перед словами, которых не должно быть в искомых документах"s);}
        }
        return {text, is_minus, IsStopWord(text)};
    }

    struct Query {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };

    Query ParseQuery(const std::string& text) const {
        Query query;
        for (const std::string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.data);
                } else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }

    // Existence required
    double ComputeWordInverseDocumentFreq(const std::string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }
    template <typename Status_check>
    std::vector<Document> FindAllDocuments(const Query& query, Status_check status_check) const {
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
};