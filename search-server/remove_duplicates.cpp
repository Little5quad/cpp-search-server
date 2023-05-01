#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server){
    std::vector<int> duplicates_id;
    std::set<std::set<std::string>> list;
    for (auto id = search_server.begin(); id != search_server.end(); ++id){
        std::map<std::string, double> words_in_doc = search_server.GetWordFrequencies(*id);
        std::set<std::string> words;
        for (auto [word, freq] : words_in_doc){
            words.insert(word);
        }
        auto emplace = list.emplace(words);
        if (!emplace.second){
            duplicates_id.push_back(*id);
        } 
    }
    for (int id : duplicates_id){
        std::cout << "Duplicate document id: "s << id << std::endl;
        search_server.RemoveDocument(id);
    }
}