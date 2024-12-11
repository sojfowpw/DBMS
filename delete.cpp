#include "delete.h"

bool isColumnExist(const string& tableName, const string& columnName, tNode* tableHead) { // поиск колонки
    tNode* currentTable = tableHead; // указатель на голову структуры
    while (currentTable) {
        if (currentTable->table == tableName) { // ищем заданную таблицу
            Node* currentColumn = currentTable->column; // указатель на первую колонку в искомой таблицк
            while (currentColumn) {
                if (currentColumn->column == columnName) { // ищем колонку
                    return true; // если нашли
                }
                currentColumn = currentColumn->next;
            }
            return false; // если колонки нет
        }
        currentTable = currentTable->next;
    }
    return false;
}

void del(const string& command, tableJson& tjs) { // удаление
    istringstream iss(command); // поток ввода для обработки строки команды
    string word;
    iss >> word; // "DELETE"
    iss >> word; // "FROM"
    if (word != "FROM") {
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
    
    iss >> word; // "WHERE"
    if (word != "WHERE") {
        cerr << "Некорректная команда.\n";
        return;
    }
    iss >> word; // таблица и колонка
    string table, column; // названия таблицы и колонки из команды
    bool dot = false; // поиск точки 
    for (size_t i = 0; i < word.size(); i++) {
        if (word[i] == '.') {
            dot = true;
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
    if (table != tableName) { // правильность таблицы
        cerr << "Некорректная команда.\n";
        return;
    }
    if (isColumnExist(tableName, column, tjs.tablehead) == false) { // проверка на существование колонки
        cerr << "Такой колонки нет.\n";
        return;
    }
    iss >> word; // "="
    if (word != "=") {
        cerr << "Некорректная команда.\n";
        return;
    }
    string value; // удаляемое значение
    iss >> word;
    if (word[0] != '\'' || word[word.size() - 1] != '\'') { // проверка на кавычки
        cerr << "Некорректная команда.\n";
        return;
    }
    for (size_t i = 1; i < word.size() - 1; i++) { // вытаскиваем значение из кавычек
        value += word[i];
    }

    if (isLocked(tableName, tjs.schemeName) == true) { // проверяем, доступна ли таблица для работы
        cerr << "Таблица заблокирована.\n";
        return;
    }
    locker(tableName, tjs.schemeName); // блокируем таблицу для удаления

    int amountCsv = 1; // ищем количество созданных csv файлов
    while (true) {
        string filePath = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/" + tableName + "/" + to_string(amountCsv) + ".csv";
        ifstream file(filePath);
        if (!file.is_open()) { // если файл нельзя открыть, его нет
            break;
        }
        file.close();
        amountCsv++;
    }
    bool deletedStr = false; // для определения существования заданного значения
    for (size_t iCsv = 1; iCsv < amountCsv; iCsv++) { // просматриваем все созданные файлы csv
        string filePath = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/" + tableName + "/" + to_string(iCsv) + ".csv";
        rapidcsv::Document doc(filePath); // открываем файл
        int columnIndex = doc.GetColumnIdx(column); // считываем индекс искомой колонки
        size_t amountRow = doc.GetRowCount(); // считываем количество строк в файле
        for (size_t i = 0; i < amountRow; ++i) { // проходимся по строкам
            if (doc.GetCell<string>(columnIndex, i) == value) { // извлекаем значение (индекс колонки, номер строки)
                doc.RemoveRow(i); // удаляем строку
                deletedStr = true;
                --amountRow; // уменьшаем количество строк
                --i;
            }
        }
        doc.Save(filePath); 
    }
    if (deletedStr == false) {
        cout << "Указанное значение не найдено.\n";
    }
    locker(tableName, tjs.schemeName); // разблокировка
}