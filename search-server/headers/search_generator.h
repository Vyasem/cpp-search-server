#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <random>

class SearchGenerator {
public:
    std::string GenerateWord(int max_length) {
        const int length = std::uniform_int_distribution(1, max_length)(generator);
        std::string word;
        word.reserve(length);
        for (int i = 0; i < length; ++i) {
            word.push_back(std::uniform_int_distribution(static_cast<int>('a'), static_cast<int>('z'))(generator));
        }
        return word;
    }
    std::vector<std::string> GenerateDictionary(int word_count, int max_length) {
        std::vector<std::string> words;
        words.reserve(word_count);
        for (int i = 0; i < word_count; ++i) {
            words.push_back(GenerateWord(max_length));
        }
        sort(words.begin(), words.end());
        words.erase(unique(words.begin(), words.end()), words.end());
        return words;
    }
    std::string GenerateQuery(const std::vector<std::string>& dictionary, int max_word_count, double minus_prob = 0) {
        const int word_count = std::uniform_int_distribution(1, max_word_count)(generator);
        std::string query;
        for (int i = 0; i < word_count; ++i) {
            if (!query.empty()) {
                query.push_back(' ');
            }
            if (std::uniform_real_distribution<>(0, 1)(generator) < minus_prob) {
                query.push_back('-');
            }
            query += dictionary[std::uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
        }
        return query;
    }

    std::vector<std::string> GenerateQueries(const std::vector<std::string>& dictionary, int query_count, int max_word_count) {
        std::vector<std::string> queries;
        queries.reserve(query_count);
        for (int i = 0; i < query_count; ++i) {
            queries.push_back(GenerateQuery(dictionary, max_word_count));
        }
        return queries;
    }
private:
	std::mt19937 generator;
};