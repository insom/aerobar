#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <chrono>
#include <thread>
#include <filesystem>
#include <iomanip>
#include <ctime>
#include <math.h>

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

        double t = (double)time(NULL);
        t = t + 3600; // BMT, not GMT, for Internet time
        double beats = (int)((t / 8.64)) % 10000;
        int minutes = (wattHours * 60) / watts;
        if(minutes < 60 && minutes > 0) {
            cout
                << "%{F#f00} LOW POWER %{F#555}|%{A:/home/insom/Bin/zzz:} Sleep %{A} ";
        }

        cout
            << setprecision(3)
            << "%{r} "
            << watts << "W | "
            << wattHours << "Wh | ";
        if(minutes < 60 && minutes > 0) cout << "%{F#f00}";
        if(minutes > 0) {
            cout
                << floor(minutes / 60) << "h"
                << setfill('0')
                << setw(2)
                << minutes % 60 << "m";
        } else {
            cout
                << "FULL";
        }
        if(minutes < 60 && minutes > 0) cout << "%{F#555}";
        cout << " | "
            << percentage << "% | @"
            << beats / 10
            << " "
            << endl;

        std::this_thread::sleep_for(1s);
    }
}
