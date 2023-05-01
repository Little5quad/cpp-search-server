#pragma once

#include <iostream>

#include "search_server.h"

using namespace std;

template <typename Type, typename Hai>
ostream& operator<<(ostream& out, const pair<Type, Hai>& container) {
    out << "("s << container.first << ", "s << container.second << ")"s;
    return out;
}
template <typename Type>
ostream& Print(ostream& out, const Type& container) {
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
ostream& operator<<(ostream& out, const vector<Type>& container) {
    out << "["s;
    for (const Type& i : container) {
        if (i != container.back()) {
            out << i << ", "s;
        }
        else out << i;
    }
    out << "]"s;
    return out;
}
template <typename Type>
ostream& operator<<(ostream& out, const set<Type>& container) {
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
ostream& operator<<(ostream& out, const map<Key, Value>& container) {
    out << "{";
    bool is_first = true;
    for (const auto& [key, value] : container) {
        if (!is_first) {
            out << ", "s;
        }
        is_first = false;
        out << key << ": "s << value;
    }
    out << "}";
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

void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line, const string& hint);

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename T>
void RunTestImpl(const T& test, const string& t_str) {
    test();
    cerr << t_str << " OK"s << endl;
}

#define RUN_TEST(func) RunTestImpl((func), #func)

// -------- Начало модульных тестов поисковой системы ----------

bool isRelativelyEqual(const double& x1, const double& x2, double EPSILON);

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent();

/*
Разместите код остальных тестов здесь
*/
void TestAddDocumentContent();

void TestMinusWords();

void TestMatchingWords();

void TestRelevanceSort();

void TestRating();

void TestFuncDoumentFiltr(); 

void TestSearchStatusDocs();

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer();
