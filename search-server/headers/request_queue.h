#pragma once
#include <string>
#include <vector>
#include <deque>

#include "document.h"
#include "search_server.h"

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
    	std::vector<Document> result = search.FindTopDocuments(raw_query, document_predicate);
    	QueryResult qResult{result, result.size()};
    	dequeAction(qResult);
    	return result;
    }
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    std::vector<Document> AddFindRequest(const std::string& raw_query);
    int GetNoResultRequests()const;
private:
    int noResultCount;
    int counter;
    struct QueryResult {
    	std::vector<Document> result;
    	std::size_t size;
    };
    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    const SearchServer &search;
    void dequeAction(const QueryResult& result);
    void push(const QueryResult& result);
    void pop();
};
