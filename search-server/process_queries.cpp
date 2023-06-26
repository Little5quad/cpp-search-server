#include "process_queries.h"

std::vector<std::vector<Document>> ProcessQueries(const SearchServer& search_server, const std::vector<std::string>& queries){
    std::vector<std::vector<Document>> result(queries.size());
    
    std::transform(std::execution::par, queries.begin(), queries.end(), result.begin(), [&search_server](const std::string& lhs){
        return search_server.FindTopDocuments(lhs);
    });
    
    return result;
}

std::list<Document> ProcessQueriesJoined(const SearchServer& search_server, const std::vector<std::string>& queries){
    std::list<Document> buff;
    
    for (std::vector<Document>& vd : ProcessQueries(search_server, queries)){
        for(Document& d : vd){
             buff.push_back(d);
        }
    }
    
    return buff;
}