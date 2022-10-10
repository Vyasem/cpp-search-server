#include <stdexcept>
#include <execution>
#include "headers/string_processing.h"
#include "headers/search_server.h"

SearchServer::SearchServer() {}

SearchServer::SearchServer(const std::string& stopWordsContainer) :SearchServer(SplitIntoWords(stopWordsContainer)) {}
SearchServer::SearchServer(std::string_view stopWordsContainer) :SearchServer(SplitIntoWords(stopWordsContainer)) {}

void SearchServer::AddDocument(int documentId, std::string_view document, DocumentStatus status, const std::vector<int>& docRating) {
	CheckDocumentId(documentId);
	const std::vector<std::string_view> words = SplitIntoWordsNoStop(document, stopWords);
	documentsIds.insert(documentId);
	int size = words.size();
	double tf = 1.0 / size;

	for (std::string_view word : words) {
		documents[static_cast<std::string>(word)][documentId] += tf;
		wordFreq[documentId][static_cast<std::string>(word)] += tf;
	}

	documentsRatingStatus[documentId].rating = ComputeAverageRating(docRating);
	documentsRatingStatus[documentId].status = status;
}

std::vector<Document> SearchServer::FindTopDocuments(std::string_view rawQuery, DocumentStatus status)const {
	return FindTopDocuments(rawQuery, [status](int documentId, DocumentStatus documentStatus, int rating) {
		return documentStatus == status;
	});
}

std::vector<Document> SearchServer::FindTopDocuments(std::string_view rawQuery)const {
	return FindTopDocuments(rawQuery, DocumentStatus::ACTUAL);
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::sequenced_policy&, std::string_view rawQuery, int documentId) {
	return MatchDocument(rawQuery, documentId);
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::parallel_policy&, std::string_view rawQuery, int documentId) {
	Query queryWords = ParseQuery(rawQuery);
	DocumentStatus status = documentsRatingStatus.at(documentId).status;
	std::vector<std::string_view> findWords(queryWords.plusWords.size());
	bool exit = false;
	std::for_each(std::execution::par, queryWords.minusWords.begin(), queryWords.minusWords.end(), [&](std::string_view word) {
		if (!exit && documents.count(static_cast<std::string>(word)) && documents.at(static_cast<std::string>(word)).count(documentId)) {
			exit = true;
		}
		});

	if (exit) {
		return { {}, status };
	}
	auto resCopy = std::copy_if(std::execution::par, queryWords.plusWords.begin(), queryWords.plusWords.end(), findWords.begin(), [&](std::string_view word) {
		return (documents.count(static_cast<std::string>(word)) && documents.at(static_cast<std::string>(word)).count(documentId));
		});
	std::sort(std::execution::par, findWords.begin(), resCopy);
	auto lastPlus = std::unique(findWords.begin(), resCopy);
	findWords.erase(lastPlus, findWords.end());
	return { findWords, status };
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(std::string_view rawQuery, int documentId)const {
	Query queryWords = ParseQuery(rawQuery);
	std::sort(queryWords.plusWords.begin(), queryWords.plusWords.end());
	auto lastPlus = std::unique(queryWords.plusWords.begin(), queryWords.plusWords.end());
	queryWords.plusWords.erase(lastPlus, queryWords.plusWords.end());
	std::vector<std::string_view> findWords;
	DocumentStatus status = documentsRatingStatus.at(documentId).status;
	for (std::string_view word : queryWords.minusWords) {
		if (documents.count(static_cast<std::string>(word)) && documents.at(static_cast<std::string>(word)).count(documentId)) {
			return { {}, status };
		}
	}

	for (std::string_view word : queryWords.plusWords) {
		if (documents.count(static_cast<std::string>(word)) && documents.at(static_cast<std::string>(word)).count(documentId)) {
			findWords.push_back(word);
		}
	}
	return { findWords, status };
}

unsigned SearchServer::GetDocumentCount()const {
	return documentsIds.size();
}

std::set<int>::const_iterator SearchServer::begin()const {
	return documentsIds.begin();
}

std::set<int>::const_iterator SearchServer::end()const {
	return documentsIds.end();
}
const std::map<std::string_view, double>& SearchServer::GetWordFrequencies(int documentId)const {
	static std::map<std::string_view, double> wordFreqRes;
	if (documentsIds.count(documentId) == 0) {
		return wordFreqRes;
	}

	wordFreqRes.insert(wordFreq.at(documentId).begin(), wordFreq.at(documentId).end());
	return wordFreqRes;

}
void SearchServer::RemoveDocument(int documentId) {
	if (documentsIds.count(documentId) > 0) {
		for (const auto& [word, tf] : wordFreq.at(documentId)) {
			if (documents.at(word).count(documentId) > 0) {
				documents.at(word).erase(documentId);
			}
		}
		documentsIds.erase(documentId);
	}
}
bool SearchServer::CheckWord(const std::string& word)const {
	for (const char ch : word) {
		int code = int(ch);
		if (code >= 0 && code < 32) {
			return false;
		}
	}
	return true;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
	if (ratings.size() == 0) {
		return 0;
	}
	return std::accumulate(ratings.begin(), ratings.end(), 0) / static_cast<int>(ratings.size());
}

void SearchServer::CheckDocumentId(int documentId)const {
	if (documentsRatingStatus.count(documentId)) {
		throw std::invalid_argument("document id alredy exist");
	}

	if (documentId < 0) {
		throw std::invalid_argument("document id less that 0");
	}
}

std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop(std::string_view text, const std::set<std::string>& stopWords)const {
	std::vector<std::string_view> words;
	for (std::string_view word : SplitIntoWords(text)) {
		if (!CheckWord(static_cast<std::string>(word))) {
			std::string wordPrint(word);
			throw std::invalid_argument("the word " + wordPrint + " contains wrong symbol");
		}
		if (stopWords.count(static_cast<std::string>(word)) == 0) {
			words.push_back(word);
		}
	}
	return words;
}

bool SearchServer::IsStopWord(const std::string& word)const {
	return stopWords.count(word);
}

SearchServer::Query SearchServer::ParseQuery(std::string_view text)const {
	Query query;
	for (std::string_view word : SplitIntoWords(text)) {
		const QueryWord queryWord = ParseQueryWord(word);
		if (!queryWord.isStop) {
			if (queryWord.isMinus) {
				query.minusWords.push_back(queryWord.data);
			}
			else {
				query.plusWords.push_back(queryWord.data);
			}
		}
	}
	return query;
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string_view word)const {
	unsigned size = word.size();
	if (word[0] == '-' && (size == 1 || word[1] == '-' || word[1] == ' ')) {
		throw std::invalid_argument("query word contains extra -");
	}

	if (!CheckWord(static_cast<std::string>(word))) {
		throw std::invalid_argument("query word contains a wrong character");
	}

	bool isMinus = false;
	if (word[0] == '-') {
		isMinus = true;
		word = word.substr(1);
	}

	return {
		word,
		isMinus,
		IsStopWord(static_cast<std::string>(word))
	};
}