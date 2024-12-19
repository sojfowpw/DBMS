#include "parser.h"
#include "insert.h"
#include "delete.h"
#include "select.h"
#include "api.h"

#include "parser.cpp"
#include "insert.cpp"
#include "delete.cpp"
#include "select.cpp"
#include "api.cpp"
#include "http.cpp"

#include <iostream>
#include <sys/socket.h> // функции для работы с сокетами
#include <netinet/in.h> // структуры данных для портов
#include <unistd.h> // функции для работы с системными вызовами
#include <string.h>
#include <thread>
#include <sstream>
#include <mutex>

using namespace std;

void handleRequest(const string& command, tableJson& tjs, mutex& mtx) { // обработка итоговых запросов
    unique_lock<mutex> lock(mtx);
    if (command == "") {
        cerr << "Некорректная команда.\n";
        return;
    }
    if (command.find("INSERT") == 0) { // вставка
        insert(command, tjs);
        return;
    }
    if (command.find("DELETE") == 0) { // удаление
        del(command, tjs);
        return;
    }
    lock.unlock();
}

void handleTCPConnection(int newSocket, tableJson& tjs, mutex& mtx) {
    char buffer[1024] = {0};
    int valread = read(newSocket, buffer, 1024);
    if (valread <= 0) {
        cerr << "Клиент отсоединился\n";
    }
    string command = string(buffer, valread);
    cout << "Команда, полученная от клиента: " << command << endl;
    handleRequest(command, tjs, mtx);
    send(newSocket, buffer, valread, 0);
    memset(buffer, 0, sizeof(buffer));
    close(newSocket);
}

int main() {
    mutex mtx;
    tableJson tjs;
    parsing(tjs);
    pair<string, int> db_url;
    vector<string> lots = parsingLots(db_url); // парсинг лотов в вектор
    for (auto& lot : lots) { // запись лотов в таблицу lot
        string cmd = "INSERT INTO lot VALUES ('" + lot + "')";
        insert(cmd, tjs);
    }
    string lotPath = "/home/kali/Documents/GitHub/practice3_2024/" + tjs.schemeName + "/lot/1.csv";
    rapidcsv::Document doc(lotPath);
    size_t amountRow = doc.GetRowCount();
    for (size_t i = 0; i < amountRow; i++) { // заполняем таблицу с парами
        for (size_t j = i + 1; j < amountRow; j++) {
            string cmd = "INSERT INTO pair VALUES ('" + doc.GetCell<string>(0, i) + "', '" + doc.GetCell<string>(0, j) + "')";
            insert(cmd, tjs);
        }
    }
    cout << "\n\n";

    cout << "Загрузка сервера tcp\n";
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); // создание сокета для сервера
    // AF_INET сокет используется для работы с IPv4 - протокол передачи информации внутри сети интернет
    // SOCK_STREAM - сокет типа TCP
    // использование протокола по умолчанию для данного типа сокета
    if (serverSocket == -1) {
        cerr << "Не удалось создать сокет\n";
        return 1;
    }

    sockaddr_in serverAddress; // определение адреса сервера, тип данных для хранения адреса сокета
    serverAddress.sin_family = AF_INET; // семейство адресов IPv4
    serverAddress.sin_addr.s_addr = INADDR_ANY; // 32 битный IPv4
    serverAddress.sin_port = htons(db_url.second); // преобразует номер порта 7432 из хостового порядка байтов в сетевой порядок байтов
    // привязываем сокет к указанному адресу и порту
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) { // (struct sockaddr*)&serverAddress - указатель на структуру sockaddr_in
        cerr << "Связь не удалась\n";
        close(serverSocket);
        return 1;
    }
    // прослушивание входящих соединений
    if (listen(serverSocket, 3) < 0) { // 3 максимальное количество соединений в очереди
        cerr << "Прослушивание не удалось\n";
        close(serverSocket);
        return 1;
    }
    cout << "Ожидание входящих соединений\n";

    thread httpThread([&]() {
        startHTTPServer(tjs);
    });
    httpThread.detach();

    vector<thread> clientThreads;
    
    while (true) { // принятие соединений
        sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress); // размер 
        int newSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength); // принятие клиента
        if (newSocket < 0) {
            cerr << "Соединение не принято\n";
            close(serverSocket);
            continue;
        }
        clientThreads.emplace_back(thread(handleTCPConnection, newSocket, ref(tjs), ref(mtx)));
    }
    for (auto& t : clientThreads) {
            t.join();
        }
    close(serverSocket);
    return 0;
}