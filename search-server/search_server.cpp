#include <stdexcept>
#include "headers/string_processing.h"
#include "headers/search_server.h"

SearchServer::SearchServer(){}

SearchServer::SearchServer(const std::string& stopWords):SearchServer(SplitIntoWords(stopWords)){}

void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& docRating){
	checkDocumentId(document_id);
	const std::vector<std::string> words = SplitIntoWordsNoStop(document, stop_words);
	documentsIds.insert(document_id);
	int size = words.size();
	double tf = 1.0 / size;

	std::set<std::string> uniqueWords;
	for(const std::string& word: words){
		tf += documents[word][document_id];
		documents[word][document_id] = tf;
		wordFreq[document_id][word] = tf;
		uniqueWords.insert(word);
	}

	int stringHash = 0;
	for(const std::string uWord: uniqueWords){
		stringHash += (std::hash<std::string>{}(uWord) % 65537);
	}
	documentsHash[document_id] = stringHash;
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

std::set<int>::const_iterator SearchServer::begin()const{
	return documentsIds.begin();
};

std::set<int>::const_iterator SearchServer::end()const{
	return documentsIds.end();
};
const std::map<std::string, double>& SearchServer::GetWordFrequencies(int document_id)const{
	const static std::map<std::string, double> wordFreqEmpty;
	if(documentsIds.count(document_id) == 0){
		return wordFreqEmpty;
	}
	return wordFreq.at(document_id);

};
void SearchServer::RemoveDocument(int document_id){
	if(documentsIds.count(document_id) > 0){
		for(const auto& [word, tf]: wordFreq.at(document_id)){
			if(documents.at(word).count(document_id) > 0){
				documents.at(word).erase(document_id);
			}
		}
		documentsIds.erase(document_id);
	}
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

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings){
	if(ratings.size() == 0){
		return 0;
	}
	return std::accumulate(ratings.begin(), ratings.end(), 0) / static_cast<int>(ratings.size());
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
