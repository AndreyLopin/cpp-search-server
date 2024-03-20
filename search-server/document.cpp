#include "document.h"
#include <string>

using namespace std;

std::ostream& operator<<(std::ostream& out, const Document& document) {
    out << "{ document_id = "s << document.id;
    out << ", relevance = "s << document.relevance;
    out << ", rating = "s << document.rating;
    out << " }"s;
    return out;
}