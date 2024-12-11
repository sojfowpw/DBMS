#pragma once
#include <iostream>
#include <fstream>
#include <filesystem> // директории
#include "json.hpp" // json

#include "insert.h" // структура таблиц

using namespace std;
using json = nlohmann::json; 
namespace fs = filesystem;

struct tableJson { 
    int tableSize; // размер колонок
    string schemeName; // имя схемы
    tNode* tablehead; // указатель на голову таблицы
};

void removeDirectory(const fs::path& directoryPath); // удаление директории
void createDirectoriesAndFiles(const fs::path& schemePath, const json& structure, tableJson& tjs); // создание полной директории и файлов
void parsing(tableJson& tjs); // парсинг схемы