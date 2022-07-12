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
	std::map<std::set<std::string>, std::set<int>> documentsHash;
	SearchServer();
	SearchServer(const std::string& stopWordsContainer);
	template<typename Container>
	SearchServer(const Container& stopWordsContainer);
	void AddDocument(int documentId, const std::string& document, DocumentStatus status, const std::vector<int>& docRating);

	template <typename Predicat>
	std::vector<Document> FindTopDocuments(const std::string& rawQuery, Predicat filter)const;

	std::vector<Document> FindTopDocuments(const std::string& rawQuery, DocumentStatus status)const;

	std::vector<Document> FindTopDocuments(const std::string& rawQuery)const;

	std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& rawQuery, int documentId)const;
	std::set<int>::const_iterator begin()const;
	std::set<int>::const_iterator end()const;
	unsigned GetDocumentCount()const;
	const std::map<std::string, double>& GetWordFrequencies(int documentId)const;
	void RemoveDocument(int documentId);
private:
	struct RatingStatus{
		int rating;
		DocumentStatus status;
	};
	std::set<int> documentsIds;
	std::map<int, std::map<std::string, double>> wordFreq;
	std::map<std::string, std::map<int, double>> documents;
	std::set<std::string> stopWords;
	std::map<int, RatingStatus> documentsRatingStatus;
	struct Query {
		std::set<std::string> plusWords;
		std::set<std::string> minusWords;
	};
	struct QueryWord {
		std::string data;
		bool isMinus;
		bool isStop;
	};

	bool CheckWord(const std::string& word)const;
	void CheckDocumentId(int documentId)const;
	static int ComputeAverageRating(const std::vector<int>& ratings);
	std::vector<std::string> SplitIntoWordsNoStop(const std::string& text, const std::set<std::string>& stopWords)const;
	bool IsStopWord(const std::string& word)const;
	Query ParseQuery(const std::string& text)const;
	QueryWord ParseQueryWord(std::string word)const;
	template <typename Predicat>
	std::vector<Document> FindAllDocuments(const Query& queryWords, Predicat filter)const;
};

template<typename Container>
SearchServer::SearchServer(const Container& stopWordsContainer){
	for (const std::string& word : stopWordsContainer){
		if(!CheckWord(word)){
			throw std::invalid_argument("stop word contains a wrong character");
		}
		stopWords.insert(word);
	}
}

template <typename Predicat>
std::vector<Document> SearchServer::FindTopDocuments(const std::string& rawQuery, Predicat filter)const{
	const Query queryWords = ParseQuery(rawQuery);
	std::vector<Document> allDoc = FindAllDocuments(queryWords, filter);
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
std::vector<Document> SearchServer::FindAllDocuments(const Query& queryWords, Predicat filter)const{
	std::vector<Document> matched_documents;
	std::map<int, double> documentToRelevance;
	for(const std::string& word: queryWords.plusWords){
		if(documents.find(word) != documents.end()){
			double idf = log(GetDocumentCount() * 1.0 /  documents.at(word).size());
			for(const auto& [documentId, documentTf] : documents.at(word)){
				if(filter(documentId, documentsRatingStatus.at(documentId).status, documentsRatingStatus.at(documentId).rating)){
					double tdIdf = idf * documentTf;
					documentToRelevance[documentId] += tdIdf;
				}
			}
		}
	}
	for(const std::string& word: queryWords.minusWords){
		if(documents.find(word) != documents.end()){
			for(const auto& [documentId, documentTf]: documents.at(word)){
				documentToRelevance.erase(documentId);
			}
		}
	}
	for(const auto& [id, relevance]: documentToRelevance){
		matched_documents.push_back({id, relevance, documentsRatingStatus.at(id).rating});
	}
	return matched_documents;
}
