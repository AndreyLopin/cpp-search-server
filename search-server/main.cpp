#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word = ""s;
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

struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        vector<string> document_words = SplitIntoWordsNoStop(document);
        for (const string& word : document_words) {
            documents_[word][document_id] += 1.0 / document_words.size();
        }
        document_count_++;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:    
    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };
    
    struct QueryWord {
        bool isMinusWord;
        bool isStopWord;
        string word;
    };

    map<string, map<int, double>> documents_;
    int document_count_ = 0;
    set<string> stop_words_;

    bool IsStopWord(const string& word) const {
            return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }
    
    QueryWord ParseQueryWord(string word) const {
        bool isMinusWord = false;
        if (word[0] == '-') {
            isMinusWord = true;
            word = word.substr(1);
        }
        return {isMinusWord, IsStopWord(word), word};
    }

    Query ParseQuery(const string& text) const {
        Query query_words;
        for (const string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
            
            if (!query_word.isStopWord) {
                if (!query_word.isMinusWord) {
                    query_words.plus_words.insert(query_word.word);
                } else {
                    query_words.minus_words.insert(query_word.word);
                }
            }
        }
        
        return query_words;
    }

    vector<Document> FindAllDocuments(const Query& query_words) const {
        vector<Document> matched_documents;
        map<int, double> find_documents;
        
        for (const string& word : query_words.plus_words) {
            if (documents_.count(word) > 0) {
                for (const auto& [id, tf] : documents_.at(word)) {
                    find_documents[id] += log(static_cast<double>(document_count_) / documents_.at(word).size()) * tf;
                }
            }
        }
        
        for (const string& word : query_words.minus_words) {
            if (documents_.count(word) > 0) {
                for (const auto& [id, tf] : documents_.at(word)) {
                    find_documents.erase(id);
                }
            }
        }
        
        for (const auto& [id, relevance] : find_documents) {
            matched_documents.push_back({id, relevance});
        }
        return matched_documents;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}