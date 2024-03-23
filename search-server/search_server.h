#pragma once

#include "document.h"
#include "string_processing.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON_ = 1e-6;

class SearchServer {
public:
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words);

    explicit SearchServer(const std::string& stop_words_text);

    void AddDocument(int document_id, const std::string& document, DocumentStatus status,
                                   const std::vector<int>& ratings);
                                   
    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentPredicate document_predicate) const;

    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus status) const;

    std::vector<Document> FindTopDocuments(const std::string& raw_query) const;

    int GetDocumentCount() const;    
    int GetDocumentId(int index) const;
    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const;

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };
    const std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::map<int, DocumentData> documents_;
    std::vector<int> documents_id_;

    bool IsStopWord(const std::string& word) const;
    static bool IsValidWord(const std::string& word);
    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;
    static int ComputeAverageRating(const std::vector<int>& ratings);

    struct QueryWord {
        std::string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(std::string text) const;

    struct Query {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };

    Query ParseQuery(const std::string& text) const;

    // Existence required
    double ComputeWordInverseDocumentFreq(const std::string& word) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query& query,
                                    DocumentPredicate document_predicate) const;
};

template <typename StringContainer>
SearchServer::SearchServer(const StringContainer& stop_words)
    : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {
    for(const std::string& s : stop_words_){
        if (!IsValidWord(s)) {
            throw std::invalid_argument("Error in stop words.");
        }
    }
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentPredicate document_predicate) const {
    SearchServer::Query query = ParseQuery(raw_query);
    std::vector<Document> result = FindAllDocuments(query, document_predicate);
    std::sort(result.begin(), result.end(),
            [](const Document& lhs, const Document& rhs) {
        if (std::abs(lhs.relevance - rhs.relevance) < EPSILON_) {
                return lhs.rating > rhs.rating;
            } else {
                return lhs.relevance > rhs.relevance;
            }
        });
    if (result.size() > MAX_RESULT_DOCUMENT_COUNT) {
        result.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return result;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query,
                                    DocumentPredicate document_predicate) const {
    std::map<int, double> document_to_relevance;
    for (const std::string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto &[document_id, term_freq] : word_to_document_freqs_.at(word)) {
            if (document_predicate(document_id, documents_.at(document_id).status, documents_.at(document_id).rating)) {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }
    }

    for (const std::string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        for (const auto &[document_id, _] : word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(document_id);
        }
    }

    std::vector<Document> matched_documents;
    for (const auto &[document_id, relevance] : document_to_relevance) {
        matched_documents.push_back(
            {document_id, relevance, documents_.at(document_id).rating});
    }
    return matched_documents;
}