#include <iostream>
#include <fstream>
#include <functional>
#include <algorithm>
#include <set>
#include <map>

using namespace std;

hash<string> hash_fn;

set<size_t> fillStopWords();
void readPages(set<size_t> stopWords, map<size_t, map<unsigned int, unsigned int>> invertedIndex);
bool isInInvIndex(string word, map<size_t, map<unsigned int, unsigned int>> invertedIndex);
bool isStopWord(string word, set<size_t> stopWords);
map<unsigned int, unsigned int> newPage(unsigned int id);
void updateFreq(string word, unsigned int pageId, map<size_t, map<unsigned int, unsigned int>> &invertedIndex);

int main() {
    map<size_t, map<unsigned int, unsigned int>> invertedIndex;
    set<size_t> stopWords = fillStopWords();

    /* Determina un hash a partir de la palabra a leer. */
    string word;
    
    /* Se imprime la cantidad de elementos del set. */
    cout << stopWords.size() << endl;

    /* 
     * Ciclo que, dada una palabra, retorna true si se encuentra en el set
     * o false en caso contrario.
     */
    while (true) {
        cin >> word;
        if (word == "0")
            break;
        readPages(stopWords, invertedIndex);
    }
    return 0;
}

set<size_t> fillStopWords() {
    ifstream stopWordsFile;
    string line;

    /* Set que contendrá las stopWords. */
    set<size_t> stopWords;

    stopWordsFile.open("sw.txt");
    if (stopWordsFile.is_open())
        while (getline(stopWordsFile, line))

            /* Inserta en el set el hash de la palabra. */
            stopWords.insert( hash_fn(line) );

    return stopWords;
}

void readPages(set<size_t> stopWords, map<size_t, map<unsigned int, unsigned int>> invertedIndex) {
    ifstream pagesFile;
    string word;
    unsigned int pageId = 0;

    pagesFile.open("pages/users.dcc.uchile.cl.txt");

    if (pagesFile) {
        while (pagesFile >> word) {
            if (!isStopWord(word, stopWords)) {
                
                /*
                 * Si la palabra no está en el índice invertido, 
                 * se agrega, si no, se actualiza la frecuencia.
                 * */
                if (!isInInvIndex(word, invertedIndex)) {
                    invertedIndex[hash_fn(word)] = newPage(pageId);
                } else {
                    updateFreq(word, pageId, invertedIndex);
                }
            }
            cout << word << " " << invertedIndex[hash_fn(word)][pageId] << endl;
        }
    }
}

bool isStopWord(string word, set<size_t> stopWords) {
    return (stopWords.find( hash_fn(word) ) != stopWords.end());
}

bool isInInvIndex(string word, map<size_t, map<unsigned int, unsigned int>> invertedIndex) {
    return (invertedIndex.find( hash_fn(word) ) != invertedIndex.end());
}

/* 
 * Agrega un nuevo map correspondiente a un pageId nuevo con frecuencia 1,
 * en el caso de que la palabra no exista en el índice invertido.
 */
map<unsigned int, unsigned int> newPage(unsigned int id) {
    map<unsigned int, unsigned int> newFreqs;
    newFreqs[id] = 1;
    return newFreqs;
}

/* Actualiza la frecuencia para el documento pageId o agrega un nuevo pageId con frecuencia 1. */
void updateFreq(string word, unsigned int pageId, map<size_t, map<unsigned int, unsigned int>> &invertedIndex) {
    (invertedIndex[hash_fn(word)].find(pageId) != invertedIndex[hash_fn(word)].end()) ?
        invertedIndex[hash_fn(word)][pageId] += 1 : invertedIndex[hash_fn(word)][pageId] = 1;
}
