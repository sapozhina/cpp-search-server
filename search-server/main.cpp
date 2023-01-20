#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;


/*------------------------------------класс SearchServer-------------------------------------------------------*/

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
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

struct Document {
    int id;
    double relevance;
    int rating;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document, DocumentStatus status,
                     const vector<int>& ratings) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    }
  
    
          template <typename Status_check>  
        vector<Document> FindTopDocuments(const string& raw_query,
                                      Status_check status_check ) const {
        const Query query = ParseQuery(raw_query);
        
        auto matched_documents = FindAllDocuments(query,  status_check );

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 if (abs(lhs.relevance - rhs.relevance) < 1e-6) {
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
    
     vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status ) const {
       return FindTopDocuments(raw_query,
            [status](int document_id, DocumentStatus status1, int rating)
            {
                return status1 == status;
            });
    }
    vector<Document> FindTopDocuments(const string& raw_query) const {
       return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }
    int GetDocumentCount() const {
        return documents_.size();
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query,
                                                        int document_id) const {
        const Query query = ParseQuery(raw_query);
        vector<string> matched_words;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        for (const string& word : query.minus_words) {
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

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    static int ComputeAverageRating(const vector<int>& ratings) {
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
        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        // Word shouldn't be empty
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        return {text, is_minus, IsStopWord(text)};
    }

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWords(text)) {
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
    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }
    template <typename Status_check>
    vector<Document> FindAllDocuments(const Query& query, Status_check status_check) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
               for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                if (status_check (document_id, documents_.at(document_id).status,documents_.at(document_id).rating )) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }    
        

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back(
                {document_id, relevance, documents_.at(document_id).rating});
        }
        return matched_documents;
                                   
}
};

/*---------------------------ФРЕЙМВОРК ТЕСТОВ--------------------------------------*/
template < typename key,  typename value > 
ostream& operator<<(ostream& out, const pair<key,value>& p1 ) {
     
    out<<p1.first; 
    out<< ": "s;
     out<<p1.second; 
     
    return out;
}


template <typename Cont>
void Print(ostream& out, const Cont& x){
    bool a=1;
    for ( const auto& x1: x) {
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
ostream& operator<<( ostream& out, const vector<Element1>& x ) {
 out<<"["s; 
   Print(  out,  x) ;
     out<<"]"s;
    return out;
}

template <typename Element2>
ostream& operator<<( ostream& out, const set<Element2>& x ) {
 out<<"{"s; 
   Print(  out,  x) ;
     out<<"}"s;
    return out;
}


template < typename key,  typename value>
ostream& operator<<( ostream& out, const map <key, value>& x ) {
 out<<"{"s; 
   Print(  out,  x) ;
     out<<"}"s;
    return out;
}

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
                     const string& func, unsigned line, const string& hint) {
    if (t != u) {
        cout << boolalpha;
        cout << file << "("s << line << "): "s << func << ": "s;
        cout << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cout << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line,
                const string& hint) {
    if (!value) {
        cout << file << "("s << line << "): "s << func << ": "s;
        cout << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))


template <typename TestFunc >
void RunTestImpl( TestFunc testfunc, const string& testfunc_str ) {
   testfunc();
   cerr<< testfunc_str<<" OK"s<< endl;  
}

#define RUN_TEST(func) RunTestImpl ((func), #func)

/* ----------------------------------------------------------------------*/
/*-----------------------------------------ТЕСТЫ-------------------------------------------*/

void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(),
                    "Stop words must be excluded from documents"s);
    }
}

/*
Разместите код остальных тестов здесь
*/
// проверяем исключение документов с минус-словами
void TestExcludedocumentsWithMinusWords(){
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    ASSERT_EQUAL_HINT((server.FindTopDocuments("in -the"s).size()), 0, ("документы с минус-словами должны исключаться из результатов поиска"s));
}
// проверяем матчинг документов
void TestDocumentMatching(){
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    const auto result=server.MatchDocument("in city"s, doc_id);
    vector<string> words=get<0>(result);
   ASSERT_EQUAL((words.size()),2);
   ASSERT_EQUAL_HINT((words[0]), "city"s, "метод MatchDocument работает некорректно"s);
    ASSERT_EQUAL_HINT((words[1]),"in"s, "метод MatchDocument работает некорректно"s);
}

// проверяем cортировку документов
void TestDocumentSorting(){
    
    SearchServer server;
    server.AddDocument(0, "cat dog bite bat"s, DocumentStatus::ACTUAL, {-7, 4, 3});
    server.AddDocument(1, "whale fox bite bat bite"s, DocumentStatus::ACTUAL, {-4, 5, 5});
    server.AddDocument(2, "box craft"s, DocumentStatus::ACTUAL, {-5, 9, 0});
    const auto result=server.FindTopDocuments("bite"s );
    ASSERT_EQUAL ( (result.size()),2);
    ASSERT_HINT ( ( result[0].relevance>result[1].relevance), "сортирока документов по релевантности работает некорректно"s);
}
//проверяем вычисление рейтинга документов
void TestDocumentRating(){
     SearchServer server;
    server.AddDocument(0, "cat dog bite bat"s, DocumentStatus::ACTUAL, {-7, 4, 3});
    server.AddDocument(1, "whale fox bite bat bite"s, DocumentStatus::ACTUAL, {-4, 5, 5});
    server.AddDocument(2, "box craft"s, DocumentStatus::ACTUAL, {-5, 9, 0});
    const auto result=server.FindTopDocuments("bite"s );
    ASSERT_EQUAL( (result.size()), 2);
     ASSERT_EQUAL_HINT ( (result[0].rating), 2 , "неправильно считается рейтинг документов"s);
     ASSERT_EQUAL_HINT ( (result[1].rating), 0, "неправильно считается рейтинг документов"s);
}
//проверяем использование предиката пользователя
void TestUserPredicate(){
     SearchServer server;
    server.AddDocument(0, "cat dog bite bat"s, DocumentStatus::ACTUAL, {-7, 4, 3});
    server.AddDocument(1, "whale fox bite bat bite"s, DocumentStatus::ACTUAL, {-4, 5, 5});
    server.AddDocument(2, "box craft"s, DocumentStatus::ACTUAL, {-5, 9, 0});
    const auto result=server.FindTopDocuments("bite"s, [] (int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; }) ;
    ASSERT_EQUAL ((result.size()), 1);
    ASSERT_EQUAL_HINT  ((result[0].id), 0, "возможность фильтрации по предикату пользователя не реализована"s);
     
}

//проверяем поиск документов с заданным статусом
void TestDocumentStatus(){
     SearchServer server;
    server.AddDocument(0, "cat dog bite bat"s, DocumentStatus::ACTUAL, {-7, 4, 3});
    server.AddDocument(1, "whale fox bite bat bite"s, DocumentStatus::BANNED, {-4, 5, 5});
    server.AddDocument(2, "box craft"s, DocumentStatus::IRRELEVANT, {-5, 9, 0});
    const auto result=server.FindTopDocuments("bite"s, DocumentStatus::BANNED  ) ;
    ASSERT_EQUAL ((result.size()), 1);
   ASSERT_EQUAL_HINT ((result[0].id), 1, "нужно починить поиск документов с заданным статусом"s );
     
}

//проверяем вычисление релевантности
void TestDocumentRelevance(){
     SearchServer server;
    const double ACCEPTED=0.001;
    server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
    server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
    server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
     
    const auto result=server.FindTopDocuments("пушистый ухоженный кот"s) ;
   ASSERT_EQUAL ((result.size()), 3);
    ASSERT_HINT ((result[0].relevance-0.866434<ACCEPTED), "некорректно вычисляется релевантность документов"s );
  
    ASSERT_HINT( (result[2].relevance-0.173287<ACCEPTED) , "некорректно вычисляется релевантность документов"s );
     
}

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST (TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST (TestExcludedocumentsWithMinusWords);
    RUN_TEST (TestDocumentMatching);
    RUN_TEST (TestDocumentSorting);
    RUN_TEST (TestDocumentRating);
    RUN_TEST (TestUserPredicate);
    RUN_TEST (TestDocumentStatus);
    RUN_TEST (TestDocumentRelevance);
    // Не забудьте вызывать остальные тесты здесь
}





int main() {
    TestSearchServer();
    // Если вы видите эту строку, значит все тесты прошли успешно
    cout << "Search server testing finished"s << endl;
}