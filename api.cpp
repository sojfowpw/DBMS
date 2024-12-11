string generateKey() {
    const string charset = "0123456789abcdef";
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distribution(0, charset.size() - 1);
    string key;
    for (int i = 0; i < 32; i++) {
        key += charset[distribution(gen)];
    }
    return key;
}

void createUser(const httplib::Request& req, httplib::Response& res, tableJson& tjs, string& username) {
    if (req.body.empty()) {
        res.set_content("{\"error\": \"Request body is empty\"}", "application/json");
        return;
    }
    json requestBody;
    requestBody = json::parse(req.body);
    username = requestBody["username"];
    string key = generateKey();

    string insertCmd = "INSERT INTO user VALUES ('" + username + "', '" + key + "')";
    insert(insertCmd, tjs);
    json response;
    response["key"] = key;
    res.set_content(response.dump(), "application/json"); 
}

void fillUserLot(tableJson& tjs, const string& username) { // заполняем таблицу user lot
    string filename1 = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/user/1.csv";
    string userInd;
    rapidcsv::Document doc1(filename1);
    size_t amountRow1 = doc1.GetRowCount();
    for (size_t i = 0; i < amountRow1; i++) { 
        if (doc1.GetCell<string>(1, i) == username) {
            userInd = doc1.GetCell<string>(0, i);
        }
    }

    string filename2 = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/lot/1.csv";
    vector<string> lots;
    rapidcsv::Document doc2(filename2);
    size_t amountRow2 = doc2.GetRowCount();
    for (size_t i = 0; i < amountRow2; i++) { 
        lots.push_back(doc2.GetCell<string>(0, i));
    }
    vector<string> commands;
    for (auto& lot : lots) {
        string cmd = "INSERT INTO user_lot VALUES ('" + userInd + "', '" + lot + "', '1000')";
        insert(cmd, tjs);
    }
}

void getLots(const httplib::Request& req, httplib::Response& res, tableJson& tjs) { // запрос get lot
    string filename = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/lot/1.csv";
    rapidcsv::Document doc(filename);
    vector<pair<int, string>> lots;
    size_t amountRow = doc.GetRowCount();
    for (size_t i = 0; i < amountRow; i++) {
        lots.emplace_back(doc.GetCell<int>(0, i), doc.GetCell<string>(1, i));
    }

    json response = json::array();
    for (auto& lot : lots) {
        json lotJson;
        lotJson["lot_id"] = lot.first;
        lotJson["name"] = lot.second;
        response.push_back(lotJson);
    }
    res.set_content(response.dump(), "application/json");
}

void getPairs(const httplib::Request& req, httplib::Response& res, tableJson& tjs) { // запрос get pair
    string filename = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/pair/1.csv";
    rapidcsv::Document doc(filename);
    vector<tuple<int, int, int>> pairs;
    size_t amountRow = doc.GetRowCount();
    for (size_t i = 0; i < amountRow; i++) {
        pairs.emplace_back(doc.GetCell<int>(0, i), doc.GetCell<int>(1, i), doc.GetCell<int>(2, i));
    }

    json response = json::array();
    for (auto& pair : pairs) {
        json pairJson;
        pairJson["pair_id"] = get<0>(pair);
        pairJson["first_lot_id"] = get<1>(pair);
        pairJson["second_lot_id"] = get<2>(pair);
        response.push_back(pairJson);
    }
    res.set_content(response.dump(), "application/json");
}

void getBalance(const httplib::Request& req, httplib::Response& res, tableJson& tjs) { // запрос get balance
    string userKey = req.get_header_value("X-USER-KEY");
    string userId;
    string filename1 = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/user/1.csv";
    rapidcsv::Document doc1(filename1);
    size_t amountRow1 = doc1.GetRowCount();
    for (size_t i = 0; i < amountRow1; i++) {
        if (doc1.GetCell<string>(2, i) == userKey) {
            userId = doc1.GetCell<string>(0, i);
        }
    }
    string filename2 = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/user_lot/1.csv";
    rapidcsv::Document doc2(filename2);
    size_t amountRow2 = doc2.GetRowCount();
    json response = json::array();
    json balanceJson;
    for (size_t i = 0; i < amountRow2; i++) { 
        if (doc2.GetCell<string>(1, i) == userId) {
            balanceJson["lot_id"] = doc2.GetCell<int>(2, i);
            balanceJson["quantity"] = doc2.GetCell<float>(3, i);
            response.push_back(balanceJson);
        }
    }
    res.set_content(response.dump(), "application/json");
}

vector<string> parsingLots() { // парсинг json схемы с лотами
    string filename = "/home/kali/Documents/GitHub/practice3_2024/lots.json"; 
    ifstream file(filename); // открываем файл для чтения
    if (!file.is_open()) {
        cerr << "Не удалось открыть файл: " << filename << endl;
    }
    string json_content, line; // читаем построчно содержимое
    while (getline(file, line)) {
        json_content += line;
    }
    file.close();

    json jparsed;
    jparsed = json::parse(json_content);
    vector<string> lots = jparsed["lots"].get<vector<string>>(); // парсим лоты в вектор
    return lots;
}