#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <cctype>

using namespace std;


// Я вынесла их в глобальные, чтобы все функции могли видеть данные, так дегче данные между функциями передать, кажетсч
vector<string> file_names; 
unordered_map<string, unordered_map<string, int>> word_counts; // Ключ - имя файла. Значение - ещё один map, где ключ - слово, значение - сколько раз оно встретилось (тяжело)
unordered_map<string, int> total_words; // Ключ - имя файла, значение - общее количество слов в этом файле (нужно для знаменателя TF)
unordered_map<string, int> doc_frequency; // Ключ - слово, значение - в скольких разных документах оно встречается (нужно для IDF)
int N = 0; // ппросто счётчик успешно загруженных документов:)

// функция для очистки слоыа
string clean_word(string word) {
    string cleaned = ""; // сюда тольуко буквы и цифры
    for (char c : word) { 
        // isalnum проверяет, является ли символ буквой или цифрой
        if (isalnum((unsigned char)c)) {
            cleaned += c; // если это буква или чиселка, добавляю в новую строку
        }
    }

    transform(cleaned.begin(), cleaned.end(), cleaned.begin(),
              [](unsigned char ch) { return tolower(ch); });

    return cleaned; // очищенное слово в нижнем регистре
}

// загрузка доков
void load_corpus() {
    // файл со списком документов
    ifstream list_file("documents.txt");
    if (!list_file) { //база проверка что файл открылся
        cerr << "Error: documents.txt not found. Please place it in the program folder." << endl;
        exit(1); 
    }

    string filename;
    while (getline(list_file, filename)) {
        if (filename.empty()) continue; // для пропуска пустых строк

        ifstream doc_file(filename); // сам документ
        if (!doc_file) {
            cout << "Warning: file '" << filename << "' not found, skipping." << endl;
            continue; 
        }

        file_names.push_back(filename); // запоминаю имя в списке, чтобы потом выводить результаты в том же порядке

        set<string> unique_in_doc;
        string raw_word;

        while (doc_file >> raw_word) {
            string w = clean_word(raw_word); // чистим слово
            if (w.empty()) continue; // Если после очистки осталось пусто, то пропускаем

            word_counts[filename][w]++; 
            total_words[filename]++;  
            unique_in_doc.insert(w);    
        }

        // После чтения всего документа обновляю DF для каждого уникального слова
        for (const string& w : unique_in_doc) {
            doc_frequency[w]++; // Увеличиваю счётчик документов, где встретилось это слово хотя бы один раз
        }

        N++; // общее колво обработанных документов
    }
}

// расчет IDF
// Формула: IDF(w) = log(N / df(w))
double calc_idf(string word) {
    // если слова вообще нет в нашей базе, значит оно ни в одном документе не встречалось
    if (doc_frequency.find(word) == doc_frequency.end()) return 0.0;
    return log((double)N / doc_frequency[word]);
}

// расчет TF
// Формула: TF(w, d) = count(w, d) / |d|
double calc_tf(string word, string filename) {
    // если файла нет - TF = 0
    if (total_words.find(filename) == total_words.end()) return 0.0;
    // ну и на ноль делить нельзя
    if (total_words[filename] == 0) return 0.0;

    int count = 0;
    // сначала проверяю, есть ли файл в map, потом ищу слово внутри него
    if (word_counts.count(filename) && word_counts[filename].count(word)) {
        count = word_counts[filename][word];
    }
    // делим количество вхождений на общее число слов
    return (double)count / total_words[filename];
}

// команда word
void handle_word(string w) {
    cout << "Word: " << w << endl;
    cout << "Documents total: " << N << endl;

    // колво доков со словом. Если слова нет df = 0
    int df = doc_frequency.count(w) ? doc_frequency[w] : 0;
    cout << "Documents with word: " << df << endl;

    // Ввывод идф
    cout << fixed << setprecision(4) << "IDF: " << calc_idf(w) << endl;

    cout << "Appears in:" << endl;
    // Пперебор файлов (в том порядке в котором были загружены!!!!!!!)
    for (string f : file_names) {
        // если слово есть, то вывод с дефисом
        if (word_counts.count(f) && word_counts[f].count(w)) {
            cout << "- " << f << endl;
        }
    }
}


void handle_word_in_doc(string w, string doc) {
    cout << "Word: " << w << endl;
    cout << "Document: " << doc << endl;

    int count = 0;
    if (word_counts.count(doc) && word_counts[doc].count(w)) {
        count = word_counts[doc][w];
    }
    cout << "Count: " << count << endl;

    double tf = calc_tf(w, doc);
    double idf = calc_idf(w);
    cout << fixed << setprecision(4) << "TF: " << tf << endl;
    cout << fixed << setprecision(4) << "TF-IDF: " << tf * idf << endl;
}

void handle_doc(string doc) {
    // если такой файл не загружали -- ошибка
    if (!total_words.count(doc)) {
        cout << "Document not found: " << doc << endl;
        return;
    }

    cout << "Document: " << doc << endl;
    cout << "Total words: " << total_words[doc] << endl;

    int unique = word_counts.count(doc) ? word_counts[doc].size() : 0;
    cout << "Unique words: " << unique << endl;
    cout << "Top words:" << endl;

    // Чтобы отсортировать слова по TF-IDF, мне нужно собрать их в вектор пар 
    vector<pair<string, double>> scores;
    if (word_counts.count(doc)) {
        // Прохожу по всем словам в документе через итератор
        for (auto it = word_counts[doc].begin(); it != word_counts[doc].end(); ++it) {
            string w = it->first;
            double tf = calc_tf(w, doc);
            double idf = calc_idf(w);
            // Складываю пару как слово и его TF-IDF
            scores.push_back({w, tf * idf});
        }
    }

    // Требование задания: использовать std::sort и лямбда-функцию(кошмар)
    // Лямбда принимает две пары и возвращает true, если первая должна идти раньше второй. a.second > b.second означает сортировку по убыванию
    sort(scores.begin(), scores.end(), [](pair<string, double> a, pair<string, double> b) {
        return a.second > b.second;
    });

    // вывод 5 лучших слов
    for (int i = 0; i < 5 && i < scores.size(); i++) {
        cout << i + 1 << ". " << scores[i].first << "(" << fixed << setprecision(4) << scores[i].second << ")" << endl;
    }
}


void handle_query(vector<string> words) {
    cout << "Query:";
    for (string w : words) cout << " " << w; // вывод через пробел
    cout << endl;

    cout << "Results:" << endl;
    vector<pair<string, double>> results;

    // счет для каждого документа
    for (string f : file_names) {
        double score = 0.0;
        for (string w : words) {
            score += calc_tf(w, f) * calc_idf(w);
        }
        results.push_back({f, score});
    }

    // сортировка по релевантности 
    sort(results.begin(), results.end(), [](pair<string, double> a, pair<string, double> b) {
        return a.second > b.second;
    });

    int rank = 1;
    for (int i = 0; i < results.size(); i++) {
        // вывожу только те документы, у которых score > 0
        if (results[i].second > 0.0) {
            cout << rank << ". " << results[i].first << "(" << fixed << setprecision(4) << results[i].second << ")" << endl;
            rank++;
        }
    }
}


int main() {
    load_corpus(); // загружаем и обрабатываем доки
    cout << "Ready. Enter commands (WORD, WORD_IN_DOC, DOC, QUERY) or Ctrl+D to exit:" << endl;

    string line;
    // getline читает всю строку целиком
    while (getline(cin, line)) {
        if (line.empty()) continue; // пропуск пустого ввода

        istringstream iss(line); //  поток из строки, чтобы удобно разбивать её на слова
        string cmd;
        iss >> cmd; // первое слово(дороже второго) - это команда

        if (cmd == "WORD") {
            string w;
            if (iss >> w) handle_word(w); // Если слово есть после команды, передаю его
            else cout << "Error: please provide a word after WORD." << endl;
        }
        else if (cmd == "WORD_IN_DOC") {
            string w, d;
            // Пытаюсь прочитать и слово, и имя документа. Если чего-то не хватает - ошибка
            if (iss >> w >> d) handle_word_in_doc(w, d);
            else cout << "Error: format WORD_IN_DOC <word> <document>" << endl;
        }
        else if (cmd == "DOC") {
            string d;
            if (iss >> d) handle_doc(d);
            else cout << "Error: please provide a document name after DOC." << endl;
        }
        else if (cmd == "QUERY") {
            vector<string> words;
            string w;
            // Читаю все оставшиеся слова в строке и кладу их в вектор
            while (iss >> w) words.push_back(w);

            if (!words.empty()) handle_query(words);
            else cout << "Error: please provide at least one word after QUERY." << endl;
        }
        else {
            // Если ввели что-то другое, подсказываю доступные команды так уж и быть
            cout << "Unknown command. Available: WORD, WORD_IN_DOC, DOC, QUERY" << endl;
        }
    }
    return 0;
}
