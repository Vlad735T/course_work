#include <iostream>
#include <string>
#include <random>
#include <vector>
#include <nlohmann/json.hpp>


#include <filesystem>
#include <fstream>
#include <sstream>

using namespace std;
using json = nlohmann::json;

class Person{
public:
    string name;
    string surname;
    string middle_name;
};

int generation_number(const double& min, const double& max){
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

string generation_set_letters(){
    
    string letters = "ABEKMHOPCTYX";

    string result;

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, letters.size() - 1);


    for(int i = 0; i < 3; i++){
        result += letters[dis(gen)];
    }
    return result;
}

string merger(){
    
    string letters = generation_set_letters();
    int number = generation_number(0, 1000);
    int region = generation_number(1, 199); 
    
    string result = letters.substr(0, 1) + to_string(number) 
                    + letters.substr(1) + "," + to_string(region);
    return result;

}






Person generation_person(const vector<string>& names, 
                        const vector<string>& surnames, const vector<string>& middle_names){

    Person person;
    person.name = names[generation_number(0, names.size() - 1)];
    person.surname  = surnames[generation_number(0, surnames.size() - 1)];
    person.middle_name  = middle_names[generation_number(0, middle_names.size() - 1)];

    return person;
}

//FROM ENV
string get_env_variable(const string& key, const string& env_file_path) {
    ifstream env_file(env_file_path);
    if (!env_file.is_open()) {
        cerr << "Error opening .env file\n";
        exit(-1);
    }

    string line;
    while (getline(env_file, line)) {
        line.erase(remove(line.begin(), line.end(), ' '), line.end()); // Удаляем пробелы
        if (line.find(key + "=") == 0) {
            return line.substr(key.length() + 1);
        }
    }

    cerr << "Key " << key << " not found in .env file\n";
    return "";
}

vector<string> split_string(const string& str, char delimiter) {
    vector<string> result;
    stringstream ss(str);
    string token;
    while (getline(ss, token, delimiter)) {
        result.push_back(token);
    }
    return result;
}

void CREATE_DB(const vector<Person>& people, const json& structure, const string& file_name){

    if (filesystem::exists(file_name)){
        cout << "The table has already been created!!!\n";
        return;
    }

    ofstream db_file(file_name);
    if (!db_file.is_open()){
        cerr << "Error opening db file\n";
        exit(-1);
    }

    db_file << structure["surname"].get<string>() << "," << structure["name"].get<string>() 
            << "," << structure["middle_name"].get<string>() << "," << structure["brand_of_car"].get<string>() << "," 
            << structure["number_of_car"].get<string>() << "," << structure["region"].get<string>() 
            << "," << structure["power"].get<string>() << "," << structure["engine_volume"].get<string>() 
            << "," << structure["release_year"].get<string>() << "\n";



    string env_file_path = "/home/kln735/Application_with_DB/.env";
    string brands_car_value = get_env_variable("BRANDS_CAR", env_file_path);
    if (brands_car_value.empty()) {
        exit(-1);
    }

    vector<string> brands = split_string(brands_car_value, ',');


    for(auto it = 0; it < people.size() ; it++){
        db_file << people[it].surname << "," << people[it].name 
        << "," << people[it].middle_name << "," << brands[generation_number(0, brands.size() - 1)] 
        << "," << merger() << "," << generation_number(13, 1001) << "," 
        << generation_number(0.8, 28.2) << ","<< generation_number(2000, 2024) <<"\n";
    }



    db_file.close();
}







int main(){

    ifstream json_file("/home/kln735/Application_with_DB/server/configuration.json");
    if(!json_file.is_open()){
        cerr << "Error opening json file\n";
        exit(-1);
    }

    json config;
    json_file >> config;

    vector<string> name = config["names"].get<vector<string>>();
    vector<string> sur_name = config["surnames"].get<vector<string>>();
    vector<string> middle_names = config["middle_name"].get<vector<string>>();

    int numPeople = 10'000; 
    vector<Person> people;
    for (int i = 0; i < numPeople; ++i) {
        Person person = generation_person(name, sur_name, middle_names);
        people.push_back(person);
    }

    try {
        CREATE_DB(people, config["structure"], "database.csv");
    } catch (const std::exception& e) {
        cerr << "JSON Error: " << e.what() << endl;
        return -1;
    }


    return 0;
}