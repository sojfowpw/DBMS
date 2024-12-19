#pragma once
#include <string>
#include <vector>
#include <fstream>
#include "parser.h"
#include "httplib.h"

string generateKey();
void createUser(const httplib::Request& req, httplib::Response& res, tableJson& tjs, string& username, pair<string, int> db_url);
void fillUserLot(tableJson& tjs, const string& username); // заполняем таблицу user lot
void getLots(const httplib::Request& req, httplib::Response& res, tableJson& tjs); // запрос get lot
void getPairs(const httplib::Request& req, httplib::Response& res, tableJson& tjs); // запрос get pair
void getBalance(const httplib::Request& req, httplib::Response& res, tableJson& tjs); // запрос get balance
vector<string> parsingLots(pair<string, int>& db_url); // парсинг json схемы с лотами
void updateBalance(string userId, string pairId, float quantity, float price, string type, tableJson& tjs); // обновление баланса
void processing(tableJson& tjs, pair<string, int> db_url);
void createOrder(const httplib::Request& req, httplib::Response& res, tableJson& tjs, pair<string, int> db_url); // запрос на создание ордера
void getOrder(const httplib::Request& req, httplib::Response& res, tableJson& tjs); // запрос get order
void delOrder(const httplib::Request& req, httplib::Response& res, tableJson& tjs, pair<string, int> db_url);