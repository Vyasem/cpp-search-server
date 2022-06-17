#pragma once
#include <iostream>
#include <string>
#include <stdexcept>
#include <iterator>
#include "document.h"
#include "paginator.h"

void PrintDocument(const Document& document);
void PrintMatchDocumentResult(int document_id, const std::vector<std::string>& words, DocumentStatus status);
std::ostream& operator<<(std::ostream& os, const Document& document);
template <typename Iterator>
std::ostream& operator<<(std::ostream& os, IteratorRange<Iterator> range){
	for(auto it = range.begin(); it != range.end(); ++it){
		os << *it;
	}
	return os;
}

