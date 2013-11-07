#include <iostream>
#include <fstream>
#include <functional>
#include <algorithm>
#include <set>
#include <map>
#include <boost/filesystem.hpp>

using namespace std;
namespace fsystem = boost::filesystem;

hash<string> hash_fn;

set<size_t> stopWords;
map<size_t, map<unsigned int, unsigned int>> globalInvertedIndex;

void readPage(map<size_t, map<unsigned int, unsigned int>> &invertedIndex,string archiveToRead, unsigned int pageId);
set<size_t> fillStopWords();
unsigned int genInvertedIndex(map<size_t, map<unsigned int, unsigned int>> &invertedIndex, fsystem::path dir_path, unsigned int pageId);
bool isInInvIndex(string word, map<size_t, map<unsigned int, unsigned int>> &invertedIndex);
bool isStopWord(string word, map<size_t, map<unsigned int, unsigned int>> &invertedIndex);
void updateFreq(string word, unsigned int pageId, map<size_t, map<unsigned int, unsigned int>> &invertedIndex);
map<unsigned int, unsigned int> newPage(unsigned int id);

int main() {

    /* Se abre el directorio 'dir_path' */
    fsystem::path dir_path("pages");

    /* Se genera el set stopWords */
    stopWords = fillStopWords();

    /* Se crea el índice invertido */
    genInvertedIndex(globalInvertedIndex, dir_path, 1);

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

unsigned int genInvertedIndex(map<size_t, map<unsigned int, unsigned int>> &invertedIndex, fsystem::path dir_path, unsigned int pageId) {

    fsystem::directory_iterator end_itr;
    for ( fsystem::directory_iterator itr( dir_path ); itr != end_itr; ++itr ) {
        if ( is_directory(itr->status()) )
            pageId = genInvertedIndex(invertedIndex, itr->path().string(), pageId);
        else {
            cout << "[" << pageId << "] " << itr->path().filename() << "... ";
            readPage(invertedIndex,itr->path().string(),pageId);
            cout << "done!\n";

            pageId++;
        }
    }

    return pageId;
}

void readPage(map<size_t, map<unsigned int, unsigned int>> &invertedIndex, string archiveToRead, unsigned int pageId) {

    ifstream pagesFile;
    string word;

    pagesFile.open(archiveToRead);

    if (pagesFile) {
        while (pagesFile >> word) {
            if (!isStopWord(word, invertedIndex)) {

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
        }
    }
}

bool isStopWord(string word, map<size_t, map<unsigned int, unsigned int>> &invertedIndex) {
    return (stopWords.find( hash_fn(word) ) != stopWords.end());
}

bool isInInvIndex(string word, map<size_t, map<unsigned int, unsigned int>> &invertedIndex) {
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
