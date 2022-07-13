#include <stdexcept>
#include "headers/string_processing.h"
#include "headers/search_server.h"

SearchServer::SearchServer(){}

SearchServer::SearchServer(const std::string& stopWordsContainer):SearchServer(SplitIntoWords(stopWordsContainer)){}

void SearchServer::AddDocument(int documentId, const std::string& document, DocumentStatus status, const std::vector<int>& docRating){
	CheckDocumentId(documentId);
	const std::vector<std::string> words = SplitIntoWordsNoStop(document, stopWords);
	documentsIds.insert(documentId);
	int size = words.size();
	double tf = 1.0 / size;

	for(const std::string& word: words){
		tf += documents[word][documentId];
		documents[word][documentId] = tf;
		wordFreq[documentId][word] = tf;
	}
 	documentsRatingStatus[documentId].rating = ComputeAverageRating(docRating);
	documentsRatingStatus[documentId].status = status;
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& rawQuery, DocumentStatus status)const{
	return FindTopDocuments(rawQuery, [status](int documentId, DocumentStatus documentStatus, int rating){
		return documentStatus == status;
	});
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& rawQuery)const{
	return FindTopDocuments(rawQuery, DocumentStatus::ACTUAL);
}

std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& rawQuery, int documentId)const{
	Query queryWords = ParseQuery(rawQuery);
	std::vector<std::string> findWords;
	DocumentStatus status = documentsRatingStatus.at(documentId).status;
	std::tuple<std::vector<std::string>, DocumentStatus> result;
	for(const std::string& word: queryWords.minusWords){
		if(documents.count(word) && documents.at(word).count(documentId)){
			result = {findWords, status};
			return result;
		}
	}

	for(const std::string& word: queryWords.plusWords){
		if(documents.count(word) && documents.at(word).count(documentId)){
			findWords.push_back(word);
		}
	}

	result = {findWords, status};
	return result;
}

unsigned SearchServer::GetDocumentCount()const{
	return documentsIds.size();
}

std::set<int>::const_iterator SearchServer::begin()const{
	return documentsIds.begin();
}

std::set<int>::const_iterator SearchServer::end()const{
	return documentsIds.end();
}
const std::map<std::string, double>& SearchServer::GetWordFrequencies(int documentId)const{
	const static std::map<std::string, double> wordFreqEmpty;
	if(documentsIds.count(documentId) == 0){
		return wordFreqEmpty;
	}
	return wordFreq.at(documentId);

};
void SearchServer::RemoveDocument(int documentId){
	if(documentsIds.count(documentId) > 0){
		for(const auto& [word, tf]: wordFreq.at(documentId)){
			if(documents.at(word).count(documentId) > 0){
				documents.at(word).erase(documentId);
			}
		}
		documentsIds.erase(documentId);
	}
}
bool SearchServer::CheckWord(const std::string& word)const{
	for(const char ch: word){
		int code = int(ch);
		if(code >= 0 && code < 32){
			return false;
		}
	}
	return true;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings){
	if(ratings.size() == 0){
		return 0;
	}
	return std::accumulate(ratings.begin(), ratings.end(), 0) / static_cast<int>(ratings.size());
}

void SearchServer::CheckDocumentId(int documentId)const{
	if(documentsRatingStatus.count(documentId)){
		throw std::invalid_argument("document id alredy exist");
	}

	if(documentId < 0){
		throw std::invalid_argument("document id less that 0");
	}
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text, const std::set<std::string>& stopWords)const{
	std::vector<std::string> words;
	for (const std::string& word : SplitIntoWords(text)){
		if(!CheckWord(word)){
			throw std::invalid_argument("the word "+word+" contains wrong symbol");
		}
		if (stopWords.count(word) == 0){
			words.push_back(word);
		}
	}
	return words;
}

bool SearchServer::IsStopWord(const std::string& word)const{
	return stopWords.count(word);
}

SearchServer::Query SearchServer::ParseQuery(const std::string& text)const{
	Query query;
	for (const std::string& word : SplitIntoWords(text)){
		const QueryWord queryWord = ParseQueryWord(word);
		if(!queryWord.isStop){
			if(queryWord.isMinus){
				query.minusWords.insert(queryWord.data);
			}else{
				query.plusWords.insert(queryWord.data);
			}
		}
	}
	return query;
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string word)const{
	unsigned size = word.size();
	if(word[0] == '-' && (size == 1 || word[1] == '-' || word[1] == ' ')){
		throw std::invalid_argument("query word contains extra -");
	}

	if(!CheckWord(word)){
		throw std::invalid_argument("query word contains a wrong character");
	}

	bool isMinus = false;
	if(word[0] == '-'){
		isMinus = true;
		word = word.substr(1);
	}

	return {
		word,
		isMinus,
		IsStopWord(word)
	};
}
