#include "headers/document.h"

std::ostream& operator<<(std::ostream& os, const Document& document){
	os << "{ document_id = " << document.id << ", relevance = " << document.relevance << ", rating = " << document.rating << " }";
	return os;
}


