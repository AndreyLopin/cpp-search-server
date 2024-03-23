#include "request_queue.h"

RequestQueue::RequestQueue(const SearchServer& search_server)
            : search_server_(search_server)
            , no_results_requests_(0)
            , current_time_(0) {
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