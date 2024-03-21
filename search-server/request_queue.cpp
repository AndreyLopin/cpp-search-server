#include "request_queue.h"

RequestQueue::RequestQueue(const SearchServer& search_server)
            : search_server_(search_server)
            , no_results_requests_(0)
            , current_time_(0) {
}

int RequestQueue::GetNoResultRequests() const {
    return no_results_requests_;
}