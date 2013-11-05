#include <iostream>
#include <fstream>
#include <functional>
#include <set>

using namespace std;

hash<string> hash_fn;

set<size_t> fillStopWords();
void readPages(set<size_t> stopWords);

int main() {
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
        /* 
         * El método find retorna un iterador al elemento si éste es encontrado,
         * o el iterador que retorna el método end() en caso contrario.
         */
        //cout << (stopWords.find( hash_fn(word) ) != stopWords.end()) << endl;
        readPages(stopWords);
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

void readPages(set<size_t> stopWords) {
    ifstream pagesFile;
    string line;

    pagesFile.open("pages/users.dcc.uchile.cl.txt");

    if (pagesFile)
        while (pagesFile >> line) {
            if (line == "\n")
                cout << "\\n" << endl;
            cout << line << endl;
        }
}
