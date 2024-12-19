#include <iostream>
#include <sys/socket.h> // функции для работы с сокетами
#include <netinet/in.h> // структуры данных для портов
#include <arpa/inet.h> // функции для работы с сетевыми адресами
#include <unistd.h> // функции для работы с системными вызовами
#include <string.h>

using namespace std;

void db_request(pair<string, int> db_url, string request) {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0); // создание сокета клиента
    if (clientSocket == -1) {
        cerr << "Не удалось создать сокет\n";
    }
    // определение адреса сервера
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(db_url.second);

    if (inet_pton(AF_INET, db_url.first.c_str(), &serverAddress.sin_addr) <= 0) { // преобразует текстовое представление IP-адреса в двоичную форму
        cerr << "Неправильный адрес\n";
        close(clientSocket);
    }
    // соединение с сервером
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        cerr << "Соединение не принято\n";
        close(clientSocket);
    }
    char buffer[1024] = {0};
    send(clientSocket, request.c_str(), request.length(), 0); // отправляет сообщение на сервер, c_str - указатель на строку
    int valread = read(clientSocket, buffer, 1024); // чтение данных в буфер, valread - количество байт
    if (valread <= 0) {
        cerr << "Сервер отсоединился\n";
    }
    memset(buffer, 0, sizeof(buffer)); // очищаем и заполняем буфер нулями
    close(clientSocket);
}