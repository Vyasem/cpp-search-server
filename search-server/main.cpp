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
using namespace std::string_literals;
unsigned MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;


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
	std::vector<Document> FindTopDocuments(const std::string& raw_query, Predicat filter)const {
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

// -----------установка макросов ---------------
template<typename T>
void runTest(T f, const std::string& functionName){
	f();
	std::cerr << functionName << " OK" << std::endl;
}

#define RUN_TEST(func) runTest(func, #func)

template<typename T, typename U>
void assertEqual(const T& lhs,const U& rhs, const std::string& lhsText, const std::string& rhsText, const std::string& fileName, const int lineNumber, const std::string& functionName){
	if(lhs != rhs){
		std::cerr << std::boolalpha;
		std::cerr << fileName << "("s << lineNumber << "): "s << functionName << ": ASSERT(" << lhsText << " != " << rhsText << ") failed." << std::endl;
		abort();
	}
}
template<typename T, typename U>
void assertEqualHint(const T& lhs,const U& rhs, const std::string& lhsText, const std::string& rhsText, const std::string& fileName, const int lineNumber, const std::string& functionName, const std::string& hint){
	if(lhs != rhs){
		std::cerr << std::boolalpha;
		std::cerr << fileName << "("s << lineNumber << "): "s << functionName << ": ASSERT(" << lhsText << " != " << rhsText << ") failed. Hint: " << hint << std::endl;
		abort();
	}
}

#define ASSERT_EQUAL(lhs, rhs) assertEqual((lhs), (rhs), #lhs, #rhs, __FILE__, __LINE__, __FUNCTION__)
#define ASSERT_EQUAL_HINT(lhs, rhs, hint) assertEqualHint((lhs), (rhs), #lhs, #rhs, __FILE__, __LINE__, __FUNCTION__, (hint))

void userAssert(bool expr, const std::string& text, const std::string& fileName, const int lineNumber, const std::string& functionName){
	if(!expr){
		std::cerr << std::boolalpha;
		std::cerr << fileName << "("s << lineNumber << "): "s << functionName << ": ASSERT(" << text << ") failed." << std::endl;
		abort();
	}
}

void userAssertHint(bool expr, const std::string& text, const std::string& fileName, const int lineNumber, const std::string& functionName, const std::string& hint){
	if(!expr){
		std::cerr << std::boolalpha;
		std::cerr << fileName << "("s << lineNumber << "): "s << functionName << ": ASSERT(" << text << ") failed. Hint: " << hint << std::endl;
		abort();
	}
}

#define ASSERT(expr) userAssert((expr), #expr, __FILE__, __LINE__, __FUNCTION__)
#define ASSERT_HINT(expr, hint) userAssertHint((expr), #expr, __FILE__, __LINE__, __FUNCTION__, (hint))
// -----------установка макросов ---------------
// -------- Начало модульных тестов поисковой системы ----------


SearchServer genereateServerObject(){
	SearchServer server;
	const std::string stopWords = "greater why not near without sure most had mr still never greatest be she"s;
	server.SetStopWords(stopWords);
	server.AddDocument(0, "highly respect inquietude finished had greater none speaking", DocumentStatus::ACTUAL, {1, 5, 8});
	server.AddDocument(1, "having regret round kept remainder myself why not weather wished he made taste soon assistance eyes near", DocumentStatus::ACTUAL, {2, 3, 9});
	server.AddDocument(3, "without inquietude invited never ladies relation reasonable secure humoured", DocumentStatus::ACTUAL, {1, 2});
	server.AddDocument(4, "smiling sure furnished purse had most offered adapted called correct does domestic", DocumentStatus::BANNED, {5});
	server.AddDocument(5, "excellence mr still alteration depending never seven first greatest three park", DocumentStatus::REMOVED, {4, 5, 7, 9});
	server.AddDocument(6, "suspicion be miles bed sure continue instantly sentiments rejoiced laughing rapid she", DocumentStatus::IRRELEVANT, {5});
	return server;
}


void TestExcludeStopWordsFromAddedDocumentContent() {
	SearchServer server = genereateServerObject();
	const auto found_docs = server.FindTopDocuments("humoured"s);
	ASSERT_EQUAL(static_cast<int>(found_docs.size()), 1);
	const Document& doc0 = found_docs[0];
	ASSERT_EQUAL_HINT(doc0.id, 3, "Wrong ID");
	ASSERT_HINT(server.FindTopDocuments("without"s).empty(), "Must be empty");
	ASSERT(!server.FindTopDocuments("weather"s).empty());

}

void TestExcludeMinusWord(){
	SearchServer server = genereateServerObject();
	ASSERT(server.FindTopDocuments("-highly speaking"s).empty());
	ASSERT_HINT(!server.FindTopDocuments("excellence"s, DocumentStatus::REMOVED).empty(),  "Wrong!");
}

void TestMatchDocument(){
	SearchServer server = genereateServerObject();
	std::vector<std::string> words;
	DocumentStatus status;
	std::tie(words, status) = server.MatchDocument( "car regret round"s, 1);
	ASSERT_EQUAL(static_cast<int>(words.size()), 2);
	std::tie(words, status) = server.MatchDocument( "root invited -relation"s, 3);
	ASSERT(words.empty());
	std::tie(words, status) = server.MatchDocument( "-root invited"s, 3);
	ASSERT_EQUAL_HINT(static_cast<int>(words.size()), 1, "It must be 1");
	std::tie(words, status) = server.MatchDocument( ""s, 0);
	ASSERT(words.empty());
	std::tie(words, status) = server.MatchDocument( "root -having -regret"s, 1);
	ASSERT(words.empty());
}

void TestRelevanceSort(){
	SearchServer server = genereateServerObject();
	const std::vector<Document> result = server.FindTopDocuments("invited inquietude weather made assistance finished"s);
	ASSERT_EQUAL(result[0].id, 0);
	ASSERT_EQUAL(result[1].id, 3);
	ASSERT_EQUAL_HINT(result[2].id, 1, "It's bad value");
}


void TestRatingCalculate(){
	SearchServer server = genereateServerObject();
	const std::vector<Document> result = server.FindTopDocuments("highly regret invited purse alteration sure"s);
	ASSERT_EQUAL(result[0].rating, 4);
	ASSERT_EQUAL(result[1].rating, 1);
	ASSERT_EQUAL(result[2].rating, 4);
}

void TestStatusSearch(){
	SearchServer server = genereateServerObject();
	{
		const std::vector<Document> result = server.FindTopDocuments("highly regret invited purse alteration sure"s);
		ASSERT_EQUAL(static_cast<int>(result.size()), 3);
		ASSERT_EQUAL(result[0].id, 0);
		ASSERT_EQUAL(result[1].id, 3);
		ASSERT_EQUAL(result[2].id, 1);
	}

	{
		const std::vector<Document> result = server.FindTopDocuments("highly regret invited purse alteration sure suspicion"s, DocumentStatus::IRRELEVANT);
		ASSERT_EQUAL(static_cast<int>(result.size()), 1);
		ASSERT_EQUAL(result[0].id, 6);
	}

	{
		const std::vector<Document> result = server.FindTopDocuments("highly regret invited purse alteration sure suspicion"s, DocumentStatus::BANNED);
		ASSERT_EQUAL(static_cast<int>(result.size()), 1);
		ASSERT_EQUAL(result[0].id, 4);
	}

	{
		const std::vector<Document> result = server.FindTopDocuments("highly regret invited purse alteration sure suspicion"s, DocumentStatus::REMOVED);
		ASSERT_EQUAL(static_cast<int>(result.size()), 1);
		ASSERT_EQUAL(result[0].id, 5);
	}
}

void TestRelevanceValue(){
	SearchServer server = genereateServerObject();
	const std::vector<Document> result = server.FindTopDocuments("invited inquietude weather made assistance finished"s);
	ASSERT(std::abs(result[0].relevance - 0.481729) < 1e-6);
	ASSERT(std::abs(result[1].relevance - 0.41291) < 1e-6);
	ASSERT_HINT(std::abs(result[2].relevance - 0.383948) < 1e-6, "Left value must be less that 1e-6");
}

void TestPredicatUse(){
	SearchServer server = genereateServerObject();
	{
		const std::vector<Document> result = server.FindTopDocuments("invited inquietude weather made assistance finished"s, [](int document_id, DocumentStatus documentStatus, int rating){
			 return rating == 4;
		 });
		ASSERT_EQUAL(static_cast<int>(result.size()), 2);
		ASSERT_EQUAL(result[0].id, 0);
		ASSERT_EQUAL(result[1].id, 1);
	}

	{
		const std::vector<Document> result = server.FindTopDocuments("highly regret invited purse alteration sure suspicion"s, [](int document_id, DocumentStatus documentStatus, int rating){
			 return documentStatus == DocumentStatus::BANNED;
		 });
		ASSERT_EQUAL(result[0].id, 4);
	}

	{
		const std::vector<Document> result = server.FindTopDocuments("highly regret invited purse alteration sure suspicion"s, [](int document_id, DocumentStatus documentStatus, int rating){
			 return document_id == 6;
		 });
		ASSERT_EQUAL(result[0].rating, 5);
	}

	{
		const std::vector<Document> result = server.FindTopDocuments("highly regret invited purse alteration sure suspicion"s, [](int document_id, DocumentStatus documentStatus, int rating){
			 return document_id == 59;
		 });
		ASSERT(result.empty());
	}

	{
		const std::vector<Document> result = server.FindTopDocuments("highly regret invited purse alteration sure suspicion"s, [](int document_id, DocumentStatus documentStatus, int rating){
			 return rating == 5;
		 });
		ASSERT_EQUAL(static_cast<int>(result.size()), 2);
		ASSERT_EQUAL(result[0].id, 4);
		ASSERT_EQUAL(result[1].id, 6);
	}

}

void TestDocumentCount(){
	SearchServer server = genereateServerObject();
	int count = server.GetDocumentCount();
	ASSERT_EQUAL(count, 6);
}


void TestSearchServer() {
	RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
	RUN_TEST(TestExcludeMinusWord);
	RUN_TEST(TestMatchDocument);
	RUN_TEST(TestRelevanceSort);
	RUN_TEST(TestRatingCalculate);
	RUN_TEST(TestStatusSearch);
	RUN_TEST(TestRelevanceValue);
	RUN_TEST(TestPredicatUse);
	RUN_TEST(TestDocumentCount);
}

// --------- Окончание модульных тестов поисковой системы -----------


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
	TestSearchServer();
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

