

#include "test_server.h"



void TestAddingDocumentsIncreasesDocumentCount () {
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    const std::vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        ASSERT_EQUAL_HINT((server.GetDocumentCount()), 0, "изначально в сервере не должно быть документов"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_EQUAL_HINT((server.GetDocumentCount()), 1, "не работает метод добавления документов"s);
        
    }
     
}



void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    const std::vector<int> ratings = {1, 2, 3};
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

 
// проверяем исключение документов с минус-словами
void TestExcludedocumentsWithMinusWords(){
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    const std::vector<int> ratings = {1, 2, 3};
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(3, "dog in question"s, DocumentStatus::ACTUAL, {6, 8, 1});
    auto result= server.FindTopDocuments("in -the"s);
    ASSERT_EQUAL_HINT((result.size()), 1, ("документы с минус-словами должны исключаться из результатов поиска"s));
    ASSERT_EQUAL_HINT((result[0].id), 3, ("документы с минус-словами должны исключаться из результатов поиска"s));
}
// проверяем матчинг документов
void TestDocumentMatching(){
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    const std::vector<int> ratings = {1, 2, 3};
    
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    
    {
        const auto [words, status] = server.MatchDocument("in city"s, doc_id);
        ASSERT_EQUAL((words.size()), 2);
        const std::vector<std::string> expected_result = {"city", "in"s};
        ASSERT_EQUAL_HINT(words, expected_result, "метод MatchDocument работает некорректно"s);
        ASSERT_EQUAL_HINT(status, DocumentStatus::ACTUAL, "метод MatchDocument работает некорректно"s);
    }

    {
        const auto [words, status] = server.MatchDocument("in -city", doc_id);
        ASSERT_EQUAL_HINT(words.size(), 0, "метод MatchDocument работает некорректно при наличии стоп-слов в запросе"s);
    }

    {
        server.SetStopWords("in");
        const auto [words, status] = server.MatchDocument("in city"s, doc_id);
        const std::vector<std::string> expected_result = {"city"s};
        ASSERT_EQUAL_HINT(words, expected_result, "метод MatchDocument работает некорректно при добавлении стоп-слов"s);

    }
}

// проверяем cортировку документов
void TestDocumentSorting(){
    
    SearchServer server;
    server.AddDocument(0, "cat dog bite bat"s, DocumentStatus::ACTUAL, {-7, 4, 3});
    server.AddDocument(1, "whale fox bite bat bite"s, DocumentStatus::ACTUAL, {-4, 5, 5});
    server.AddDocument(2, "box craft"s, DocumentStatus::ACTUAL, {-5, 9, 0});
    const auto result = server.FindTopDocuments("bite"s);
    ASSERT_EQUAL((result.size()), 2);
    ASSERT_HINT((result[0].relevance>result[1].relevance), "сортирока документов по релевантности работает некорректно"s);
}
//проверяем вычисление рейтинга документов
void TestDocumentRating(){
    SearchServer server;
    const int doc_id1 = 0;
    const std::string content1 = "cat dog bite bat"s;
    const std::vector<int> ratings1 = {-7, 4, 3};
    const int doc_id2 = 1;
    const std::string content2 = "whale fox bite bat bite"s;
    const std::vector<int> ratings2 = {-4, 5, 5};
    const int doc_id3 = 2;
    const std::string content3 = "box craft"s;
    const std::vector<int> ratings3 = {-5, 9, 0};
    server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
    server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
    server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings3);
    const auto result = server.FindTopDocuments("bite"s);
    ASSERT_EQUAL(result.size(), 2);
    
    int ratings_sum_doc1 = 0;
    for (int rating: ratings1) {
        ratings_sum_doc1 += rating;
    }
    int expected_rating1 = ratings_sum_doc1 / static_cast<int>(ratings1.size());
    ASSERT_EQUAL_HINT(result[1].rating, expected_rating1 , "неправильно считается рейтинг документов"s);
}
//проверяем использование предиката пользователя
void TestUserPredicate(){
     SearchServer server;
    server.AddDocument(0, "cat dog bite bat"s, DocumentStatus::ACTUAL, {-7, 4, 3});
    server.AddDocument(1, "whale fox bite bat bite"s, DocumentStatus::ACTUAL, {-4, 5, 5});
    server.AddDocument(2, "box craft"s, DocumentStatus::ACTUAL, {-5, 9, 0});
    const auto result = server.FindTopDocuments("bite"s, [] (int document_id, DocumentStatus status, int rating) {return document_id % 2 == 0;}) ;
    ASSERT_EQUAL(result.size(), 1);
    ASSERT_EQUAL_HINT(result[0].id, 0, "возможность фильтрации по предикату пользователя не реализована"s);
     
}

//проверяем поиск документов с заданным статусом
void TestDocumentStatus(){
    SearchServer server;
    server.AddDocument(0, "cat dog bite bat"s, DocumentStatus::ACTUAL, {-7, 4, 3});
    server.AddDocument(1, "whale fox bite bat bite"s, DocumentStatus::BANNED, {-4, 5, 5});
    server.AddDocument(2, "box craft"s, DocumentStatus::IRRELEVANT, {-5, 9, 0});
    const auto result = server.FindTopDocuments("bite"s, DocumentStatus::BANNED) ;
    ASSERT_EQUAL(result.size(), 1);
    ASSERT_EQUAL_HINT(result[0].id, 1, "нужно починить поиск документов с заданным статусом"s);

    const auto result2 = server.FindTopDocuments("bite"s, DocumentStatus::REMOVED) ;
    ASSERT_EQUAL_HINT(result2.size(), 0, "неправильно работает поиск документа по статусу, не соответствующему статусу документа"s);
    
     
}

//проверяем вычисление релевантности
void TestDocumentRelevance(){
    SearchServer server;
    const int doc_id1 = 0;
    const std::string content1 = "cat dog bite bat"s;
    const std::vector<int> ratings1 = {-7, 4, 3};
    const int doc_id2 = 1;
    const std::string content2 = "whale fox bite bat bite"s;
    const std::vector<int> ratings2 = {-4, 5, 5};
    const int doc_id3 = 2;
    const std::string content3 = "box craft"s;
    const std::vector<int> ratings3 = {-5, 9, 0};
    const int DOCUMENT_COUNT = 3;

    server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
    server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
    server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings3);
    const auto result = server.FindTopDocuments("bite box"s);
    
    double IDF_bite = log(static_cast<double>(DOCUMENT_COUNT) / 2.0); // слово bite содержится в двух документах
    double IDF_box = log(static_cast<double>(DOCUMENT_COUNT) / 1.0); // слово box содержится в одном документе
 
    ASSERT_EQUAL((result.size()), 3);

    const double TF_bite_1 = 1.0 / 4.0; // слово bite встречается 1 раз в документе из 4 слов
    const double TF_bite_2 = 2.0 / 5.0; // слово bite встречается 2 раза в документе из 5 слов
    const double TF_box_3 = 1.0 / 2.0;    // слово box встречается 1 раз в документе из 2 слов
    const double expected_relevance1 = TF_bite_1 * IDF_bite; //0.044
    const double expected_relevance2 = TF_bite_2 * IDF_bite; //0.07
    const double expected_relevance3 = TF_box_3 * IDF_box;  //0.23

    ASSERT_HINT((abs(result[0].relevance - expected_relevance3) < ACCEPTED_RELEVANCE_DIFFERENCE), "некорректно вычисляется релевантность документов"s);
    ASSERT_HINT((abs(result[1].relevance - expected_relevance2) < ACCEPTED_RELEVANCE_DIFFERENCE), "некорректно вычисляется релевантность документов"s);
    ASSERT_HINT((abs(result[2].relevance - expected_relevance1) < ACCEPTED_RELEVANCE_DIFFERENCE), "некорректно вычисляется релевантность документов"s);
     
}


// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestExcludedocumentsWithMinusWords);
    RUN_TEST(TestDocumentMatching);
    RUN_TEST(TestDocumentSorting);
    RUN_TEST(TestDocumentRating);
    RUN_TEST(TestUserPredicate);
    RUN_TEST(TestDocumentStatus);
    RUN_TEST(TestDocumentRelevance);
    RUN_TEST(TestAddingDocumentsIncreasesDocumentCount);  
    // Не забудьте вызывать остальные тесты здесь
}

