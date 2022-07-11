#include "headers/search_server.h"
#include "headers/remove_duplicates.h"
void RemoveDuplicates(SearchServer& search_server){
	std::set<int> removeIds;
	for(auto i = search_server.documentsHash.begin(); i != search_server.documentsHash.end(); ++i){
		for(auto j = i; j != search_server.documentsHash.end(); ++j){
			if(i->second == j->second && i->first != j->first){
				int removeId = (i->first >  j->first) ? i->first : j->first;
				removeIds.insert(removeId);
			}
		}
	}
	for(const int id: removeIds){
		search_server.RemoveDocument(id);
		search_server.documentsHash.erase(id);
		std::cout << "Found duplicate document id " << id << "\n";
	}
}
