#include <iostream>
#include <string>
#include <random>
#include <vector>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <memory>
#include <unordered_map>

using namespace std;
using json = nlohmann::json;

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
        return "";
    }
};

vector<string> split_string(const string& str, char delimiter) {
    vector<string> result;
    stringstream ss(str);
    string token;
    while (getline(ss, token, delimiter)) {
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

        string result = letters.substr(0, 1) + number_stream.str() + letters.substr(1) + "," + to_string(region);
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

public:
    DatabaseManager(shared_ptr<IRandomGenerator> random_gen, shared_ptr<IStringGenerator> plate_gen, 
                    const vector<string>& car_brands, shared_ptr<Merger> merger_instance)
        : rng(random_gen), plate_generator(plate_gen), brands(car_brands), merger(merger_instance) {} 


    void create_db(const vector<Person>& people, const json& structure, const string& file_name) {
        if (filesystem::exists(file_name)) {
            cout << "The table has already been created!!!\n";
            return;
        }

        ofstream db_file(file_name);
        if (!db_file.is_open()) {
            cerr << "Error opening db file\n";
            exit(-1);
        }

        db_file << structure["surname"].get<string>() << "," << structure["name"].get<string>()
                << "," << structure["middle_name"].get<string>() << "," << structure["brand_of_car"].get<string>()
                << "," << structure["number_of_car"].get<string>() << "," << structure["region"].get<string>()
                << "," << structure["power"].get<string>() << "," << structure["engine_volume"].get<string>()
                << "," << structure["release_year"].get<string>() << "\n";

        for (const auto& person : people) {
            string license_plate = merger->generate();
            int power = rng->generate(13, 1001);
            double engine_volume = rng->generate(80, 282) / 10.0; // Переводим в диапазон 0.8 - 28.2
            int release_year = rng->generate(2000, 2024);
            string brand = brands[rng->generate(0, brands.size() - 1)];

            db_file << person.surname << "," << person.name << "," << person.middle_name << "," << brand << ","
                    << license_plate << "," << power << "," << engine_volume << "," << release_year << "\n";
        }

        db_file.close();
    }
};

int main() {
    ifstream json_file("/home/kln735/Application_with_DB/server/configuration.json");
    if (!json_file.is_open()) {
        cerr << "Error opening json file\n";
        exit(-1);
    }

    json config;
    json_file >> config;

    vector<string> names = config["names"].get<vector<string>>();
    vector<string> surnames = config["surnames"].get<vector<string>>();
    vector<string> middle_names = config["middle_name"].get<vector<string>>();

    EnvReader env_reader("/home/kln735/Application_with_DB/.env");
    string brands_str = env_reader.get_variable("BRANDS_CAR");
    vector<string> brands = split_string(brands_str, ',');

    auto rng = make_shared<RandomNumberGenerator>();
    auto plate_gen = make_shared<LicensePlateGenerator>(rng);
    auto merger = make_shared<Merger>(rng, plate_gen);
    PersonFactory person_factory(rng);

    int numPeople = 10000;
    vector<Person> people;
    for (int i = 0; i < numPeople; ++i) {
            people.push_back(person_factory.generate_person(names, surnames, middle_names));
    }

    DatabaseManager db_manager(rng, plate_gen, brands, merger);
    db_manager.create_db(people, config["structure"], "database.csv");

    return 0;
}
