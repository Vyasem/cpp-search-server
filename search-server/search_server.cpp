#include<vector>
#include <string>
#include <set>
#include <tuple>
#include <numeric>
#include <map>
#include <cmath>
#include <stdexcept>
#include <algorithm>

#include "headers/search_server.h"

SearchServer::SearchServer(){}

SearchServer::SearchServer(const std::string& stopWords):SearchServer(SplitIntoWords(stopWords)){}

void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& docRating){
	checkDocumentId(document_id);
	const std::vector<std::string> words = SplitIntoWordsNoStop(document, stop_words);
	documentsIds.push_back(document_id);
	int size = words.size();
	double tf = 1.0 / size;
	for(const std::string& word: words){
		tf += documents[word][document_id];
		documents[word][document_id] = tf;
	}
	documentsRating[document_id] = ComputeAverageRating(docRating);
	documentStatus[document_id] = status;
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentStatus status)const{
	return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus documentStatus, int rating){
		return documentStatus == status;
	});
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query)const{
	return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query, int document_id)const{
	Query query_words = ParseQuery(raw_query);
	std::vector<std::string> findWords;
	DocumentStatus status = documentStatus.at(document_id);
	std::tuple<std::vector<std::string>, DocumentStatus> result;
	for(const std::string& word: query_words.minus_words){
		if(documents.count(word) && documents.at(word).count(document_id)){
			result = {findWords, status};
			return result;
		}
	}

	for(const std::string& word: query_words.plus_words){
		if(documents.count(word) && documents.at(word).count(document_id)){
			findWords.push_back(word);
		}
	}

	result = {findWords, status};
	return result;
}

unsigned SearchServer::GetDocumentCount()const{
	return documentsIds.size();
}

int SearchServer::GetDocumentId(int index)const{
	return documentsIds.at(index);
}


bool SearchServer::checkWord(const std::string& word)const{
	for(const char ch: word){
		int code = int(ch);
		if(code >= 0 && code < 32){
			return false;
		}
	}
	return true;
}

void SearchServer::checkDocumentId(int document_id)const{
	if(documentStatus.count(document_id)){
		throw std::invalid_argument("document id alredy exist");
	}

	if(document_id < 0){
		throw std::invalid_argument("document id less that 0");
	}
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text, const std::set<std::string>& stop_words)const{
	std::vector<std::string> words;
	for (const std::string& word : SplitIntoWords(text)){
		if(!checkWord(word)){
			throw std::invalid_argument("the word "+word+" contains wrong symbol");
		}
		if (stop_words.count(word) == 0){
			words.push_back(word);
		}
	}
	return words;
}

std::vector<std::string> SearchServer::SplitIntoWords(const std::string& text)const{
	std::vector<std::string> words;
	std::string word;
	for (const char c : text) {
		if (c == ' ') {
			if (!word.empty()) {
				words.push_back(word);
				word.clear();
			}
		} else {
			word += c;
		}
	}
	if (!word.empty()) {
		words.push_back(word);
	}
	return words;
}

bool SearchServer::IsStopWord(const std::string& word)const{
	return stop_words.count(word);
}

SearchServer::Query SearchServer::ParseQuery(const std::string& text)const{
	Query query;
	for (const std::string& word : SplitIntoWords(text)){
		const QueryWord queryWord = ParseQueryWord(word);
		if(!queryWord.is_stop){
			if(queryWord.is_minus){
				query.minus_words.insert(queryWord.data);
			}else{
				query.plus_words.insert(queryWord.data);
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

	if(!checkWord(word)){
		throw std::invalid_argument("query word contains a wrong character");
	}

	bool is_minus = false;
	if(word[0] == '-'){
		is_minus = true;
		word = word.substr(1);
	}

	return {
		word,
		is_minus,
		IsStopWord(word)
	};
}
