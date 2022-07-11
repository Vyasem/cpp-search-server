#include <iostream>
#include <string>
#include <vector>

#include "headers/log_duration.h"
#include "headers/search_server.h"
#include "headers/paginator.h"
#include "headers/remove_duplicates.h"
#include "headers/request_queue.h"

using namespace std::string_literals;

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
			const int document_id = *(search_server.begin() + index);
			const auto [words, status] = search_server.MatchDocument(query, document_id);
			PrintMatchDocumentResult(document_id, words, status);
		}
	} catch (const std::exception& e){
		std::cerr << "Ошибка матчинга документов на запрос " << query << ": " << e.what() << std::endl;
		abort();
	}
}

int mainOld(){
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
	LOG_DURATION_STREAM("Operation time", std::cout);
	FindTopDocuments(server, "inquietude weather still myself");
	//const auto search_results = server.FindTopDocuments("inquietude weather still myself");
	const auto search_results = request_queue.AddFindRequest("inquietude weather still myself");
	int page_size = 2;
	const auto pages = Paginate(search_results, page_size);
	for(auto page = pages.begin(); page != pages.end(); ++page){
		std::cout << *page << std::endl;
		std::cout << "Page break"s << std::endl;
	}
	LOG_DURATION_STREAM("Operation time", std::cout);
	FindTopDocuments(server, "excellence inquietude weather");
	LOG_DURATION_STREAM("Operation time", std::cout);
	MatchDocuments(server, "inquietude weather furnished");
	LOG_DURATION_STREAM("Operation time", std::cout);
	MatchDocuments(server, "inquietude weather still");
	return 0;
}

int main(){
	std::string stopWords = "and with";
	SearchServer search_server(stopWords);

	AddDocument(search_server, 1, "funny pet and nasty rat", DocumentStatus::ACTUAL, {7, 2, 7});
	AddDocument(search_server, 2, "funny pet with curly hair", DocumentStatus::ACTUAL, {1, 2});

	// дубликат документа 2, будет удалён
	AddDocument(search_server, 3, "funny pet with curly hair", DocumentStatus::ACTUAL, {1, 2});

	// отличие только в стоп-словах, считаем дубликатом
	AddDocument(search_server, 4, "funny pet and curly hair", DocumentStatus::ACTUAL, {1, 2});

	// множество слов такое же, считаем дубликатом документа 1
	AddDocument(search_server, 5, "funny funny pet and nasty nasty rat", DocumentStatus::ACTUAL, {1, 2});

	// добавились новые слова, дубликатом не является
	AddDocument(search_server, 6, "funny pet and not very nasty rat", DocumentStatus::ACTUAL, {1, 2});

	// множество слов такое же, как в id 6, несмотря на другой порядок, считаем дубликатом
	AddDocument(search_server, 7, "very nasty rat and not very funny pet", DocumentStatus::ACTUAL, {1, 2});

	// есть не все слова, не является дубликатом
	AddDocument(search_server, 8, "pet with rat and rat and rat", DocumentStatus::ACTUAL, {1, 2});

	// слова из разных документов, не является дубликатом
	AddDocument(search_server, 9, "nasty rat with curly hair", DocumentStatus::ACTUAL, {1, 2});

	std::cout << "Before duplicates removed: "s << search_server.GetDocumentCount() << std::endl;
	RemoveDuplicates(search_server);
	std::cout << "After duplicates removed: "s << search_server.GetDocumentCount() << std::endl;
	/*for(auto i = search_server.documentsHash.begin(); i != search_server.documentsHash.end(); ++i){
		for(auto j = search_server.documentsHash.begin(); j != search_server.documentsHash.end(); ++j){
			auto iSecond = i->second;
			auto jSecond = j->second;
			auto iFirst = i->first;
			auto jFirst = j->first;
			if(iSecond == jSecond && iFirst != jFirst){
				std::cout << iFirst << ": " << iSecond << " = " << jFirst << ": " << jSecond << std::endl;
			}
		}
		std::cout << i->first << ": " << i->second  << std::endl;

	}*/
	return 0;
}


