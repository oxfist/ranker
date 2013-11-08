#include <iostream>
#include <fstream>
#include <functional>
#include <algorithm>
#include <set>
#include <map>
#include <boost/filesystem.hpp>

using namespace std;
namespace fsystem = boost::filesystem;

typedef map<unsigned int, unsigned int> TermFrec;
typedef map<size_t, TermFrec> IIndex;
typedef map<size_t, double> TfIdf;
typedef map<unsigned int, TfIdf> ResultQuery;

set<size_t> stopWords;
hash<string> hash_fn;

IIndex globalInvertedIndex;
ResultQuery result;

set<size_t> fillStopWords();
void readPage(IIndex &invertedIndex,string archiveToRead, unsigned int pageId);
unsigned int genInvertedIndex(IIndex &invertedIndex, fsystem::path dir_path, unsigned int pageId);
bool isInInvIndex(string word, IIndex &invertedIndex);
bool isStopWord(string word);
void updateFreq(string word, unsigned int pageId, IIndex &invertedIndex);
void calculateTfIdf(double totalDocs, string query);
TermFrec newPage(unsigned int id);
void showResults();

int main() {

    unsigned int totalDocs;

    /* Se abre el directorio 'dir_path' */
    fsystem::path dir_path("pages");

    /* Se genera el set stopWords */
    stopWords = fillStopWords();

    /* Se crea el índice invertido */
    totalDocs = genInvertedIndex(globalInvertedIndex, dir_path, 1);

    /* Se calcula tf-idf para cada uno de los terminos */
    calculateTfIdf((double) totalDocs-1, "asd");

    cout << "Resultados parciales con map ResultQuery\n";

    /* Se muestran los resultados parciales */
    showResults();

    return 0;
}

void calculateTfIdf(double totalDocs, string query) {

    IIndex::iterator itr;
    TermFrec::iterator itr_tf;

    double tf_idf;

    itr = globalInvertedIndex.find(hash_fn(query));

    if ( itr != globalInvertedIndex.end() ) {
        cout << "------------------------------------------\n";
        cout << "term: " << itr->first << "\n";
        for (itr_tf = itr->second.begin(); itr_tf != itr->second.end(); ++itr_tf) {
            cout << "doc:" << itr_tf->first << " - frec: " << itr_tf->second << "\n";

            tf_idf = itr_tf->second * log (totalDocs/itr->second.size());

            result[itr_tf->first][itr->first] = tf_idf;
        }
    }

    cout << "------------------------------------------\n";

}

void showResults() {
    ResultQuery::iterator itr_query;
    TfIdf::iterator itr_frec;

    for (itr_query = result.begin(); itr_query != result.end(); ++itr_query) {
        cout << "doc: " << itr_query->first << "\n";
        for (itr_frec = itr_query->second.begin(); itr_frec != itr_query->second.end(); ++itr_frec)
            cout << "term: " << itr_frec->first << " tfidf: "<< itr_frec->second << "\n";
    }
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

unsigned int genInvertedIndex(IIndex &invertedIndex, fsystem::path dir_path, unsigned int pageId) {

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

void readPage(IIndex &invertedIndex, string archiveToRead, unsigned int pageId) {

    ifstream pagesFile;
    string word;

    pagesFile.open(archiveToRead);

    if (pagesFile) {
        while (pagesFile >> word) {
            if (!isStopWord(word)) {

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

bool isStopWord(string word) {
    return (stopWords.find( hash_fn(word) ) != stopWords.end());
}

bool isInInvIndex(string word, IIndex &invertedIndex) {
    return (invertedIndex.find( hash_fn(word) ) != invertedIndex.end());
}

/*
 * Agrega un nuevo map correspondiente a un pageId nuevo con frecuencia 1,
 * en el caso de que la palabra no exista en el índice invertido.
 */
TermFrec newPage(unsigned int id) {
    TermFrec newFreqs;
    newFreqs[id] = 1;
    return newFreqs;
}

/* Actualiza la frecuencia para el documento pageId o agrega un nuevo pageId con frecuencia 1. */
void updateFreq(string word, unsigned int pageId, IIndex &invertedIndex) {
    (invertedIndex[hash_fn(word)].find(pageId) != invertedIndex[hash_fn(word)].end()) ?
        invertedIndex[hash_fn(word)][pageId] += 1 : invertedIndex[hash_fn(word)][pageId] = 1;
}
