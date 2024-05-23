#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

using namespace std;

int main() {
    // Переменные для хранения информации о WSA, адресов и сокетов
    WSADATA wsaData;
    ADDRINFO hints;
    ADDRINFO* addrResult = nullptr;
    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ConnectSocket = INVALID_SOCKET;
    char recvBuffer[512]; // Буфер для приема данных

    // Сообщение, которое сервер отправляет клиенту
    const char* sendBuffer = "Hello from server";

    // Инициализация Winsock
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup failed with result: " << result << endl;
        return 1;
    }

    // Настройка структуры hints для getaddrinfo
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // Потоковый сокет (TCP)
    hints.ai_protocol = IPPROTO_TCP; // Протокол TCP
    hints.ai_flags = AI_PASSIVE; // Для привязки

    // Разрешение адреса и порта сервера
    result = getaddrinfo(NULL, "666", &hints, &addrResult);
    if (result != 0) {
        cout << "getaddrinfo failed with error: " << result << endl;
        WSACleanup();
        return 1;
    }

    // Создание сокета для прослушивания
    // Мы создаем сокет, чтобы сервер мог прослушивать входящие подключения.
    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        cout << "Socket creation failed" << endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Привязка сокета к IP-адресу и порту
    // Привязываем сокет к адресу и порту, чтобы сервер мог получать данные через этот сокет.
    result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cout << "Bind failed, error: " << result << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Прослушивание входящих соединений
    // Начинаем прослушивать на созданном сокете входящие соединения.
    result = listen(ListenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        cout << "Listen failed, error: " << result << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Ожидание входящего соединения
    // Ожидаем подключения клиента. Когда клиент подключается, возвращается новый сокет для общения с этим клиентом.
    ConnectSocket = accept(ListenSocket, NULL, NULL);
    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Accept failed, error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Закрытие сокета прослушивания, так как соединение установлено
    // Закрываем сокет прослушивания, так как он больше не нужен.
    closesocket(ListenSocket);

    // Цикл приема данных от клиента и отправки ответа
    do {
        ZeroMemory(recvBuffer, 512); // Очистка буфера приема
        result = recv(ConnectSocket, recvBuffer, 512, 0); // Прием данных от клиента
        if (result > 0) {
            cout << "Received " << result << " bytes" << endl;
            cout << "Received data: " << recvBuffer << endl;

            // Отправка ответа клиенту
            // Отправляем ответ клиенту с помощью функции send.
            result = send(ConnectSocket, sendBuffer, (int)strlen(sendBuffer), 0);
            if (result == SOCKET_ERROR) {
                cout << "Send failed, error: " << result << endl;
                closesocket(ConnectSocket);
                freeaddrinfo(addrResult);
                WSACleanup();
                return 1;
            }
        }
        else if (result == 0) {
            cout << "Connection closing" << endl; // Клиент закрыл соединение
        }
        else {
            cout << "Recv failed, error: " << WSAGetLastError() << endl;
            closesocket(ConnectSocket);
            freeaddrinfo(addrResult);
            WSACleanup();
            return 1;
        }
    } while (result > 0);

    // Остановка отправки и закрытие соединения
    // Закрываем отправку данных через сокет, указывая, что отправка завершена.
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        cout << "Shutdown failed, error: " << result << endl;
        closesocket(ConnectSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Очистка и завершение работы
    // Закрываем сокет и очищаем ресурсы перед завершением работы.
    closesocket(ConnectSocket);
    freeaddrinfo(addrResult);
    WSACleanup();
    return 0;
}
