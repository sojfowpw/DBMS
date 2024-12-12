#include "api.h"
#include "delete.h"

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
        pairJson["sale_lot_id"] = get<1>(pair);
        pairJson["buy_lot_id"] = get<2>(pair);
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

void updateBalance(string userId, string pairId, string type, float quantity, float price, tableJson& tjs) { // обновление баланса
    int sell, buy;
    float newBalance;
    string pairFile = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/pair/1.csv";
    rapidcsv::Document docP(pairFile);
    size_t pairRow = docP.GetRowCount();
    for (size_t i = 0; i < pairRow; i++) {
        if (docP.GetCell<string>(0, i) == pairId) {
            if (type == "buy") {
                buy = docP.GetCell<int>(1, i);
                sell = docP.GetCell<int>(2, i);
                string userLotFile = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/user_lot/1.csv";
                rapidcsv::Document docUL(userLotFile);
                size_t userLotRow = docUL.GetRowCount();
                for (size_t j = 0; j < userLotRow; j++) {
                    if (docUL.GetCell<string>(1, j) == userId) {
                        if (docUL.GetCell<int>(2, j) == buy) {
                            newBalance = docUL.GetCell<float>(3, j) + quantity; // добавляем купленное количество
                            docUL.SetCell<float>(3, j, newBalance);
                        }
                        if (docUL.GetCell<int>(2, j) == sell) {
                            newBalance = docUL.GetCell<float>(3, j) - (quantity * price); // вычитаем стоимость покупки
                            docUL.SetCell<float>(3, j, newBalance);
                        }
                    }
                }
                docUL.Save(userLotFile);
            }
            if (type == "sell") {
                sell = docP.GetCell<int>(1, i);
                buy = docP.GetCell<int>(2, i);
                string userLotFile = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/user_lot/1.csv";
                rapidcsv::Document docUL(userLotFile);
                size_t userLotRow = docUL.GetRowCount();
                for (size_t j = 0; j < userLotRow; j++) {
                    if (docUL.GetCell<string>(1, j) == userId) {
                        if (docUL.GetCell<int>(2, j) == buy) {
                            newBalance = docUL.GetCell<float>(3, j) + (quantity * price); // добавляем купленное количество
                            docUL.SetCell<float>(3, j, newBalance);
                        }
                        if (docUL.GetCell<int>(2, j) == sell) {
                            newBalance = docUL.GetCell<float>(3, j) - quantity; // вычитаем стоимость покупки
                            docUL.SetCell<float>(3, j, newBalance);
                        }
                    }
                }
                docUL.Save(userLotFile);
            }
        }
    }
}

void processing(tableJson& tjs) {
    string orderId, userId, pairId, type;
    float quantity, price;
    string filename = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/order/1.csv";
    rapidcsv::Document doc(filename);
    size_t amountRow;
    amountRow = doc.GetRowCount();
    for (size_t i = 0; i < amountRow; i++) {
        if (doc.GetCell<string>(6, i) == "0" && doc.GetCell<string>(5, i) == "buy") {
            orderId = doc.GetCell<string>(0, i);
            userId = doc.GetCell<string>(1, i);
            pairId = doc.GetCell<string>(2, i);
            quantity = doc.GetCell<float>(3, i);
            price = doc.GetCell<float>(4, i);
            type = doc.GetCell<string>(5, i);
            break;
        }
    }
    bool isFind = false;
    string q, p;
    for (size_t i = 0; i < amountRow; i++) {
        if (doc.GetCell<string>(0, i) != orderId && doc.GetCell<string>(1, i) != userId) {
            if (doc.GetCell<string>(2, i) == pairId && doc.GetCell<string>(6, i) == "0" && doc.GetCell<string>(5, i) != type) {
                if (doc.GetCell<float>(3, i) <= quantity && doc.GetCell<float>(4, i) <= price) {
                    isFind = true;
                    q = doc.GetCell<string>(3, i);
                    p = doc.GetCell<string>(4, i);
                    doc.SetCell<string>(6, i, "closed");
                    doc.Save(filename);
                    updateBalance(doc.GetCell<string>(1, i), doc.GetCell<string>(2, i), doc.GetCell<string>(5, i),
                    doc.GetCell<float>(3, i), doc.GetCell<float>(4, i), tjs);
                    break;
                }
            }
        }
    }
    if (isFind == false) {
        return;
    }
    string insertCmd = "";
    for (size_t i = 0; i < amountRow; i++) {
        if (doc.GetCell<string>(0, i) == orderId) {
            float diff = quantity - stof(q);
            if (diff != 0) {
                insertCmd = "INSERT INTO order VALUES ('" + userId + "', '" + pairId + "', '" + to_string(diff) + "', '" +
                to_string(price) + "', '" + type + "', '0')";
            }
            doc.SetCell<string>(3, i, q);
            doc.SetCell<string>(4, i, p);
            doc.SetCell<string>(6, i, "closed");
            doc.Save(filename);
            updateBalance(userId, pairId, type, stof(q), stof(p), tjs);
        }
    }
    if (insertCmd != "") {
        insert(insertCmd, tjs);
    }
}

void createOrder(const httplib::Request& req, httplib::Response& res, tableJson& tjs) { // запрос на создание ордера
    if (req.body.empty()) {
        res.set_content("{\"error\": \"Request body is empty\"}", "application/json");
        return;
    }
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

    json requestBody;
    requestBody = json::parse(req.body);
    int pairId = requestBody["pair_id"];
    float quantity = requestBody["quantity"];
    float price = requestBody["price"];
    string type = requestBody["type"];
    string insertCmd = "INSERT INTO order VALUES ('" + userId + "', '" + to_string(pairId) + "', '" + to_string(quantity) +
    "', '" + to_string(price) + "', '" + type + "', '0')";
    insert(insertCmd, tjs);

    string filePath2 = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/order/1.csv";
    rapidcsv::Document doc2(filePath2);
    size_t amountRow2 = doc2.GetRowCount();
    int orderId;
    for (size_t i = 0; i < amountRow2; i++) {
        orderId = doc2.GetCell<int>(0, i);
    }
    json response;
    response["order_id"] = orderId;

    processing(tjs);

    res.set_content(response.dump(), "application/json");
}

void getOrder(const httplib::Request& req, httplib::Response& res, tableJson& tjs) { // запрос get order
    string filename = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/order/1.csv";
    rapidcsv::Document doc(filename);
    vector<vector<string>> orders;
    size_t amountRow;
    amountRow = doc.GetRowCount();
    for (size_t i = 0; i < amountRow; i++) {
        vector<std::string> row;
        row.push_back(doc.GetCell<string>(0, i)); // order_id
        row.push_back(doc.GetCell<string>(1, i)); // user_id
        row.push_back(doc.GetCell<string>(2, i)); // pair_id
        row.push_back(doc.GetCell<string>(3, i)); // quantity
        row.push_back(doc.GetCell<string>(4, i)); // price
        row.push_back(doc.GetCell<string>(5, i)); // type
        row.push_back(doc.GetCell<string>(6, i)); // closed
        orders.push_back(row);
    }

    json response = json::array();
    for (auto& order : orders) {
        json orderJson;
        orderJson["order_id"] = stoi(order[0]);
        orderJson["user_id"] = stoi(order[1]);
        orderJson["pair_id"] = stoi(order[2]);
        orderJson["quantity"] = stof(order[3]);
        orderJson["price"] = stof(order[4]);
        orderJson["type"] = order[5];
        orderJson["closed"] = order[6];
        response.push_back(orderJson);
    }
    res.set_content(response.dump(), "application/json");
}

void delOrder(const httplib::Request& req, httplib::Response& res, tableJson& tjs) {
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
    json requestBody;
    requestBody = json::parse(req.body);
    int orderId = requestBody["order_id"];

    string filename2 = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/order/1.csv";
    rapidcsv::Document doc2(filename2);
    size_t amountRow2 = doc2.GetRowCount();
    for (size_t i = 0; i < amountRow2; i++) {
        if (doc2.GetCell<string>(1, i) == userId && doc2.GetCell<int>(0, i) == orderId && doc2.GetCell<string>(6, i) == "0") {
            string delCmd = "DELETE FROM order WHERE order.order_id = '" + to_string(orderId) + "'";
            del(delCmd, tjs);
            break;
        }
    }
    json response;
    response["message"] = "Order deleted successfully";
    res.set_content(response.dump(), "application/json");
}