#include "select.h"

void splitDot(const string& word, string& table, string& column, tableJson& tjs) { 
    bool dot = false; // поиск точки
    for (size_t i = 0; i < word.size(); i++) {
        if (word[i] == '.') {
            dot = true;
            continue;
        }
        if (word[i] == ',') {
            continue;
        }
        if (!dot) { // разделяем таблицу и колонку
            table += word[i];
        }
        else {
            column += word[i];
        }
    }
    if (!dot) { // если точка не найдена
        cerr << "Некорректная команда.\n";
        return;
    }
    if (isTableExist(table, tjs.tablehead) == false) { // проверка на существование таблицы
        cerr << "Такой таблицы нет.\n";
        return;
    }
    if (isColumnExist(table, column, tjs.tablehead) == false) { // проверка на сущетсвование колонки
        cerr << "Такой колонки нет.\n";
        return;
    }
}

string ignoreQuotes(const string& word) { // отделение от кавычек строки
    string slovo;
    for (size_t i = 0; i < word.size(); i++) {
        if (word[i] != '\'') {
            slovo += word[i];
        }
    }
    return slovo;
}

bool findDot(const string& word) { // наличие точки в слове
    bool dot = false;
    for (size_t i = 0; i < word[i]; i++) {
        if (word[i] == '.') {
            dot = true;
        }
    }
    return dot;
}

int countCsv(tableJson& tjs, const string& table) {
    int amountCsv = 1; // ищем количество созданных csv файлов
    while (true) {
        string filePath = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/" + table + "/" + to_string(amountCsv) + ".csv";
        ifstream file(filePath);
        if (!file.is_open()) { // если файл нельзя открыть, его нет
            break;
        }
        file.close();
        amountCsv++;
    }
    return amountCsv;
}

void crossJoin(tableJson& tjs, const string& table1, const string& table2, const string& column1, const string& column2) {
    int amountCsv1 = countCsv(tjs, table1); // количество файлов 1 таблицы
    int amountCsv2 = countCsv(tjs, table2); // количество файлов 2 таблицы
    for (size_t iCsv1 = 1; iCsv1 < amountCsv1; iCsv1++) {
        string filePath1 = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/" + table1 + "/" + to_string(iCsv1) + ".csv";
        rapidcsv::Document doc1(filePath1); // открываем файл 1
        int columnIndex1 = doc1.GetColumnIdx(column1); // считываем индекс искомой колонки 1
        size_t amountRow1 = doc1.GetRowCount(); // считываем количество строк в файле 1
        for (size_t i = 0; i < amountRow1; ++i) { // проходимся по строкам 1
            for (size_t iCsv2 = 1; iCsv2 < amountCsv2; iCsv2++) {
                string filePath2 = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/" + table2 + "/" + to_string(iCsv2) + ".csv";
                rapidcsv::Document doc2(filePath2); // открываем файл 2
                int columnIndex2 = doc2.GetColumnIdx(column2); // считываем индекс искомой колонки 2
                size_t amountRow2 = doc2.GetRowCount(); // считываем количество строк в файле2
                for (size_t j = 0; j < amountRow2; ++j) {
                    cout << doc1.GetCell<string>(0, i) << ": ";
                    cout << doc1.GetCell<string>(columnIndex1, i) << "  |   ";
                    cout << doc2.GetCell<string>(0, j) << ": ";
                    cout << doc2.GetCell<string>(columnIndex2, j) << endl;
                }
            }
        }
    }
}

bool checkCond(tableJson& tjs, const string& table, const string& column, const string& tcond, const string& ccond, const string& s) {
    if (s != "") {
        int amountCsv = countCsv(tjs, table);
        for (size_t iCsv = 1; iCsv < amountCsv; iCsv++) { // просматриваем все созданные файлы csv
            string filePath = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/" + table + "/" + to_string(iCsv) + ".csv";
            rapidcsv::Document doc(filePath); // открываем файл
            int columnIndex = doc.GetColumnIdx(column); // считываем индекс искомой колонки
            size_t amountRow = doc.GetRowCount(); // считываем количество строк в файле
            for (size_t i = 0; i < amountRow; ++i) { // проходимся по строкам
                if (doc.GetCell<string>(columnIndex, i) == s) { // извлекаем значение (индекс колонки, номер строки)
                    return true;
                }
            }
        }
    }
    else {
        bool condition = true;
        int amountCsv = countCsv(tjs, table);
        for (size_t iCsv = 1; iCsv < amountCsv; iCsv++) {
            string pk1, pk2;
            string pk1Path = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/" + table + "/" + table + "_id_sequence.txt";
            string pk2Path = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/" + tcond + "/" + tcond + "_id_sequence.txt";
            ifstream file1(pk1Path);
            if (!file1.is_open()) {
                cerr << "Не удалось открыть файл.\n";
                return false;
            }
            file1 >> pk1;
            file1.close();
            ifstream file2(pk2Path);
            if (!file2.is_open()) {
                cerr << "Не удалось открыть файл.\n";
                return false;
            }
            file2 >> pk2;
            file2.close();
            if (pk1 != pk2) {
                return false;
            }

            string filePath1 = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/" + table + "/" + to_string(iCsv) + ".csv";
            string filePath2 = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/" + tcond + "/" + to_string(iCsv) + ".csv";
            rapidcsv::Document doc1(filePath1); // открываем файл
            int columnIndex1 = doc1.GetColumnIdx(column); // считываем индекс искомой колонки
            size_t amountRow1 = doc1.GetRowCount(); // считываем количество строк в файле
            rapidcsv::Document doc2(filePath2); // открываем файл
            int columnIndex2 = doc2.GetColumnIdx(ccond); // считываем индекс искомой колонки
            for (size_t i = 0; i < amountRow1; ++i) { // проходимся по строкам
                if (doc1.GetCell<string>(columnIndex1, i) != doc2.GetCell<string>(columnIndex2, i)) {
                    condition = false;
                }
            }
        }
        if (condition) {
            return true;
        }
    }
    return false;
}

void select(const string& command, tableJson& tjs) { // выбор данных
    istringstream iss(command); // поток ввода для обработки строки команды
    string word;
    iss >> word; // "SELECT"
    iss >> word; // "таблица 1"
    string table1, column1; // строка для 1 таблицы и колонки
    splitDot(word, table1, column1, tjs); // разделяем таблицу и колонку
    iss >> word; // "таблица 2"
    string table2, column2; // строка для 2 таблицы и колонки
    splitDot(word, table2, column2, tjs); // разделяем таблицу и колонку

    iss >> word; // "FROM"
    if (word != "FROM") {
        cerr << "Некорректная команда.\n";
        return;
    }
    iss >> word; // таблица 1
    string tab1;
    for (size_t i = 0; i < word.size(); i++) {
        if (word[i] != ',') {
            tab1 += word[i];
        }
    }
    if (tab1 != table1) {
        cerr << "Некорректная команда.\n";
        return;
    }
    iss >> word; // таблица 2
    if (word != table2) {
        cerr << "Некорректная команда.\n";
        return;
    }

    iss >> word; // "WHERE"
    if (word != "WHERE") {
        crossJoin(tjs, table1, table2, column1, column2);
        return;
    }
    iss >> word; // первая таблица и колонка
    string t1, c1;
    splitDot(word, t1, c1, tjs);
    iss >> word; // "="
    if (word != "=") {
        cerr << "Некорректная команда.\n";
        return;
    }
    iss >> word; // первое условие
    string t1cond = "", c1cond = "", s1 = "";
    if (findDot(word)) {
        splitDot(word, t1cond, c1cond, tjs);
    }
    else {
        s1 = ignoreQuotes(word);
    }
    string oper;
    iss >> oper; // оператор
    if (oper != "AND" && oper != "OR") {
        if (checkCond(tjs, t1, c1, t1cond, c1cond, s1)) {
            crossJoin(tjs, table1, table2, column1, column2);
            return;
        }
        else {
            cout << "Условие не выполняется.\n";
            return;
        }
    }
    iss >> word; // вторая таблица и колонка
    string t2, c2;
    splitDot(word, t2, c2, tjs);
    iss >> word; // "="
    if (word != "=") {
        cerr << "Некорректная команда.\n";
        return;
    }
    iss >> word; // второе условие
    string t2cond = "", c2cond = "", s2 = "";
    if (findDot(word)) {
        splitDot(word, t2cond, c2cond, tjs);
    }
    else {
        s2 = ignoreQuotes(word);
    }
    if (oper == "AND") {
        if (checkCond(tjs, t1, c1, t1cond, c1cond, s1) == true && checkCond(tjs, t2, c2, t2cond, c2cond, s2) == true) {
            crossJoin(tjs, table1, table2, column1, column2);
            return;
        }
        else {
            cout << "Условие не выполняется.\n";
            return;
        }
    }
    if (oper == "OR") {
        if (checkCond(tjs, t1, c1, t1cond, c1cond, s1) == true || checkCond(tjs, t2, c2, t2cond, c2cond, s2) == true) {
            crossJoin(tjs, table1, table2, column1, column2);
            return;
        }
        else {
            cout << "Условие не выполняется.\n";
            return;
        }
    }
}