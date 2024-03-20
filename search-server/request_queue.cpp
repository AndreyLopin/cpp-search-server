#include "request_queue.h"

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
    std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
        return AddFindRequest(raw_query, [status](int, DocumentStatus document_status, int) {
            return document_status == status;
        });
    }
    std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
        return AddFindRequest(raw_query, DocumentStatus::ACTUAL);
    }
    int RequestQueue::GetNoResultRequests() const {
        return no_results_requests_;
    }