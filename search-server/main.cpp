#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>
#include <map>
#include <cmath>
#include <tuple>

unsigned MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;
using namespace std::string_literals;

enum class DocumentStatus{
	ACTUAL,
	IRRELEVANT,
	BANNED,
	REMOVED
};

struct Document {
    int id;
    double relevance;
    int rating;
};

class SearchServer{
public:

	void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& docRating) {
	    const std::vector<std::string> words = SplitIntoWordsNoStop(document, stop_words);
	    int size = words.size();
        double tf = 1.0 / size;
	    for(const std::string& word: words){
	    	tf += documents[word][document_id];
	    	documents[word][document_id] = tf;
	    }
	    documentsRating[document_id] = ComputeAverageRating(docRating);
	    documentStatus[document_id] = status;
	    ++documentsCount;
	}

	void SetStopWords(const std::string& text) {
	    for (const std::string& word : SplitIntoWords(text)) {
	        stop_words.insert(word);
	    }
	}

	template <typename Predicat>
	std::vector<Document> FindTopDocuments(const std::string& raw_query, Predicat status)const {
		 const std::set<std::string> query_words = ParseQuery(raw_query, stop_words);
		 std::vector<Document> allDoc = FindAllDocuments(query_words, status);
		 sort(allDoc.begin(), allDoc.end(), [](const Document& lhs, const Document& rhs) {
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

	std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus status)const {
		 return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus documentStatus, int rating){
			 return documentStatus == status;
		 });
	}

	std::vector<Document> FindTopDocuments(const std::string& raw_query)const {
		 return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
	}

	std::tuple<	std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const{
		std::set<std::string> query_words = ParseQuery(raw_query, stop_words);
		std::vector<std::string> findWords;
		DocumentStatus status = documentStatus.at(document_id);
		for(const std::string& word: query_words){
			if(word[0] == '-'){
				std::string newWord = word.substr(1);
				if(documents.count(newWord) && documents.at(newWord).count(document_id)){
					return {findWords, status};
				}
			}

			if(documents.count(word) && documents.at(word).count(document_id)){
				findWords.push_back(word);
			}
		}

		return {findWords, status};
	}

	int GetDocumentCount(){
		return documentsCount;
	}
private:
	int documentsCount = 0;
	std::map<std::string, std::map<int, double>> documents;
	std::set<std::string> stop_words;
	std::map<int, int> documentsRating;
	std::map<int, DocumentStatus> documentStatus;

	static int ComputeAverageRating(const std::vector<int>& ratings) {
		int ratingSumm = 0;
		for(const int& value: ratings){
			ratingSumm += value;
		}
		//static_cast позволяет привести значение к типу int
		//без использования дополнительной переменной
		return ratingSumm / static_cast<int>(ratings.size());
	}

	std::vector<std::string> SplitIntoWordsNoStop(const std::string& text, const std::set<std::string>& stop_words) const {
		std::vector<std::string> words;
	    for (const std::string& word : SplitIntoWords(text)) {
	        if (stop_words.count(word) == 0) {
	            words.push_back(word);
	        }
	    }
	    return words;
	}

	std::vector<std::string> SplitIntoWords(const std::string& text)const {
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

	template <typename Predicat>
	std::vector<Document> FindAllDocuments(const std::set<std::string>& query_words, Predicat status) const{
		std::vector<Document> matched_documents;
		std::map<int, double> documentToRelevance;
		for(const std::string& word: query_words){
			if(word[0] == '-'){
				continue;
			}

			if(documents.find(word) != documents.end()){
				double idf = log(documentsCount * 1.0 /  documents.at(word).size());
				for(const auto& [documentId, documentTf] : documents.at(word)){
					if(status(documentId, documentStatus.at(documentId), documentsRating.at(documentId))){
						double td_idf = idf * documentTf;
						documentToRelevance[documentId] += td_idf;
					}

				}
			}
		}

		for(const std::string& word: query_words){
			if(word[0] != '-'){
				continue;
			}

			std::string newWord = word.substr(1);

			if(documents.find(newWord) != documents.end()){
				for(const auto& [documentId, documentTf]: documents.at(newWord)){
					documentToRelevance.erase(documentId);
				}
			}
		}

		for(const auto& [id, relevance]: documentToRelevance){
            matched_documents.push_back({id, relevance, documentsRating.at(id)});
		}

		return matched_documents;
	}

	std::set<std::string> ParseQuery(const std::string& text, const std::set<std::string>& stop_words) const{
		std::set<std::string> query_words;
	    for (const std::string& word : SplitIntoWordsNoStop(text, stop_words)) {
	        query_words.insert(word);
	    }
	    return query_words;
	}
};

void PrintDocument(const Document& document) {
	std::cout << "{ document_id = " << document.id << ", relevance = " << document.relevance << ", rating = " << document.rating << " }" << std::endl;
}

void PrintMatchDocumentResult(int document_id, const std::vector<std::string>& words, DocumentStatus status) {
	std::cout << "{ document_id = " << document_id << ", status = " << static_cast<int>(status) << ", words =";
	for(const std::string& word: words){
		std::cout << ' ' << word;
	}
	std::cout << "}" << std::endl;
}

int main() {
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);

    search_server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
    search_server.AddDocument(3, "ухоженный скворец евгений"s,         DocumentStatus::BANNED, {9});

    std::cout << "ACTUAL by default:"s << std::endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s)) {
        PrintDocument(document);
    }

    std::cout << "BANNED:"s << std::endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED)) {
        PrintDocument(document);
    }

    std::cout << "Even ids:"s << std::endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })) {
        PrintDocument(document);
    }

    return 0;
}

