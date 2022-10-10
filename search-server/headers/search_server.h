#pragma once
#include <vector>
#include <string>
#include <list>
#include <set>
#include <tuple>
#include <numeric>
#include <map>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <functional>
#include <execution>
#include <typeinfo>
#include <string_view>
#include <type_traits>
#include <future>

#include "concurrent_map.h"
#include "document.h"

using namespace std::string_literals;

const unsigned MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;

class SearchServer{
public:
	SearchServer();

	SearchServer(const std::string& stopWordsContainer);
	SearchServer(std::string_view stopWordsContainer);	

	template<typename Container>
	SearchServer(const Container& stopWordsContainer);

	void AddDocument(int documentId, std::string_view document, DocumentStatus status, const std::vector<int>& docRating);

	template <typename Predicat>
	std::vector<Document> FindTopDocuments(std::string_view rawQuery, Predicat filter)const;
	std::vector<Document> FindTopDocuments(std::string_view rawQuery, DocumentStatus status)const;
	std::vector<Document> FindTopDocuments(std::string_view rawQuery)const;

	template <typename Predicat>
	std::vector<Document> FindTopDocumentsParallel(std::string_view rawQuery, Predicat filter)const;

	template <typename Execution, typename Predicat>
	std::vector<Document> FindTopDocuments(const Execution& policy, std::string_view rawQuery, Predicat filter)const;
	template <typename Execution>
	std::vector<Document> FindTopDocuments(const Execution& policy, std::string_view rawQuery, DocumentStatus status)const;
	template <typename Execution>
	std::vector<Document> FindTopDocuments(const Execution& policy, std::string_view rawQuery)const;

	

	std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::execution::parallel_policy& _Ex, std::string_view rawQuery, int documentId);
	std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::execution::sequenced_policy& _Ex, std::string_view rawQuery, int documentId);
	std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::string_view rawQuery, int documentId)const;
	
	std::set<int>::const_iterator begin()const;
	std::set<int>::const_iterator end()const;
	unsigned GetDocumentCount()const;
	const std::map<std::string_view, double>& GetWordFrequencies(int documentId)const;

	template<typename Execution>
	void RemoveDocument(Execution&& _Ex, int documentId);

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
		std::vector<std::string_view> plusWords;
		std::vector<std::string_view> minusWords;
	};
	struct QueryWord {
		std::string_view data;
		bool isMinus;
		bool isStop;
	};
	bool CheckWord(const std::string& word)const;
	void CheckDocumentId(int documentId)const;
	static int ComputeAverageRating(const std::vector<int>& ratings);
	std::vector<std::string_view> SplitIntoWordsNoStop(std::string_view text, const std::set<std::string>& stopWords)const;
	bool IsStopWord(const std::string& word)const;
	Query ParseQuery(std::string_view text)const;
	QueryWord ParseQueryWord(std::string_view word)const;
	template <typename Predicat>
	std::vector<Document> FindAllDocuments(const Query& queryWords, Predicat filter)const;
	template <typename Predicat>
	std::vector<Document> FindAllDocumentsParallel(const Query& queryWords, Predicat filter)const;
};

template<typename Container>
SearchServer::SearchServer(const Container& stopWordsContainer){
	for (std::string_view wordView : stopWordsContainer) {
		if (!CheckWord(static_cast<std::string>(wordView))) {
			throw std::invalid_argument("stop word contains a wrong character");
		}
		stopWords.insert(static_cast<std::string>(wordView));
	}
}

template <typename Predicat>
std::vector<Document> SearchServer::FindTopDocuments(std::string_view rawQuery, Predicat filter)const{
	Query queryWords = ParseQuery(rawQuery);	

	std::sort(queryWords.plusWords.begin(), queryWords.plusWords.end());
	std::sort(queryWords.minusWords.begin(), queryWords.minusWords.end());

	auto lastMinus = std::unique(queryWords.minusWords.begin(), queryWords.minusWords.end());
	queryWords.minusWords.erase(lastMinus, queryWords.minusWords.end());

	auto lastPlus = std::unique(queryWords.plusWords.begin(), queryWords.plusWords.end());
	queryWords.plusWords.erase(lastPlus, queryWords.plusWords.end());

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
std::vector<Document>  SearchServer::FindTopDocumentsParallel(std::string_view rawQuery, Predicat filter)const {
	Query queryWords = ParseQuery(rawQuery);


	std::future<void> ps = std::async(std::sort<decltype(queryWords.plusWords.begin())>, queryWords.plusWords.begin(), queryWords.plusWords.end());
	std::future<void> ms = std::async(std::sort<decltype(queryWords.minusWords.begin())>, queryWords.minusWords.begin(), queryWords.minusWords.end());
	ps.get();
	ms.get();
	
	std::future<std::vector<std::string_view>::iterator> lastMinus = std::async(std::unique<decltype(queryWords.minusWords.begin())>, queryWords.minusWords.begin(), queryWords.minusWords.end());
	std::future<std::vector<std::string_view>::iterator> lastPlus = std::async(std::unique<decltype(queryWords.plusWords.begin())>, queryWords.plusWords.begin(), queryWords.plusWords.end());
	
	std::future<void> pe = std::async([&queryWords, &lastPlus]{ queryWords.plusWords.erase(lastPlus.get(), queryWords.plusWords.end()); });
	std::future<void> me = std::async([&queryWords, &lastMinus] { queryWords.minusWords.erase(lastMinus.get(), queryWords.minusWords.end()); });
	pe.get();
	me.get();

	std::vector<Document> allDoc = FindAllDocumentsParallel(queryWords, filter);
	std::sort(allDoc.begin(), allDoc.end(), [](const Document& lhs, const Document& rhs) {
		if (std::abs(lhs.relevance - rhs.relevance) < EPSILON) {
			return lhs.rating > rhs.rating;
		}
		return lhs.relevance > rhs.relevance;
		});
	if (allDoc.size() > MAX_RESULT_DOCUMENT_COUNT) {
		allDoc.resize(MAX_RESULT_DOCUMENT_COUNT);
	}
	return allDoc;
}

template <typename Execution, typename Predicat>
std::vector<Document>  SearchServer::FindTopDocuments(const Execution& policy, std::string_view rawQuery, Predicat filter)const{
	if constexpr (std::is_same_v<Execution, std::execution::sequenced_policy>){
		return FindTopDocuments(rawQuery, filter);
	}
	return FindTopDocumentsParallel(rawQuery, filter);
}

template <typename Execution>
std::vector<Document>  SearchServer::FindTopDocuments(const Execution& policy, std::string_view rawQuery, DocumentStatus status)const{
	if constexpr (std::is_same_v<Execution, std::execution::sequenced_policy>) {
		return FindTopDocuments(rawQuery, status);
	}
	return FindTopDocuments(policy, rawQuery, [status](int documentId, DocumentStatus documentStatus, int rating) {
		return documentStatus == status;
	});
	
}

template <typename Execution>
std::vector<Document>  SearchServer::FindTopDocuments(const Execution& policy, std::string_view rawQuery)const{
	if constexpr (std::is_same_v<Execution, std::execution::sequenced_policy>) {
		return FindTopDocuments(rawQuery);
	}
	return FindTopDocuments(policy, rawQuery, DocumentStatus::ACTUAL);
}

template <typename Predicat>
std::vector<Document> SearchServer::FindAllDocuments(const Query& queryWords, Predicat filter)const{
	std::vector<Document> matched_documents;
	std::map<int, double> documentToRelevance;
	for(std::string_view word : queryWords.plusWords){		
		if(documents.find(static_cast<std::string>(word)) != documents.end()){
			double idf = log(GetDocumentCount() * 1.0 /  documents.at(static_cast<std::string>(word)).size());
			for(const auto& [documentId, documentTf] : documents.at(static_cast<std::string>(word))){
				if(filter(documentId, documentsRatingStatus.at(documentId).status, documentsRatingStatus.at(documentId).rating)){
					double tdIdf = idf * documentTf;
					documentToRelevance[documentId] += tdIdf;
				}
			}
		}
	}
	for(std::string_view word : queryWords.minusWords){		
		if(documents.find(static_cast<std::string>(word)) != documents.end()){
			for(const auto& [documentId, documentTf]: documents.at(static_cast<std::string>(word))){
				documentToRelevance.erase(documentId);
			}
		}
	}
	for(const auto& [id, relevance]: documentToRelevance){
		matched_documents.push_back({id, relevance, documentsRatingStatus.at(id).rating});
	}
	return matched_documents;
}


template <typename Predicat>
std::vector<Document> SearchServer::FindAllDocumentsParallel(const Query& queryWords, Predicat filter)const {
	std::vector<Document> matched_documents;
	matched_documents.reserve(documents.size());
	std::map<int, double> documentToRelevance;

	int thread_count = 8;
	
	ConcurrentMap<int, double> cm(thread_count);

	auto relevanceHandler = [&](std::vector<std::string_view>::const_iterator begin, std::vector<std::string_view>::const_iterator end) {
		std::for_each(begin, end, [&](std::string_view word) {
			if (documents.find(static_cast<std::string>(word)) != documents.end()) {
				double idf = log(GetDocumentCount() * 1.0 / documents.at(static_cast<std::string>(word)).size());
				for (const auto& [documentId, documentTf] : documents.at(static_cast<std::string>(word))) {
					if (filter(documentId, documentsRatingStatus.at(documentId).status, documentsRatingStatus.at(documentId).rating)) {
						double tdIdf = idf * documentTf;
						cm[documentId].tdIdf += tdIdf;
					}
				}
			}
		});
	};

	auto beginW = queryWords.plusWords.begin();
	auto endW = queryWords.plusWords.end();
	auto middleLeftW = beginW;
	std::vector<std::future<void>> relevanceFutures;
	for (int i = thread_count; i > 0; --i){
		if (i < thread_count) {
			beginW = middleLeftW;
		}

		if (i == 1){
			middleLeftW = endW;
		}else {
			middleLeftW = beginW + ((endW - middleLeftW) / i);
		}		
		relevanceFutures.push_back(std::async(relevanceHandler, beginW, middleLeftW));
	}

	for (std::future<void>& relevanceResult : relevanceFutures) {
		relevanceResult.get();
	}

	auto& documentsList = cm.BuildOrdinaryMap();

	std::for_each(queryWords.minusWords.begin(), queryWords.minusWords.end(), [&](std::string_view word) {
		if (documents.find(static_cast<std::string>(word)) != documents.end()) {
			std::for_each(documents.at(static_cast<std::string>(word)).begin(), documents.at(static_cast<std::string>(word)).end(), [&](const auto& docInner) {
				std::for_each(documentsList.begin(), documentsList.end(), [&](auto& item) {
					std::lock_guard g(item.mutex);
					item.data.erase(docInner.first);
				});
			});
		}
	});


	for (auto& item : documentsList) {
		std::for_each(item.data.begin(), item.data.end(), [&](const auto& itemInner) {
			Document* destination;
			{
				std::lock_guard guard(item.mutex);
				destination = &matched_documents.emplace_back();
			}
			*destination = { itemInner.first, itemInner.second, documentsRatingStatus.at(itemInner.first).rating };
		});		
	}
	
	return matched_documents;
}

template<typename Execution>
void SearchServer::RemoveDocument(Execution&& _Ex, int documentId) {
	if (documentsIds.count(documentId) > 0) {
		const auto wordFreqPointer = &(wordFreq[documentId]);
		std::vector<const std::string*> words(wordFreqPointer->size());
		std::transform(_Ex, wordFreqPointer->begin(), wordFreqPointer->end(), words.begin(), [&](const auto& pair) {
			return &(pair.first);
		});
		std::for_each(_Ex, words.begin(), words.end(), [&](const std::string* word) {
			if (documents.at(*word).count(documentId) > 0) {
				documents.at(*word).erase(documentId);
			}
		});
		documentsIds.erase(documentId);
	}
}
