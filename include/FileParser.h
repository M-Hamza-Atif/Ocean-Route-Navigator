#ifndef FILEPARSER_H
#define FILEPARSER_H

#include <fstream>
#include <iostream>
#include <string>
#include "DataStructs.h"

using namespace std;

struct FileParser {
    
    void loadPorts(string filename, SimpleVector<Port>& ports) {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << "Error: Could not open " << filename << endl;
            return;
        }

        string name;
        int charge;
        while (file >> name >> charge) {
            Port p;
            p.name = name;
            p.dailyCharge = charge;
            
            Point pt = getPortCoordinates(name);
            p.x = pt.x;
            p.y = pt.y;
            
            ports.push_back(p);
        }
        file.close();
    }

    void loadRoutes(string filename, SimpleVector<Route>& routes) {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << "Error: Could not open " << filename << endl;
            return;
        }

        string origin, dest, date, depT, arrT, company;
        int cost;

        while (file >> origin >> dest >> date >> depT >> arrT >> cost >> company) {
            Route r;
            r.origin = origin;
            r.destination = dest;
            r.departureDate = date;
            r.depTime = depT;
            r.arrTime = arrT;
            r.cost = cost;
            r.company = company;
            
            int depMin = timeToMinutes(depT);
            int arrMin = timeToMinutes(arrT);
            int diff = arrMin - depMin;
            
            if (diff < 0) {
                diff += 1440;
            }
            r.durationMinutes = diff;
            
            routes.push_back(r);
        }
        file.close();
    }
};

#endif