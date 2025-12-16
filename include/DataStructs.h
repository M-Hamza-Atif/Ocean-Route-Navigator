#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H

#include <string>
#include "SimpleVector.h"
#include "SimpleList.h"
#include "SimpleQueue.h"

using namespace std;

// Date representation structure
struct Date {
    int day, month, year;
};

// Ship in a docking queue
struct Ship {
    string name;
    string company;
    int waitTime;  // Minutes the ship has been waiting
};

// Route connection between ports (graph edge)
struct Route {
    string origin;
    string destination;
    string departureDate;  // Format: DD/MM/YYYY
    string depTime;        // Format: HH:MM
    string arrTime;        // Format: HH:MM
    int cost;
    string company;
    int durationHours;     // Route duration in hours
    int durationMinutes;   // Route duration in minutes (for pathfinding)
};

// Parse time string "HH:MM" to minutes from midnight
inline int timeToMinutes(string timeStr) {
    int h = stoi(timeStr.substr(0, 2));
    int m = stoi(timeStr.substr(3, 2));
    return h * 60 + m;
}

// Parse date string "DD/MM/YYYY" or "D/MM/YYYY" to Date struct
inline Date parseDate(string dateStr) {
    Date d;
    size_t firstSlash = dateStr.find('/');
    size_t secondSlash = dateStr.find('/', firstSlash + 1);
    
    d.day = stoi(dateStr.substr(0, firstSlash));
    d.month = stoi(dateStr.substr(firstSlash + 1, secondSlash - firstSlash - 1));
    d.year = stoi(dateStr.substr(secondSlash + 1));
    return d;
}

// Convert date and time to total minutes since epoch (Jan 1, 2024)
inline long long dateTimeToMinutes(Date d, int timeMinutes) {
    int daysInMonth[] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};  // 2024 is leap year
    
    long long totalDays = 0;
    for (int m = 1; m < d.month; m++) {
        totalDays += daysInMonth[m];
    }
    totalDays += d.day - 1;  // Day 1 = 0 days elapsed
    
    return totalDays * 1440 + timeMinutes;  // 1440 minutes per day
}

// Port node in the graph
struct Port {
    string name;
    int dailyCharge;
    int x, y;  // Screen coordinates for visualization
    
    // Docking queues (one per shipping company)
    SimpleVector<SimpleQueue<Ship>> companyQueues;
    SimpleVector<string> companyNames;
    
    Port() : name(""), dailyCharge(0), x(0), y(0) {
        initializeCompanies();
    }
    
    Port(string n, int charge, int xPos, int yPos) 
        : name(n), dailyCharge(charge), x(xPos), y(yPos) {
        initializeCompanies();
    }
    
    void initializeCompanies() {
        companyNames.push_back("MaerskLine");
        companyNames.push_back("MSC");
        companyNames.push_back("COSCO");
        companyNames.push_back("CMA_CGM");
        companyNames.push_back("HapagLloyd");
        companyNames.push_back("ONE");
        companyNames.push_back("Evergreen");
        companyNames.push_back("PIL");
        companyNames.push_back("ZIM");
        companyNames.push_back("YangMing");
        
        for (int i = 0; i < companyNames.size(); i++) {
            companyQueues.push_back(SimpleQueue<Ship>());
        }
    }
};

// Port coordinate lookup (map dimensions: 1050x700)
struct Point { int x, y; };

inline Point getPortCoordinates(string portName) {
    if (portName == "Chittagong") return {745, 358};
    if (portName == "Karachi") return {678, 350};
    if (portName == "Dublin") return {476, 257};
    if (portName == "Marseille") return {510, 292};
    if (portName == "Jeddah") return {604, 359};
    if (portName == "Colombo") return {714, 402};
    if (portName == "Sydney") return {907, 521};
    if (portName == "Melbourne") return {891, 531};
    if (portName == "Montreal") return {294, 284};
    if (portName == "Doha") return {635, 349};
    if (portName == "Manila") return {823, 380};
    if (portName == "Alexandria") return {575, 332};
    if (portName == "Istanbul") return {574, 301};
    if (portName == "Singapore") return {776, 416};
    if (portName == "Busan") return {848, 318};
    if (portName == "HongKong") return {804, 357};
    if (portName == "Durban") return {577, 505};
    if (portName == "Oslo") return {524, 231};
    if (portName == "Dubai") return {645, 351};
    if (portName == "Genoa") return {519, 288};
    if (portName == "Osaka") return {865, 321};
    if (portName == "Lisbon") return {469, 307};
    if (portName == "Hamburg") return {522, 257};
    if (portName == "Rotterdam") return {510, 258};
    if (portName == "Helsinki") return {564, 228};
    if (portName == "Mumbai") return {696, 370};
    if (portName == "Jakarta") return {787, 434};
    if (portName == "Tokyo") return {877, 318};
    if (portName == "Athens") return {558, 309};
    if (portName == "CapeTown") return {545, 519};
    if (portName == "Stockholm") return {544, 232};
    if (portName == "PortLouis") return {658, 485};
    if (portName == "Shanghai") return {825, 329};
    if (portName == "NewYork") return {291, 299};
    if (portName == "AbuDhabi") return {644, 352};
    if (portName == "Copenhagen") return {528, 249};
    if (portName == "Vancouver") return {154, 271};
    if (portName == "LosAngeles") return {168, 323};
    if (portName == "Antwerp") return {504, 265};
    if (portName == "London") return {494, 263};

    return {525, 350};  // Default fallback
}

#endif