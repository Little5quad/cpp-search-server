ostream& operator<<(ostream& out, DocumentStatus ds) {
  string s;
  switch (ds) {
    case (DocumentStatus::ACTUAL) : {
      s = "ACTUAL";
      break;
    }
    case (DocumentStatus::IRRELEVANT) : {
      s = "IRRELEVANT";
      break;
    }
    case (DocumentStatus::BANNED) : {
      s = "BANNED";
      break;
    }
    case (DocumentStatus::REMOVED) : {
      s = "REMOVED";
      break;
    }
  }
  out << s;
  return out;
}

template <typename Type, typename Hai>
ostream& operator<<(ostream& out, const pair<Type, Hai>& container){
     out<<"("s<<container.first<<", "s<<container.second<<")"s;
    return out;
}

template <typename Type>
ostream& Print(ostream& out, const Type& container){
    bool is_first = true;
    for (const auto& element : container) {
        if (!is_first) {
             out << ", "s;
        }
     is_first = false;
     out << element;
    }
    return out;
}

template <typename Type>
ostream& operator<<(ostream& out, const vector<Type>& container){
    out << "["s;
    for (const Type& i: container){
        if (i != container.back()){
            out << i << ", "s;
        }else out << i;   
    }
    out<<"]"s;
    return out;
}

template <typename Type>
ostream& operator<<(ostream& out, const set<Type>& container){
     bool is_first = true;
     out << "{"s;
    for (const Type& element : container) {
        if (!is_first) {
             out << ", "s;
        }
     is_first = false;
     out << element;
    }
    out << "}"s;
    return out;
}

template <typename Key, typename Value>
ostream& operator<<(ostream& out, const map<Key, Value>& container){
    out << "{";
    bool is_first = true;
    for (const auto& [key, value] : container) {
        if (!is_first) {
             out << ", "s;
        }
     is_first = false;
     out <<key<<": "s<<value;
    }
    out<< "}";
    return out;
}

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file, const string& func, unsigned line, const string& hint) {
    if (t != u) {
        cerr << boolalpha;
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cerr << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line, const string& hint) {
    if (!value) {
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename T>
void RunTestImpl(const T& test, const string& t_str) {     
    test();
    cerr << t_str << " OK"s << endl;
}

#define RUN_TEST(func) RunTestImpl((func), #func)

// -------- Начало модульных тестов поисковой системы ----------

bool isRelativelyEqual(const double& x1, const double& x2, double EPSILON = 1e-6){
    return abs(x1 - x2) < EPSILON;
}

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
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
void TestAddDocumentContent() {
    const int doc_id = 42;
    const string content = "        cat    in    the  city    "s;
    const vector<int> ratings = {-1, 2, -5};
    const int doc_id_2 = 24;
    const string content_2 = "dog of a village"s;
    const vector<int> ratings_2 = {2, 3, 4};
    // Сначала убеждаемся, что поиск документа даёт пустой результат
    {
        SearchServer server;
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(), "Stop words must be excluded from documents"s);
    }
    // Затем убеждаемся, что поиск слова после добавления даёт результат в виде этого документа
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
        ASSERT_EQUAL(doc0.rating, -1);
        ASSERT(isRelativelyEqual(doc0.relevance, 0.17328679513998632, 1e-6));
    }
 
}

void TestMinusWords() {
// Поддержка минус-слов. Документы, содержащие минус-слова поискового запроса, не должны включаться в результаты поиска.
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {0, 1, 2, 3, 4};
    const int doc_id_2 = 24;
    const string content_2 = "dog of a village"s;
    const vector<int> ratings_2 = {2, 3, 4};
    const int doc_id_3 = 23;
    const string content_3 = "cat of a village"s;
    const vector<int> ratings_3 = {3, 4, 5};
    // Сначала убеждаемся, что поиск документа даёт результат с двумя строками
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);
        server.AddDocument(doc_id_3, content_3, DocumentStatus::ACTUAL, ratings_3);
        const auto found_docs = server.FindTopDocuments("cat in"s);
        ASSERT_EQUAL(found_docs.size(), 2);
        const Document& doc0 = found_docs[0];
        const Document& doc1 = found_docs[1];
        ASSERT_EQUAL(doc0.id, doc_id);
        ASSERT_EQUAL(doc0.rating, 2);
        ASSERT(isRelativelyEqual(doc0.relevance, 0.37601934919406854, 1e-6));
        ASSERT_EQUAL(doc1.id, doc_id_3);
        ASSERT_EQUAL(doc1.rating, 4);
        ASSERT(isRelativelyEqual(doc1.relevance, 0.1013662770270411, 1e-6));
    }
    // Затем убеждаемся, что поиск документа при запросе с минус словом исключает документ с этим минус словом
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);
        server.AddDocument(doc_id_3, content_3, DocumentStatus::ACTUAL, ratings_3);
        const auto found_docs = server.FindTopDocuments("cat -in"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id_3);
        ASSERT_EQUAL(doc0.rating, 4);
        ASSERT(isRelativelyEqual(doc0.relevance, 0.1013662770270411, 1e-6));
    }
 
}

void TestMatchingWords() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    const int doc_id_2 = 24;
    const string content_2 = "dog of village"s;
    const vector<int> ratings_2 = {2, 3, 4};
    const int doc_id_3 = 23;
    const string content_3 = "cat of a new village"s;
    const vector<int> ratings_3 = {3, 4, 5};
 
    int document_count_old;
    int document_count_new;
 
    {
        SearchServer server;
        tuple<vector<string>, DocumentStatus> match;
 
        document_count_old = server.GetDocumentCount();
        server.AddDocument(doc_id, content, DocumentStatus::BANNED, ratings);
        document_count_new = server.GetDocumentCount();
        ASSERT_EQUAL_HINT((document_count_new - document_count_old), 1, "Expected one added document"s);
        document_count_old = document_count_new;
        server.AddDocument(doc_id_2, content_2, DocumentStatus::IRRELEVANT, ratings_2);
        document_count_new = server.GetDocumentCount();
        ASSERT_EQUAL_HINT((document_count_new - document_count_old), 1, "Expected one added document"s);
        document_count_old = document_count_new;
        server.AddDocument(doc_id_3, content_3, DocumentStatus::ACTUAL, ratings_3);
        document_count_new = server.GetDocumentCount();
        ASSERT_EQUAL_HINT((document_count_new - document_count_old), 1, "Expected one added document"s);
 
        match = server.MatchDocument("cat in the city"s, doc_id);
        ASSERT_EQUAL(get<0>(match).size(), 4);
        ASSERT_EQUAL(get<1>(match), DocumentStatus::BANNED);//status writing check
        match = server.MatchDocument("dog of village"s, doc_id_2);
        ASSERT_EQUAL(get<0>(match).size(), 3);
        ASSERT_EQUAL(get<1>(match), DocumentStatus::IRRELEVANT);//status writing check
        match = server.MatchDocument("cat of a new village"s, doc_id_3);
        ASSERT_EQUAL(get<0>(match).size(), 5);
        ASSERT_EQUAL(get<1>(match), DocumentStatus::ACTUAL);//status writing check
        //Прошли проверку добавления документа
 
        match = server.MatchDocument("dog"s, 42);//no words in document
        ASSERT_EQUAL(get<0>(match).size(), 0);
        match = server.MatchDocument("dog"s, 24);//some words in document
        ASSERT_EQUAL(get<0>(match).size(), 1);//one word
        ASSERT_EQUAL(get<1>(match), DocumentStatus::IRRELEVANT);//status writing check
        match = server.MatchDocument("cat -village"s, 42);//some words in document no minus
        ASSERT_EQUAL(get<0>(match).size(), 1);//one word
        ASSERT_EQUAL(get<1>(match), DocumentStatus::BANNED);//status writing check
        match = server.MatchDocument("cat -village"s, 23);//some words in document with minus
        ASSERT_EQUAL(get<0>(match).size(), 0);
    }
 
}

void TestRelevanceSort() {
    const int doc_id_1 = 42;
    const string content_1 = "cat in the city"s;
    const vector<int> ratings_1 = {-1, -2, -3};
    const int doc_id_2 = 24;
    const string content_2 = "dog of a hidden village"s;
    const vector<int> ratings_2 = {0, 3, 0};
    const int doc_id_3 = 23;
    const string content_3 = "silent assasin village cat in the village of darkest realms"s;
    const vector<int> ratings_3 = {0, 4, 0};
    const int doc_id_4 = 12;
    const string content_4 = "cat of the loan village"s;
    const vector<int> ratings_4 = {0, 5, 0};
    const int doc_id_5 = 238;
    const string content_5 = "parrot in the village of dwarfes"s;
    const vector<int> ratings_5 = {0, 6, 0};
    const int doc_id_6 = 9001;
    const string content_6 = "parrot barking like a dog in the village of dwarfes which is in the middle of elves forest"s;
    const vector<int> ratings_6 = {6, 7, 8};
 
    {//Сортировка по убыванию релевантности, относительное сравнение релевантности 
        SearchServer server;
        //server.SetStopWords("in the"s);//stop words could interfere
        server.AddDocument(doc_id_1, content_1, DocumentStatus::ACTUAL, ratings_1);
        server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);
        server.AddDocument(doc_id_3, content_3, DocumentStatus::ACTUAL, ratings_3);
        server.AddDocument(doc_id_4, content_4, DocumentStatus::ACTUAL, ratings_4);
        server.AddDocument(doc_id_5, content_5, DocumentStatus::ACTUAL, ratings_5);
        server.AddDocument(doc_id_6, content_6, DocumentStatus::ACTUAL, ratings_6);
        const auto found_docs = server.FindTopDocuments("cat in the loan village"s);
        ASSERT_EQUAL(found_docs.size(), 5);
        const Document& doc0 = found_docs[0];
        const Document& doc1 = found_docs[1];
        const Document& doc2 = found_docs[2];
        const Document& doc3 = found_docs[3];
        const Document& doc4 = found_docs[4];
        ASSERT_EQUAL(doc0.id, doc_id_4);
        ASSERT(isRelativelyEqual(doc0.relevance, 0.56990995267518185, 1e-6));
        ASSERT_EQUAL(doc1.id, doc_id_1);
        ASSERT(isRelativelyEqual(doc1.relevance, 0.32023346136551606, 1e-6));
        ASSERT_EQUAL(doc2.id, doc_id_3);
        ASSERT(isRelativelyEqual(doc2.relevance, 0.16455769590499733, 1e-6));
        ASSERT_EQUAL(doc3.id, doc_id_5);
        ASSERT(isRelativelyEqual(doc3.relevance, 0.12835137028267893, 1e-6));
        ASSERT_EQUAL(doc4.id, doc_id_6);
        ASSERT(isRelativelyEqual(doc4.relevance, 0.075438604811010695, 1e-6));
    }
}

void TestRating() {
    const int doc_id_1 = 42;
    const string content_1 = "cat in the city"s;
    const vector<int> ratings_1 = {-1, -2, -3};
    const int doc_id_2 = 24;
    const string content_2 = "dog of a hidden village"s;
    const vector<int> ratings_2 = {0, 3, 0};
    const int doc_id_3 = 23;
    const string content_3 = "silent assasin village cat in the village of darkest realms"s;
    const vector<int> ratings_3 = {0, 4, 0};
    const int doc_id_4 = 12;
    const string content_4 = "cat of the loan village"s;
    const vector<int> ratings_4 = {0, 5, 0};
    const int doc_id_5 = 238;
    const string content_5 = "parrot in the village of dwarfes"s;
    const vector<int> ratings_5 = {0, 6, 0};
    const int doc_id_6 = 9001;
    const string content_6 = "parrot barking like a dog in the village of dwarfes which is in the middle of elves forest"s;
    const vector<int> ratings_6 = {6, 7, 8};
 
    {//подсчёт рейтинга
        SearchServer server;
        //server.SetStopWords("in the"s);//stop words could interfere
        server.AddDocument(doc_id_1, content_1, DocumentStatus::ACTUAL, ratings_1);
        server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);
        server.AddDocument(doc_id_3, content_3, DocumentStatus::ACTUAL, ratings_3);
        server.AddDocument(doc_id_4, content_4, DocumentStatus::ACTUAL, ratings_4);
        server.AddDocument(doc_id_5, content_5, DocumentStatus::ACTUAL, ratings_5);
        server.AddDocument(doc_id_6, content_6, DocumentStatus::ACTUAL, ratings_6);
        const auto found_docs = server.FindTopDocuments("cat in the loan village"s);
        ASSERT_EQUAL(found_docs.size(), 5);
        const Document& doc0 = found_docs[0];
        const Document& doc1 = found_docs[1];
        const Document& doc2 = found_docs[2];
        const Document& doc3 = found_docs[3];
        const Document& doc4 = found_docs[4];
        ASSERT_EQUAL(doc0.id, doc_id_4);
        ASSERT_EQUAL(doc0.rating, 1);
        ASSERT_EQUAL(doc1.id, doc_id_1);
        ASSERT_EQUAL(doc1.rating, -2);
        ASSERT_EQUAL(doc2.id, doc_id_3);
        ASSERT_EQUAL(doc2.rating, 1);
        ASSERT_EQUAL(doc3.id, doc_id_5);
        ASSERT_EQUAL(doc3.rating, 2);
        ASSERT_EQUAL(doc4.id, doc_id_6);
        ASSERT_EQUAL(doc4.rating, 7);
    }
}
 
void TestFuncDoumentFiltr() {
    const int doc_id_1 = 42;
    const string content_1 = "cat in the city"s;
    const vector<int> ratings_1 = {-1, -2, -3};
    const int doc_id_2 = 24;
    const string content_2 = "dog of a hidden village"s;
    const vector<int> ratings_2 = {0, 3, 0};
    const int doc_id_3 = 23;
    const string content_3 = "silent assasin village cat in the village of darkest realms"s;
    const vector<int> ratings_3 = {0, 4, 0};
    const int doc_id_4 = 12;
    const string content_4 = "cat of the loan village"s;
    const vector<int> ratings_4 = {0, 5, 0};
    const int doc_id_5 = 238;
    const string content_5 = "parrot in the village of dwarfes"s;
    const vector<int> ratings_5 = {0, 6, 0};
    const int doc_id_6 = 9001;
    const string content_6 = "parrot barking like a dog in the village of dwarfes which is in the middle of elves forest"s;
    const vector<int> ratings_6 = {6, 7, 8};
 
    SearchServer server;
    //server.SetStopWords("in the"s);//stop words could interfere
    server.AddDocument(doc_id_1, content_1, DocumentStatus::ACTUAL, ratings_1);
    server.AddDocument(doc_id_2, content_2, DocumentStatus::BANNED, ratings_2);
    server.AddDocument(doc_id_3, content_3, DocumentStatus::ACTUAL, ratings_3);
    server.AddDocument(doc_id_4, content_4, DocumentStatus::ACTUAL, ratings_4);
    server.AddDocument(doc_id_5, content_5, DocumentStatus::ACTUAL, ratings_5);
    server.AddDocument(doc_id_6, content_6, DocumentStatus::ACTUAL, ratings_6);
    {//Проверим чётные номера документов
        const auto found_docs = server.FindTopDocuments("cat in the loan village"s, [](int document_id, DocumentStatus , int ) { return document_id % 2 == 0; });
        ASSERT_EQUAL(found_docs.size(), 4);
        const Document& doc0 = found_docs[0];
        const Document& doc1 = found_docs[1];
        const Document& doc2 = found_docs[2];
        const Document& doc3 = found_docs[3];
        ASSERT_EQUAL(doc0.id, doc_id_4);
        ASSERT_EQUAL(doc0.rating, 1);
        ASSERT(isRelativelyEqual(doc0.relevance, 0.56990995267518185, 1e-6));
        ASSERT_EQUAL(doc1.id, doc_id_1);
        ASSERT_EQUAL(doc1.rating, -2);
        ASSERT(isRelativelyEqual(doc1.relevance, 0.32023346136551606, 1e-6));
        ASSERT_EQUAL(doc2.id, doc_id_5);
        ASSERT_EQUAL(doc2.rating, 2);
        ASSERT(isRelativelyEqual(doc2.relevance, 0.12835137028267893, 1e-6));
        ASSERT_EQUAL(doc3.id, doc_id_2);
        ASSERT_EQUAL(doc3.rating, 1);
        ASSERT(isRelativelyEqual(doc3.relevance, 0.03646431135879092, 1e-6));
    }
    {//Проверим многострадальный статус, хватит забаненого на всякий случай
        const auto found_docs = server.FindTopDocuments("cat in the loan village"s, [](int, DocumentStatus status, int ) { return status == DocumentStatus::BANNED; });
            ASSERT_EQUAL(found_docs.size(), 1);
            const Document& doc0 = found_docs[0];
            ASSERT_EQUAL(doc0.id, doc_id_2);
            ASSERT_EQUAL(doc0.rating, 1);
            ASSERT(isRelativelyEqual(doc0.relevance, 0.03646431135879092, 1e-6));
    }
    {//проверим рейтинг
        const auto found_docs = server.FindTopDocuments("cat in the loan village"s, [](int, DocumentStatus, int rating) { return rating > 0; });
        ASSERT_EQUAL(found_docs.size(), 5);
        const Document& doc0 = found_docs[0];
        const Document& doc1 = found_docs[1];
        const Document& doc2 = found_docs[2];
        const Document& doc3 = found_docs[3];
        const Document& doc4 = found_docs[4];
        ASSERT_EQUAL(doc0.id, doc_id_4);
        ASSERT_EQUAL(doc0.rating, 1);
        ASSERT(isRelativelyEqual(doc0.relevance, 0.56990995267518185, 1e-6));
        ASSERT_EQUAL(doc1.id, doc_id_3);
        ASSERT_EQUAL(doc1.rating, 1);
        ASSERT(isRelativelyEqual(doc1.relevance, 0.16455769590499733, 1e-6));
        ASSERT_EQUAL(doc2.id, doc_id_5);
        ASSERT_EQUAL(doc2.rating, 2);
        ASSERT(isRelativelyEqual(doc2.relevance, 0.12835137028267893, 1e-6));
        ASSERT_EQUAL(doc3.id, doc_id_6);
        ASSERT_EQUAL(doc3.rating, 7);
        ASSERT(isRelativelyEqual(doc3.relevance, 0.075438604811010695, 1e-6));
        ASSERT_EQUAL(doc4.id, doc_id_2);
        ASSERT_EQUAL(doc4.rating, 1);
        ASSERT(isRelativelyEqual(doc4.relevance, 0.03646431135879092, 1e-6));
    }
}
 
void TestSearchStatusDocs() {
    const int doc_id_1 = 42;
    const string content_1 = "cat in the city"s;
    const vector<int> ratings_1 = {0, 2, 0};
    const int doc_id_2 = 24;
    const string content_2 = "dog of a hidden village"s;
    const vector<int> ratings_2 = {0, 3, 0};
    const int doc_id_3 = 23;
    const string content_3 = "silent assasin village cat in the village of darkest realms"s;
    const vector<int> ratings_3 = {0, 4, 0};
    const int doc_id_4 = 12;
    const string content_4 = "cat of the loan village"s;
    const vector<int> ratings_4 = {0, 5, 0};
    const int doc_id_5 = 238;
    const string content_5 = "parrot in the village of dwarfes"s;
    const vector<int> ratings_5 = {0, 6, 0};
    const int doc_id_6 = 9001;
    const string content_6 = "parrot barking like a dog in the village of dwarfes which is in the middle of elves forest"s;
    const vector<int> ratings_6 = {6, 7, 8};
    const double EPSILON = 1e-6;
    // Сначала убеждаемся, что поиск документа даёт результат с двумя строками
    {
        SearchServer server;
        server.SetStopWords("in the"s);//stop words could interfere
        server.AddDocument(doc_id_1, content_1, DocumentStatus::ACTUAL, ratings_1);
        server.AddDocument(doc_id_2, content_2, DocumentStatus::BANNED, ratings_2);
        server.AddDocument(doc_id_3, content_3, DocumentStatus::IRRELEVANT, ratings_3);
        server.AddDocument(doc_id_4, content_4, DocumentStatus::ACTUAL, ratings_4);
        server.AddDocument(doc_id_5, content_5, DocumentStatus::REMOVED, ratings_5);
        server.AddDocument(doc_id_6, content_6, DocumentStatus::IRRELEVANT, ratings_6);
        {//Ищем актуальные (Вообще должно совпадать с прошлыми вызовами без аргумента, но хитрые пользователи могут сломать и это)
            const auto found_docs = server.FindTopDocuments("cat in the loan village"s, DocumentStatus::ACTUAL);
            ASSERT_EQUAL(found_docs.size(), 2);
            const Document& doc0 = found_docs[0];
            const Document& doc1 = found_docs[1];
            ASSERT_EQUAL(doc0.id, doc_id_4);
            ASSERT_EQUAL(doc0.rating, 1);
            ASSERT(isRelativelyEqual(doc0.relevance, 0.6668070516454887, EPSILON));
            ASSERT_EQUAL(doc1.id, doc_id_1);
            ASSERT_EQUAL(doc1.rating, 0);
            ASSERT(isRelativelyEqual(doc1.relevance, 0.34657359027997264, EPSILON));
        }
        {//Ищем забаненные
            const auto found_docs = server.FindTopDocuments("cat in the loan village"s, DocumentStatus::BANNED);
            ASSERT_EQUAL(found_docs.size(), 1);
            const Document& doc0 = found_docs[0];
            ASSERT_EQUAL(doc0.id, doc_id_2);
            ASSERT_EQUAL(doc0.rating, 1);
            ASSERT(isRelativelyEqual(doc0.relevance, 0.03646431135879092, EPSILON));
        }
        {//Ищем нерелевантные
            const auto found_docs = server.FindTopDocuments("cat in the loan village"s, DocumentStatus::IRRELEVANT);
            ASSERT_EQUAL(found_docs.size(), 2);
            const Document& doc0 = found_docs[0];
            const Document& doc1 = found_docs[1];
            ASSERT_EQUAL(doc0.id, doc_id_3);
            ASSERT_EQUAL(doc0.rating, 1);
            ASSERT(isRelativelyEqual(doc0.relevance, 0.13222378676848182, EPSILON));
            ASSERT_EQUAL(doc1.id, doc_id_6);
            ASSERT_EQUAL(doc1.rating, 7);
            ASSERT(isRelativelyEqual(doc1.relevance, 0.013022968342425327, EPSILON));
        }
        {//Ищем удалённые
            const auto found_docs = server.FindTopDocuments("cat in the loan village"s, DocumentStatus::REMOVED);
            ASSERT_EQUAL(found_docs.size(), 1);
            const Document& doc0 = found_docs[0];
            ASSERT_EQUAL(doc0.id, doc_id_5);
            ASSERT_EQUAL(doc0.rating, 2);
            ASSERT(isRelativelyEqual(doc0.relevance, 0.045580389198488648, EPSILON));
        }
    }
}

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestAddDocumentContent);
    RUN_TEST(TestMinusWords);
    RUN_TEST(TestMatchingWords);
    RUN_TEST(TestRelevanceSort);
    RUN_TEST(TestRating);
    RUN_TEST(TestFuncDoumentFiltr);
    RUN_TEST(TestSearchStatusDocs);
    // Не забудьте вызывать остальные тесты здесь
}
