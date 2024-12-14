#include <iostream>
#include <string>
#include <random>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <regex>    // for search special symbol

#include <locale>
#include <codecvt>

#include <nlohmann/json.hpp>

#include <pqxx/pqxx> // for postgresql

using namespace std;
using json = nlohmann::json;

//Класс для чтения перемененных окружения из файла .env
class EnvReader {
private:
    unordered_map<string, string> variables;

public:
    explicit EnvReader(const string& env_file_path) {
        ifstream env_file(env_file_path);
        if (!env_file.is_open()) {
            cerr << "Error opening .env file\n";
            exit(-1);
        }

        string line;
        while (getline(env_file, line)) {
            line.erase(remove(line.begin(), line.end(), ' '), line.end()); // Удаляем пробелы
            size_t pos = line.find('=');
            if (pos != string::npos) {
                string key = line.substr(0, pos);
                string value = line.substr(pos + 1);
                variables[key] = value;
            }
        }
    }

    string get_variable(const string& key) const {
        auto it = variables.find(key);
        if (it != variables.end()) {
            return it->second;
        }
        cerr << "Key " << key << " not found in .env file\n";
        exit(-1);
    }

    void check_special_characters(const string& str) const {
        string special_chars = "!»\" №;%:?*()+=\\-./’”:{[]}@#$^&<>\|1234567890";
        for (char ch : str) {
            if (special_chars.find(ch) != string::npos) {
                cerr << "Error: Special character '" << ch << "' found in string.\n";
                exit(-1);
            }
        }
    }
};

vector<string> split_string(const string& str, char delimiter, const EnvReader& reader) {
    vector<string> result;
    stringstream ss(str);
    string token;
    while (getline(ss, token, delimiter)) {
        // Проверяем на наличие специальных символов в каждом токене
        reader.check_special_characters(token);

        if (token.empty()) {
            cerr << "Error: The 'BRANDS_CAR' environment variable contains empty strings or spaces!!!\n";
            exit(-1);
        }

        result.push_back(token);
    }
    return result;
}




// Интерфейс для генерации случайных чисел
class IRandomGenerator {
public:
    virtual int generate(int min, int max) = 0;
    virtual ~IRandomGenerator() = default;
};

class RandomNumberGenerator : public IRandomGenerator {
public:
    int generate(int min, int max) override {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(min, max);
        return dis(gen);
    }
};

// Интерфейс для генерации строк
class IStringGenerator {
public:
    virtual string generate() = 0;
    virtual ~IStringGenerator() = default;
};

class LicensePlateGenerator : public IStringGenerator {
private:
    shared_ptr<IRandomGenerator> rng;

public:
    explicit LicensePlateGenerator(shared_ptr<IRandomGenerator> random_gen) : rng(random_gen) {}

    string generate() override {
        string letters = "ABEKMHOPCTYX";
        string result;

        for (int i = 0; i < 3; i++) {
            result += letters[rng->generate(0, letters.size() - 1)];
        }
        return result;
    }
};

// Класс для генерации полного номера машины
class Merger {
private:
    shared_ptr<IRandomGenerator> rng;
    shared_ptr<IStringGenerator> plate_generator;

public:
    Merger(shared_ptr<IRandomGenerator> random_gen, shared_ptr<IStringGenerator> plate_gen)
        : rng(random_gen), plate_generator(plate_gen) {}

    string generate() {
        string letters = plate_generator->generate();
        int number = rng->generate(0, 999);
        int region = rng->generate(1, 199);

        stringstream number_stream;
        number_stream << setw(3) << setfill('0') << number;

        string result = letters.substr(0, 1) + number_stream.str() + letters.substr(1);
        return result;
    }
};





// Генерация людей с помощью фабрики
class Person {
public:
    string name;
    string surname;
    string middle_name;

    Person(string n, string s, string m) : name(move(n)), surname(move(s)), middle_name(move(m)) {}
};

class PersonFactory {
private:
    shared_ptr<IRandomGenerator> rng;

public:
    explicit PersonFactory(shared_ptr<IRandomGenerator> random_gen) : rng(random_gen) {}

    Person generate_person(const vector<string>& names, const vector<string>& surnames, const vector<string>& middle_names) {
        string name = names[rng->generate(0, names.size() - 1)];
        string surname = surnames[rng->generate(0, surnames.size() - 1)];
        string middle_name = middle_names[rng->generate(0, middle_names.size() - 1)];
        return Person(name, surname, middle_name);
    }
};



// Менеджер базы данных
class DatabaseManager {
private:
    shared_ptr<IRandomGenerator> rng;
    shared_ptr<IStringGenerator> plate_generator;
    shared_ptr<Merger> merger;
    vector<string> brands;

    string generation_number_phone() {
        int operate_code = rng->generate(900, 999);
        int first_part = rng->generate(0, 999);
        int sec_with_third_parts = rng->generate(0, 99);
        stringstream phone;

        phone << "8" << operate_code << setw(3) << setfill('0') << first_part
            << setw(2) << setfill('0') << sec_with_third_parts << setw(2) << setfill('0') << sec_with_third_parts;

        return phone.str();
    }

    string generate_random_string(int length) {
        const string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, chars.size() - 1);

        string result;
        for (int i = 0; i < length; ++i) {
            result += chars[dis(gen)];
        }
        return result;
    }

    string generate_random_email() {
        vector<string> domains = {
            "@yandex.ru", "@mail.ru", "@bk.ru", "@list.ru",
            "@inbox.ru", "@gmail.com", "@outlook.com",
            "@microsoft.com", "@icloud.com"
        };

        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> domain_dis(0, domains.size() - 1);

        string random_string = generate_random_string(7);
        string selected_domain = domains[domain_dis(gen)];

        return random_string + selected_domain;
    }

public:
    DatabaseManager(shared_ptr<IRandomGenerator> random_gen, shared_ptr<IStringGenerator> plate_gen,
        const vector<string>& car_brands, shared_ptr<Merger> merger_instance) :
        rng(random_gen), plate_generator(plate_gen), brands(car_brands), merger(merger_instance) {}

    void create_db(const vector<Person>& people, const json& structure, const string& file_name, const string& conn_str) {

        try {
            // Подключение к PostgreSQL для проверки и создания базы данных
            pqxx::connection conn("dbname=postgres user=admin password=Qwerty1!@ host=127.0.0.1 port=5432");

            // pqxx::connection conn("dbname=dyr user=admin password=Qwerty1!@ host=postgres_db port=5432");

            if (!conn.is_open()) {
                cerr << "Failed to connect to PostgreSQL database: postgres" << endl;
                exit(1);
            }
            cout << "Successfully connected to PostgreSQL for database creation!" << endl;

            // Создание базы данных, если она еще не существует
            pqxx::nontransaction non_txn(conn);  // Используем nontransaction для создания базы данных
            pqxx::result r = non_txn.exec("SELECT 1 FROM pg_database WHERE datname = 'data_based_of_cars';");
            if (r.empty()) {
                try {
                    cout << "Database data_based_of_cars does not exist. Creating it..." << endl;
                    non_txn.exec("CREATE DATABASE data_based_of_cars;");
                    cout << "Database created successfully." << endl;
                } catch (const std::exception &e) {
                    cerr << "Error creating database: " << e.what() << endl;
                }
            } else {
                cout << "Database data_based_of_cars already exists." << endl;
            }


            // Подключаемся к базе данных data_based_of_cars
            pqxx::connection conn_db("dbname=data_based_of_cars user=admin password=Qwerty1!@ host=127.0.0.1 port=5432");
            if (!conn_db.is_open()) {
                cerr << "Failed to connect to PostgreSQL database: data_based_of_cars" << endl;
                exit(1);
            }
            cout << "Successfully connected to data_based_of_cars!" << endl;

            pqxx::work txn_db(conn_db);  // Создаем транзакцию для операций с таблицами

            // Создание таблиц
            txn_db.exec(R"(
            CREATE TABLE IF NOT EXISTS owners(
                id_owner SERIAL PRIMARY KEY,
                surname VARCHAR(255),
                name VARCHAR(255),
                middle_name VARCHAR(255),
                phone_number VARCHAR(255),
                email VARCHAR(255)
            );
            )");

            txn_db.exec(R"(
            CREATE TABLE IF NOT EXISTS cars(
                id_car SERIAL PRIMARY KEY,
                brand_of_car  VARCHAR(255),
                number_of_car  VARCHAR(10),
                region  INTEGER,
                power INTEGER,
                engine_volume NUMERIC(4, 1),
                release_year  INTEGER,
                id_owner INTEGER REFERENCES owners(id_owner)
            );
            )");

            // Проверка на наличие данных в таблицах 
            pqxx::result check_result = txn_db.exec("SELECT COUNT(*) FROM owners, cars WHERE owners.id_owner = cars.id_owner;");
            if (check_result[0][0].as<int>() == 0) {
                cout << "Tables 'owners' or 'cars' are empty. Inserting data..." << endl;

                // Генерация и вставка данных для owners и cars
                for (const auto& person : people) {
                    try {
                        string number_phone = generation_number_phone();
                        string email = generate_random_email();

                        pqxx::result owner_result = txn_db.exec_params(
                            "INSERT INTO owners (surname, name, middle_name, phone_number, email) VALUES ($1, $2, $3, $4, $5) RETURNING id_owner;",
                            person.surname, person.name, person.middle_name, number_phone, email
                        );
                        int owner_id = owner_result[0]["id_owner"].as<int>();

                        // Вставка данных для cars для каждого владельца
                        int num_cars_for_owner = rng->generate(1, 3);
                        for (int i = 0; i < num_cars_for_owner; i++) {
                            string license_plate = merger->generate();
                            int power = rng->generate(13, 1001);
                            double engine_volume = rng->generate(80, 282) / 10.0;
                            int release_year = rng->generate(2000, 2024);
                            int reg = rng->generate(1, 199);
                            string brand = brands[rng->generate(0, brands.size() - 1)];

                            txn_db.exec_params(
                                "INSERT INTO cars (brand_of_car, number_of_car, region, power, engine_volume, release_year, id_owner) VALUES ($1, $2, $3, $4, $5, $6, $7);",
                                brand, license_plate, reg, power, engine_volume, release_year, owner_id);
                        }

                    } catch (const exception& e) {
                        cerr << "Error inserting data for " << person.surname << ": " << e.what() << endl;
                    }
                }
            } else {
                cout << "Tables 'owners' and 'cars' already contain data. Skipping insertion." << endl;
            }

            // Завершаем транзакцию
            txn_db.commit();

        } catch (const exception& e) {
            cerr << "Error: " << e.what() << endl;
            exit(1);
        }
    }

};


bool validate_json_structure(const json& j) {
    // Ожидаемая структура
    json expected_structure = {
        {"name", ""},
        {"surname", ""},
        {"middle_name", ""},
        {"brand_of_car", ""},
        {"number_of_car", ""},
        {"region", ""},
        {"power", ""},
        {"engine_volume", ""},
        {"release_year", ""}
    };

    string special_chars = "!»\" №;%:?*()+=\\-./’”:{[]}@#$^&<>\|1234567890";

    for (const auto& item : expected_structure.items()) {
        const string& key = item.key();

        // Проверка наличия ключа
        if (!j.contains(key)) {
            cerr << "Error: Missing key '" << key << "' in the structure!" << endl;
            return false;
        }

        // Проверка, что значение является строкой
        if (!j[key].is_string()) {
            cerr << "Error: The value of key '" << key << "' is not a string!" << endl;
            return false;
        }

        // Получаем строку
        string value = j[key].get<string>();

        // Проверка на пустое значение
        if (value.empty()) {
            cerr << "Error: The value of key '" << key << "' is empty!" << endl;
            return false;
        }

        // Проверка на наличие специальных символов
        for (char ch : value) {
            if (special_chars.find(ch) != string::npos) {
                cerr << "Error: The value of key '" << key << "' contains an invalid character '" << ch << "'!" << endl;
                return false;
            }
        }
    }

    return true;
}

bool validate_string_array(const json& j, const string& field_name) {

    if (!j.contains(field_name) || j[field_name].is_null() || j[field_name].empty() || !j[field_name].is_array()) {
        cerr << "Error: The '" << field_name << "' field is missing, null, empty, or not an array in the JSON file!!!\n";
        return false;
    }

    string special_chars = "!»\" №;%:?*()+=\\-./’”:{[]}@#$^&<>\|1234567890";

    for (const auto& item : j[field_name]) {
        // Проверяем, что элемент является строкой и не пустой
        if (!item.is_string() || item.empty()) {
            cerr << "Error: The '" << field_name << "' field contains non-string elements, empty strings, or strings with only spaces in the JSON file!!!\n";
            return false;
        }

        for (char ch : item.get<string>()) {
            if (special_chars.find(ch) != string::npos) {
                cerr << "Error: The string '" << item.get<string>() << "' in the field '" << field_name << "' contains an invalid character '" << ch << "'!!!\n";
                return false;
            }
        }
    }

    return true;
}




int main() {

    // Устанавливаем локаль для поддержки UTF-8
    setlocale(LC_ALL, "en_US.UTF-8");

    filesystem::path current_path = filesystem::current_path();
    filesystem::path json_path = current_path.parent_path() / "server" / "configuration.json";

    // filesystem::path json_path = "/app/configuration.json";

    ifstream json_file(json_path);
    if (!json_file.is_open()) {
        cerr << "Error: The JSON file was not opened. Please check if the correct directory is specified or if the file exists at the specified path!!!\n";
        exit(-1);
    }
    cout << "JSON PATH: " << json_path << endl;

    json config;
    json_file >> config;
    if (!validate_string_array(config, "names") || !validate_string_array(config, "surnames") ||
        !validate_string_array(config, "middle_name") || !validate_json_structure(config["structure"])) {
        exit(-1);
    }
    vector<string> names = config["names"].get<vector<string>>();
    vector<string> surnames = config["surnames"].get<vector<string>>();
    vector<string> middle_names = config["middle_name"].get<vector<string>>();


    EnvReader env_path(current_path.parent_path() / ".env");
    // filesystem::path env_path = "/app/.env";

    EnvReader env_reader(env_path);
    string brands_str = env_reader.get_variable("BRANDS_CAR");
    vector<string> brands = split_string(brands_str, ',', env_reader);


    auto rng = make_shared<RandomNumberGenerator>();
    auto plate_gen = make_shared<LicensePlateGenerator>(rng);
    auto merger = make_shared<Merger>(rng, plate_gen);
    PersonFactory person_factory(rng);

    int numPeople = 5;
    vector<Person> people;
    for (int i = 0; i < numPeople; ++i) {
        people.push_back(person_factory.generate_person(names, surnames, middle_names));
    }

    DatabaseManager db_manager(rng, plate_gen, brands, merger);
    string conn_str = "dbname=postgres user=admin password=Qwerty1!@ host=127.0.0.1 port=5432";
    db_manager.create_db(people, config["structure"], "database.csv", conn_str);

    return 0;
}
