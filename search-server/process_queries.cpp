#include <execution>
#include <vector>
#include <string>
#include <iterator>
#include <algorithm>
#include <iterator>
#include <tuple>
#include "headers/search_server.h"
#include "headers/process_queries.h"
#include "headers/document.h"


std::vector<std::vector<Document>> ProcessQueries(const SearchServer& search_server, const std::vector<std::string>& queries) {
	std::vector<std::vector<Document>> result(queries.size());
	std::transform(std::execution::par, queries.begin(), queries.end(), result.begin(), [&search_server](const std::string& query) {
		return search_server.FindTopDocuments(query);
	});
	return result;
}

std::vector<Document> ProcessQueriesJoined(const SearchServer& search_server, const std::vector<std::string>& queries) {
	std::vector<Document> result;
	for (const std::vector<Document>& arDoc : ProcessQueries(search_server, queries)){
		for (const Document& doc : arDoc){
			result.push_back(doc);
		}
	}
	return result;
}