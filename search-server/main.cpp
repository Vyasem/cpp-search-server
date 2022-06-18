#include <iostream>
#include <string>
#include <vector>

#include "headers/search_server.h"
#include "headers/paginator.h"
#include "headers/request_queue.h"


void PrintDocument(const Document& document){
	std::cout << document << std::endl;
}

void PrintMatchDocumentResult(int document_id, const std::vector<std::string>& words, DocumentStatus status) {
	std::cout << "{ document_id = " << document_id << ", status = " << static_cast<int>(status) << ", words =";
	for(const std::string& word: words){
		std::cout << ' ' << word;
	}
	std::cout << "}" << std::endl;
}

void AddDocument(SearchServer& search_server, int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings){
	try {
		search_server.AddDocument(document_id, document, status, ratings);
	} catch (const std::exception& e){
		std::cerr << "Ошибка добавления документа " << document_id << ": " << e.what() << std::endl;
		abort();
	}
}

void FindTopDocuments(const SearchServer& search_server, const std::string& raw_query){
	std::cout << "Результаты поиска по запросу: " << raw_query << std::endl;
	try {
		for (const Document& document : search_server.FindTopDocuments(raw_query)){
			PrintDocument(document);
		}
	} catch (const std::exception& e){
		std::cerr << "Ошибка поиска: " << e.what() << std::endl;
		abort();
	}
}

void MatchDocuments(const SearchServer& search_server, const std::string& query){
	try {
		std::cout << "Матчинг документов по запросу: " << query << std::endl;
		const int document_count = search_server.GetDocumentCount();
		for (int index = 0; index < document_count; ++index){
			const int document_id = search_server.GetDocumentId(index);
			const auto [words, status] = search_server.MatchDocument(query, document_id);
			PrintMatchDocumentResult(document_id, words, status);
		}
	} catch (const std::exception& e){
		std::cerr << "Ошибка матчинга документов на запрос " << query << ": " << e.what() << std::endl;
		abort();
	}
}

int main(){
	using namespace std::string_literals;
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


