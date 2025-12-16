#include <iostream>
#include "../include/SimpleVector.h"
#include "../include/DataStructs.h"
#include "../include/FileParser.h"
#include "../include/Graph.h"
#include "../include/Visualizer.h"

using namespace std;

int main() {
    // Load data using FileParser
    SimpleVector<Port> ports;
    SimpleVector<Route> routes;
    FileParser parser;

    cout << "Loading Ports..." << endl;
    parser.loadPorts("../data/PortCharges.txt", ports);
    
    cout << "Loading Routes..." << endl;
    parser.loadRoutes("../data/Routes.txt", routes);

    cout << "Total Ports Loaded: " << ports.size() << endl;
    cout << "Total Routes Loaded: " << routes.size() << endl;

    // Build the Graph
    Graph graph;

    // Add all ports to the graph
    for (int i = 0; i < ports.size(); i++) {
        graph.addPort(ports[i]);
    }
    cout << "Added " << graph.ports.size() << " ports to graph." << endl;

    // Add all routes as edges to the graph
    for (int i = 0; i < routes.size(); i++) {
        graph.addEdge(routes[i].origin, routes[i].destination, routes[i].cost, &routes[i]);
    }
    cout << "Added edges from " << routes.size() << " routes." << endl;

    // Initialize traffic - populate ports with dummy ships
    cout << "Initializing port traffic..." << endl;
    graph.initializeTraffic();

    // Display the graph structure
    graph.displayGraph();

    // Create the visualizer and run
    cout << "Starting OceanRoute Nav Visualizer..." << endl;
    OceanVisualizer visualizer(&graph);
    visualizer.run();

    return 0;
}