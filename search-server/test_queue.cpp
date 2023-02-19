#include "test_framework.h"
#include "search_server.h"
#include "test_queue.h"
#include "request_queue.h"

void TestIncreasingEmptyCount(){
    SearchServer server;
    server.AddDocument(0, "cat dog bite bat"s, DocumentStatus::ACTUAL, {-7, 4, 3});
    server.AddDocument(1, "whale fox bite bat bite"s, DocumentStatus::BANNED, {-4, 5, 5});
    server.AddDocument(2, "box craft"s, DocumentStatus::IRRELEVANT, {-5, 9, 0});
    RequestQueue que(server);
    que.AddFindRequest("bullshit"s);
    int result = que.GetNoResultRequests();
    ASSERT_EQUAL_HINT(1, result, "не увеличивается число пустых запросов"s);
     
}

// провереям что добавление непустого запроса не увеличивает число пустых запросов
void TestNotIncreasingEmptyCount(){
    SearchServer server;
    server.AddDocument(0, "cat dog bite bat"s, DocumentStatus::ACTUAL, {-7, 4, 3});
    server.AddDocument(1, "whale fox bite bat bite"s, DocumentStatus::BANNED, {-4, 5, 5});
    server.AddDocument(2, "box craft"s, DocumentStatus::IRRELEVANT, {-5, 9, 0});
    RequestQueue que(server);
    que.AddFindRequest("bullshit"s);
    int result = que.GetNoResultRequests();
    ASSERT_EQUAL_HINT(1, result, "не увеличивается число пустых запросов"s);

    que.AddFindRequest("bat"s);
    result = que.GetNoResultRequests();
    ASSERT_EQUAL_HINT(1, result, "добавление непустого запроса  увеличивает число пустых запросов"s);
     
}

// провереям что происходит после заполнения дека
void TestWhatHappensAfterDequeIsFull(){
    SearchServer server;
    server.AddDocument(0, "cat dog bite bat"s, DocumentStatus::ACTUAL, {-7, 4, 3});
    server.AddDocument(1, "whale fox bite bat bite"s, DocumentStatus::BANNED, {-4, 5, 5});
    server.AddDocument(2, "box craft"s, DocumentStatus::IRRELEVANT, {-5, 9, 0});
    RequestQueue que(server);
    for (int i = 0; i < 1439; ++i) {
         que.AddFindRequest("bullshit"s);
    }
    
    int result = que.GetNoResultRequests();
    ASSERT_EQUAL_HINT(1439, result, "не увеличивается число пустых запросов"s);

    que.AddFindRequest("bat"s);
    result = que.GetNoResultRequests();
    ASSERT_EQUAL_HINT(1439, result, "добавление непустого запроса  увеличивает число пустых запросов"s);
     
}

void TestRequestQueQue() {
     RUN_TEST(TestIncreasingEmptyCount);
     RUN_TEST(TestNotIncreasingEmptyCount);
     RUN_TEST(TestWhatHappensAfterDequeIsFull);
    // Не забудьте вызывать остальные тесты здесь
}