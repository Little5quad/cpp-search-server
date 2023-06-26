#include "document.h"

using namespace std;

void PrintDocument(const Document& document) {
    cout << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating << " }"s << endl;
}

ostream& operator<<(ostream& out, const Document& document) {
    out << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating << " }"s;
    return out;
}

ostream& operator<<(ostream& out, DocumentStatus ds) {
    string s;
    switch (ds) {
    case (DocumentStatus::ACTUAL): {
        s = "ACTUAL";
        break;
    }
    case (DocumentStatus::IRRELEVANT): {
        s = "IRRELEVANT";
        break;
    }
    case (DocumentStatus::BANNED): {
        s = "BANNED";
        break;
    }
    case (DocumentStatus::REMOVED): {
        s = "REMOVED";
        break;
    }
    }
    out << s;
    return out;
}