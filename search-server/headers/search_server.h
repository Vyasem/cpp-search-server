#pragma once
#include <vector>
#include <string>
#include <set>
#include <tuple>
#include <numeric>
#include <map>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <functional>

#include "document.h"

const unsigned MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;

class SearchServer{
public:
	inline static constexpr int INVALID_DOCUMENT_ID = -1;
	//Контейнер который хранит хэши уникальных слов документа
	std::map<int, int> documentsHash;
	SearchServer();
	SearchServer(const std::string& stopWords);
	template<typename Container>
	SearchServer(const Container& stopWords);
	void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& docRating);
	template <typename Predicat>
	std::vector<Document> FindTopDocuments(const std::string& raw_query, Predicat filter)const;
	std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus status)const;
	std::vector<Document> FindTopDocuments(const std::string& raw_query)const;
	std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id)const;
	std::vector<int>::const_iterator begin()const;
	std::vector<int>::const_iterator end()const;
	unsigned GetDocumentCount()const;
	const std::map<std::string, double>& GetWordFrequencies(int document_id)const;
	void RemoveDocument(int document_id);
private:
	std::vector<int> documentsIds;
	std::map<std::string, std::map<int, double>> documents;
	std::set<std::string> stop_words;
	std::map<int, int> documentsRating;
	std::map<int, DocumentStatus> documentStatus;
	struct Query {
		std::set<std::string> plus_words;
		std::set<std::string> minus_words;
	};
	struct QueryWord {
		std::string data;
		bool is_minus;
		bool is_stop;
	};
	bool checkWord(const std::string& word)const;
	void checkDocumentId(int document_id)const;
	static int ComputeAverageRating(const std::vector<int>& ratings);
	std::vector<std::string> SplitIntoWordsNoStop(const std::string& text, const std::set<std::string>& stop_words)const;
	bool IsStopWord(const std::string& word)const;
	Query ParseQuery(const std::string& text)const;
	QueryWord ParseQueryWord(std::string word)const;
	template <typename Predicat>
	std::vector<Document> FindAllDocuments(const Query& query_words, Predicat filter)const;
};

template<typename Container>
SearchServer::SearchServer(const Container& stopWords){
	for (const std::string& word : stopWords){
		if(!checkWord(word)){
			throw std::invalid_argument("stop word contains a wrong character");
		}
		stop_words.insert(word);
	}
}

template <typename Predicat>
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, Predicat filter)const{
	const Query query_words = ParseQuery(raw_query);
	std::vector<Document> allDoc = FindAllDocuments(query_words, filter);
	std::sort(allDoc.begin(), allDoc.end(), [](const Document& lhs, const Document& rhs){
		if(std::abs(lhs.relevance - rhs.relevance) < EPSILON){
			return lhs.rating > rhs.rating;
		}
		return lhs.relevance > rhs.relevance;
	});
	if(allDoc.size() > MAX_RESULT_DOCUMENT_COUNT){
		allDoc.resize(MAX_RESULT_DOCUMENT_COUNT);
	}
	return allDoc;
}

template <typename Predicat>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query_words, Predicat filter)const{
	std::vector<Document> matched_documents;
	std::map<int, double> documentToRelevance;
	for(const std::string& word: query_words.plus_words){
		if(documents.find(word) != documents.end()){
			double idf = log(GetDocumentCount() * 1.0 /  documents.at(word).size());
			for(const auto& [documentId, documentTf] : documents.at(word)){
				if(filter(documentId, documentStatus.at(documentId), documentsRating.at(documentId))){
					double td_idf = idf * documentTf;
					documentToRelevance[documentId] += td_idf;
				}
			}
		}
	}
	for(const std::string& word: query_words.minus_words){
		if(documents.find(word) != documents.end()){
			for(const auto& [documentId, documentTf]: documents.at(word)){
				documentToRelevance.erase(documentId);
			}
		}
	}
	for(const auto& [id, relevance]: documentToRelevance){
		matched_documents.push_back({id, relevance, documentsRating.at(id)});
	}
	return matched_documents;
}
