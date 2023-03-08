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


void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings) {
    if (document_id<0) {throw std::invalid_argument("Попытка добавить документ с отрицательным id"s);}
    if (documents_.count(document_id)) {throw std::invalid_argument("Попытка добавить документ c id ранее добавленного документа"s);}                
    const std::vector<std::string> words = SplitIntoWordsNoStop(document);
    const double inv_word_count = 1.0 / words.size();
    for (const std::string& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
        ID_to_word_freqs_[document_id][word] += inv_word_count;
        ID_to_words_list_[document_id].insert(word);
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    docIDs_.push_back(document_id);
}
  
    
int SearchServer::GetDocumentCount() const {
    return documents_.size();
}
    
     
std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query, int document_id) const {
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

std::vector<int>::const_iterator SearchServer::begin() const{
    return docIDs_.begin();
}

std::vector<int>::const_iterator SearchServer::end() const{
    return docIDs_.end();
}
    
    

bool SearchServer::IsValidWord(const std::string& word) {
    // A valid word must not contain special characters
    return std::none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
    });
}

bool SearchServer::IsStopWord(const std::string& word) const {
    return stop_words_.count(word) > 0;
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
    std::vector<std::string> words;
    for (const std::string& word : SplitIntoWords(text)) {
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = std::accumulate(ratings.begin(), ratings.end(), 0);
    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string text) const {
    bool is_minus = false;
    // Word shouldn't be empty
    if (text[0] == '-') {
        is_minus = true;
        text = text.substr(1);
        if(text.empty()) {
            throw std::invalid_argument("Отсутствие текста после символа «минус» в поисковом запросе"s);
            }
        if(text[0]=='-') {
            throw std::invalid_argument("Наличие более чем одного минуса перед словами, которых не должно быть в искомых документах"s);
        }
    }
    return {text, is_minus, IsStopWord(text)};
}
    
SearchServer::Query SearchServer::ParseQuery(const std::string& text) const {
    Query query;
    for (const std::string& word : SplitIntoWords(text)) {
        const QueryWord query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                query.minus_words.insert(query_word.data);
            } 
            else {
                query.plus_words.insert(query_word.data);
            }
        }
    }
    return query;
}

double SearchServer::ComputeWordInverseDocumentFreq(const std::string& word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

const std::map<std::string, double>& SearchServer::GetWordFrequencies(int document_id) const {
    if (ID_to_word_freqs_.count(document_id)) {
        return ID_to_word_freqs_.at(document_id);
    }
    static const std::map<std::string, double> res;
    return res;  
}
    
const std::set<std::string>& SearchServer::GetWordsList(int document_id) const{
    if (ID_to_words_list_.count(document_id)) {
        return ID_to_words_list_.at(document_id);
    }
    static const std::set<std::string> res;
    return res;
}

void SearchServer::RemoveDocument(int document_id) {
    auto iter = std::find(docIDs_.begin(), docIDs_.end(), document_id );
    docIDs_.erase(iter);
    ID_to_word_freqs_.erase(document_id);
    ID_to_words_list_.erase(document_id);
    documents_.erase(document_id);
    for ( auto& [word, map]: word_to_document_freqs_) {
        map.erase(document_id);
    }
}