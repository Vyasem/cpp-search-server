#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>
#include <map>
#include <cmath>

unsigned MAX_RESULT_DOCUMENT_COUNT = 5;

struct Document {
    int id;
    double relevance;
};

std::string ReadLine() {
	std::string s;
    getline(std::cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    std::cin >> result;
    ReadLine();
    return result;
}

class SearchServer{
public:
	void AddDocument(int document_id, const std::string& document) {
	    const std::vector<std::string> words = SplitIntoWordsNoStop(document, stop_words);
	    int size = words.size();
	    for(const std::string& word: words){
	    	double tf = 1.0 / size;
	    	tf += documents[word][document_id];
	    	documents[word][document_id] = tf;
	    }
	    ++documentsCount;
	}

	void SetStopWords(const std::string& text) {
	    for (const std::string& word : SplitIntoWords(text)) {
	        stop_words.insert(word);
	    }
	}

	std::vector<Document> FindTopDocuments(const std::string& raw_query)const {
		 const std::set<std::string> query_words = ParseQuery(raw_query, stop_words);
		 std::vector<Document> allDoc = FindAllDocuments(query_words);
		 sort(allDoc.begin(), allDoc.end(), [](const Document& lhs, const Document& rhs) {
		        return lhs.relevance > rhs.relevance;
		 });
		 if(allDoc.size() > MAX_RESULT_DOCUMENT_COUNT){
			 allDoc.resize(MAX_RESULT_DOCUMENT_COUNT);
		 }
		 return allDoc;
	}
private:
	int documentsCount = 0;
	std::map<std::string, std::map<int, double>> documents;
	std::set<std::string> stop_words;

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

	std::vector<Document> FindAllDocuments(const std::set<std::string>& query_words) const{
		std::vector<Document> matched_documents;
		std::map<int, double> documentToRelevance;
		for(const std::string& word: query_words){
			if(word[0] == '-'){
				continue;
			}

			if(documents.find(word) != documents.end()){
				double idf = log(documentsCount * 1.0 /  documents.at(word).size());
				for(const auto& [documentId, documentTf] : documents.at(word)){
					double td_idf = idf * documentTf;
					documentToRelevance[documentId] += td_idf;
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
					documentToRelevance[documentId] = 0;
				}
			}
		}

		for(const auto& [id, relevance]: documentToRelevance){
			if(relevance > 0){
				matched_documents.push_back({id, relevance});
			}
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

const SearchServer CreateSearchServer(){
	const std::string stop_words_joined = ReadLine();
	SearchServer server;
	server.SetStopWords(stop_words_joined);
	const int document_count = ReadLineWithNumber();
	for (int document_id = 0; document_id < document_count; ++document_id) {
		server.AddDocument(document_id, ReadLine());
	}

	return server;
}

int main() {
	const SearchServer server = CreateSearchServer();
    const std::string query = ReadLine();
    for (const Document& item : server.FindTopDocuments(query)) {
    	std::cout << "{ document_id = " << item.id << ", relevance = " << item.relevance << " }" << std::endl;
    }
}
