#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <boost/asio.hpp>
#include <sstream>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic> // Для атомарных переменных
#include <memory> 

using namespace std;
using boost::asio::ip::tcp;

atomic<int> num_of_conn_clients(0);        
atomic<bool> server_running(true);     // Флаг работы сервера
const size_t MAX_BUFFER_SIZE = 8192;

vector<shared_ptr<tcp::socket>> active_socket_clients;
mutex socket_mutex;                             // Синхронизация доступа к client_sockets

void handle_client(shared_ptr<tcp::socket> socket, int client_id) {
    try {
        string client_ip = socket->remote_endpoint().address().to_string();

        num_of_conn_clients++;
        cout << "Client " << client_id << " connected with IP: " << client_ip << endl;
        cout << "Currently, " << num_of_conn_clients.load() << " client(s) connected." << endl;

        pqxx::connection conn("dbname=data_based_of_cars user=admin password=Qwerty1!@ host=127.0.0.1 port=5432");
        if (!conn.is_open()) {
            cerr << "Error with client " << client_id << ": Cannot connect to database" << endl;
            return;
        }
        cout << "Database connected successfully for client " << client_id << endl;

        boost::asio::streambuf buffer;

        while (server_running) {
            cout << "Waiting for client " << client_id << " message..." << endl;

            // Проверяем флаг перед началом операции
            if (!server_running) break;

            boost::system::error_code ec;
            boost::asio::read_until(*socket, buffer, "\n", ec);

            if (ec) {
                if (ec == boost::asio::error::eof) {
                    cout << "Client " << client_id << " disconnected gracefully." << endl;
                } else {
                    cerr << "Error with client " << client_id << ": " << ec.message() << endl;
                }
                break;
            }

            std::istream input_stream(&buffer);
            string command;
            getline(input_stream, command);

            cout << "Received command from client " << client_id << ": " << command << endl;

            if (command == "1") {
                cout << "Processing request for data from client " << client_id << "..." << endl;
                pqxx::work txn(conn);

                string owners_query = "SELECT * FROM owners;";
                string cars_query = "SELECT * FROM cars;";

                pqxx::result owners_result = txn.exec(owners_query);
                pqxx::result cars_result = txn.exec(cars_query);

                ostringstream response_stream;

                response_stream << "Owners:\n";
                for (const auto& row : owners_result) {
                    response_stream << "ID Owner: " << row["id_owner"].as<string>()
                                    << ", Surname: " << row["surname"].as<string>()
                                    << ", Name: " << row["name"].as<string>()
                                    << ", Middle Name: " << row["middle_name"].as<string>()
                                    << ", Phone: " << row["phone_number"].as<string>()
                                    << ", Email: " << row["email"].as<string>() << "\n";
                }

                response_stream << "\nCars:\n";
                for (const auto& row : cars_result) {
                    response_stream << "ID Car: " << row["id_car"].as<string>()
                                    << ", Brand: " << row["brand_of_car"].as<string>()
                                    << ", Number: " << row["number_of_car"].as<string>()
                                    << ", Region: " << row["region"].as<string>()
                                    << ", Power: " << row["power"].as<string>()
                                    << ", Engine Volume: " << row["engine_volume"].as<string>()
                                    << ", Release Year: " << row["release_year"].as<string>()
                                    << ", Owner ID: " << row["id_owner"].as<string>() << "\n";
                }

                string response = response_stream.str();
                size_t total_size = response.size();
                size_t offset = 0;

                while (offset < total_size) {
                    size_t chunk_size = min(MAX_BUFFER_SIZE, total_size - offset);
                    boost::asio::write(*socket, boost::asio::buffer(response.data() + offset, chunk_size));
                    offset += chunk_size;
                }

                string end_marker = "END_OF_DATA\n";
                boost::asio::write(*socket, boost::asio::buffer(end_marker));

                txn.commit();
            } else if (command == "exit") {
                cout << "Client " << client_id << " disconnected." << endl;
                break;
            } else {
                string error_message = "Unknown command. Use '1' to fetch data or 'exit' to quit.\n";
                boost::asio::write(*socket, boost::asio::buffer(error_message));
            }

            buffer.consume(buffer.size());
        }

    } catch (const exception& e) {
        cerr << "Error with client " << client_id << ": " << e.what() << endl;
    }

    num_of_conn_clients--;
    cout << "Client " << client_id << " disconnected. Currently, " << num_of_conn_clients.load() << " client(s) connected." << endl;

    socket->close();
}

int main() {
    try {
        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 7432));
        cout << "Server running on port 7432..." << endl;

        vector<thread> threads;
        int client_id = 1;

        // Поток для обработки сигналов завершения работы
        thread shutdown_thread([&]() {
            string command;
            while (true) {
                cin >> command;
                if (command == "shutdown") {
                    cout << "Shutting down server..." << endl;
                    server_running = false;

                    // Закрываем все активные сокеты
                    {
                        lock_guard<mutex> lock(socket_mutex);
                        for (auto& socket : active_socket_clients) {
                            if (socket && socket->is_open()) {
                                socket->close();
                            }
                        }
                    }

                    // Останавливаем io_context
                    io_context.stop();
                    break;
                }
            }
        });

        while (server_running) {
            shared_ptr<tcp::socket> socket = make_shared<tcp::socket>(io_context);
            acceptor.accept(*socket);

            {
                lock_guard<mutex> lock(socket_mutex);
                active_socket_clients.push_back(socket);
            }

            cout << "Client " << client_id << " connected." << endl;

            // Запускаем обработку клиента в отдельном потоке
            threads.emplace_back(thread(handle_client, socket, client_id));
            client_id++;
        }

        for (auto& th : threads) {
            if (th.joinable()) {
                th.join();
            }
        }

        if (shutdown_thread.joinable()) {
            shutdown_thread.join();
        }

    } catch (const exception& e) {
        cerr << "Exception: " << e.what() << endl;
    }

    return 0;
}
