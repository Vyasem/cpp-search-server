#include <string>
#include <vector>
#include <deque>
#include "headers/search_server.h"
#include "headers/request_queue.h"
#include "headers/document.h"


RequestQueue::RequestQueue(const SearchServer& search_server):search(search_server){
	noResultCount = 0;
	counter = 0;
}
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
	return AddFindRequest(raw_query, [status](int document_id, DocumentStatus documentStatus, int rating){
		return documentStatus == status;
	});
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
	return AddFindRequest(raw_query, DocumentStatus::ACTUAL);
}

int RequestQueue::GetNoResultRequests() const {
	return noResultCount;
}

void RequestQueue::dequeAction(const QueryResult& result){
	if(counter == min_in_day_){
		pop();
	}
	push(result);
}

void RequestQueue::push(const QueryResult& result){
	if(result.size == 0){
		++noResultCount;
	}
	requests_.push_back(result);
	++counter;
}

void RequestQueue::pop(){
	QueryResult& lastLement = requests_.front();
	if(lastLement.size == 0){
		--noResultCount;
	}
	requests_.pop_front();
	--counter;
}
