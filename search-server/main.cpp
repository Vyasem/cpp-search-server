#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>
#include <map>
#include <cmath>
#include <tuple>
#include <numeric>
#include <cassert>
#include <optional>
#include <stdexcept>

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
	Document() = default;
	Document(int id_, double relevance_, int rating_):id(id_), relevance(relevance_), rating(rating_){}
    int id = 0;
    double relevance = 0.0;
    int rating = 0;
};

class SearchServer{
public:
	inline static constexpr int INVALID_DOCUMENT_ID = -1;
	SearchServer(const std::string& stopWords){
		 for (const std::string& word : SplitIntoWords(stopWords)) {
			 for(const char ch: word){
				int code = int(ch);
				if(code >= 0 && code < 32){
					throw std::invalid_argument(" stop word contains a wrong character");
				}
			 }
			 stop_words.insert(word);
		}
	}

	SearchServer(const std::vector<std::string>& stopWords){
		for (const std::string& word : stopWords) {
			for(const char ch: word){
				int code = int(ch);
				if(code >= 0 && code < 32){
					throw std::invalid_argument(" stop word contains a wrong character");
				}
			 }
			stop_words.insert(word);
		}
	}
	SearchServer(const std::set<std::string>& stopWords): stop_words(stopWords){
		for(const std::string& word: stopWords){
			for(const char ch: word){
				int code = int(ch);
				if(code >= 0 && code < 32){
					throw std::invalid_argument(" stop word contains a wrong character");
				}
			 }
		 }
	}
	void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& docRating) {
		if(!checkDocument(document, document_id)){
			throw std::invalid_argument(" invalid add document");
		}
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

	template <typename Predicat>
	std::vector<Document> FindTopDocuments(const std::string& raw_query, Predicat filter)const {
		if(!checkQuery(raw_query)){
			throw std::invalid_argument(" a query string is invalid");
		}
		 const std::set<std::string> query_words = ParseQuery(raw_query, stop_words);
		 std::vector<Document> allDoc = FindAllDocuments(query_words, filter);
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

	std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const{
		if(!checkQuery(raw_query)){
			throw std::invalid_argument(" a query string is invalid");
		}
		std::set<std::string> query_words = ParseQuery(raw_query, stop_words);
		std::vector<std::string> findWords;
		DocumentStatus status = documentStatus.at(document_id);
		std::tuple<std::vector<std::string>, DocumentStatus> result;
		for(const std::string& word: query_words){
			if(word[0] == '-'){
				std::string newWord = word.substr(1);
				if(documents.count(newWord) && documents.at(newWord).count(document_id)){
					result = {findWords, status};
					return result;
				}
			}

			if(documents.count(word) && documents.at(word).count(document_id)){
				findWords.push_back(word);
			}
		}
		result = {findWords, status};
		return result;
	}

	int GetDocumentCount() const{
		return documentsCount;
	}

	int GetDocumentId(int index) const {
		if(index < 0 || index >= documentsCount){
			throw std::out_of_range("out of range");
		}
		return index;
	}

private:
	int documentsCount = 0;
	std::map<std::string, std::map<int, double>> documents;
	std::set<std::string> stop_words;
	std::map<int, int> documentsRating;
	std::map<int, DocumentStatus> documentStatus;

	bool checkQuery(const std::string& query)const{
		unsigned size = query.size();
		for(unsigned i = 0; i < size; ++i){
			int code = int(query[i]);
			if((query[i] == '-' && (i < (size - 1) && query[i+1] == '-')) || (code >= 0 && code < 32)){
				return false;
			}

			if(query[i] == '-' && (i == size - 1 ||  query[i+1] == ' ')){
				return false;
			}
		}
		return true;
	}

	bool checkDocument(const std::string& document, int document_id)const{
		unsigned size = document.size();
		if(documentStatus.count(document_id)){
			return false;
		}
		for(unsigned i = 0; i < size; ++i){
			int code = int(document[i]);
			if((code >= 0 && code < 32) || document_id < 0){
				return false;
			}

		}
		return true;
	}

	static int ComputeAverageRating(const std::vector<int>& ratings) {
		return std::accumulate(ratings.begin(), ratings.end(), 0) / static_cast<int>(ratings.size());
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
	std::vector<Document> FindAllDocuments(const std::set<std::string>& query_words, Predicat filter) const{
		std::vector<Document> matched_documents;
		std::map<int, double> documentToRelevance;
		for(const std::string& word: query_words){
			if(word[0] == '-'){
				continue;
			}

			if(documents.find(word) != documents.end()){
				double idf = log(documentsCount * 1.0 /  documents.at(word).size());
				for(const auto& [documentId, documentTf] : documents.at(word)){
					if(filter(documentId, documentStatus.at(documentId), documentsRating.at(documentId))){
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

void AddDocument(SearchServer& search_server, int document_id, const std::string& document, DocumentStatus status,
                 const std::vector<int>& ratings) {
    try {
        search_server.AddDocument(document_id, document, status, ratings);
    } catch (const std::exception& e) {
        std::cerr << "Ошибка добавления документа "s << document_id << ": "s << e.what() << std::endl;
    }
}

void FindTopDocuments(const SearchServer& search_server, const std::string& raw_query) {
	std::cout << "Результаты поиска по запросу: "s << raw_query << std::endl;
    try {
        for (const Document& document : search_server.FindTopDocuments(raw_query)) {
            PrintDocument(document);
        }
    } catch (const std::exception& e) {
    	std::cerr << "Ошибка поиска: "s << e.what() << std::endl;
    }
}

void MatchDocuments(const SearchServer& search_server, const std::string& query) {
    try {
    	std::cout << "Матчинг документов по запросу: "s << query << std::endl;
        const int document_count = search_server.GetDocumentCount();
        for (int index = 0; index < document_count; ++index) {
            const int document_id = search_server.GetDocumentId(index);
            const auto [words, status] = search_server.MatchDocument(query, document_id);
            PrintMatchDocumentResult(document_id, words, status);
        }
    } catch (const std::exception& e) {
    	std::cerr << "Ошибка матчинга документов на запрос "s << query << ": "s << e.what() << std::endl;
    }
}

int main() {
		const std::string stopWords = "greater why not near without sure most had mr still never greatest be she"s;
		SearchServer server(stopWords);
		AddDocument(server, 0, "highly respect inquietude finished had greater none speaking", DocumentStatus::ACTUAL, {1, 5, 8});
		AddDocument(server, 1, "having regret round kept remainder myself why not weather wished he made taste soon assistance eyes near", DocumentStatus::ACTUAL, {2, 3, 9});
		AddDocument(server, 3, "without inquietude invited never ladies relation reasonable secure humoured", DocumentStatus::ACTUAL, {1, 2});
		AddDocument(server, -4, "smiling sure furnished purse had most offered adapted called correct does domestic", DocumentStatus::BANNED, {5});
		AddDocument(server, 3, "excellence mr still alteration depending never seven first greatest three park", DocumentStatus::REMOVED, {4, 5, 7, 9});
		FindTopDocuments(server, "inquietude weather");
		FindTopDocuments(server, "excellence inquietude weather -");
		MatchDocuments(server, "inquietude weather");
		MatchDocuments(server, "inquietude --weather");
		return 0;
}


