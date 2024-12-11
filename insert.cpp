#include "insert.h"

bool isLocked(const string& tableName, const string& schemeName) { // проверка блокировки таблицы
    string fileName = "/home/kali/Documents/GitHub/practice3_2024/" + schemeName + "/" + tableName + "/" + tableName + "_lock.txt";
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Не удалось открыть файл.\n";
    }
    string current; // чтение текущего значения блокировки
    file >> current;
    file.close();
    if (current == "locked") {
        return true; // заблокирована
    }
    return false; // разблокирована
}

void locker(const string& tableName, const string& schemeName) { // изменение состояния блокировки
    string fileName = "/home/kali/Documents/GitHub/practice3_2024/" + schemeName + "/" + tableName + "/" + tableName + "_lock.txt";
    ifstream fileIn(fileName);
    if (!fileIn.is_open()) {
        cerr << "Не удалось открыть файл.\n";
        return;
    }
    string current; // чтение текущего значения блокировки
    fileIn >> current;
    fileIn.close();
    ofstream fileOut(fileName); // перезаписываем файл
    if (!fileOut.is_open()) {
        cerr << "Не удалось открыть файл.\n";
        return;
    }
    if (current == "locked") { // если таблица заблокирована, меняем на разблокирована
        fileOut << "unlocked";
    } else {
        fileOut << "locked"; // если была разблокирована, становится заблокирована
    }
    fileOut.close();
}

bool isTableExist(const string& tableName, tNode* tableHead) { // проверка: существует ли таблица
    tNode* current = tableHead; // указатель на голову списка таблиц
    while (current) {
        if (current->table == tableName) {
            return true; // таблица с указанным именем существует
        }
        current = current->next;
    }
    return false; // такой таблицы нет
}

void copyColumnsName(const string& fileFrom, const string& fileTo) { // копирование названия колонок
    string columns; // строка для колонок
    ifstream fileF(fileFrom); // открываем файл для чтения колонок
    if (!fileF.is_open()) {
        cerr << "Не удалось открыть файл.\n";
        return;
    }
    fileF >> columns;
    fileF.close();
    ofstream fileT(fileTo); // открываем файл для записи колонок
    if (!fileT.is_open()) {
        cerr << "Не удалось открыть файл.\n";
        return;
    }
    fileT << columns << endl;
    fileT.close();
}

void insert(const string& command, tableJson& tjs) { // вставка
    istringstream iss(command); // поток ввода для обработки строки команды
    string word;
    iss >> word; // "INSERT"
    iss >> word; // "INTO"
    if (word != "INTO") {
        cerr << "Некорректная команда.\n";
        return;
    }
    string tableName; // имя таблицы из команды
    iss >> word; // название таблицы
    if (isTableExist(word, tjs.tablehead) == false) {
        cerr << "Такой таблицы нет.\n";
        return;
    }
    tableName = word;
    iss >> word; // "VALUES"
    if (word != "VALUES") {
        cerr << "Некорректная команда.\n";
        return;
    }
    string values; // строка для вытаскивания значений вставки
    while (iss >> word) {
        values += word;
    }
    if (values.front() != '(' || values.back() != ')') {
        cerr << "Некорректная команда.\n";
        return;
    }

    if (isLocked(tableName, tjs.schemeName) == true) { // проверяем, доступна ли таблица для работы
        cerr << "Таблица заблокирована.\n";
        return;
    }
    locker(tableName, tjs.schemeName); // блокируем таблицу для вставки
    int currentPk; // текущий первичный ключ
    string pkFile = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/" + tableName + "/" + tableName + "_id_sequence.txt";
    ifstream fileIn(pkFile);
    if (!fileIn.is_open()) {
        cerr << "Не удалось открыть файл.\n";
        return;
    }
    fileIn >> currentPk; // считываем ключ в переменную
    fileIn.close();
    ofstream fileOut(pkFile);
    if (!fileOut.is_open()) {
        cerr << "Не удалось открыть файл.\n";
        return;
    }
    currentPk++;
    fileOut << currentPk; // увеличиваем на 1 и записываем новый ключ в файл
    fileOut.close();


    int csvNum = 1; // номер файла csv
    while (true) {
        string csvFile = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/" + tableName + "/" + to_string(csvNum) + ".csv";
        ifstream fileIn(csvFile); // открываем csv файл
        if (!fileIn.is_open()) {
            ofstream fileOut(csvFile); // если такого файла нет, создаём новый
            if (!fileOut.is_open()) {
                cerr << "Не удалось открыть файл.\n";
                return;
            }
            fileOut.close();
        }
        else {
            fileIn.close();
        }
        rapidcsv::Document doc(csvFile); // считываем csv в doc
        if (doc.GetRowCount() < tjs.tableSize) { // если количество строк не превышает лимит
            break; // завершаем цикл
        }
        csvNum++; // иначе продолжаем искать пустые файлы
    }
    string csvOne = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/" + tableName + "/1.csv";
    string csvF = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/" + tableName + "/" + to_string(csvNum) + ".csv";
    rapidcsv::Document doc_(csvF); // считываем содержимое файла с помощью rapid
    if (doc_.GetRowCount() == 0 && doc_.GetColumnCount() == 0) { // если текущий файл пуст
        copyColumnsName(csvOne, csvF); // записываем в него названия колонок
    }
    ofstream csv(csvF, ios::app);
    if (!csv.is_open()) {
        cerr << "Не удалось открыть файл.\n";
        return;
    }
    csv << currentPk << ","; // записываем текущий первичный ключ
    for (int i = 0; i < values.size(); i++) {
        if (values[i] == '\'') {
            i++;
            while (values[i] != '\'') {
                csv << values[i]; // записываем сами значения
                i++;
            }
            if (values[i + 1] != ')') {
                csv << ","; // если слово не последнее, ставим ,
            }
            else {
                csv << endl;
            }
        }
    }
    csv.close();
    locker(tableName, tjs.schemeName); // разблокировка
}