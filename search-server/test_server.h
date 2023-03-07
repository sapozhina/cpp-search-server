#pragma once


#include "test_framework.h"
#include "search_server.h"

void TestAddingDocumentsIncreasesDocumentCount ();
void TestExcludeStopWordsFromAddedDocumentContent();
void TestExcludedocumentsWithMinusWords();
void TestDocumentMatching();
void TestDocumentSorting();
void TestDocumentRating();
void TestUserPredicate();
void TestDocumentStatus();
void TestDocumentRelevance();
void TestSearchServer() ;