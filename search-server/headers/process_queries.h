#pragma once

#include <string>
#include <vector>
#include <execution>
#include <tuple>
#include "search_server.h"
#include "document.h"

std::vector<std::vector<Document>> ProcessQueries(const SearchServer& search_server, const std::vector<std::string>& queries);
std::vector<Document> ProcessQueriesJoined(const SearchServer& search_server, const std::vector<std::string>& queries);

