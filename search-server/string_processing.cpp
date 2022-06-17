#include <iostream>
#include <string>
#include <stdexcept>
#include <iterator>
#include "headers/document.h"
#include "headers/paginator.h"

void PrintDocument(const Document& document){
	std::cout << "{ document_id = " << document.id << ", relevance = " << document.relevance << ", rating = " << document.rating << " }" << std::endl;
}

void PrintMatchDocumentResult(int document_id, const std::vector<std::string>& words, DocumentStatus status) {
	std::cout << "{ document_id = " << document_id << ", status = " << static_cast<int>(status) << ", words =";
	for(const std::string& word: words){
		std::cout << ' ' << word;
	}
	std::cout << "}" << std::endl;
}

std::ostream& operator<<(std::ostream& os, const Document& document){
	os << "{ document_id = " << document.id << ", relevance = " << document.relevance << ", rating = " << document.rating << " }";
	return os;
}
