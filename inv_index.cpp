/* Copyright 2013 <Andrés Caro Q.> */
#include <boost/tuple/tuple.hpp>
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <functional>
#include <algorithm>
#include <set>
#include <map>
#include <vector>
#include <queue>
#include <utility>

namespace fsystem = boost::filesystem;

/* Mapea el docId a la cantidad de veces que el término aparece. */
typedef std::map<unsigned int, unsigned int> TermFreq;

/* Mapea un hash de std::string al map de tfs. */
typedef std::map<size_t, TermFreq> InvIndex;

/* Mapea un entero a un puntaje. */
typedef std::map<size_t, double> Score;

/* Mapea un docId con un map de hashes correspondiente a términos. */
typedef std::map<unsigned int, Score> QueryResult;

std::set<size_t> stopWords;
std::hash<std::string> hash_fn;
std::vector<Score> scoresVector;

InvIndex globalInvertedIndex;

/* Mapea un docId al tf-idf de todos los términos. */
QueryResult result;

class Prioritize {
 public:
    int operator() (const std::pair<unsigned int, double>& p1,
        const std::pair<unsigned int, double>& p2) {
        return p1.second > p2.second;
    }
};

std::set<size_t> fillStopWords();
void readPage
(InvIndex &invertedIndex, std::string archiveToRead, unsigned int pageId);
unsigned int genInvertedIndex
(InvIndex &invertedIndex, fsystem::path dir_path, unsigned int pageId);
bool isInInvIndex(std::string word, InvIndex &invertedIndex);
bool isStopWord(std::string word);
void updateFreq
(std::string word, unsigned int pageId, InvIndex &invertedIndex);
void computeTfIdf
(double totalDocs, std::string queryWord,
    std::map<unsigned int, unsigned int>* numTermsInDoc);
TermFreq newPage(unsigned int id);
void showResults();
void readQueries(std::ifstream *queryFile, unsigned int totalDocs);
void rankDocs
(std::string line, unsigned int totalDocs, unsigned int numTerms,
    std::map<unsigned int, unsigned int>* numTermsInDoc);

int main() {
    unsigned int totalDocs;
    std::ifstream queryFile;

    queryFile.open("queries.txt");

    /* Se abre el directorio 'dir_path' */
    fsystem::path dir_path("pages");

    /* Se genera el set stopWords */
    stopWords = fillStopWords();

    /* Se crea el índice invertido */
    totalDocs = genInvertedIndex(globalInvertedIndex, dir_path, 1);

    readQueries(&queryFile, totalDocs-1);

    // std::cout << "Resultados parciales con map QueryResult\n";

    /* Se muestran los resultados parciales */
    // showResults();

    queryFile.close();

    return 0;
}

void readQueries(std::ifstream *queryFile, unsigned int totalDocs) {
    std::string line, queryWord;
    /* Contador de términos de la query. */
    unsigned int numTerms;
    /* 
     * Map de docId a cantidad de términos
     * de la query que aparecen en él.
     */
    std::map<unsigned int, unsigned int> numTermsInDoc;

    while (queryFile) {
        getline((*queryFile), line);
        if (line == "") {
            break;
        }
        numTerms = 0;
        boost::tokenizer<> tokens(line);

        /* Se recorre la línea parseada como una colección. */
        for (auto beg : tokens) {
            /* Se calcula tf-idf para cada uno de los términos de la query. */
            computeTfIdf(static_cast<double>(totalDocs), beg, &numTermsInDoc);
            numTerms++;
        }
        /* Computar el score de los documentos para la query dada. */
        rankDocs(line, totalDocs, numTerms, &numTermsInDoc);
        numTermsInDoc.clear();
    }
}

void rankDocs
(std::string line, unsigned int totalDocs, unsigned int numTerms,
    std::map<unsigned int, unsigned int>* numTermsInDoc) {
    boost::tokenizer<> tokens(line);
    std::map<unsigned int, double> scores;
    std::map<unsigned int, double> sumOfTfIdf;
    std::priority_queue
        <std::pair<unsigned int, double>,
        std::vector<std::pair<unsigned int, double>>, Prioritize> topK;
    std::cout << "Rankeando para: \"" << line << "\"\n";

    for (auto beg : tokens) {
        /* Se recupera el posting list del término dado. */
        auto postingList = globalInvertedIndex[hash_fn(beg)];

        /* 
         * Se recorre el posting list para sacar el tf-idf
         * del map que tiene los resultados por docId.
         */
        for (auto pairDocFreq : postingList) {
            unsigned int denom = numTerms*(*numTermsInDoc)[pairDocFreq.first];
            /* 
             * Se recupera el mapeo de término con tf-idf del map
             * que tiene los resultados para el documento dado.
             */
            auto termTfIdfs = result[pairDocFreq.first];
            std::cout << "doc: " << pairDocFreq.first << "\n";

            /* Se busca el tf-idf para el término dado. */
            auto pairTermTfIdf = termTfIdfs.find(hash_fn(beg));
            if (pairTermTfIdf != termTfIdfs.end()) {
                std::cout << "tf-idf a sumar: ";
                std::cout << (*pairTermTfIdf).second << "\n";

                /* Se suma el tf-idf de la palabra actual de la query. */
                scores[pairDocFreq.first] += ((*pairTermTfIdf).second/denom);
            }
        }
    }
    for (unsigned int i = 1; i < totalDocs; i++) {
        if (topK.size() < 3) {
            topK.push(std::pair<unsigned int, double>(i, scores[i]));
        } else if (topK.top().second < scores[i]) {
            topK.pop();
            topK.push(std::pair<unsigned int, double>(i, scores[i]));
        }
    }
    std::cout << "Imprimiendo\n";
    while (!topK.empty()) {
        std::cout << topK.top().first << " " << topK.top().second << "\n";
        topK.pop();
    }
}

void computeTfIdf
(double totalDocs, std::string queryWord,
    std::map<unsigned int, unsigned int>* numTermsInDoc) {
    /* tf-idf de la query ingresada. */
    double tf_idf;

    auto itr = globalInvertedIndex.find(hash_fn(queryWord));

    if (itr != globalInvertedIndex.end()) {
        std::cout << "------------------------------------------\n";
        std::cout << "term: \"" << queryWord << "\" - ";
        std::cout << itr->first << "\n";

        /* Se itera sobre el map con docId y frecuencias para cada término. */
        /* El recorrido se hace por referencia para una mayor eficiencia. */
        for (auto &itr_tf : (*itr).second) {
            std::cout << "doc: " << itr_tf.first;
            std::cout << " - frec: " << itr_tf.second << "\n";

            /* 
             * Se aumenta el contador de docs en los que está
             * el término.
             */
            (*numTermsInDoc)[itr_tf.first] += 1;

            /*
             * Frecuencia del término en el documento multiplicada
             * por el logaritmo de la cantidad de documentos dividida
             * por la cantidad de documentos en los que se encuentra
             * el término.
             */
            tf_idf = itr_tf.second * log(totalDocs/(*itr).second.size());
            std::cout << "tf-idf: " << tf_idf << "\n";

            /* Se guarda el tf-idf asociado al docId y el término. */
            result[itr_tf.first][(*itr).first] = tf_idf;
        }
    }

    std::cout << "------------------------------------------\n";
}

void showResults() {
    /* Los iteradores se hacen constantes para mayor eficiencia. */
    for (const auto itr_query : result) {
        std::cout << "doc: " << itr_query.first << "\n";
        for (const auto itr_frec : itr_query.second) {
            std::cout << "term: " << itr_frec.first;
            std::cout << " tfidf: "<< itr_frec.second << "\n";
        }
    }
}

std::set<size_t> fillStopWords() {
    std::ifstream stopWordsFile;
    std::string line;

    /* Set que contendrá las stopWords. */
    std::set<size_t> stopWords;

    stopWordsFile.open("sw.txt");
    if (stopWordsFile.is_open()) {
        while (getline(stopWordsFile, line)) {
            /* Inserta en el set el hash de la palabra. */
            stopWords.insert(hash_fn(line));
        }
    }

    return stopWords;
}

/* 
 * Se recorre el directorio padre, se genera el índice invertido con todos los
 * documentos encontrados al interior y se retorna la cantidad total éstos.
 */
unsigned int genInvertedIndex
(InvIndex &invertedIndex, fsystem::path dir_path, unsigned int pageId) {
    fsystem::directory_iterator end_itr;

    /* Se recorre todo el path hasta que no queden elementos por recorrer. */
    for ( fsystem::directory_iterator itr( dir_path );
            itr != end_itr; ++itr ) {
        /* 
         * Si el elemento es directorio, se ingresa 
         * y se guarda el Id del último doc.
         */
        if (is_directory(itr->status())) {
            pageId = genInvertedIndex(invertedIndex, itr->path().string(),
                    pageId);
        } else {
            std::cout << "[" << pageId << "] ";
            std::cout << itr->path().filename() << "... ";
            readPage(invertedIndex, itr->path().string(), pageId);
            std::cout << "done!\n";

            pageId++;
        }
    }

    return pageId;
}

void readPage
(InvIndex &invertedIndex, std::string fileToRead, unsigned int pageId) {
    std::ifstream pagesFile;
    std::string word;

    pagesFile.open(fileToRead);

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

    pagesFile.close();
}

bool isStopWord(std::string word) {
    return (stopWords.find( hash_fn(word) ) != stopWords.end());
}

bool isInInvIndex(std::string word, InvIndex &invertedIndex) {
    return (invertedIndex.find(hash_fn(word)) != invertedIndex.end());
}

/*
 * Agrega un nuevo map correspondiente a un pageId nuevo con frecuencia 1,
 * en el caso de que la palabra no exista en el índice invertido.
 */
TermFreq newPage(unsigned int id) {
    TermFreq newFreqs;
    newFreqs[id] = 1;
    return newFreqs;
}

/* 
 * Actualiza la frecuencia para el documento pageId o
 * agrega un nuevo pageId con frecuencia 1. 
 */
void updateFreq
(std::string word, unsigned int pageId, InvIndex &invertedIndex) {
    /*
     * Si se encuentra la página en el índice invertido, entonces
     * se aumenta la frecuencia del término en la página, si no,
     * se agrega la nueva página para el término dado y se asigna
     * frecuencia 1.
     */
    (invertedIndex[hash_fn(word)].find(pageId) !=
     invertedIndex[hash_fn(word)].end()) ?
        invertedIndex[hash_fn(word)][pageId] += 1 :
        invertedIndex[hash_fn(word)][pageId] = 1;
}
