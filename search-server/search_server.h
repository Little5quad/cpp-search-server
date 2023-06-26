#pragma once
#include "string_processing.h"
#include "document.h"
#include "concurrent_map.h"

#include <execution>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <deque>
using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

class SearchServer {
public:
    
    SearchServer() = default;
    
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words);
    
    explicit SearchServer(const string& stop_words_text);
    
    explicit SearchServer(string_view stop_words_text);
 
    void AddDocument(int document_id, std::string_view document, DocumentStatus status,
                     const vector<int>& ratings);
    
    template <typename DocumentPredicate>
    vector<Document> FindTopDocuments(string_view raw_query,
                                      DocumentPredicate document_predicate) const;
    
    template <typename ExecutionPolicy, typename DocumentPredicate>
    vector<Document> FindTopDocuments(const ExecutionPolicy& policy, string_view raw_query,
                                      DocumentPredicate document_predicate) const;
 
    vector<Document> FindTopDocuments(string_view raw_query, DocumentStatus status) const;
    
    template <typename ExecutionPolicy>
    vector<Document> FindTopDocuments(const ExecutionPolicy& policy, string_view raw_query,
                                      DocumentStatus status) const;
 
    vector<Document> FindTopDocuments(string_view raw_query) const;
    
     template <typename ExecutionPolicy>
    vector<Document> FindTopDocuments(const ExecutionPolicy& policy, string_view raw_query) const;
    
    int GetDocumentCount() const;
    
    //int GetDocumentId(int index) const;
    
    auto begin() const{
        return document_ids_.begin();
    }
    
    auto end() const{
        return document_ids_.end();
    }
 
   tuple<vector<string_view>, DocumentStatus> MatchDocument(string_view raw_query,
                                                        int document_id) const;
                                                                                
   tuple<vector<string_view>, DocumentStatus> MatchDocument(const std::execution::sequenced_policy&, string_view raw_query,
                                                        int document_id) const;
    
   tuple<vector<string_view>, DocumentStatus> MatchDocument(const std::execution::parallel_policy&, string_view raw_query,
                                                        int document_id) const;
    
   const map<string_view, double>& GetWordFrequencies(int document_id) const;

   void RemoveDocument(int document_id);
 
   void RemoveDocument(const std::execution::sequenced_policy&, int document_id);
   
   void RemoveDocument(const std::execution::parallel_policy&, int document_id);
    
private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };
    
    std::deque<std::string> storage_;
    const set<string, std::less<>> stop_words_;
    map<string_view, map<int, double>> word_to_document_freqs_;
    map<int, map<string_view, double>> word_freqs_by_id_;
    map<int, DocumentData> documents_;
    set<int> document_ids_;
    
    bool IsStopWord(string_view word) const;
    
    static bool IsValidWord(string_view word);
    
    vector<string_view> SplitIntoWordsNoStop(string_view text) const;
    
    static int ComputeAverageRating(const vector<int>& ratings);
   
    struct QueryWord {
        string_view data;
        bool is_minus;
        bool is_stop;
    };
    
    QueryWord ParseQueryWord(string_view text) const;
    
    struct Query {
       vector<string_view> plus_words;
       vector<string_view> minus_words;
    };
    
    Query ParseQuery(const bool flag, string_view text) const;
    
    void VectorEraseDuplicate(const std::execution::sequenced_policy, std::vector<std::string_view>& vec) const;
        
    void VectorEraseDuplicate(const std::execution::parallel_policy, std::vector<std::string_view>& vec) const;
    
    Query ParseQuery(string_view text) const;
    
    double ComputeWordInverseDocumentFreq(string_view word) const;
 
    template <typename DocumentPredicate>
    vector<Document> FindAllDocuments(const Query& query,
                                      DocumentPredicate document_predicate) const;
    
    template <typename ExecutionPolicy, typename DocumentPredicate>
    vector<Document> FindAllDocuments(ExecutionPolicy policy, const Query& query,
                                      DocumentPredicate document_predicate) const;
};

template <typename StringContainer>
    SearchServer::SearchServer(const StringContainer& stop_words)
        : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {
        if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
            throw invalid_argument("Some of stop words are invalid"s);
        }
    }

template <typename DocumentPredicate>
    vector<Document> SearchServer::FindTopDocuments(string_view raw_query,
                                      DocumentPredicate document_predicate) const {
        const auto query = ParseQuery(false, raw_query);
 
        auto matched_documents = FindAllDocuments(query, document_predicate);
 
        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance
                     || (abs(lhs.relevance - rhs.relevance) < 1e-6 && lhs.rating > rhs.rating);
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }
 template <typename ExecutionPolicy, typename DocumentPredicate>
    vector<Document> SearchServer::FindTopDocuments(const ExecutionPolicy& policy, std::string_view raw_query,
                                      DocumentPredicate document_predicate) const{
      if constexpr (is_same_v<ExecutionPolicy, std::execution::sequenced_policy>){
        return FindTopDocuments(raw_query, document_predicate);
    }else{
           const auto query = ParseQuery(false, raw_query);
 
            auto matched_documents = FindAllDocuments(policy, query, document_predicate);

            sort(matched_documents.begin(), matched_documents.end(),
                 [](const Document& lhs, const Document& rhs) {
                     return lhs.relevance > rhs.relevance
                         || (abs(lhs.relevance - rhs.relevance) < 1e-6 && lhs.rating > rhs.rating);
                 });
            if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
                matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
            }
            return matched_documents;
      }
 }

template <typename ExecutionPolicy>
    vector<Document> SearchServer::FindTopDocuments(const ExecutionPolicy& policy, std::string_view raw_query,
                                      DocumentStatus status) const{
    if constexpr (is_same_v<ExecutionPolicy, std::execution::sequenced_policy>){
        return FindTopDocuments(
                raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
                    return document_status == status;
                });
    }else{
        return FindTopDocuments(policy, raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
                return document_status == status;
            });
    }
}

 template <typename ExecutionPolicy>
    vector<Document> SearchServer::FindTopDocuments(const ExecutionPolicy& policy, std::string_view raw_query) const{
        if constexpr (is_same_v<ExecutionPolicy, std::execution::sequenced_policy>){
            return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }else{
        return FindTopDocuments(policy, raw_query, DocumentStatus::ACTUAL);
    }
}

template <typename DocumentPredicate>
    vector<Document> SearchServer::FindAllDocuments( const Query& query,
                                      DocumentPredicate document_predicate) const {
        map<int, double> document_to_relevance;
        for (string_view word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                const auto& document_data = documents_.at(document_id);
                if (document_predicate(document_id, document_data.status, document_data.rating)) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }
        for (string_view word : query.minus_words) {
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

template <typename ExecutionPolicy, typename DocumentPredicate>
    vector<Document> SearchServer::FindAllDocuments(ExecutionPolicy policy, const Query& query,
                                      DocumentPredicate document_predicate) const{
    if constexpr (is_same_v<ExecutionPolicy, std::execution::sequenced_policy>){
        return FindAllDocuments(query, document_predicate);
    }else{
        ConcurrentMap<int, double> document_to_relevance(100);
        for_each(std::execution::par, 
                query.plus_words.begin(),
                query.plus_words.end(),
                [&](std::string_view word){
                    /*if (word_to_document_freqs_.count(word) == 0) {
                    continue;
                }*/
                    if (word_to_document_freqs_.count(word) != 0){
                    const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
                    for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                        const auto& document_data = documents_.at(document_id);
                        if (document_predicate(document_id, document_data.status, document_data.rating)) {
                            document_to_relevance[document_id] += term_freq * inverse_document_freq;
                        }
                    }}
                });
        for_each (std::execution::par, 
                query.minus_words.begin(),
                query.minus_words.end(),
                [&](std::string_view word) {
                /*if (word_to_document_freqs_.count(word) == 0) {
                    continue;
                }*/
                    if (word_to_document_freqs_.count(word) != 0) {
                for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                    document_to_relevance.Erase(document_id);
                }}
            });
         vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance.BuildOrdinaryMap()) {
            matched_documents.push_back(
                {document_id, relevance, documents_.at(document_id).rating});
        }
        return matched_documents;
    }
}
 
void PrintMatchDocumentResult(int document_id, const vector<string>& words, DocumentStatus status);