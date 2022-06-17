#include <iostream>
#include <string>
#include <stdexcept>
#include "headers/document.h"
#include "headers/search_server.h"
#include "headers/string_processing.h"
#include "headers/read_input_functions.h"

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
