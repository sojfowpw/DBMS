#pragma once
#include <string>
#include <vector>
#include <fstream>
#include "parser.h"
#include "httplib.h"

string generateKey();
void createUser(const httplib::Request& req, httplib::Response& res, tableJson& tjs, string& username);
void fillUserLot(tableJson& tjs, const string& username); // заполняем таблицу user lot
vector<string> parsingLots(); // парсинг json схемы с лотами
void getLots(const httplib::Request& req, httplib::Response& res, tableJson& tjs); // запрос get lot
void getPairs(const httplib::Request& req, httplib::Response& res, tableJson& tjs); // запрос get pair
void getBalance(const httplib::Request& req, httplib::Response& res, tableJson& tjs); // запрос get balance
void updateBalance(string userId, string pairId, float quantity, float price, string type, tableJson& tjs); // обновление баланса
void createOrder(const httplib::Request& req, httplib::Response& res, tableJson& tjs); // запрос на создание ордера
void getOrder(const httplib::Request& req, httplib::Response& res, tableJson& tjs); // запрос get order