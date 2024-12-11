#pragma once
#include <string>
#include <vector>
#include <fstream>
#include "parser.h"
#include "httplib.h"

string generateKey();
void createUser(const httplib::Request& req, httplib::Response& res, tableJson& tjs);
vector<string> parsingLots(); // парсинг json схемы с лотами
void getLots(const httplib::Request& req, httplib::Response& res, tableJson& tjs); // запрос get lot
void getPairs(const httplib::Request& req, httplib::Response& res, tableJson& tjs); // запрос get pair