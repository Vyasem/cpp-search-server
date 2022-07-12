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
		for (auto it = search_server.begin(); it != search_server.end(); ++it){
			const int document_id = *it;
			const auto [words, status] = search_server.MatchDocument(query, document_id);
			PrintMatchDocumentResult(document_id, words, status);
		}
	} catch (const std::exception& e){
		std::cerr << "Ошибка матчинга документов на запрос " << query << ": " << e.what() << std::endl;
		abort();
	}
}

void testRemoveDuplicate(){
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
		LOG_DURATION("Remove Duplicates");
		RemoveDuplicates(search_server);
		std::cout << "After duplicates removed: "s << search_server.GetDocumentCount() << std::endl;
}

void testRemoveDocument(){
		std::string stopWords = "and with";
		SearchServer search_server(stopWords);
		AddDocument(search_server, 1, "funny pet and nasty rat", DocumentStatus::ACTUAL, {7, 2, 7});
		AddDocument(search_server, 2, "funny pet with curly hair", DocumentStatus::ACTUAL, {1, 2});
		AddDocument(search_server, 6, "funny pet and not very nasty rat", DocumentStatus::ACTUAL, {1, 2});
		AddDocument(search_server, 8, "pet with rat and rat and rat", DocumentStatus::ACTUAL, {1, 2});
		AddDocument(search_server, 9, "nasty rat with curly hair", DocumentStatus::ACTUAL, {1, 2});

		std::cout << "Before document removed: \n";
		for(const int doc_id: search_server){
			std::cout << doc_id << ' ';
		}
		std::cout << "\n";
		search_server.RemoveDocument(6);
		std::cout << "After document removed: \n";
		for(const int doc_id: search_server){
			std::cout << doc_id << ' ';
		}
		std::cout << "\n";
}

int main(){
	testRemoveDuplicate();
	testRemoveDocument();
	return 0;
}


