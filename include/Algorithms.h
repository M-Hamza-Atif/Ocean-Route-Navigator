#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "Graph.h"
#include "MinHeap.h"
#include "SimpleVector.h"
#include "DataStructs.h"
#include <cmath>
#include <string>
#include <iomanip>

using namespace std;

enum SearchMode {
    MODE_DIJKSTRA_COST,
    MODE_DIJKSTRA_TIME,
    MODE_ASTAR_COST,
    MODE_ASTAR_TIME
};

struct DijkstraState {
    int u;
    double currentMetric;
    double estimatedTotal;

    DijkstraState() : u(-1), currentMetric(0), estimatedTotal(0) {}
    DijkstraState(int node, double metric, double estimated)
        : u(node), currentMetric(metric), estimatedTotal(estimated) {}

    bool operator<(const DijkstraState& other) const {
        return estimatedTotal < other.estimatedTotal;
    }

    bool operator>(const DijkstraState& other) const {
        return estimatedTotal > other.estimatedTotal;
    }
};

struct PathResult {
    SimpleVector<int> path;
    double totalCost;

    PathResult() : totalCost(0) {}
};

inline double calculateHeuristic(Graph& g, int currentIdx, int targetIdx, SearchMode mode) {
    if (mode == MODE_DIJKSTRA_COST || mode == MODE_DIJKSTRA_TIME) {
        return 0.0;
    }

    if (mode == MODE_ASTAR_COST) {
        return 0.0;
    }

    if (mode == MODE_ASTAR_TIME) {
        Port& p1 = g.ports[currentIdx];
        Port& p2 = g.ports[targetIdx];
        double dx = p1.x - p2.x;
        double dy = p1.y - p2.y;
        return sqrt(dx * dx + dy * dy);
    }

    return 0.0;
}

inline PathResult findPath(Graph& g, string startName, string endName, SearchMode mode, int startDay = 1) {
    PathResult result;

    int startIdx = g.getIndex(startName);
    int endIdx = g.getIndex(endName);

    if (startIdx == -1 || endIdx == -1) {
        return result;
    }

    int n = g.ports.size();

    SimpleVector<double> minMetric;
    SimpleVector<double> arrivalTimes;
    SimpleVector<int> parent;
    SimpleVector<bool> visited;

    for (int i = 0; i < n; i++) {
        minMetric.push_back(1e18);
        arrivalTimes.push_back(1e18);
        parent.push_back(-1);
        visited.push_back(false);
    }

    arrivalTimes[startIdx] = 0.0;
    minMetric[startIdx] = 0.0;
    
    double decemberStartMinutes = 482400.0;
    double dayStartMinutes = decemberStartMinutes + (startDay - 1) * 1440.0;
    double dayEndMinutes = decemberStartMinutes + startDay * 1440.0 - 1.0;

    MinHeap<DijkstraState> pq;
    double h = calculateHeuristic(g, startIdx, endIdx, mode);
    pq.push(DijkstraState(startIdx, minMetric[startIdx], minMetric[startIdx] + h));

    while (!pq.empty()) {
        DijkstraState current = pq.top();
        pq.pop();

        int u = current.u;

        if (visited[u]) {
            continue;
        }
        visited[u] = true;

        if (u == endIdx) {
            break;
        }

        Node<Edge>* edgeNode = g.adjList[u].head;
        while (edgeNode != nullptr) {
            Edge& edge = edgeNode->data;
            int v = edge.destinationPortIndex;

            if (!visited[v]) {
                Route* route = edge.routeData;
                
                Date depDate = parseDate(route->departureDate);
                int depTimeMin = timeToMinutes(route->depTime);
                long long departureDatetime = dateTimeToMinutes(depDate, depTimeMin);

                Date arrDate = parseDate(route->departureDate);
                int arrTimeMin = timeToMinutes(route->arrTime);
                long long arrivalDatetime = dateTimeToMinutes(arrDate, arrTimeMin);

                if (arrTimeMin < depTimeMin) {
                    arrivalDatetime += 1440;
                }

                double currentArrival = arrivalTimes[u];

                if (u == startIdx) {
                    if (departureDatetime < dayStartMinutes || departureDatetime > dayEndMinutes) {
                        edgeNode = edgeNode->next;
                        continue;
                    }
                    currentArrival = (double)departureDatetime;
                } else {
                    int cycleCount = 0;
                    int maxCycles = 1;
                    
                    while (departureDatetime < currentArrival && cycleCount < maxCycles) {
                        departureDatetime += 10080;
                        arrivalDatetime += 10080;
                        cycleCount++;
                    }
                    
                    if (departureDatetime < currentArrival) {
                        edgeNode = edgeNode->next;
                        continue;
                    }
                }

                double waitMins = (departureDatetime > currentArrival) ? (departureDatetime - currentArrival) : 0.0;

                if (mode == MODE_DIJKSTRA_COST || mode == MODE_ASTAR_COST) {
                    double layoverCost = 0.0;
                    if (waitMins > 720.0 && u != startIdx && u != endIdx) {
                        double days = ceil(waitMins / 1440.0);
                        layoverCost = days * g.ports[u].dailyCharge;
                    }

                    double newMetric = minMetric[u] + edge.weight + layoverCost;

                    if (newMetric < minMetric[v]) {
                        minMetric[v] = newMetric;
                        arrivalTimes[v] = (double)arrivalDatetime;
                        parent[v] = u;

                        double heuristic = calculateHeuristic(g, v, endIdx, mode);
                        double estimatedTotal = newMetric + heuristic;
                        pq.push(DijkstraState(v, newMetric, estimatedTotal));
                    }
                } else {
                    double waitPenalty = (u == startIdx) ? 0.0 : waitMins;
                    double newMetric = minMetric[u] + waitPenalty + route->durationMinutes;

                    if (newMetric < minMetric[v]) {
                        minMetric[v] = newMetric;
                        if (u == startIdx) {
                            arrivalTimes[v] = (double)arrivalDatetime;
                        } else {
                            arrivalTimes[v] = (double)arrivalDatetime;
                        }
                        parent[v] = u;

                        double heuristic = calculateHeuristic(g, v, endIdx, mode);
                        double estimatedTotal = newMetric + heuristic;
                        pq.push(DijkstraState(v, newMetric, estimatedTotal));
                    }
                }
            }

            edgeNode = edgeNode->next;
        }
    }

    if (parent[endIdx] == -1 && startIdx != endIdx) {
        return result;
    }

    SimpleVector<int> reversePath;
    int current = endIdx;
    while (current != -1) {
        reversePath.push_back(current);
        current = parent[current];
    }

    for (int i = reversePath.size() - 1; i >= 0; i--) {
        result.path.push_back(reversePath[i]);
    }

    result.totalCost = minMetric[endIdx];

    return result;
}

struct MultiLegResult {
    SimpleVector<PathResult> segments;
    bool isValid;
    int departureDay;
    double totalCostWithCharges;

    MultiLegResult() : isValid(true), departureDay(1), totalCostWithCharges(0.0) {}
};

inline PathResult findPathSegment(Graph& g, string startName, string endName, SearchMode mode, double absoluteStartTimeMin) {
    PathResult result;

    int startIdx = g.getIndex(startName);
    int endIdx = g.getIndex(endName);

    if (startIdx == -1 || endIdx == -1) {
        return result;
    }

    int n = g.ports.size();

    SimpleVector<double> minMetric;
    SimpleVector<double> arrivalTimes;
    SimpleVector<int> parent;
    SimpleVector<bool> visited;

    for (int i = 0; i < n; i++) {
        minMetric.push_back(1e18);
        arrivalTimes.push_back(1e18);
        parent.push_back(-1);
        visited.push_back(false);
    }

    arrivalTimes[startIdx] = absoluteStartTimeMin;
    minMetric[startIdx] = 0.0;

    MinHeap<DijkstraState> pq;
    double h = calculateHeuristic(g, startIdx, endIdx, mode);
    pq.push(DijkstraState(startIdx, minMetric[startIdx], minMetric[startIdx] + h));

    while (!pq.empty()) {
        DijkstraState current = pq.top();
        pq.pop();

        int u = current.u;

        if (visited[u]) continue;
        visited[u] = true;

        if (u == endIdx) break;

        Node<Edge>* edge = g.adjList[u].head;
        while (edge != nullptr) {
            int v = edge->data.destinationPortIndex;
            Route* route = edge->data.routeData;

            Date depDate = parseDate(route->departureDate);
            int depTimeMin = timeToMinutes(route->depTime);
            int arrTimeMin = timeToMinutes(route->arrTime);

            long long departureDatetime = dateTimeToMinutes(depDate, depTimeMin);
            long long arrivalDatetime = dateTimeToMinutes(depDate, arrTimeMin);

            if (arrTimeMin < depTimeMin) {
                arrivalDatetime += 1440;
            }

            bool feasible = false;
            double waitTime = 0.0;

            if (u == startIdx) {
                if (departureDatetime >= absoluteStartTimeMin) {
                    feasible = true;
                    waitTime = departureDatetime - absoluteStartTimeMin;
                }
            } else {
                if (departureDatetime >= arrivalTimes[u]) {
                    feasible = true;
                    waitTime = departureDatetime - arrivalTimes[u];
                }
            }

            if (!feasible) {
                edge = edge->next;
                continue;
            }

            double edgeWeight = 0.0;
            if (mode == MODE_DIJKSTRA_COST || mode == MODE_ASTAR_COST) {
                edgeWeight = edge->data.weight;
                if (v != endIdx && waitTime > 720.0) {
                    double layoverDays = ceil(waitTime / 1440.0);
                    edgeWeight += layoverDays * g.ports[u].dailyCharge;
                }
            } else {
                edgeWeight = route->durationMinutes + waitTime;
            }

            double newMetric = minMetric[u] + edgeWeight;
            if (newMetric < minMetric[v]) {
                minMetric[v] = newMetric;
                arrivalTimes[v] = (double)arrivalDatetime;
                parent[v] = u;

                double h = calculateHeuristic(g, v, endIdx, mode);
                pq.push(DijkstraState(v, newMetric, newMetric + h));
            }

            edge = edge->next;
        }
    }

    if (parent[endIdx] == -1 && endIdx != startIdx) {
        return result;
    }

    SimpleVector<int> reversePath;
    int current = endIdx;
    while (current != -1) {
        reversePath.push_back(current);
        current = parent[current];
    }

    for (int i = reversePath.size() - 1; i >= 0; i--) {
        result.path.push_back(reversePath[i]);
    }

    result.totalCost = minMetric[endIdx];

    return result;
}

inline MultiLegResult calculateMultiLegRoute(Graph& g, const SimpleVector<string>& stops, SearchMode mode, int startDay = 1) {
    MultiLegResult result;
    result.isValid = true;
    result.departureDay = startDay;
    result.totalCostWithCharges = 0.0;

    if (stops.size() < 2) {
        result.isValid = false;
        return result;
    }

    double decemberStartMinutes = 482400.0;
    double currentArrivalTime = decemberStartMinutes + (startDay - 1) * 1440.0;
    int finalDestIdx = g.getIndex(stops[stops.size() - 1]);
    
    for (int legIdx = 0; legIdx < stops.size() - 1; legIdx++) {
        PathResult segment;
        
        if (legIdx == 0) {
            segment = findPath(g, stops[legIdx], stops[legIdx + 1], mode, startDay);
        } else {
            segment = findPathSegment(g, stops[legIdx], stops[legIdx + 1], mode, currentArrivalTime);
        }
        
        if (segment.path.size() == 0) {
            result.isValid = false;
            return result;
        }
        
        double segmentTime = 0.0;
        double segmentCost = 0.0;
        double legArrivalTime = currentArrivalTime;
        
        for (int i = 0; i < segment.path.size() - 1; i++) {
            int u = segment.path[i];
            int v = segment.path[i + 1];
            
            Node<Edge>* edge = g.adjList[u].head;
            while (edge != nullptr) {
                if (edge->data.destinationPortIndex == v) {
                    Route* route = edge->data.routeData;
                    
                    segmentCost += edge->data.weight;
                    
                    Date depDate = parseDate(route->departureDate);
                    int depTimeMin = timeToMinutes(route->depTime);
                    int arrTimeMin = timeToMinutes(route->arrTime);
                    
                    long long departureDatetime = dateTimeToMinutes(depDate, depTimeMin);
                    long long arrivalDatetime = dateTimeToMinutes(depDate, arrTimeMin);
                    
                    if (arrTimeMin < depTimeMin) {
                        arrivalDatetime += 1440;
                    }
                    
                    while (departureDatetime < legArrivalTime) {
                        departureDatetime += 10080;
                        arrivalDatetime += 10080;
                    }
                    
                    double waitTime = departureDatetime - legArrivalTime;
                    
                    if (legIdx == 0 && i == 0) {
                        segmentTime += route->durationMinutes;
                    } else {
                        segmentTime += waitTime + route->durationMinutes;
                        
                        bool isStart = (legIdx == 0 && i == 0);
                        bool isFinalDest = (u == finalDestIdx);
                        
                        if (!isStart && !isFinalDest && waitTime > 720.0) {
                            double days = ceil(waitTime / 1440.0);
                            double portCharge = days * g.ports[u].dailyCharge;
                            segmentCost += portCharge;
                            
                            cout << "  [Port Charge] " << g.ports[u].name << ": wait=" 
                                 << (int)(waitTime / 60) << "h " << (int)((int)waitTime % 60) << "m"
                                 << ", days=" << fixed << setprecision(2) << days 
                                 << ", charge=$" << fixed << setprecision(2) << portCharge << endl;
                        }
                    }
                    
                    legArrivalTime = (double)arrivalDatetime;
                    break;
                }
                edge = edge->next;
            }
        }
        
        segment.totalCost = segmentTime;
        result.segments.push_back(segment);
        result.totalCostWithCharges += segmentCost;
        
        currentArrivalTime = legArrivalTime;
    }

    return result;
}

#endif