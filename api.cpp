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

void createUser(const httplib::Request& req, httplib::Response& res, tableJson& tjs) {
    if (req.body.empty()) {
        res.set_content("{\"error\": \"Request body is empty\"}", "application/json");
        return;
    }
    json requestBody;
    requestBody = json::parse(req.body);
    string username = requestBody["username"];
    string key = generateKey();

    string insertCmd = "INSERT INTO user VALUES ('" + username + "', '" + key + "')";
    insert(insertCmd, tjs);
    json response;
    response["key"] = key;
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