#include "search_server.h"

using namespace std;

SearchServer::SearchServer(const std::string& stop_words_text)
    : SearchServer(
        SplitIntoWords(stop_words_text)) { // Invoke delegating constructor from string container
}

void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status,
                                const std::vector<int>& ratings) {
    if(document_id < 0) {
        throw std::invalid_argument("ID is negative number."s);
    }

    if(documents_.count(document_id) == 1) {
        throw std::invalid_argument("Document already exists."s);
    }

    std::vector<std::string> words = SplitIntoWordsNoStop(document);    
    const double inv_word_count = 1.0 / static_cast<int>(words.size());

    for (const std::string& word : words) {
         word_to_document_freqs_[word][document_id] += inv_word_count;
    }

    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    documents_id_.push_back(document_id);
}

int SearchServer::GetDocumentCount() const {
    return static_cast<int>(documents_.size());
}
    
int SearchServer::GetDocumentId(int index) const {
    return documents_id_.at(static_cast<size_t>(index));
}

std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query, int document_id) const {        
    Query query = ParseQuery(raw_query);
    std::vector<std::string> matched_words;
    for (const string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    for (const std::string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }
    return std::make_tuple(matched_words, documents_.at(document_id).status);
}

bool SearchServer::IsStopWord(const std::string& word) const {
    return stop_words_.count(word) > 0;
}

bool SearchServer::IsValidWord(const string& word) {
    // A valid word must not contain special characters
    return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
    });
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
    std::vector<std::string> result;
    for (const std::string& word : SplitIntoWords(text)) {
        if(!IsValidWord(word)) {
            throw invalid_argument("Error in document."s);
        }
        if (!IsStopWord(word)) {
            result.push_back(word);
        }
    }
    return result;
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    return std::accumulate(ratings.begin(), ratings.end(), 0) / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string text) const {
    bool is_minus = false;
    // Word shouldn't be empty
    if(text == "-"s || !IsValidWord(text) || (text[0] == '-' && text[1] == '-')) {
        throw invalid_argument("Error in query."s);
    }
    if(text[0] == '-') {
        is_minus = true;
        text = text.substr(1);
    }
    return {text, is_minus, IsStopWord(text)};
}

SearchServer::Query SearchServer::ParseQuery(const std::string& text) const {
    Query query;
    for (const std::string& word : SplitIntoWords(text)) {
        QueryWord query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                query.minus_words.insert(query_word.data);
            } else {
                query.plus_words.insert(query_word.data);
            }
        }
    }
    return query;
}

double SearchServer::ComputeWordInverseDocumentFreq(const std::string& word) const {
    return std::log(GetDocumentCount() * 1.0 / static_cast<int>(word_to_document_freqs_.at(word).size()));
}