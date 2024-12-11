#pragma once
#include <iostream>
#include <fstream>
#include "insert.h"

using namespace std;

void del(const string& command, tableJson& tjs); // удаление
bool isColumnExist(const string& tableName, const string& columnName, tableJson& tjs); // поиск колонки