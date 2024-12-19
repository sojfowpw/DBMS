#include </home/kali/Documents/GitHub/practice3_2024/httplib.h>
#include "api.h"
#include <iostream>

using namespace std;

void startHTTPServer(tableJson& tjs) {
    httplib::Server svr;
    pair<string, int> db_url;
    vector<string> lots = parsingLots(db_url); // парсинг лотов в вектор

    svr.Get("/lot", [&](const httplib::Request& req, httplib::Response& res) { // передаем в переменной ip и port DB
        getLots(req, res, tjs);
    });
    svr.Get("/pair", [&](const httplib::Request& req, httplib::Response& res) {
        getPairs(req, res, tjs);
    });
    svr.Get("/balance", [&](const httplib::Request& req, httplib::Response& res) {
        getBalance(req, res, tjs);
    });
    svr.Get("/order", [&](const httplib::Request& req, httplib::Response& res) {
        getOrder(req, res, tjs);
    });
    svr.Post("/user", [&](const httplib::Request& req, httplib::Response& res) {
        string username;
        createUser(req, res, tjs, username, db_url);
        fillUserLot(tjs, username);
    });
    svr.Post("/order", [&](const httplib::Request& req, httplib::Response& res) {
        createOrder(req, res, tjs, db_url);
    });
    svr.Delete("/order", [&](const httplib::Request& req, httplib::Response& res) {
        delOrder(req, res, tjs, db_url);
    });
    cout << "Сервер http запущен.\n";
    svr.listen("127.0.0.1", 7433);
}