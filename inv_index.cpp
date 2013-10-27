#include <iostream>
#include <fstream>
#include <functional>
#include <set>

using namespace std;

set<size_t> fillStopWords();

int main() {
    set <size_t> stopWords = fillStopWords();

    /* Determina un hash a partir de la palabra a leer. */
    hash<string> hash_fn;
    string word;
    
    /* Se imprime la cantidad de elementos del set. */
    cout << stopWords.size() << endl;

    /* Ciclo que, dada una palabra, retorna true si se encuentra en el set
     * o false en caso contrario. */
    while (true) {
        cin >> word;
        if (word == "0")
            break;
        /* El método find retorna un iterador si se encuentra el elemento,
         * o el iterador de end() en caso contrario. */
        cout << (stopWords.find( hash_fn(word) ) != stopWords.end()) << endl;
    }
    return 0;
}

set<size_t> fillStopWords() {
    ifstream file;
    string line;

    /* Set que contendrá las stopWords. */
    set<size_t> stopWords;

    /* Determina un hash a partir de la palabra a leer. */
    hash<string> hash_fn;

    file.open("sw.txt");
    if (file.is_open()) {
        while (getline(file, line)) {
            /* Inserta en el set el hash de la palabra. */
            stopWords.insert( hash_fn(line) );
        }
    }
    return stopWords;
}
