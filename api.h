#pragma once
#include <string>
#include <vector>
#include <fstream>
#include "parser.h"
#include "httplib.h"

string generateKey();
void createUser(const httplib::Request& req, httplib::Response& res, tableJson& tjs);