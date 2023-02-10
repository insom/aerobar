#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <chrono>
#include <thread>
#include <filesystem>
#include <iomanip>
#include <ctime>

#define PS_DIR "/sys/class/power_supply/"

using namespace std::chrono_literals;
using namespace std;

typedef map<string, string> str_map;

str_map iniToMap(string filename) {
    ifstream f(filename);
    string line;
    map<string, string> map;

    while(getline(f, line)) {
        auto pivot = line.find_first_of("=", 0);
        auto key = line.substr(0, pivot);
        auto value = line.substr(pivot + 1);
        map[key] = value;
    }
    return map;
}

vector<str_map> batteries() {
    vector<str_map> rv;
    for (auto const& dir_entry : std::filesystem::directory_iterator{PS_DIR}) {
        if(dir_entry.path().stem().string().find("BAT") == 0) {
            rv.push_back(iniToMap(dir_entry.path() / "uevent"));
        }
    }
    return rv;
}

int main() {
    for(;;) {
        float watts = 0;
        float wattHours = 0;
        float wattHourCapacity = 0;
        float percentage = 0;

        for(auto &battery : batteries()) {
            watts += stoi(battery["POWER_SUPPLY_POWER_NOW"]) / 1e6;
            wattHours += stoi(battery["POWER_SUPPLY_ENERGY_NOW"]) / 1e6;
            wattHourCapacity += stoi(battery["POWER_SUPPLY_ENERGY_FULL"]) / 1e6;
        }
        percentage = 100 * wattHours / wattHourCapacity;
        auto now = std::chrono::steady_clock::now();

        cout
            << setprecision(2)
            << watts << "W | "
            << wattHours << "Wh | "
            << percentage << "% | "
            << now
            << endl;
        std::this_thread::sleep_for(1s);
    }
}
