#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server) {
    std::vector<int> ids_to_erase;
    std::set<std::set<std::string>> result; 
    for (const auto document_id: search_server) {
        
        auto words = search_server.GetWordsList(document_id);
        auto [iterator, bool_value] = result.insert(words);
        if (!bool_value) {
            ids_to_erase.push_back(document_id);
        }
      
    }
    for (int x: ids_to_erase) {
        search_server.RemoveDocument(x);
    }
}