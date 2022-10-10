#pragma once
#include <string>
#include <vector>
#include <deque>

#include "search_server.h"

class RequestQueue {
public:
	explicit RequestQueue(const SearchServer& searchServer);
	template <typename DocumentPredicate>
	std::vector<Document> AddFindRequest(const std::string& rawQuery, DocumentPredicate documentPredicate);

	std::vector<Document> AddFindRequest(const std::string& rawQuery, DocumentStatus status);

	std::vector<Document> AddFindRequest(const std::string& rawQuery);
	int GetNoResultRequests()const;
private:
	struct QueryResult {
		uint64_t timestamp;
		unsigned results;
	};
	std::deque<QueryResult> requests_;
	int no_results_requests_;
	uint64_t current_time_;
	const static uint64_t min_in_day_ = 1440;
	const SearchServer &search;
	void dequeAction(unsigned results_num);
	void push(const QueryResult& result);
	void pop();
};

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& rawQuery, DocumentPredicate documentPredicate) {
	std::vector<Document> result = search.FindTopDocuments(rawQuery, documentPredicate);
	dequeAction(result.size());
	return result;
}