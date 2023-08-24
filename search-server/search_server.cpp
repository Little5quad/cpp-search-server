#include "search_server.h"

  SearchServer::SearchServer(const string& stop_words_text)
        : SearchServer(SplitIntoWords(std::string_view(stop_words_text))) {
    }

SearchServer::SearchServer(string_view stop_words_text) : SearchServer(SplitIntoWords(stop_words_text)){
}

void SearchServer::AddDocument(int document_id, std::string_view document, DocumentStatus status,
                     const vector<int>& ratings) {
        if ((document_id < 0) || (documents_.count(document_id) > 0)) {
            throw invalid_argument("Invalid document_id"s);
        }
        storage_.emplace_back(document);
        const auto words = SplitIntoWordsNoStop(storage_.back());
        const double inv_word_count = 1.0 / words.size();
        for (string_view word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
            word_freqs_by_id_[document_id][word] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
        document_ids_.insert(document_id);
    }

    vector<Document> SearchServer::FindTopDocuments(string_view raw_query, DocumentStatus status) const { 
        return FindTopDocuments( 
            raw_query, [status](int document_id, DocumentStatus document_status, int rating) { 
                return document_status == status; 
            }); 
    } 
    
    vector<Document> SearchServer::FindTopDocuments(const std::execution::sequenced_policy&, string_view raw_query, 
                                      DocumentStatus status) const{
        return FindTopDocuments( 
                raw_query, [status](int document_id, DocumentStatus document_status, int rating) { 
                    return document_status == status; 
                }); 
        
    }

    vector<Document> SearchServer::FindTopDocuments(const std::execution::parallel_policy&, string_view raw_query, 
                                      DocumentStatus status) const{
        return FindTopDocuments(std::execution::par, raw_query, [status](int document_id, DocumentStatus document_status, int rating) { 
                return document_status == status; 
            }); 
    }
  
    vector<Document> SearchServer::FindTopDocuments(string_view raw_query) const { 
        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL); 
    }

    vector<Document> SearchServer::FindTopDocuments(const std::execution::sequenced_policy&, string_view raw_query) const{
        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL); 
    }
    
    vector<Document> SearchServer::FindTopDocuments(const std::execution::parallel_policy&, string_view raw_query) const{
        return FindTopDocuments(std::execution::par, raw_query, DocumentStatus::ACTUAL); 
    }

    int SearchServer::GetDocumentCount() const {
        return documents_.size();
    }
 
    tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(string_view raw_query,
                                                        int document_id) const {
        Query query = ParseQuery(false,raw_query);

        std::vector<std::string_view> plus_words_document;
        for (const std::string_view& word : query.minus_words) {
            if (word_to_document_freqs_.count(word)) {
                if (word_to_document_freqs_.at(word).count(document_id)) {
                    DocumentStatus status = documents_.at(document_id).status;
                    std::tuple<std::vector<std::string_view>, DocumentStatus> result = { plus_words_document, status };
                    return result;
                    
                }
            }
        }
        for (const std::string_view& word : query.plus_words) {
            if (word_to_document_freqs_.count(word)) {
                if (word_to_document_freqs_.at(word).count(document_id)) {
                    plus_words_document.push_back(word);
                }
            }
        }
        
        sort(plus_words_document.begin(), plus_words_document.end());

        DocumentStatus status = documents_.at(document_id).status;
        std::tuple<std::vector<std::string_view>, DocumentStatus> result = { plus_words_document, status };
        return result;
    }
    
    tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::sequenced_policy&, string_view raw_query, int document_id) const{
        return MatchDocument(raw_query, document_id);
    }

    tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::parallel_policy&, string_view raw_query, int document_id) const{
        
       Query query = ParseQuery(true,raw_query);
        
        std::vector<std::string_view> plus_words_document;

        if (!std::any_of(std::execution::par, query.minus_words.begin(), query.minus_words.end(), 
            [this,document_id](const std::string_view& word)
            {
                return word_to_document_freqs_.count(word) && word_to_document_freqs_.at(word).count(document_id);
            })) 
        {
            plus_words_document.resize(query.plus_words.size());
            auto it = std::copy_if(std::execution::par, query.plus_words.begin(), query.plus_words.end(), plus_words_document.begin(), 
                [this,document_id](const std::string_view& word)
                {
                    return word_to_document_freqs_.count(word) && word_to_document_freqs_.at(word).count(document_id);
                });
            plus_words_document.erase(it, plus_words_document.end());
            VectorEraseDuplicate(std::execution::par, plus_words_document);
        }

    return {plus_words_document, documents_.at(document_id).status};
    }

    const map<string_view, double>& SearchServer::GetWordFrequencies(int document_id) const{
        if (auto res = word_freqs_by_id_.find(document_id); res != word_freqs_by_id_.end()){
            return res->second;
        }
        
        static std::map<string_view, double> zero;
        return zero;
    }
 
    void SearchServer::RemoveDocument(int document_id){
        for (auto [word, tf] : word_freqs_by_id_.at(document_id)){
            word_to_document_freqs_.at(word).erase(document_id);
        }
        word_freqs_by_id_.erase(document_id);
        documents_.erase(document_id);
        document_ids_.erase(document_id);
    }
 
    void SearchServer::RemoveDocument(const std::execution::sequenced_policy&, int document_id){
        for (auto [word, tf] : word_freqs_by_id_.at(document_id)){
            word_to_document_freqs_.at(word).erase(document_id);
        }
        word_freqs_by_id_.erase(document_id);
        documents_.erase(document_id);
        document_ids_.erase(document_id);
    }

    void SearchServer::RemoveDocument(const std::execution::parallel_policy&, int document_id){
        
    std::vector<string_view> result(word_freqs_by_id_.at(document_id).size());
    std::transform(std::execution::par_unseq,
  word_freqs_by_id_.at(document_id).begin(),word_freqs_by_id_.at(document_id).end(), result.begin(),
        [](const auto& data) { return data.first; }
    );
 
    std::for_each(std::execution::par_unseq,
        result.begin(),result.end(),
        [this,document_id](const auto word) {
            word_to_document_freqs_.at(word).erase(document_id);
        }
    );
    word_freqs_by_id_.erase(document_id);
    documents_.erase(document_id);
    document_ids_.erase(document_id);
    }

    bool SearchServer::IsStopWord(string_view word) const {
            return stop_words_.count(word) > 0;
        }

    bool  SearchServer::IsValidWord(string_view word) {
            return none_of(word.begin(), word.end(), [](char c) {
                return c >= '\0' && c < ' ';
            });
        }

    vector<string_view> SearchServer::SplitIntoWordsNoStop(string_view text) const {
        vector<string_view> words;
        for (string_view word : SplitIntoWords(text)) {
            if (!IsValidWord(word)) {
                throw invalid_argument("Word "s + std::string(word) + " is invalid"s);
            }
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }
    int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
        int rating_sum = std::accumulate(ratings.begin(), ratings.end(), 0);
        return rating_sum / static_cast<int>(ratings.size());
    }

    SearchServer::QueryWord SearchServer::ParseQueryWord(string_view text) const {
        if (text.empty()) {
            throw invalid_argument("Query word is empty"s);
        }

        bool is_minus = false;
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        if (text.empty() || text[0] == '-' || !IsValidWord(text)) {
            throw invalid_argument("Query word "s + std::string(text) + " is invalid");
        }
        return {text, is_minus, IsStopWord(text)};
    }

   SearchServer::Query SearchServer::ParseQuery(const bool execute_policy, string_view text) const {
        Query result;
        for (const string_view word : SplitIntoWords(text)) {
            const auto query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    result.minus_words.push_back(query_word.data);
                } else {
                    result.plus_words.push_back(query_word.data);
                }
            }
        }
        if (!execute_policy) {
            VectorEraseDuplicate(std::execution::seq, result.minus_words);
            VectorEraseDuplicate(std::execution::seq, result.plus_words);
        }
        
        return result;
    }

    double SearchServer::ComputeWordInverseDocumentFreq(string_view word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }

    void SearchServer::VectorEraseDuplicate(const std::execution::sequenced_policy, std::vector<std::string_view>& vec) const {
        std::sort(vec.begin(), vec.end());
        auto last = std::unique(vec.begin(), vec.end());
        vec.erase(last, vec.end());
    }

    void SearchServer::VectorEraseDuplicate(const std::execution::parallel_policy, std::vector<std::string_view>& vec) const {
        std::sort(std::execution::par,vec.begin(), vec.end());
        auto last = std::unique(std::execution::par, vec.begin(), vec.end());
        vec.erase(last, vec.end());
    }

    void PrintMatchDocumentResult(int document_id, const vector<string>& words, DocumentStatus status) {
    cout << "{ "s
         << "document_id = "s << document_id << ", "s
         << "status = "s << static_cast<int>(status) << ", "s
         << "words ="s;
    for (const string& word : words) {
        cout << ' ' << word;
    }
    cout << "}"s << endl;
}