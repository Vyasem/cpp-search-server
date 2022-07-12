#include "headers/search_server.h"
#include "headers/remove_duplicates.h"

void RemoveDuplicates(SearchServer& searchServer){
	for(const auto& [key, value]: searchServer.documentsHash){
		unsigned idsSize = value.size();
		if(idsSize > 1){
			bool continueId = true;
			for(const int id: value){
				if(continueId){
					continueId = false;
					continue;
				}
				searchServer.RemoveDocument(id);
				std::cout << "Found duplicate document id " << id << "\n";

			}
			searchServer.documentsHash.at(key).clear();
		}
	}
}
