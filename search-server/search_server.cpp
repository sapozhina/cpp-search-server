#include "search_server.h"

 

   SearchServer::SearchServer(const std::string& stop_words) {
        for (const auto& word:SplitIntoWords(stop_words)) {
            stop_words_.insert(word);
        }
    }

    
    
    void SearchServer::SetStopWords(const std::string& text) {
        for (const std::string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }
    
    std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentStatus status) const {
       return SearchServer::FindTopDocuments(raw_query,
            [status](int document_id, DocumentStatus status1, int rating)
            {
                return status1 == status;
            });
    }
    std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query) const {
       return SearchServer::FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }


    void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status,
                     const std::vector<int>& ratings) {
        if (document_id<0) {throw std::invalid_argument("Попытка добавить документ с отрицательным id"s);}
        if (documents_.count(document_id)) {throw std::invalid_argument("Попытка добавить документ c id ранее добавленного документа"s);}                
        const std::vector<std::string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const std::string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
        docIDs_.push_back(document_id);
    }
  
    
    int SearchServer::GetDocumentCount() const {
        return documents_.size();
    }
    
     
    std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query,
                                                        int document_id) const {
        const Query query = ParseQuery(raw_query);
        std::vector<std::string> matched_words;
        for (const std::string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        for (const std::string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.clear();
                break;
            }
        }
        return {matched_words, documents_.at(document_id).status};
    }

    
    
    int SearchServer::GetDocumentId(int index) const{
    if (index<0 || index>docIDs_.size()) {throw std::out_of_range("индекс переданного документа выходит за пределы допустимого диапазона"s);} 
        return docIDs_[index];
    }

