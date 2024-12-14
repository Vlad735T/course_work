#include <iostream>
#include <boost/asio.hpp>
#include <string>

using namespace std;
using boost::asio::ip::tcp;

const size_t MAX_BUFFER_SIZE = 8192; // Максимальный размер буфера для чтения данных

void receive_data(tcp::socket& socket) {
    char buffer[MAX_BUFFER_SIZE];
    boost::system::error_code error;

    cout << "Receiving data from server...\n";

    while (true) {
        // Читаем данные из сокета в буфер
        size_t len = socket.read_some(boost::asio::buffer(buffer), error);

        // Обработка ошибок
        if (error == boost::asio::error::eof) {
            // Сервер закрыл соединение
            cout << "Connection closed by server." << endl;
            break;
        } else if (error) {
            cerr << "Error while reading from server: " << error.message() << endl;
            break;
        }

        // Печатаем полученные данные
        cout << string(buffer, len);

        // Если размер меньше буфера, значит передача завершена
        if (len < MAX_BUFFER_SIZE) {
            break;
        }
    }

    cout << "\nFinished receiving data from server." << endl;
}

int main() {
    try {
        // Создаем объект io_context для работы с асинхронными операциями
        boost::asio::io_context io_context;

        // Указываем IP-адрес и порт сервера
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("127.0.0.1", "7432"); // localhost, порт 7432

        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        cout << "Connected to the server." << endl;

        while (true) {
            // Чтение команды от пользователя
            string command;
            cout << "Enter command (1 to get data, 'exit' to disconnect): ";
            getline(cin, command);

            // Отправляем команду на сервер
            boost::asio::write(socket, boost::asio::buffer(command + "\n"));

            if (command == "exit") {
                cout << "Exiting the client application." << endl;
                break; // Закрытие соединения, если команда 'exit'
            }

            // Получение данных с сервера
            receive_data(socket);
        }

        socket.close(); // Закрываем сокет
    } catch (const std::exception& e) {
        cerr << "Exception: " << e.what() << endl;
    }

    return 0;
}
