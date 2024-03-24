#pragma once

#include "document.h"
#include "search_server.h"

#include <cstdint>
#include <deque>
#include <string>
#include <vector>

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);
    // сделаем "обёртки" для всех методов поиска, чтобы сохранять результаты для нашей статистики
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    
    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;
private:
    const SearchServer& search_server_;
    int no_results_requests_ = 0;
    uint64_t current_time_ = 0;

    struct QueryResult {
        std::vector<Document> matched_documents; //Вам не обязательно хранить все документы, достаточно просто их количества
        std::string request;
    };
    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
};

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
    ++current_time_;
    QueryResult result;
    result.matched_documents = search_server_.FindTopDocuments(raw_query, document_predicate);
    result.request = raw_query;
    if(requests_.size() == min_in_day_) {
        QueryResult delete_request = requests_.front();
        if(delete_request.matched_documents.size() == 0) {
            --no_results_requests_;
        }
        requests_.pop_front();
    }
    if(result.matched_documents.size() == 0) {
        ++no_results_requests_;
    }
    requests_.push_back(result);
    return result.matched_documents;
}