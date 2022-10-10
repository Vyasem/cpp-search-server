#include <map>
#include <string>
#include <set>
#include "headers/search_server.h"
#include "headers/remove_duplicates.h"

void RemoveDuplicates(SearchServer& searchServer){
	std::map<std::set<std::string_view>, int> words;
	std::vector<int> removeIds;
	for(const int docId : searchServer){
		std::set<std::string_view> key;
		for(const auto& [word, tf]: searchServer.GetWordFrequencies(docId)){
			key.insert(word);
		}

		int itemId = words[key];
		if(itemId > 0){
			if(itemId > docId){
				removeIds.push_back(itemId);
				words[key] = docId;
			}else{
				removeIds.push_back(docId);
			}
		}else{
			words[key] = docId;
		}
	}

	for(const int docId: removeIds){
		searchServer.RemoveDocument(docId);
		std::cout << "Found duplicate document id " << docId << "\n";
	}
}
