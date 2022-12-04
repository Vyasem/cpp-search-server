#include <iostream>
#include <string>
#include <vector>
#include <string_view>
#include <algorithm>
#include <execution>
#include <future>

#include "headers/search_generator.h"
#include "headers/search_server.h"
#include "headers/paginator.h"
#include "headers/remove_duplicates.h"
#include "headers/request_queue.h"
#include "headers/process_queries.h"
#include "headers/test.h"


using namespace std::string_literals;

void PrintDocument(const Document& document){
	std::cout << document << std::endl;
}

void PrintMatchDocumentResult(int document_id, const std::vector<std::string_view>& words, DocumentStatus status) {
	std::cout << "{ document_id = " << document_id << ", status = " << static_cast<int>(status) << ", words =";
	for(std::string_view word: words){
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

std::vector<std::string> SplitIntoWordsView(std::string_view text) {
	std::vector<std::string> words;
	std::string word;
	for (const char c : text) {
		if (c == ' ') {
			if (!word.empty()) {
				words.push_back(word);
				word.clear();
			}
		}
		else {
			word += c;
		}
	}
	if (!word.empty()) {
		words.push_back(word);
	}
	return words;
}

void FindTopTest() {
	using namespace std;
	SearchServer search_server("and with"s);
	int id = 0;
	for (
		const string& text : {
			"white cat and yellow hat"s,
			"curly cat curly tail"s,
			"nasty dog with big eyes"s,
			"nasty pigeon john"s,
		}
		) {
		search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
	}
	cout << "ACTUAL by default:"s << endl;
	// последовательная версия
	for (const Document& document : search_server.FindTopDocuments("curly nasty cat"s)) {
		PrintDocument(document);
	}
	cout << "BANNED:"s << endl;
	// последовательная версия
	for (const Document& document : search_server.FindTopDocuments(execution::seq, "curly nasty cat"s, DocumentStatus::BANNED)) {
		PrintDocument(document);
	}
	cout << "Even ids:"s << endl;
	// параллельная версия
	for (const Document& document : search_server.FindTopDocuments(execution::par, "curly nasty cat"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })) {
		PrintDocument(document);
	}
}

int main(){
	FindTopTest();
	return 0;
}  