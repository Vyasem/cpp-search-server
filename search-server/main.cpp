#include <iostream>
#include <string>
#include <vector>

using namespace std::string_literals;

#include "headers/document.h"
#include "headers/search_server.h"
#include "headers/paginator.h"
#include "headers/request_queue.h"
#include "headers/read_input_functions.h"

int main(){
	//const std::string stopWords = "greater why not near without sure most had mr still never greatest be she"s;
	//const std::set<std::string> stopWords{"greater"s, "why"s, "not"s, "near"s, "without"s, "sure"s, "most"s, "had"s, "mr"s, "still"s, "never"s, "greatest"s, "be"s, "she"s};
	const std::vector<std::string> stopWords{"greater"s, "why"s, "not"s, "near"s, "without"s, "sure"s, "most"s, "had"s, "mr"s, "still"s, "never"s, "greatest"s, "be"s, "she"s};
	SearchServer server(stopWords);
	RequestQueue request_queue(server);
	AddDocument(server, 0, "highly respect inquietude finished had greater none speaking", DocumentStatus::ACTUAL, {1, 5, 8});
	AddDocument(server, 1, "having regret round kept remainder myself why not weather wished he made taste soon assistance eyes near", DocumentStatus::ACTUAL, {2, 3, 9});
	AddDocument(server, 8, "without inquietude invited never ladies relation reasonable secure humoured", DocumentStatus::ACTUAL, {1, 2});
	AddDocument(server, 3, "smiling sure furnished purse had most offered adapted called correct does domestic", DocumentStatus::BANNED, {5});
	AddDocument(server, 7, "excellence mr still alteration depending never seven first greatest three park", DocumentStatus::REMOVED, {4, 5, 7, 9});
	//FindTopDocuments(server, "inquietude weather still myself");
	//const auto search_results = server.FindTopDocuments("inquietude weather still myself");
	const auto search_results = request_queue.AddFindRequest("inquietude weather still myself");
	int page_size = 2;
	const auto pages = Paginate(search_results, page_size);
	for(auto page = pages.begin(); page != pages.end(); ++page){
		std::cout << *page << std::endl;
		std::cout << "Page break"s << std::endl;
	}
	/*FindTopDocuments(server, "excellence inquietude weather");
	MatchDocuments(server, "inquietude weather furnished");
	MatchDocuments(server, "inquietude weather still");*/
	return 0;
}


