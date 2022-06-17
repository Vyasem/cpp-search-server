#pragma once
#include <iostream>


const unsigned MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;

enum class DocumentStatus{
	ACTUAL,
	IRRELEVANT,
	BANNED,
	REMOVED
};

struct Document{
	Document() = default;
	Document(int id_, double relevance_, int rating_):id(id_), relevance(relevance_), rating(rating_){}
	int id = 0;
	double relevance = 0.0;
	int rating = 0;
};

std::ostream& operator<<(std::ostream& os, const Document& document);
