#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include "headers/search_server.h"
#include "headers/request_queue.h"


RequestQueue::RequestQueue(const SearchServer& searchServer):search(searchServer){
	no_results_requests_ = 0;
	current_time_ = 0;
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& rawQuery, DocumentStatus status) {
	return AddFindRequest(rawQuery, [status](int document_id, DocumentStatus documentStatus, int rating){
		return documentStatus == status;
	});
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& rawQuery) {
	return AddFindRequest(rawQuery, DocumentStatus::ACTUAL);
}

int RequestQueue::GetNoResultRequests() const {
	return no_results_requests_;
}

void RequestQueue::dequeAction(unsigned results_num){
	++current_time_;
	while (!requests_.empty() && min_in_day_ <= current_time_ - requests_.front().timestamp) {
		if (0 == requests_.front().results) {
			--no_results_requests_;
		}
		requests_.pop_front();
	}
	requests_.push_back({current_time_, results_num});
	if (0 == results_num) {
		++no_results_requests_;
	}
}