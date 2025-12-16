#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include "SimpleVector.h"
#include "SimpleList.h"
#include "DataStructs.h"

using namespace std;

struct Edge {
    int destinationPortIndex;
    int weight;
    Route* routeData;

    Edge(int dest, int w, Route* r) : destinationPortIndex(dest), weight(w), routeData(r) {}
};

struct Graph {
    SimpleVector<Port> ports;
    SimpleVector<SimpleList<Edge>> adjList;

    int getIndex(string portName) {
        for (int i = 0; i < ports.size(); i++) {
            if (ports[i].name == portName) {
                return i;
            }
        }
        return -1;
    }

    void addPort(Port p) {
        ports.push_back(p);
        SimpleList<Edge> emptyList;
        adjList.push_back(emptyList);
    }

    void addEdge(string originName, string destName, int cost, Route* routePtr) {
        int u = getIndex(originName);
        int v = getIndex(destName);
        
        if (u != -1 && v != -1) {
            Edge newEdge(v, cost, routePtr);
            adjList[u].push_back(newEdge);
        }
    }

    void displayGraph() {
        for (int i = 0; i < ports.size(); i++) {
            cout << "Port [" << ports[i].name << "] connects to:";
            
            Node<Edge>* current = adjList[i].head;
            
            if (current == nullptr) {
                cout << " (no outgoing routes)";
            }
            
            while (current != nullptr) {
                int destIndex = current->data.destinationPortIndex;
                int edgeCost = current->data.weight;
                cout << " -> " << ports[destIndex].name << " ($" << edgeCost << ")";
                current = current->next;
            }
            
            cout << endl;
        }
    }

    void initializeTraffic() {
        srand(static_cast<unsigned>(time(0)));
        
        for (int i = 0; i < ports.size(); i++) {
            int numShips = rand() % 5;
            
            for (int j = 0; j < numShips; j++) {
                int companyIndex = rand() % ports[i].companyNames.size();
                string companyName = ports[i].companyNames[companyIndex];
                
                Ship dummyShip;
                dummyShip.name = "Ship_" + to_string(i) + "_" + to_string(j);
                dummyShip.company = companyName;
                dummyShip.waitTime = rand() % 1440;
                
                ports[i].companyQueues[companyIndex].push(dummyShip);
            }
        }
    }
};

#endif