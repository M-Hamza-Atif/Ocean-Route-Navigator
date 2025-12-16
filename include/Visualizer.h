#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <SFML/Graphics.hpp>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <iostream>
#include "Graph.h"
#include "DataStructs.h"
#include "SimpleVector.h"
#include "Algorithms.h"
#include "UIComponents.h"

class OceanVisualizer {
private:
    enum ScreenState { MENU, BOOKING, COMPANIES, MULTI_LEG };
    ScreenState currentScreen = MENU;
    
    sf::RenderWindow window;
    sf::Texture mapTexture;
    sf::Sprite mapSprite;
    sf::Texture iconTexture;
    sf::Sprite iconSprite;
    sf::Font font;
    sf::Font titleFont;
    Graph* graph;

    int startPortIdx = -1;
    int endPortIdx = -1;
    int hoverPortIdx = -1;
    bool isAnimating = false;
    SimpleVector<int> currentPath;
    
    bool hoverMenuBooking = false;
    bool hoverMenuCompany = false;
    bool hoverMenuMultiLeg = false;
    bool hoverMenuExit = false;
    bool hoverBack = false;
    bool hoverDijkstraCost = false;
    bool hoverDijkstraTime = false;
    bool hoverAStarCost = false;
    bool hoverAStarTime = false;
    bool hoverFindRoute = false;
    bool hoverBook = false;
    bool hoverMinusDate = false;
    bool hoverPlusDate = false;

    PathResult lastResult;
    bool pathFound = false;
    bool resultComputed = false;
    SearchMode currentMode = MODE_DIJKSTRA_COST;

    sf::RectangleShape sidePanel;
    sf::RectangleShape btnDijkstraCost;
    sf::RectangleShape btnDijkstraTime;
    sf::RectangleShape btnAStarCost;
    sf::RectangleShape btnAStarTime;
    sf::RectangleShape findRouteButton;
    sf::RectangleShape bookButton;
    sf::RectangleShape bottomPanel;
    sf::RectangleShape backButton;
    
    sf::RectangleShape menuBtnRouteBooking;
    sf::RectangleShape menuBtnCompanyRoutes;
    sf::RectangleShape menuBtnMultiLeg;
    sf::RectangleShape menuBtnExit;
    
    int selectedMenuIndex = 0;  // 0=Booking, 1=MultiLeg, 2=Companies, 3=Exit
    
    SimpleList<int> bookedItinerary;
    
    int selectedDay = 1;
    sf::RectangleShape minusDateBtn;
    sf::RectangleShape plusDateBtn;
    
    // New UI Components for Booking Panel
    InputBox* originPortInput = nullptr;
    InputBox* destPortInput = nullptr;
    InputBox* dayInput = nullptr;
    InputBox* monthInput = nullptr;
    InputBox* yearInput = nullptr;
    ToggleSwitch* algorithmToggle = nullptr;   // Dijkstra | A*
    ToggleSwitch* optimizeToggle = nullptr;    // Cost ($) | Time (T)
    
    string selectedCompanyFilter = "ALL";
    SimpleVector<string> allCompanies;
    SimpleVector<sf::RectangleShape> companyButtons;
    
    SimpleVector<string> multiLegWaypoints;
    string searchBuffer = "";
    string multiLegErrorMessage = "";
    MultiLegResult lastMultiLegResult;
    bool calculateAttempted = false;
    sf::RectangleShape searchBox;
    InputBox* multiLegSearchInput = nullptr;
    sf::RectangleShape addWaypointBtn;
    sf::RectangleShape calcJourneyBtn;
    bool searchBoxFocused = false;
    SimpleVector<sf::RectangleShape> removeWaypointBtns;  // [X] buttons for each waypoint
    bool hoverAddWaypoint = false;
    bool hoverCalcJourney = false;
    SimpleVector<bool> hoverRemoveWaypoint;

    int currentSegment = 0;
    float segmentProgress = 0.0f;
    sf::Clock animationClock;
    float shipSpeed = 100.0f;
    
    int lastDisplayedSegment = -1;
    bool journeyInfoVisible = false;

    float distance(float x1, float y1, float x2, float y2) {
        return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
    }

    // Case-insensitive port name lookup
    int findPortIndex(const string& name) {
        if (name.empty()) return -1;
        
        string lowerName = name;
        for (size_t i = 0; i < lowerName.size(); i++) {
            lowerName[i] = tolower(lowerName[i]);
        }
        
        for (int i = 0; i < graph->ports.size(); i++) {
            string portName = graph->ports[i].name;
            string lowerPort = portName;
            for (size_t j = 0; j < lowerPort.size(); j++) {
                lowerPort[j] = tolower(lowerPort[j]);
            }
            
            if (lowerPort == lowerName) {
                return i;
            }
        }
        return -1;
    }
    
    // Validate port name inputs and set visual state
    void validatePortInputs() {
        // Validate origin port
        string originText = originPortInput->getText();
        int originIdx = findPortIndex(originText);
        if (originText.empty()) {
            originPortInput->setValidationState(VALIDATION_NEUTRAL);
            startPortIdx = -1;
        } else if (originIdx != -1) {
            originPortInput->setValidationState(VALIDATION_VALID);
            startPortIdx = originIdx;
        } else {
            originPortInput->setValidationState(VALIDATION_INVALID);
            startPortIdx = -1;
        }
        
        // Validate destination port
        string destText = destPortInput->getText();
        int destIdx = findPortIndex(destText);
        if (destText.empty()) {
            destPortInput->setValidationState(VALIDATION_NEUTRAL);
            endPortIdx = -1;
        } else if (destIdx != -1) {
            destPortInput->setValidationState(VALIDATION_VALID);
            endPortIdx = destIdx;
        } else {
            destPortInput->setValidationState(VALIDATION_INVALID);
            endPortIdx = -1;
        }
    }
    
    // Update SearchMode from toggle states
    void updateSearchModeFromToggles() {
        bool isDijkstra = algorithmToggle->isLeftSelected();
        bool isCost = optimizeToggle->isLeftSelected();
        
        if (isDijkstra && isCost) {
            currentMode = MODE_DIJKSTRA_COST;
        } else if (isDijkstra && !isCost) {
            currentMode = MODE_DIJKSTRA_TIME;
        } else if (!isDijkstra && isCost) {
            currentMode = MODE_ASTAR_COST;
        } else {
            currentMode = MODE_ASTAR_TIME;
        }
    }
    
    // Get selected day from input
    int getSelectedDay() {
        string dayText = dayInput->getText();
        if (dayText.empty()) return 1;
        try {
            int d = stoi(dayText);
            if (d < 1) return 1;
            if (d > 31) return 31;
            return d;
        } catch (...) {
            return 1;
        }
    }

public:
    OceanVisualizer(Graph* g) : graph(g) {
        window.create(sf::VideoMode(1350, 850), "Ocean Route Navigator - Maritime Logistics");
        
        if (!mapTexture.loadFromFile("../assets/world_map.png")) {
            cout << "Error: Could not load world_map.png" << endl;
        }
        mapSprite.setTexture(mapTexture);
        mapSprite.setPosition(0.f, 0.f);
        
        if (!font.loadFromFile("../assets/arial.ttf")) {
            cout << "Warning: Could not load font (arial.ttf)" << endl;
        }
        
        titleFont = font;

        sidePanel.setSize(sf::Vector2f(300.f, 700.f));
        sidePanel.setPosition(1050.f, 0.f);
        sidePanel.setFillColor(sf::Color(50, 50, 60));
        
        bottomPanel.setSize(sf::Vector2f(1350.f, 150.f));
        bottomPanel.setPosition(0.f, 700.f);
        bottomPanel.setFillColor(sf::Color(30, 30, 40));

        auto styleButton = [](sf::RectangleShape& b, float x, float y) {
            b.setSize(sf::Vector2f(260.f, 40.f));
            b.setPosition(x, y);
            b.setOutlineColor(sf::Color::White);
            b.setOutlineThickness(1.5f);
        };

        float bx = 1065.f; // panel left + margin
        float by = 20.f;
        styleButton(btnDijkstraCost, bx, by);
        styleButton(btnDijkstraTime, bx, by + 50.f);
        styleButton(btnAStarCost,   bx, by + 100.f);
        styleButton(btnAStarTime,   bx, by + 150.f);

        // Initial colors (Dijkstra Cost selected)
        btnDijkstraCost.setFillColor(sf::Color(0, 120, 200));
        btnDijkstraTime.setFillColor(sf::Color(80, 80, 80));
        btnAStarCost.setFillColor(sf::Color(80, 80, 80));
        btnAStarTime.setFillColor(sf::Color(80, 80, 80));

        // Find Route button
        styleButton(findRouteButton, bx, by + 210.f);
        findRouteButton.setFillColor(sf::Color(0, 180, 0));
        
        // Book Route button
        styleButton(bookButton, bx, by + 270.f);
        bookButton.setFillColor(sf::Color(255, 140, 0));  // Orange
        
        // Date selection buttons (smaller, between Book Route and stats area)
        minusDateBtn.setSize(sf::Vector2f(40.f, 30.f));
        minusDateBtn.setPosition(bx, by + 330.f);
        minusDateBtn.setFillColor(sf::Color(100, 100, 120));
        minusDateBtn.setOutlineColor(sf::Color::White);
        minusDateBtn.setOutlineThickness(1.5f);
        
        plusDateBtn.setSize(sf::Vector2f(40.f, 30.f));
        plusDateBtn.setPosition(bx + 220.f, by + 330.f);
        plusDateBtn.setFillColor(sf::Color(100, 100, 120));
        plusDateBtn.setOutlineColor(sf::Color::White);
        plusDateBtn.setOutlineThickness(1.5f);
        
        // Back button for booking screen
        backButton.setSize(sf::Vector2f(80.f, 35.f));
        backButton.setPosition(10.f, 10.f);
        backButton.setFillColor(sf::Color(200, 80, 60));
        backButton.setOutlineColor(sf::Color(240, 120, 100));  // Lighter red border
        backButton.setOutlineThickness(2.f);
        
        // Initialize new UI components for booking panel
        float panelX = 1065.f;

        // Port input boxes
        originPortInput = new InputBox(panelX, 20.f, 260.f, 28.f, "Origin Port", font);
        originPortInput->setPlaceholder("Enter port name...");

        destPortInput = new InputBox(panelX, 80.f, 260.f, 28.f, "Destination Port", font);
        destPortInput->setPlaceholder("Enter port name...");

        // Date input boxes (3 compact fields in a row)
        dayInput = new InputBox(panelX, 140.f, 55.f, 28.f, "Day", font);
        dayInput->setPlaceholder("DD");
        dayInput->setText("1");

        monthInput = new InputBox(panelX + 65.f, 140.f, 55.f, 28.f, "Month", font);
        monthInput->setPlaceholder("MM");
        monthInput->setText("12");

        yearInput = new InputBox(panelX + 130.f, 140.f, 70.f, 28.f, "Year", font);
        yearInput->setPlaceholder("YYYY");
        yearInput->setText("2024");

        // Algorithm toggle: Dijkstra | A*
        algorithmToggle = new ToggleSwitch(panelX, 200.f, 260.f, 32.f, "Dijkstra", "A*", font);

        // Optimize toggle: Cost ($) | Time (T)
        optimizeToggle = new ToggleSwitch(panelX, 240.f, 260.f, 32.f, "Cost ($)", "Time (T)", font);

        // Reposition Find Route and Book Route buttons for new layout
        findRouteButton.setPosition(panelX, 290.f);
        bookButton.setPosition(panelX, 340.f);
        
        // Menu buttons (centered, 400x60)
        float menuBtnWidth = 400.f;
        float menuBtnHeight = 60.f;
        float menuBtnX = (1350.f - menuBtnWidth) / 2.f;
        
        auto initMenuBtn = [&](sf::RectangleShape& btn, float yPos, sf::Color fillColor, sf::Color borderColor) {
            btn.setSize(sf::Vector2f(menuBtnWidth, menuBtnHeight));
            btn.setPosition(menuBtnX, yPos);
            btn.setFillColor(fillColor);
            btn.setOutlineColor(borderColor);
            btn.setOutlineThickness(2.f);
        };
        
        // Initialize menu buttons
        initMenuBtn(menuBtnRouteBooking, 420.f, sf::Color(50, 100, 150), sf::Color(100, 160, 220));
        initMenuBtn(menuBtnMultiLeg, 500.f, sf::Color(120, 80, 150), sf::Color(170, 130, 200));
        initMenuBtn(menuBtnCompanyRoutes, 580.f, sf::Color(60, 120, 80), sf::Color(100, 180, 120));
        initMenuBtn(menuBtnExit, 660.f, sf::Color(150, 50, 50), sf::Color(200, 100, 100));
        
        // Load menu icon
        if (!iconTexture.loadFromFile("../assets/icon.jpg")) {
            std::cerr << "Warning: Could not load icon.jpg from assets folder" << std::endl;
        }
        iconSprite.setTexture(iconTexture);
        // Scale icon to target width
        sf::Vector2u iconSize = iconTexture.getSize();
        if (iconSize.x > 0 && iconSize.y > 0) {
            float targetWidth = 200.f;
            float scale = targetWidth / static_cast<float>(iconSize.x);
            iconSprite.setScale(scale, scale);
        }
        
        // Initialize company names for company routes screen (exact names from Routes.txt)
        allCompanies.push_back("MaerskLine");
        allCompanies.push_back("MSC");
        allCompanies.push_back("COSCO");
        allCompanies.push_back("CMA_CGM");
        allCompanies.push_back("HapagLloyd");
        allCompanies.push_back("ONE");
        allCompanies.push_back("Evergreen");
        allCompanies.push_back("PIL");
        allCompanies.push_back("ZIM");
        allCompanies.push_back("YangMing");
        
        // Initialize company filter buttons
        for (int i = 0; i < 11; i++) {
            sf::RectangleShape btn;
            btn.setSize(sf::Vector2f(260.f, 35.f));
            btn.setPosition(1065.f, 60.f + i * 45.f);
            btn.setFillColor(sf::Color(40, 40, 48));
            btn.setOutlineColor(sf::Color(100, 150, 255));
            btn.setOutlineThickness(2.f);
            companyButtons.push_back(btn);
        }

        searchBox.setSize(sf::Vector2f(200.f, 30.f));
        searchBox.setPosition(1065.f, 20.f);
        searchBox.setFillColor(sf::Color(60, 60, 70));
        searchBox.setOutlineColor(sf::Color::White);
        searchBox.setOutlineThickness(2.f);
        
        // Multi-leg search input (label at y=15, box at y=35)
        multiLegSearchInput = new InputBox(1065.f, 15.f, 200.f, 28.f, "Add Waypoint", font);
        multiLegSearchInput->setPlaceholder("Enter port...");
        
        // Add waypoint button
        addWaypointBtn.setSize(sf::Vector2f(50.f, 28.f));
        addWaypointBtn.setPosition(1270.f, 35.f);
        addWaypointBtn.setFillColor(sf::Color(40, 150, 60));
        addWaypointBtn.setOutlineColor(sf::Color(80, 180, 100));
        addWaypointBtn.setOutlineThickness(2.f);
        
        // Calculate journey button
        calcJourneyBtn.setSize(sf::Vector2f(260.f, 50.f));
        calcJourneyBtn.setPosition(1065.f, 640.f);
        calcJourneyBtn.setFillColor(sf::Color(40, 150, 60));
        calcJourneyBtn.setOutlineColor(sf::Color(80, 180, 100));
        calcJourneyBtn.setOutlineThickness(2.f);
    }

    // Handle mouse clicks and input events
    void handleInput(sf::Event& event) {
        // Handle keyboard navigation in menu
        if (currentScreen == MENU && event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Up) {
                selectedMenuIndex = (selectedMenuIndex - 1 + 4) % 4;
            } else if (event.key.code == sf::Keyboard::Down) {
                selectedMenuIndex = (selectedMenuIndex + 1) % 4;
            } else if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Enter) {
                // Activate selected menu option
                switch (selectedMenuIndex) {
                    case 0: currentScreen = BOOKING; break;
                    case 1: currentScreen = MULTI_LEG; break;
                    case 2: currentScreen = COMPANIES; break;
                    case 3: window.close(); break;
                }
            }
            return;
        }
        
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            float mx = static_cast<float>(mousePos.x);
            float my = static_cast<float>(mousePos.y);
            
            if (currentScreen == MENU) {
                if (menuBtnRouteBooking.getGlobalBounds().contains(mx, my)) {
                    selectedMenuIndex = 0;
                    currentScreen = BOOKING;
                } else if (menuBtnMultiLeg.getGlobalBounds().contains(mx, my)) {
                    selectedMenuIndex = 1;
                    currentScreen = MULTI_LEG;
                } else if (menuBtnCompanyRoutes.getGlobalBounds().contains(mx, my)) {
                    selectedMenuIndex = 2;
                    currentScreen = COMPANIES;
                } else if (menuBtnExit.getGlobalBounds().contains(mx, my)) {
                    window.close();
                }
                return;
            }
            
            if (currentScreen != MENU && backButton.getGlobalBounds().contains(mx, my)) {
                currentScreen = MENU;
                startPortIdx = -1;
                endPortIdx = -1;
                pathFound = false;
                resultComputed = false;
                isAnimating = false;
                currentPath.clear();
                return;
            }
            
            if (currentScreen == COMPANIES) {
                for (int i = 0; i < companyButtons.size(); i++) {
                    if (companyButtons[i].getGlobalBounds().contains(mx, my)) {
                        if (i == 0) {
                            selectedCompanyFilter = "ALL";
                            cout << "[Company Filter] All Companies selected." << endl;
                        } else {
                            selectedCompanyFilter = allCompanies[i - 1];
                            cout << "[Company Filter] " << selectedCompanyFilter << " selected." << endl;
                        }
                        break;
                    }
                }
                return;
            }
            
            if (currentScreen == MULTI_LEG) {
                // Reset validation state on click
                multiLegSearchInput->resetValidation();
                
                // Handle new InputBox for multi-leg search
                multiLegSearchInput->handleEvent(event);
                
                // Check if Enter was pressed to add waypoint
                if (multiLegSearchInput->wasSubmitted()) {
                    string searchText = multiLegSearchInput->getText();
                    if (!searchText.empty()) {
                        // Case-insensitive port name search (reuse findPortIndex)
                        int portIdx = findPortIndex(searchText);
                        
                        if (portIdx != -1) {
                            multiLegWaypoints.push_back(graph->ports[portIdx].name);
                            multiLegSearchInput->clear();
                            multiLegSearchInput->setValidationState(VALIDATION_VALID);
                            multiLegErrorMessage = "";
                            calculateAttempted = false;
                            cout << "[Multi-leg] Added waypoint: " << graph->ports[portIdx].name << endl;
                        } else {
                            multiLegSearchInput->setValidationState(VALIDATION_INVALID);
                            multiLegErrorMessage = "Port not found: " + searchText;
                            cout << "[Multi-leg] Port not found: " << searchText << endl;
                        }
                    }
                }
                
                // Check Add button click (use multiLegSearchInput)
                if (addWaypointBtn.getGlobalBounds().contains(mx, my)) {
                    string searchText = multiLegSearchInput->getText();
                    if (!searchText.empty()) {
                        int portIdx = findPortIndex(searchText);
                        
                        if (portIdx != -1) {
                            multiLegWaypoints.push_back(graph->ports[portIdx].name);
                            multiLegSearchInput->clear();
                            multiLegSearchInput->setValidationState(VALIDATION_VALID);
                            multiLegErrorMessage = "";
                            calculateAttempted = false;
                            cout << "[Multi-leg] Added waypoint: " << graph->ports[portIdx].name << endl;
                        } else {
                            multiLegSearchInput->setValidationState(VALIDATION_INVALID);
                            multiLegErrorMessage = "Port not found: " + searchText;
                            cout << "[Multi-leg] Port not found: " << searchText << endl;
                        }
                    }
                    return;
                }
                
                // Check Remove buttons
                for (int i = 0; i < removeWaypointBtns.size(); i++) {
                    if (removeWaypointBtns[i].getGlobalBounds().contains(mx, my)) {
                        cout << "[Multi-leg] Removed waypoint: " << multiLegWaypoints[i] << endl;
                        calculateAttempted = false;  // Reset flag when waypoints change
                        
                        // Remove waypoint at index i
                        SimpleVector<string> newWaypoints;
                        for (int j = 0; j < multiLegWaypoints.size(); j++) {
                            if (j != i) newWaypoints.push_back(multiLegWaypoints[j]);
                        }
                        multiLegWaypoints = newWaypoints;
                        
                        // Clear result since waypoints changed
                        lastMultiLegResult.isValid = false;
                        lastMultiLegResult.segments.clear();
                        return;
                    }
                }
                
                // Check Calculate Journey button
                if (calcJourneyBtn.getGlobalBounds().contains(mx, my)) {
                    calculateAttempted = true;  // Mark that calculation has been attempted
                    if (multiLegWaypoints.size() >= 2) {
                        cout << "[Multi-leg] Calculating journey across all dates..." << endl;
                        
                        // Try all departure dates (1-31 December) to find the best route
                        MultiLegResult bestResult;
                        bestResult.isValid = false;
                        double bestMetric = 1e18;
                        
                        for (int day = 1; day <= 31; day++) {
                            MultiLegResult result = calculateMultiLegRoute(*graph, multiLegWaypoints, MODE_DIJKSTRA_TIME, day);
                            
                            if (result.isValid) {
                                // Calculate total time for this result
                                double totalTime = 0.0;
                                for (int i = 0; i < result.segments.size(); i++) {
                                    totalTime += result.segments[i].totalCost;
                                }
                                
                                // Keep the result with minimum time
                                if (totalTime < bestMetric) {
                                    bestMetric = totalTime;
                                    bestResult = result;
                                }
                            }
                        }
                        
                        lastMultiLegResult = bestResult;
                        
                        if (lastMultiLegResult.isValid) {
                            multiLegErrorMessage = "";  // Clear error on success
                            cout << "[Multi-leg] Journey found! Day: " << lastMultiLegResult.departureDay 
                                 << " | Segments: " << lastMultiLegResult.segments.size() 
                                 << " | Total Time: " << (int)(bestMetric / 60) << "h " 
                                 << (int)bestMetric % 60 << "m" << endl;
                            
                            // Debug: Print each segment's time and path
                            for (int i = 0; i < lastMultiLegResult.segments.size(); i++) {
                                cout << "  Segment " << (i+1) << ": ";
                                for (int j = 0; j < lastMultiLegResult.segments[i].path.size(); j++) {
                                    cout << graph->ports[lastMultiLegResult.segments[i].path[j]].name;
                                    if (j < lastMultiLegResult.segments[i].path.size() - 1) cout << " -> ";
                                }
                                cout << " | Time: "
                                     << (int)(lastMultiLegResult.segments[i].totalCost / 60) << "h "
                                     << (int)lastMultiLegResult.segments[i].totalCost % 60 << "m" << endl;
                            }
                            cout << "  Total Cost with charges: $" << lastMultiLegResult.totalCostWithCharges << endl;
                        } else {
                            cout << "[Multi-leg] Journey impossible - no valid path found on any date!" << endl;
                        }
                    } else {
                        cout << "[Multi-leg] Need at least 2 waypoints to calculate journey!" << endl;
                        multiLegErrorMessage = "Need at least 2 waypoints!";
                    }
                    return;
                }
                return;
            }
            
            if (currentScreen == BOOKING) {
                // Handle new UI components first (for click focus)
                originPortInput->handleEvent(event);
                destPortInput->handleEvent(event);
                dayInput->handleEvent(event);
                monthInput->handleEvent(event);
                yearInput->handleEvent(event);
                algorithmToggle->handleEvent(event);
                optimizeToggle->handleEvent(event);
                
                // Update SearchMode based on toggles
                updateSearchModeFromToggles();
                
                if (mx < 1050.0f) {
                bool clickedPort = false;
                for (int i = 0; i < graph->ports.size(); i++) {
                    Port& port = graph->ports[i];
                    float dist = distance(mx, my, static_cast<float>(port.x), static_cast<float>(port.y));

                    if (dist < 15.0f) {  // Click radius threshold
                        clickedPort = true;

                        if (startPortIdx == i) {
                            startPortIdx = -1;
                            endPortIdx = -1;
                            originPortInput->setText("");
                            originPortInput->setValidationState(VALIDATION_NEUTRAL);
                            destPortInput->setText("");
                            destPortInput->setValidationState(VALIDATION_NEUTRAL);
                            currentPath.clear();
                            isAnimating = false;
                            if (journeyInfoVisible) {
                                journeyInfoVisible = false;
                                lastDisplayedSegment = -1;
                            }
                        } else if (endPortIdx == i) {
                            endPortIdx = -1;
                            destPortInput->setText("");
                            destPortInput->setValidationState(VALIDATION_NEUTRAL);
                            currentPath.clear();
                            isAnimating = false;
                            if (journeyInfoVisible) {
                                journeyInfoVisible = false;
                                lastDisplayedSegment = -1;
                            }
                        } else if (startPortIdx == -1) {
                            startPortIdx = i;
                            originPortInput->setText(graph->ports[i].name);
                            originPortInput->setValidationState(VALIDATION_VALID);
                            endPortIdx = -1;
                            destPortInput->setText("");
                            destPortInput->setValidationState(VALIDATION_NEUTRAL);
                            currentPath.clear();
                            isAnimating = false;
                            if (journeyInfoVisible) {
                                journeyInfoVisible = false;
                                lastDisplayedSegment = -1;
                            }
                        } else if (endPortIdx == -1 && i != startPortIdx) {
                            endPortIdx = i;
                            destPortInput->setText(graph->ports[i].name);
                            destPortInput->setValidationState(VALIDATION_VALID);
                        } else {
                            startPortIdx = i;
                            originPortInput->setText(graph->ports[i].name);
                            originPortInput->setValidationState(VALIDATION_VALID);
                            endPortIdx = -1;
                            destPortInput->setText("");
                            destPortInput->setValidationState(VALIDATION_NEUTRAL);
                            currentPath.clear();
                            isAnimating = false;
                            if (journeyInfoVisible) {
                                journeyInfoVisible = false;
                                lastDisplayedSegment = -1;
                            }
                        }
                        break;
                    }
                }
            } else {
                // Panel interactions (x >= 1050)
                if (btnDijkstraCost.getGlobalBounds().contains(mx, my)) {
                    currentMode = MODE_DIJKSTRA_COST;
                    pathFound = false; // Reset display until Find Route is clicked
                    resultComputed = false;
                    if (journeyInfoVisible) {
                        journeyInfoVisible = false;
                        lastDisplayedSegment = -1;
                    }
                } else if (btnDijkstraTime.getGlobalBounds().contains(mx, my)) {
                    currentMode = MODE_DIJKSTRA_TIME;
                    pathFound = false; // Reset display until Find Route is clicked
                    resultComputed = false;
                    if (journeyInfoVisible) {
                        journeyInfoVisible = false;
                        lastDisplayedSegment = -1;
                    }
                } else if (btnAStarCost.getGlobalBounds().contains(mx, my)) {
                    currentMode = MODE_ASTAR_COST;
                    pathFound = false; // Reset display until Find Route is clicked
                    resultComputed = false;
                    if (journeyInfoVisible) {
                        journeyInfoVisible = false;
                        lastDisplayedSegment = -1;
                    }
                } else if (btnAStarTime.getGlobalBounds().contains(mx, my)) {
                    currentMode = MODE_ASTAR_TIME;
                    pathFound = false; // Reset display until Find Route is clicked
                    resultComputed = false;
                    if (journeyInfoVisible) {
                        journeyInfoVisible = false;
                        lastDisplayedSegment = -1;
                    }
                } else if (bookButton.getGlobalBounds().contains(mx, my)) {
                    // Book Route: Copy lastResult.path into bookedItinerary
                    if (pathFound) {
                        bookedItinerary.clear();
                        for (int i = 0; i < lastResult.path.size(); i++) {
                            bookedItinerary.push_back(lastResult.path[i]);
                        }
                        cout << "\n[✓] Itinerary updated! Booked " << bookedItinerary.size() << " ports.\n" << endl;
                    }
                } else if (findRouteButton.getGlobalBounds().contains(mx, my)) {
                    // Validate port inputs first
                    validatePortInputs();
                    
                    if (startPortIdx != -1 && endPortIdx != -1 && !isAnimating) {
                        string startName = graph->ports[startPortIdx].name;
                        string endName = graph->ports[endPortIdx].name;
                        int departureDay = getSelectedDay();

                            lastResult = findPath(*graph, startName, endName, currentMode, departureDay);
                            // Mark that a search was executed for the current selection
                            resultComputed = true;

                        if (lastResult.path.size() > 0) {
                            pathFound = true;
                            currentPath = lastResult.path;
                            isAnimating = true;
                            currentSegment = 0;
                            segmentProgress = 0.0f;
                            animationClock.restart();
                            journeyInfoVisible = true;
                            lastDisplayedSegment = -1;

                            // Console output: mode, route, cost, time, hops
                            cout << "\n=== Path Found ===" << endl;
                            cout << "Mode: " << (currentMode == MODE_DIJKSTRA_COST ? "Dijkstra (Cost)" :
                                                currentMode == MODE_DIJKSTRA_TIME ? "Dijkstra (Time)" :
                                                currentMode == MODE_ASTAR_COST ? "A* (Cost)" : "A* (Time)") << endl;

                            // Route listing
                            cout << "Route: ";
                            for (int i = 0; i < lastResult.path.size(); i++) {
                                cout << graph->ports[lastResult.path[i]].name;
                                if (i < lastResult.path.size() - 1) cout << " -> ";
                            }
                            cout << endl;

                            // Calculate BOTH metrics for the path regardless of mode
                            // 1. Total Cost (routes + layovers)
                            double totalCost = 0.0;
                            if (currentMode == MODE_DIJKSTRA_COST || currentMode == MODE_ASTAR_COST) {
                                // Cost mode: algorithm already computed total cost including layovers
                                totalCost = lastResult.totalCost;
                            } else {
                                // Time mode: need to calculate total cost with layovers
                                // This requires reconstructing the journey to compute wait times and layover charges
                                double routeCosts = 0.0;
                                double layoverCosts = 0.0;
                                
                                // Start time calculation
                                double decemberStartMinutes = 482400.0;
                                double currentArrival = decemberStartMinutes + (getSelectedDay() - 1) * 1440.0;
                                
                                for (int i = 0; i < lastResult.path.size() - 1; i++) {
                                    int u = lastResult.path[i];
                                    int v = lastResult.path[i + 1];
                                    Node<Edge>* en = graph->adjList[u].head;
                                    while (en != nullptr) {
                                        if (en->data.destinationPortIndex == v) {
                                            Route* r = en->data.routeData;
                                            routeCosts += en->data.weight;
                                            
                                            // Parse route times
                                            Date depDate = parseDate(r->departureDate);
                                            int depTimeMin = timeToMinutes(r->depTime);
                                            int arrTimeMin = timeToMinutes(r->arrTime);
                                            long long departureDatetime = dateTimeToMinutes(depDate, depTimeMin);
                                            long long arrivalDatetime = dateTimeToMinutes(depDate, arrTimeMin);
                                            
                                            // Handle overnight routes
                                            if (arrTimeMin < depTimeMin) {
                                                arrivalDatetime += 1440;
                                            }
                                            
                                            // Find next valid departure (weekly cycling)
                                            while (departureDatetime < currentArrival) {
                                                departureDatetime += 10080;
                                                arrivalDatetime += 10080;
                                            }
                                            
                                            // Calculate wait and layover for intermediate ports (not at start or end)
                                            if (i > 0 && i < lastResult.path.size() - 1) {
                                                double waitMins = departureDatetime - currentArrival;
                                                if (waitMins > 720.0) {
                                                    double days = ceil(waitMins / 1440.0);
                                                    layoverCosts += days * graph->ports[u].dailyCharge;
                                                }
                                            }
                                            
                                            // Update arrival time
                                            currentArrival = (double)arrivalDatetime;
                                            break;
                                        }
                                        en = en->next;
                                    }
                                }
                                totalCost = routeCosts + layoverCosts;
                            }
                            
                            // 2. Total Time (sailing + waits)
                            long long totalTime = 0;
                            if (currentMode == MODE_DIJKSTRA_TIME || currentMode == MODE_ASTAR_TIME) {
                                // Time mode: algorithm already computed total time including waits
                                totalTime = static_cast<long long>(llround(lastResult.totalCost));
                            } else {
                                // Cost mode: need to calculate total time with waits
                                double sailTime = 0.0;
                                double waitTime = 0.0;
                                
                                // Start time calculation
                                double decemberStartMinutes = 482400.0;
                                double currentArrival = decemberStartMinutes + (getSelectedDay() - 1) * 1440.0;
                                
                                for (int i = 0; i < lastResult.path.size() - 1; i++) {
                                    int u = lastResult.path[i];
                                    int v = lastResult.path[i + 1];
                                    Node<Edge>* en = graph->adjList[u].head;
                                    while (en != nullptr) {
                                        if (en->data.destinationPortIndex == v) {
                                            Route* r = en->data.routeData;
                                            sailTime += r->durationMinutes;
                                            
                                            // Parse route times
                                            Date depDate = parseDate(r->departureDate);
                                            int depTimeMin = timeToMinutes(r->depTime);
                                            int arrTimeMin = timeToMinutes(r->arrTime);
                                            long long departureDatetime = dateTimeToMinutes(depDate, depTimeMin);
                                            long long arrivalDatetime = dateTimeToMinutes(depDate, arrTimeMin);
                                            
                                            // Handle overnight routes
                                            if (arrTimeMin < depTimeMin) {
                                                arrivalDatetime += 1440;
                                            }
                                            
                                            // Find next valid departure (weekly cycling)
                                            while (departureDatetime < currentArrival) {
                                                departureDatetime += 10080;
                                                arrivalDatetime += 10080;
                                            }
                                            
                                            // Calculate wait time for intermediate ports (not at start)
                                            if (i > 0) {
                                                double thisWait = departureDatetime - currentArrival;
                                                waitTime += thisWait;
                                            }
                                            
                                            // Update arrival time
                                            currentArrival = (double)arrivalDatetime;
                                            break;
                                        }
                                        en = en->next;
                                    }
                                }
                                totalTime = static_cast<long long>(llround(sailTime + waitTime));
                            }
                            
                            // Display both metrics
                            cout << "Total Cost: $" << fixed << setprecision(2) << totalCost << endl;
                            long long hours = totalTime / 60;
                            long long mins = totalTime % 60;
                            cout << "Total Time: " << totalTime << " minutes (" << hours << "h " << mins << "m)" << endl;

                            cout << "Hops: " << (lastResult.path.size() - 1) << endl;
                            cout << "==================" << endl;

                        } else {
                            pathFound = false;
                            currentPath.clear();
                            cout << "\n[!] No Path Found between " << startName << " and " << endName << "\n" << endl;
                        }
                    }
                }

                // Update button colors based on selection and hover state
                if (currentMode == MODE_DIJKSTRA_COST) {
                    btnDijkstraCost.setFillColor(hoverDijkstraCost ? sf::Color(40, 160, 240) : sf::Color(0, 120, 200));
                } else {
                    btnDijkstraCost.setFillColor(hoverDijkstraCost ? sf::Color(110, 110, 110) : sf::Color(80, 80, 80));
                }
                
                if (currentMode == MODE_DIJKSTRA_TIME) {
                    btnDijkstraTime.setFillColor(hoverDijkstraTime ? sf::Color(40, 160, 240) : sf::Color(0, 120, 200));
                } else {
                    btnDijkstraTime.setFillColor(hoverDijkstraTime ? sf::Color(110, 110, 110) : sf::Color(80, 80, 80));
                }
                
                if (currentMode == MODE_ASTAR_COST) {
                    btnAStarCost.setFillColor(hoverAStarCost ? sf::Color(40, 160, 240) : sf::Color(0, 120, 200));
                } else {
                    btnAStarCost.setFillColor(hoverAStarCost ? sf::Color(110, 110, 110) : sf::Color(80, 80, 80));
                }
                
                if (currentMode == MODE_ASTAR_TIME) {
                    btnAStarTime.setFillColor(hoverAStarTime ? sf::Color(40, 160, 240) : sf::Color(0, 120, 200));
                } else {
                    btnAStarTime.setFillColor(hoverAStarTime ? sf::Color(110, 110, 110) : sf::Color(80, 80, 80));
                }
            }
            }  // End of BOOKING screen logic
        }
        
        // Handle TextEntered for BOOKING screen (new input boxes)
        if (event.type == sf::Event::TextEntered && currentScreen == BOOKING) {
            originPortInput->handleEvent(event);
            destPortInput->handleEvent(event);
            dayInput->handleEvent(event);
            monthInput->handleEvent(event);
            yearInput->handleEvent(event);
            
            // Check for Enter key submissions on port inputs
            if (originPortInput->wasSubmitted() || destPortInput->wasSubmitted()) {
                validatePortInputs();
            }
        }
        
        // TextEntered for MULTI_LEG is handled by multiLegSearchInput->handleEvent()
        if (event.type == sf::Event::TextEntered && currentScreen == MULTI_LEG) {
            multiLegSearchInput->handleEvent(event);
            
            // Check for Enter key submission
            if (multiLegSearchInput->wasSubmitted()) {
                string searchText = multiLegSearchInput->getText();
                if (!searchText.empty()) {
                    int portIdx = findPortIndex(searchText);
                    
                    if (portIdx != -1) {
                        multiLegWaypoints.push_back(graph->ports[portIdx].name);
                        multiLegSearchInput->clear();
                        multiLegSearchInput->setValidationState(VALIDATION_VALID);
                        multiLegErrorMessage = "";
                        calculateAttempted = false;
                        cout << "[Multi-leg] Added waypoint via Enter: " << graph->ports[portIdx].name << endl;
                    } else {
                        multiLegSearchInput->setValidationState(VALIDATION_INVALID);
                        multiLegErrorMessage = "Port not found: " + searchText;
                        cout << "[Multi-leg] Port not found: " << searchText << endl;
                    }
                }
            } else {
                // Clear error message when user starts typing (not Enter)
                multiLegErrorMessage = "";
                calculateAttempted = false;
            }
        }
    }

    // Update hover state based on mouse position
    void updateHover() {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        float mx = static_cast<float>(mousePos.x);
        float my = static_cast<float>(mousePos.y);

        // Reset all hover states
        hoverPortIdx = -1;
        hoverMenuBooking = hoverMenuCompany = hoverMenuMultiLeg = hoverMenuExit = false;
        hoverBack = hoverDijkstraCost = hoverDijkstraTime = hoverAStarCost = hoverAStarTime = false;
        hoverFindRoute = hoverBook = hoverMinusDate = hoverPlusDate = false;
        
        if (currentScreen == MENU) {
            // Check menu button hovers
            hoverMenuBooking = menuBtnRouteBooking.getGlobalBounds().contains(mx, my);
            hoverMenuCompany = menuBtnCompanyRoutes.getGlobalBounds().contains(mx, my);
            hoverMenuMultiLeg = menuBtnMultiLeg.getGlobalBounds().contains(mx, my);
            hoverMenuExit = menuBtnExit.getGlobalBounds().contains(mx, my);
            
            // Update keyboard selection to match mouse hover
            if (hoverMenuBooking) selectedMenuIndex = 0;
            else if (hoverMenuMultiLeg) selectedMenuIndex = 1;
            else if (hoverMenuCompany) selectedMenuIndex = 2;
            else if (hoverMenuExit) selectedMenuIndex = 3;
        } else if (currentScreen == BOOKING) {
            // Check back button
            hoverBack = backButton.getGlobalBounds().contains(mx, my);
            
            // Check port hover (only in map area)
            if (mx < 1050.0f) {
                for (int i = 0; i < graph->ports.size(); i++) {
                    Port& port = graph->ports[i];
                    float dist = distance(mx, my, static_cast<float>(port.x), static_cast<float>(port.y));
                    
                    if (dist < 15.0f) {
                        hoverPortIdx = i;
                        break;
                    }
                }
            } else {
                // Check UI button hovers (in side panel)
                hoverDijkstraCost = btnDijkstraCost.getGlobalBounds().contains(mx, my);
                hoverDijkstraTime = btnDijkstraTime.getGlobalBounds().contains(mx, my);
                hoverAStarCost = btnAStarCost.getGlobalBounds().contains(mx, my);
                hoverAStarTime = btnAStarTime.getGlobalBounds().contains(mx, my);
                hoverFindRoute = findRouteButton.getGlobalBounds().contains(mx, my);
                hoverBook = bookButton.getGlobalBounds().contains(mx, my);
                hoverMinusDate = minusDateBtn.getGlobalBounds().contains(mx, my);
                hoverPlusDate = plusDateBtn.getGlobalBounds().contains(mx, my);
            }
        } else if (currentScreen == COMPANIES) {
            // Check back button
            hoverBack = backButton.getGlobalBounds().contains(mx, my);
            
            // Check port hover (only in map area)
            if (mx < 1050.0f) {
                for (int i = 0; i < graph->ports.size(); i++) {
                    Port& port = graph->ports[i];
                    float dist = distance(mx, my, static_cast<float>(port.x), static_cast<float>(port.y));
                    
                    if (dist < 15.0f) {
                        hoverPortIdx = i;
                        break;
                    }
                }
            }
        } else if (currentScreen == MULTI_LEG) {
            // Check back button
            hoverBack = backButton.getGlobalBounds().contains(mx, my);
            
            // Check port hover (only in map area)
            if (mx < 1050.0f) {
                for (int i = 0; i < graph->ports.size(); i++) {
                    Port& port = graph->ports[i];
                    float dist = distance(mx, my, static_cast<float>(port.x), static_cast<float>(port.y));
                    
                    if (dist < 15.0f) {
                        hoverPortIdx = i;
                        break;
                    }
                }
            } else {
                // Check multi-leg UI button hovers (in side panel)
                hoverAddWaypoint = addWaypointBtn.getGlobalBounds().contains(mx, my);
                hoverCalcJourney = calcJourneyBtn.getGlobalBounds().contains(mx, my);
                
                // Check remove button hovers
                for (int i = 0; i < removeWaypointBtns.size(); i++) {
                    if (i < hoverRemoveWaypoint.size()) {
                        hoverRemoveWaypoint[i] = removeWaypointBtns[i].getGlobalBounds().contains(mx, my);
                    }
                }
            }
        } else {
            // Other screens also have back button
            hoverBack = backButton.getGlobalBounds().contains(mx, my);
        }
    }

    // Draw UI elements (side panel and overlays)
    // Draw the booked itinerary in the bottom panel
    void drawBookingItinerary() {
        // Draw bottom panel background
        window.draw(bottomPanel);
        
        // Title
        sf::Text title;
        title.setFont(font);
        title.setString("Current Itinerary:");
        title.setCharacterSize(18);
        title.setFillColor(sf::Color::White);
        title.setPosition(20.f, 710.f);
        window.draw(title);
        
        // Draw linked list visualization
        if (bookedItinerary.size() > 0) {
            float startX = 20.f;
            float startY = 745.f;
            float boxWidth = 110.f;
            float boxHeight = 45.f;
            float spacing = 45.f;  // Gap between boxes (for arrow)
            
            Node<int>* current = bookedItinerary.head;
            int index = 0;
            
            while (current != nullptr) {
                int portIdx = current->data;
                float currentX = startX + (index * (boxWidth + spacing));
                
                // Stop drawing if we exceed window width
                if (currentX + boxWidth > 1330.f) {
                    break;
                }
                
                // Draw box with gradient-like fill (darker grey, blue border)
                sf::RectangleShape box(sf::Vector2f(boxWidth, boxHeight));
                box.setPosition(currentX, startY);
                box.setFillColor(sf::Color(50, 55, 70));
                box.setOutlineColor(sf::Color(100, 150, 255));
                box.setOutlineThickness(2.f);
                window.draw(box);
                
                // Draw port name (larger, cleaner font - use even size for clarity)
                sf::Text portText;
                portText.setFont(font);
                portText.setString(graph->ports[portIdx].name);
                portText.setCharacterSize(16);
                portText.setFillColor(sf::Color(220, 230, 255));
                
                // Center text in box (account for text baseline offset)
                sf::FloatRect textBounds = portText.getLocalBounds();
                portText.setPosition(
                    std::floor(currentX + (boxWidth - textBounds.width) / 2.f - textBounds.left),
                    std::floor(startY + (boxHeight - textBounds.height) / 2.f - textBounds.top)
                );
                window.draw(portText);
                
                // Draw arrow to next box (if not last)
                if (current->next != nullptr) {
                    float arrowX = currentX + boxWidth;
                    float arrowY = startY + boxHeight / 2.f;
                    
                    // Horizontal line (lighter grey-blue)
                    sf::RectangleShape arrowLine(sf::Vector2f(spacing - 10.f, 2.f));
                    arrowLine.setPosition(arrowX + 5.f, arrowY - 1.f);
                    arrowLine.setFillColor(sf::Color(140, 160, 200));
                    window.draw(arrowLine);
                    
                    // Arrow head (simple triangle approximation with lines)
                    sf::RectangleShape arrowHead1(sf::Vector2f(10.f, 2.f));
                    arrowHead1.setPosition(arrowX + spacing - 10.f, arrowY - 6.f);
                    arrowHead1.setRotation(45.f);
                    arrowHead1.setFillColor(sf::Color(140, 160, 200));
                    window.draw(arrowHead1);
                    
                    sf::RectangleShape arrowHead2(sf::Vector2f(10.f, 2.f));
                    arrowHead2.setPosition(arrowX + spacing - 10.f, arrowY + 6.f);
                    arrowHead2.setRotation(-45.f);
                    arrowHead2.setFillColor(sf::Color(140, 160, 200));
                    window.draw(arrowHead2);
                }
                
                current = current->next;
                index++;
            }
        } else {
            // No itinerary booked yet
            sf::Text emptyText;
            emptyText.setFont(font);
            emptyText.setString("No route booked yet. Find a route and click 'Book Route' to create an itinerary.");
            emptyText.setCharacterSize(14);
            emptyText.setFillColor(sf::Color(150, 150, 150));
            emptyText.setPosition(20.f, 755.f);
            window.draw(emptyText);
        }
    }

    // Draw company information panel at the bottom
    void drawCompanyInfoPanel() {
        // Draw bottom panel background
        window.draw(bottomPanel);

        // Calculate statistics for the selected company
        int totalRoutes = 0;
        double minCost = 1e18, maxCost = 0, totalCost = 0;
        int minDuration = 999999, maxDuration = 0;
        SimpleVector<int> uniquePortIndices;

        // Iterate through all edges to gather company stats
        for (int i = 0; i < graph->ports.size(); i++) {
            Node<Edge>* edge = graph->adjList[i].head;
            while (edge != nullptr) {
                Route* route = edge->data.routeData;
                bool matches = (selectedCompanyFilter == "ALL") || (route->company == selectedCompanyFilter);

                if (matches) {
                    totalRoutes++;
                    totalCost += route->cost;
                    if (route->cost < minCost) minCost = route->cost;
                    if (route->cost > maxCost) maxCost = route->cost;
                    if (route->durationMinutes < minDuration) minDuration = route->durationMinutes;
                    if (route->durationMinutes > maxDuration) maxDuration = route->durationMinutes;

                    // Track unique origin port
                    bool foundOrigin = false;
                    for (int j = 0; j < uniquePortIndices.size(); j++) {
                        if (uniquePortIndices[j] == i) { foundOrigin = true; break; }
                    }
                    if (!foundOrigin) uniquePortIndices.push_back(i);

                    // Track unique destination port
                    int destIdx = edge->data.destinationPortIndex;
                    bool foundDest = false;
                    for (int j = 0; j < uniquePortIndices.size(); j++) {
                        if (uniquePortIndices[j] == destIdx) { foundDest = true; break; }
                    }
                    if (!foundDest) uniquePortIndices.push_back(destIdx);
                }
                edge = edge->next;
            }
        }

        // Handle edge cases
        if (minCost > 1e17) minCost = 0;
        if (minDuration > 999998) minDuration = 0;
        double avgCost = (totalRoutes > 0) ? totalCost / totalRoutes : 0;

        // Title
        sf::Text title;
        title.setFont(font);
        string titleStr = (selectedCompanyFilter == "ALL") ? "All Companies Overview" : selectedCompanyFilter + " - Route Information";
        title.setString(titleStr);
        title.setCharacterSize(16);
        title.setFillColor(sf::Color(100, 200, 255));
        title.setStyle(sf::Text::Bold);
        title.setPosition(20.f, 710.f);
        window.draw(title);

        // Statistics section (left side)
        float statsX = 20.f;
        float statsY = 738.f;
        float lineSpacing = 18.f;

        auto drawStatLine = [&](const string& label, const string& value, sf::Color valueColor) {
            sf::Text labelText;
            labelText.setFont(font);
            labelText.setString(label);
            labelText.setCharacterSize(13);
            labelText.setFillColor(sf::Color(180, 180, 180));
            labelText.setPosition(statsX, statsY);
            window.draw(labelText);

            sf::Text valueText;
            valueText.setFont(font);
            valueText.setString(value);
            valueText.setCharacterSize(13);
            valueText.setFillColor(valueColor);
            valueText.setStyle(sf::Text::Bold);
            valueText.setPosition(statsX + 110.f, statsY);
            window.draw(valueText);

            statsY += lineSpacing;
            };

        // Format values
        char minCostStr[32], maxCostStr[32], avgCostStr[32];
        snprintf(minCostStr, sizeof(minCostStr), "$%.0f", minCost);
        snprintf(maxCostStr, sizeof(maxCostStr), "$%.0f", maxCost);
        snprintf(avgCostStr, sizeof(avgCostStr), "$%.0f", avgCost);

        string minDurStr = to_string(minDuration / 60) + "h " + to_string(minDuration % 60) + "m";
        string maxDurStr = to_string(maxDuration / 60) + "h " + to_string(maxDuration % 60) + "m";

        drawStatLine("Total Routes:", to_string(totalRoutes), sf::Color(100, 255, 100));
        drawStatLine("Ports Served:", to_string(uniquePortIndices.size()), sf::Color(100, 200, 255));
        drawStatLine("Cost Range:", string(minCostStr) + " - " + string(maxCostStr), sf::Color(255, 200, 100));
        drawStatLine("Avg Cost:", string(avgCostStr), sf::Color(255, 200, 100));
        drawStatLine("Duration:", minDurStr + " - " + maxDurStr, sf::Color(200, 150, 255));

        // Ports section (right side) - show ports served as flowing text
        float portsX = 320.f;
        float portsY = 708.f;

        sf::Text portsTitle;
        portsTitle.setFont(font);
        portsTitle.setString("Ports Served:");
        portsTitle.setCharacterSize(15);
        portsTitle.setFillColor(sf::Color(255, 200, 100));
        portsTitle.setStyle(sf::Text::Bold);
        portsTitle.setPosition(portsX, portsY);
        window.draw(portsTitle);

        // Display ports as flowing comma-separated text (cleaner look, shows all ports)
        float textStartX = portsX;
        float textStartY = portsY + 24.f;
        float maxTextWidth = 1000.f;  // Max width before wrapping
        float lineHeight = 22.f;
        float currentTextX = textStartX;
        float currentTextY = textStartY;
        
        for (int i = 0; i < uniquePortIndices.size(); i++) {
            string portName = graph->ports[uniquePortIndices[i]].name;
            
            // Add comma separator except for first item
            if (i > 0) {
                portName = ",  " + portName;
            }
            
            sf::Text portText;
            portText.setFont(font);
            portText.setString(portName);
            portText.setCharacterSize(14);
            portText.setFillColor(sf::Color(180, 210, 255));  // Light blue - more vibrant
            
            sf::FloatRect textBounds = portText.getLocalBounds();
            
            // Check if we need to wrap to next line
            if (currentTextX + textBounds.width > textStartX + maxTextWidth && i > 0) {
                currentTextX = textStartX;
                currentTextY += lineHeight;
                
                // Remove leading comma for wrapped line
                portName = graph->ports[uniquePortIndices[i]].name;
                portText.setString(portName);
            }
            
            // Stop if we exceed vertical space
            if (currentTextY > 835.f) {
                sf::Text moreText;
                moreText.setFont(font);
                moreText.setString(" ..." + to_string(uniquePortIndices.size() - i) + " more");
                moreText.setCharacterSize(13);
                moreText.setFillColor(sf::Color(120, 120, 120));
                moreText.setPosition(currentTextX, currentTextY);
                window.draw(moreText);
                break;
            }
            
            portText.setPosition(currentTextX, currentTextY);
            window.draw(portText);
            
            currentTextX += portText.getLocalBounds().width + 2.f;
        }

    }
    
    // Draw company routes UI (side panel with company filter buttons)
    void drawCompanyRoutesUI() {
        // Back button
        if (hoverBack) backButton.setFillColor(sf::Color(230, 110, 90));
        else backButton.setFillColor(sf::Color(200, 80, 60));
        window.draw(backButton);
        sf::Text backText;
        backText.setFont(font);
        backText.setString("BACK");
        backText.setCharacterSize(16);
        backText.setFillColor(sf::Color::White);
        backText.setPosition(backButton.getPosition().x + 15.f, backButton.getPosition().y + 8.f);
        window.draw(backText);
        
        // Side panel
        window.draw(sidePanel);
        
        // Title
        sf::Text title;
        title.setFont(font);
        title.setString("Select Company");
        title.setCharacterSize(22);
        title.setFillColor(sf::Color::White);
        title.setStyle(sf::Text::Bold);
        title.setPosition(1065.f, 20.f);
        window.draw(title);
        
        // Draw company filter buttons (toggle theme: blue selected, grey unselected)
        auto drawCompanyButton = [&](sf::RectangleShape& btn, const string& label, bool isSelected) {
            // Update button color based on selection
            btn.setFillColor(isSelected ? sf::Color(60, 120, 200, 100) : sf::Color(40, 40, 48));
            window.draw(btn);
            
            sf::Text btnText;
            btnText.setFont(font);
            btnText.setString(label);
            btnText.setCharacterSize(14);
            // Update text color
            btnText.setFillColor(isSelected ? sf::Color::White : sf::Color(160, 160, 160));
            btnText.setPosition(btn.getPosition().x + 10.f, btn.getPosition().y + 8.f);
            window.draw(btnText);
        };
        
        // All Companies button
        drawCompanyButton(companyButtons[0], "All Companies", selectedCompanyFilter == "ALL");
        
        // Individual company buttons
        for (int i = 0; i < allCompanies.size(); i++) {
            drawCompanyButton(companyButtons[i + 1], allCompanies[i], selectedCompanyFilter == allCompanies[i]);
        }
        
        // Draw port hover tooltip
        if (hoverPortIdx != -1) {
            Port& hoveredPort = graph->ports[hoverPortIdx];
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            float tooltipX = static_cast<float>(mousePos.x) + 15.0f;
            float tooltipY = static_cast<float>(mousePos.y) + 15.0f;

            // Build tooltip content
            string tooltipStr = "Port: " + hoveredPort.name + "\n";
            tooltipStr += "Daily Charge: $" + to_string(hoveredPort.dailyCharge) + "\n";
            tooltipStr += "--- Docking Queues ---\n";
            
            // Count total lines and add company queue info
            int totalLines = 4; // Base lines (Port, Daily Charge, separator, empty line at end)
            bool hasQueues = false;
            for (int i = 0; i < hoveredPort.companyNames.size(); i++) {
                int queueSize = hoveredPort.companyQueues[i].size();
                if (queueSize > 0) {
                    tooltipStr += hoveredPort.companyNames[i] + ": " + to_string(queueSize) + "\n";
                    totalLines++;
                    hasQueues = true;
                }
            }
            
            if (!hasQueues) {
                tooltipStr += "(No ships waiting)\n";
                totalLines++;
            }
            
            // Calculate tooltip size based on content
            float tooltipWidth = 200.0f;
            float lineHeight = 15.0f;
            float tooltipHeight = totalLines * lineHeight + 10.0f;

            // Adjust position to keep tooltip on screen
            if (tooltipX + tooltipWidth > 1050) tooltipX = mousePos.x - tooltipWidth - 15.0f;
            if (tooltipY + tooltipHeight > 700) tooltipY = mousePos.y - tooltipHeight - 15.0f;

            sf::RectangleShape tooltipBg;
            tooltipBg.setSize(sf::Vector2f(tooltipWidth, tooltipHeight));
            tooltipBg.setPosition(tooltipX, tooltipY);
            tooltipBg.setFillColor(sf::Color(40, 40, 40, 230));
            tooltipBg.setOutlineColor(sf::Color::White);
            tooltipBg.setOutlineThickness(1.5f);
            window.draw(tooltipBg);

            sf::Text tooltipText;
            tooltipText.setFont(font);
            tooltipText.setString(tooltipStr);
            tooltipText.setCharacterSize(12);
            tooltipText.setFillColor(sf::Color::White);
            tooltipText.setPosition(tooltipX + 5, tooltipY + 5);
            window.draw(tooltipText);
        }
        
        // Draw legend
        drawLegend(COMPANIES);

        // Draw company info panel at bottom
        drawCompanyInfoPanel();

        window.display();
    }
    
    // Helper method to draw dynamic legend box
    void drawLegend(ScreenState state) {
        // Legend box dimensions and position
        float legendX = 20.f;
        float legendY = 520.f;
        float legendWidth = 220.f;
        float legendHeight = 170.f;
        
        // Semi-transparent dark background
        sf::RectangleShape legendBox;
        legendBox.setSize(sf::Vector2f(legendWidth, legendHeight));
        legendBox.setPosition(legendX, legendY);
        legendBox.setFillColor(sf::Color(0, 0, 0, 180));
        legendBox.setOutlineColor(sf::Color::White);
        legendBox.setOutlineThickness(2.f);
        window.draw(legendBox);
        
        // Title
        sf::Text legendTitle;
        legendTitle.setFont(font);
        legendTitle.setString("MAP LEGEND");
        legendTitle.setCharacterSize(16);
        legendTitle.setFillColor(sf::Color::White);
        legendTitle.setStyle(sf::Text::Bold);
        legendTitle.setPosition(legendX + 10.f, legendY + 10.f);
        window.draw(legendTitle);
        
        float itemY = legendY + 40.f; // Starting Y for legend items
        float itemSpacing = 22.f;      // Space between items
        
        // Helper lambda to draw a legend item with a circle
        auto drawCircleItem = [&](sf::Color color, const string& label) {
            sf::CircleShape circle(6.f);
            circle.setFillColor(color);
            circle.setOutlineColor(sf::Color::White);
            circle.setOutlineThickness(1.f);
            circle.setPosition(legendX + 15.f, itemY);
            window.draw(circle);
            
            sf::Text text;
            text.setFont(font);
            text.setString(label);
            text.setCharacterSize(12);
            text.setFillColor(sf::Color::White);
            text.setPosition(legendX + 35.f, itemY - 2.f);  // Align with circle center
            window.draw(text);
            
            itemY += itemSpacing;
        };
        
        // Helper lambda to draw a legend item with a line
        auto drawLineItem = [&](sf::Color color, const string& label) {
            sf::RectangleShape line(sf::Vector2f(20.f, 3.f));
            line.setFillColor(color);
            line.setPosition(legendX + 12.f, itemY + 6.f);
            window.draw(line);
            
            sf::Text text;
            text.setFont(font);
            text.setString(label);
            text.setCharacterSize(12);
            text.setFillColor(sf::Color::White);
            text.setPosition(legendX + 35.f, itemY - 2.f);  // Align with line center
            window.draw(text);
            
            itemY += itemSpacing;
        };
        
        // Draw legend items based on screen state
        switch (state) {
            case BOOKING:
                drawCircleItem(sf::Color::Green, "Start Port");
                drawCircleItem(sf::Color::Red, "End Port");
                drawCircleItem(sf::Color::Magenta, "Port");
                drawLineItem(sf::Color(230, 130, 30), "Optimal Path");  // Orange
                drawLineItem(sf::Color(100, 200, 255), "Direct Connection");  // Blue
                drawLineItem(sf::Color(128, 128, 128), "Available Route");
                break;
                
            case COMPANIES:
                drawLineItem(sf::Color(100, 200, 255), "Selected Company Route");  // Blue
                drawCircleItem(sf::Color::Magenta, "Port");
                break;
                
            case MULTI_LEG:
                drawCircleItem(sf::Color::Green, "Start Port");
                drawCircleItem(sf::Color(100, 150, 255), "Intermediary Port");
                drawCircleItem(sf::Color::Red, "End Port");
                drawLineItem(sf::Color(230, 130, 30), "Calculated Path");  // Orange
                drawCircleItem(sf::Color::Magenta, "Port");
                break;
                
            default:
                break;
        }
    }
    
    // Draw multi-leg world (map with waypoint markers)
    void drawMultiLegWorld() {
        window.clear(sf::Color::Black);
        window.draw(mapSprite);
        
        // Draw all ports as small magenta dots
        for (int i = 0; i < graph->ports.size(); i++) {
            Port& port = graph->ports[i];
            sf::CircleShape circle(3.0f);
            circle.setPosition(static_cast<float>(port.x) - 3.0f, static_cast<float>(port.y) - 3.0f);
            circle.setFillColor(sf::Color::Magenta);
            window.draw(circle);
        }
        
        // Draw waypoint markers
        for (int i = 0; i < multiLegWaypoints.size(); i++) {
            int portIdx = graph->getIndex(multiLegWaypoints[i]);
            if (portIdx == -1) continue;
            
            Port& port = graph->ports[portIdx];
            
            // Determine color based on position
            sf::Color waypointColor;
            sf::Color borderColor;
            if (i == 0) {
                waypointColor = sf::Color::Green;  // Start
                borderColor = sf::Color(0, 100, 0);  // Dark green border
            } else if (i == multiLegWaypoints.size() - 1) {
                waypointColor = sf::Color::Red;    // End
                borderColor = sf::Color(100, 0, 0);  // Dark red border
            } else {
                waypointColor = sf::Color(100, 150, 255);   // Blue (lighter for visibility)
                borderColor = sf::Color(50, 80, 150);  // Dark blue border
            }
            
            // Draw larger circle for waypoint
            sf::CircleShape marker(8.0f);
            marker.setPosition(static_cast<float>(port.x) - 8.0f, static_cast<float>(port.y) - 8.0f);
            marker.setFillColor(waypointColor);
            marker.setOutlineColor(borderColor);
            marker.setOutlineThickness(2.f);
            window.draw(marker);
            
            // Draw waypoint number (black text for contrast)
            sf::Text numText;
            numText.setFont(font);
            numText.setString(to_string(i + 1));
            numText.setCharacterSize(12);
            numText.setFillColor(sf::Color::Black);
            numText.setStyle(sf::Text::Bold);
            sf::FloatRect numBounds = numText.getLocalBounds();
            // Better centering accounting for text baseline
            numText.setPosition(
                std::floor(static_cast<float>(port.x) - numBounds.width / 2.0f - numBounds.left),
                std::floor(static_cast<float>(port.y) - numBounds.height / 2.0f - numBounds.top - 1.0f)
            );
            window.draw(numText);
        }
        
        // Draw path if calculated
        if (lastMultiLegResult.isValid && lastMultiLegResult.segments.size() > 0) {
            // Draw all segments as connected orange lines
            for (int seg = 0; seg < lastMultiLegResult.segments.size(); seg++) {
                PathResult& segment = lastMultiLegResult.segments[seg];
                
                for (int i = 0; i < segment.path.size() - 1; i++) {
                    int u = segment.path[i];
                    int v = segment.path[i + 1];
                    Port& fromPort = graph->ports[u];
                    Port& toPort = graph->ports[v];
                    
                    for (int offset = -2; offset <= 2; offset++) {
                        sf::VertexArray line(sf::Lines, 2);
                        line[0].position = sf::Vector2f(
                            static_cast<float>(fromPort.x) + offset * 0.3f,
                            static_cast<float>(fromPort.y) + offset * 0.3f
                        );
                        line[0].color = sf::Color(230, 130, 30);
                        line[1].position = sf::Vector2f(
                            static_cast<float>(toPort.x) + offset * 0.3f,
                            static_cast<float>(toPort.y) + offset * 0.3f
                        );
                        line[1].color = sf::Color(230, 130, 30);
                        window.draw(line);
                    }
                }
            }
            
            // Draw all intermediate ports along the path with blue markers and numbering
            // Build a list of all unique ports in order
            SimpleVector<int> allPathPorts;
            for (int seg = 0; seg < lastMultiLegResult.segments.size(); seg++) {
                PathResult& segment = lastMultiLegResult.segments[seg];
                for (int i = 0; i < segment.path.size(); i++) {
                    // Skip if this port is already in the list (avoid duplicates at waypoint junctions)
                    bool alreadyAdded = false;
                    for (int j = 0; j < allPathPorts.size(); j++) {
                        if (allPathPorts[j] == segment.path[i]) {
                            alreadyAdded = true;
                            break;
                        }
                    }
                    if (!alreadyAdded) {
                        allPathPorts.push_back(segment.path[i]);
                    }
                }
            }
            
            // Draw markers for all ports in the path
            for (int i = 0; i < allPathPorts.size(); i++) {
                int portIdx = allPathPorts[i];
                Port& port = graph->ports[portIdx];
                
                // Determine color: green for first, red for last, light blue for intermediate
                sf::Color portColor;
                sf::Color borderColor;
                if (i == 0) {
                    portColor = sf::Color::Green;  // Start
                    borderColor = sf::Color(0, 100, 0);  // Dark green border
                } else if (i == allPathPorts.size() - 1) {
                    portColor = sf::Color::Red;    // End
                    borderColor = sf::Color(100, 0, 0);  // Dark red border
                } else {
                    portColor = sf::Color(100, 150, 255);
                    borderColor = sf::Color(50, 80, 150);  // Dark blue border
                }
                
                // Draw circle marker
                sf::CircleShape marker(8.0f);
                marker.setPosition(static_cast<float>(port.x) - 8.0f, static_cast<float>(port.y) - 8.0f);
                marker.setFillColor(portColor);
                marker.setOutlineColor(borderColor);
                marker.setOutlineThickness(2.f);
                window.draw(marker);
                
                // Draw port number (black text for visibility)
                sf::Text numText;
                numText.setFont(font);
                numText.setString(to_string(i + 1));
                numText.setCharacterSize(12);
                numText.setFillColor(sf::Color::Black);
                numText.setStyle(sf::Text::Bold);
                sf::FloatRect numBounds = numText.getLocalBounds();
                numText.setPosition(
                    std::floor(static_cast<float>(port.x) - numBounds.width / 2.0f - numBounds.left),
                    std::floor(static_cast<float>(port.y) - numBounds.height / 2.0f - numBounds.top - 1.0f)
                );
                window.draw(numText);
            }
        }
        
        // Draw port hover tooltip
        if (hoverPortIdx != -1) {
            Port& hoveredPort = graph->ports[hoverPortIdx];
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            float tooltipX = static_cast<float>(mousePos.x) + 15.0f;
            float tooltipY = static_cast<float>(mousePos.y) + 15.0f;

            // Build tooltip content
            string tooltipStr = "Port: " + hoveredPort.name + "\n";
            tooltipStr += "Daily Charge: $" + to_string(hoveredPort.dailyCharge) + "\n";
            tooltipStr += "--- Docking Queues ---\n";
            
            // Count total lines and add company queue info
            int totalLines = 4; // Base lines (Port, Daily Charge, separator, empty line at end)
            bool hasQueues = false;
            for (int i = 0; i < hoveredPort.companyNames.size(); i++) {
                int queueSize = hoveredPort.companyQueues[i].size();
                if (queueSize > 0) {
                    tooltipStr += hoveredPort.companyNames[i] + ": " + to_string(queueSize) + "\n";
                    totalLines++;
                    hasQueues = true;
                }
            }
            
            if (!hasQueues) {
                tooltipStr += "(No ships waiting)\n";
                totalLines++;
            }
            
            // Calculate tooltip size based on content
            float tooltipWidth = 200.0f;
            float lineHeight = 15.0f;
            float tooltipHeight = totalLines * lineHeight + 10.0f;

            // Adjust position to keep tooltip on screen
            if (tooltipX + tooltipWidth > 1050) tooltipX = mousePos.x - tooltipWidth - 15.0f;
            if (tooltipY + tooltipHeight > 700) tooltipY = mousePos.y - tooltipHeight - 15.0f;

            sf::RectangleShape tooltipBg;
            tooltipBg.setSize(sf::Vector2f(tooltipWidth, tooltipHeight));
            tooltipBg.setPosition(tooltipX, tooltipY);
            tooltipBg.setFillColor(sf::Color(40, 40, 40, 230));
            tooltipBg.setOutlineColor(sf::Color::White);
            tooltipBg.setOutlineThickness(1.5f);
            window.draw(tooltipBg);

            sf::Text tooltipText;
            tooltipText.setFont(font);
            tooltipText.setString(tooltipStr);
            tooltipText.setCharacterSize(12);
            tooltipText.setFillColor(sf::Color::White);
            tooltipText.setPosition(tooltipX + 5, tooltipY + 5);
            window.draw(tooltipText);
        }
    }
    
    // Draw multi-leg UI (side panel with search and waypoint list)
    void drawMultiLegUI() {
        // Back button
        if (hoverBack) backButton.setFillColor(sf::Color(230, 110, 90));
        else backButton.setFillColor(sf::Color(200, 80, 60));
        window.draw(backButton);
        sf::Text backText;
        backText.setFont(font);
        backText.setString("BACK");
        backText.setCharacterSize(16);
        backText.setFillColor(sf::Color::White);
        backText.setPosition(backButton.getPosition().x + 15.f, backButton.getPosition().y + 8.f);
        window.draw(backText);
        
        // Side panel
        window.draw(sidePanel);
        
        // Draw multi-leg search input
        multiLegSearchInput->draw(window);
        
        // Add button
        if (hoverAddWaypoint) {
            addWaypointBtn.setFillColor(sf::Color(50, 180, 80));
            addWaypointBtn.setOutlineColor(sf::Color(100, 200, 120));
        } else {
            addWaypointBtn.setFillColor(sf::Color(40, 150, 60));
            addWaypointBtn.setOutlineColor(sf::Color(80, 180, 100));
        }
        window.draw(addWaypointBtn);
        
        sf::Text addText;
        addText.setFont(font);
        addText.setString("ADD");
        addText.setCharacterSize(12);
        addText.setFillColor(sf::Color::White);
        addText.setStyle(sf::Text::Bold);
        addText.setPosition(addWaypointBtn.getPosition().x + 8.f, addWaypointBtn.getPosition().y + 7.f);
        window.draw(addText);
        
        // Error message (if any) - positioned below input box (label at 15, box at 35, height 28 = ends at 63)
        if (!multiLegErrorMessage.empty()) {
            sf::Text errorText;
            errorText.setFont(font);
            errorText.setString(multiLegErrorMessage);
            errorText.setCharacterSize(12);
            errorText.setFillColor(sf::Color(255, 100, 100));
            errorText.setPosition(1065.f, 68.f);
            window.draw(errorText);
        }
        
        // Waypoint list title
        sf::Text waypointTitle;
        waypointTitle.setFont(font);
        waypointTitle.setString("Waypoints:");
        waypointTitle.setCharacterSize(18);
        waypointTitle.setFillColor(sf::Color::White);
        waypointTitle.setStyle(sf::Text::Bold);
        waypointTitle.setPosition(1065.f, 88.f);
        window.draw(waypointTitle);
        
        // Draw waypoints list
        removeWaypointBtns.clear();
        hoverRemoveWaypoint.clear();
        
        for (int i = 0; i < multiLegWaypoints.size(); i++) {
            float yPos = 120.f + i * 35.f;
            
            // Waypoint text
            sf::Text wpText;
            wpText.setFont(font);
            wpText.setString(to_string(i + 1) + ". " + multiLegWaypoints[i]);
            wpText.setCharacterSize(14);
            wpText.setFillColor(sf::Color::White);
            wpText.setPosition(1070.f, yPos);
            window.draw(wpText);
            
            // Remove button [X]
            sf::RectangleShape removeBtn;
            removeBtn.setSize(sf::Vector2f(25.f, 25.f));
            removeBtn.setPosition(1295.f, yPos - 2.f);
            removeBtn.setFillColor(sf::Color(180, 30, 30));
            removeBtn.setOutlineColor(sf::Color::White);
            removeBtn.setOutlineThickness(1.f);
            
            removeWaypointBtns.push_back(removeBtn);
            hoverRemoveWaypoint.push_back(false);
            
            window.draw(removeBtn);
            
            sf::Text xText;
            xText.setFont(font);
            xText.setString("X");
            xText.setCharacterSize(14);
            xText.setFillColor(sf::Color::White);
            xText.setStyle(sf::Text::Bold);
            xText.setPosition(removeBtn.getPosition().x + 7.f, removeBtn.getPosition().y + 3.f);
            window.draw(xText);
        }
        
        // Calculate journey button
        if (hoverCalcJourney) {
            calcJourneyBtn.setFillColor(sf::Color(50, 180, 80));
            calcJourneyBtn.setOutlineColor(sf::Color(100, 200, 120));
        } else {
            calcJourneyBtn.setFillColor(sf::Color(40, 150, 60));
            calcJourneyBtn.setOutlineColor(sf::Color(80, 180, 100));
        }
        window.draw(calcJourneyBtn);
        
        sf::Text calcText;
        calcText.setFont(font);
        calcText.setString("Calculate Journey");
        calcText.setCharacterSize(18);
        calcText.setFillColor(sf::Color::White);
        calcText.setStyle(sf::Text::Bold);
        sf::FloatRect calcBounds = calcText.getLocalBounds();
        calcText.setPosition(
            calcJourneyBtn.getPosition().x + (calcJourneyBtn.getSize().x - calcBounds.width) / 2.f,
            calcJourneyBtn.getPosition().y + 8.f
        );
        window.draw(calcText);
        // Algorithm label inside the Calculate Journey button
        sf::Text algoText;
        algoText.setFont(font);
        algoText.setString("Dijkstra (Time)");
        algoText.setCharacterSize(12);
        algoText.setFillColor(sf::Color::White);
        algoText.setStyle(sf::Text::Bold);  // Make it bold
        sf::FloatRect algoBounds = algoText.getLocalBounds();
        algoText.setPosition(
            calcJourneyBtn.getPosition().x + (calcJourneyBtn.getSize().x - algoBounds.width) / 2.f,
            calcJourneyBtn.getPosition().y + 30.f
        );
        window.draw(algoText);
        
        // Draw bottom panel with journey details if valid
        window.draw(bottomPanel);
        
        if (lastMultiLegResult.isValid && lastMultiLegResult.segments.size() > 0) {
            // Title
            sf::Text journeyTitle;
            journeyTitle.setFont(font);
            journeyTitle.setString("Multi-Leg Journey:");
            journeyTitle.setCharacterSize(16);
            journeyTitle.setFillColor(sf::Color(255, 200, 100));
            journeyTitle.setStyle(sf::Text::Bold);
            journeyTitle.setPosition(20.f, 710.f);
            window.draw(journeyTitle);
            
            // Left side: Display segment information
            float segX = 20.f;
            float segY = 738.f;
            double totalTime = 0.0;
            
            for (int seg = 0; seg < lastMultiLegResult.segments.size(); seg++) {
                PathResult& segment = lastMultiLegResult.segments[seg];
                
                // Build compact route string: "Leg 1: A -> B -> C"
                string routeStr = "Leg " + to_string(seg + 1) + ": ";
                for (int i = 0; i < segment.path.size(); i++) {
                    routeStr += graph->ports[segment.path[i]].name;
                    if (i < segment.path.size() - 1) routeStr += " -> ";
                }
                
                sf::Text segText;
                segText.setFont(font);
                segText.setString(routeStr);
                segText.setCharacterSize(14);
                segText.setFillColor(sf::Color(180, 210, 255));  // Light blue, more vibrant
                segText.setPosition(segX, segY);
                window.draw(segText);
                
                totalTime += segment.totalCost;
                segY += 20.f;
                
                // If we run out of vertical space, stop
                if (segY > 820.f) break;
            }
            
            // Right side: Display totals in vertical format
            float totalsX = 1065.f;  // Align with side panel
            float totalsY = 738.f;
            
            // Format time
            int totalHours = (int)(totalTime / 60);
            int totalMins = (int)totalTime % 60;
            string timeStr = to_string(totalHours) + "h " + to_string(totalMins) + "m";
            
            // Format cost with 2 decimal places
            char costStr[32];
            snprintf(costStr, sizeof(costStr), "$%.2f", lastMultiLegResult.totalCostWithCharges);
            
            // Draw "Total Time:" label
            sf::Text timeLabel;
            timeLabel.setFont(font);
            timeLabel.setString("Total Time:");
            timeLabel.setCharacterSize(15);
            timeLabel.setFillColor(sf::Color(180, 180, 180));
            timeLabel.setPosition(totalsX, totalsY);
            window.draw(timeLabel);
            
            // Draw time value
            sf::Text timeValue;
            timeValue.setFont(font);
            timeValue.setString(timeStr);
            timeValue.setCharacterSize(15);
            timeValue.setFillColor(sf::Color(200, 150, 255));
            timeValue.setStyle(sf::Text::Bold);
            timeValue.setPosition(totalsX + 95.f, totalsY);
            window.draw(timeValue);
            
            // Draw "Total Cost:" label
            sf::Text costLabel;
            costLabel.setFont(font);
            costLabel.setString("Total Cost:");
            costLabel.setCharacterSize(15);
            costLabel.setFillColor(sf::Color(180, 180, 180));
            costLabel.setPosition(totalsX, totalsY + 22.f);
            window.draw(costLabel);
            
            // Draw cost value
            sf::Text costValue;
            costValue.setFont(font);
            costValue.setString(string(costStr));
            costValue.setCharacterSize(15);
            costValue.setFillColor(sf::Color(255, 200, 100));
            costValue.setStyle(sf::Text::Bold);
            costValue.setPosition(totalsX + 95.f, totalsY + 22.f);
            window.draw(costValue);
            
        } else if (calculateAttempted && !lastMultiLegResult.isValid && multiLegWaypoints.size() >= 2) {
            // Show "Journey impossible" message in bottom panel
            sf::Text impossibleText;
            impossibleText.setFont(font);
            impossibleText.setString("Journey impossible. No valid path found!");
            impossibleText.setCharacterSize(16);
            impossibleText.setFillColor(sf::Color(255, 100, 100));
            impossibleText.setStyle(sf::Text::Bold);
            impossibleText.setPosition(20.f, 740.f);
            window.draw(impossibleText);
        }
        
        // Draw legend
        drawLegend(MULTI_LEG);
        
        window.display();
    }
    
    void drawBookingUI() {
        // Draw back button
        window.draw(backButton);
        sf::Text backText;
        backText.setFont(font);
        backText.setString("BACK");
        backText.setCharacterSize(16);
        backText.setFillColor(sf::Color::White);
        backText.setPosition(backButton.getPosition().x + 15.f, backButton.getPosition().y + 8.f);
        window.draw(backText);
        
        // Apply hover effects to booking UI buttons
        if (hoverBack) backButton.setFillColor(sf::Color(230, 110, 90));
        else backButton.setFillColor(sf::Color(200, 80, 60));
        
        // Modern styled Find Route button (green with blue outline)
        if (hoverFindRoute) {
            findRouteButton.setFillColor(sf::Color(50, 180, 80));
            findRouteButton.setOutlineColor(sf::Color(100, 200, 120));
        } else {
            findRouteButton.setFillColor(sf::Color(40, 150, 60));
            findRouteButton.setOutlineColor(sf::Color(80, 180, 100));
        }
        findRouteButton.setOutlineThickness(2.f);
        
        // Book Route button
        if (hoverBook) {
            bookButton.setFillColor(sf::Color(255, 160, 50));
            bookButton.setOutlineColor(sf::Color(255, 200, 100));
        } else {
            bookButton.setFillColor(sf::Color(230, 130, 30));
            bookButton.setOutlineColor(sf::Color(255, 170, 80));
        }
        bookButton.setOutlineThickness(2.f);
        
        // Side panel background
        window.draw(sidePanel);

        // Helper to draw a labeled button (centered text, modern styling)
        auto drawLabeledButton = [&](sf::RectangleShape& btn, const string& label, float textOffsetY) {
            window.draw(btn);
            sf::Text t;
            t.setFont(font);
            t.setString(label);
            t.setCharacterSize(16);
            t.setStyle(sf::Text::Bold);
            t.setFillColor(sf::Color::White);
            // Center text horizontally and vertically with floor for crisp rendering
            sf::FloatRect textBounds = t.getLocalBounds();
            float textX = std::floor(btn.getPosition().x + (btn.getSize().x - textBounds.width) / 2.f - textBounds.left);
            float textY = std::floor(btn.getPosition().y + (btn.getSize().y - textBounds.height) / 2.f - textBounds.top);
            t.setPosition(textX, textY);
            window.draw(t);
        };

        // Draw new UI components for booking panel
        originPortInput->draw(window);
        destPortInput->draw(window);
        dayInput->draw(window);
        monthInput->draw(window);
        yearInput->draw(window);
        algorithmToggle->draw(window);
        optimizeToggle->draw(window);
        
        drawLabeledButton(findRouteButton, "Find Route", 10.f);
        drawLabeledButton(bookButton, "Book Route", 10.f);

        // Compute stats for display (use getSelectedDay() for day value)
        int displayDay = getSelectedDay();
        double computedCost = 0.0;
        bool costComputed = false;
        long long totalMinutes = -1;
        bool timeComputed = false;
        int hops = 0;

        if (pathFound && lastResult.path.size() >= 1) {
            hops = lastResult.path.size() - 1;

            // Calculate BOTH metrics regardless of mode (same logic as console)
            // 1. Total Cost (routes + layovers)
            if (currentMode == MODE_DIJKSTRA_COST || currentMode == MODE_ASTAR_COST) {
                computedCost = lastResult.totalCost;
                costComputed = true;
            } else {
                // Time mode: calculate total cost with layovers (using weekly cycling)
                double routeCosts = 0.0;
                double layoverCosts = 0.0;
                
                // Start time calculation
                double decemberStartMinutes = 482400.0;
                double currentArrival = decemberStartMinutes + (getSelectedDay() - 1) * 1440.0;
                
                for (int i = 0; i < lastResult.path.size() - 1; i++) {
                    int u = lastResult.path[i];
                    int v = lastResult.path[i + 1];
                    Node<Edge>* en = graph->adjList[u].head;
                    while (en != nullptr) {
                        if (en->data.destinationPortIndex == v) {
                            Route* r = en->data.routeData;
                            routeCosts += en->data.weight;
                            
                            // Parse route times
                            Date depDate = parseDate(r->departureDate);
                            int depTimeMin = timeToMinutes(r->depTime);
                            int arrTimeMin = timeToMinutes(r->arrTime);
                            long long departureDatetime = dateTimeToMinutes(depDate, depTimeMin);
                            long long arrivalDatetime = dateTimeToMinutes(depDate, arrTimeMin);
                            
                            // Handle overnight routes
                            if (arrTimeMin < depTimeMin) {
                                arrivalDatetime += 1440;
                            }
                            
                            // Find next valid departure (weekly cycling)
                            while (departureDatetime < currentArrival) {
                                departureDatetime += 10080;
                                arrivalDatetime += 10080;
                            }
                            
                            // Calculate port charges at intermediate ports
                            if (i > 0 && i < lastResult.path.size() - 1) {
                                double waitMins = departureDatetime - currentArrival;
                                if (waitMins > 720.0) {
                                    double days = std::ceil(waitMins / 1440.0);
                                    layoverCosts += days * graph->ports[u].dailyCharge;
                                }
                            }
                            
                            // Update arrival time
                            currentArrival = (double)arrivalDatetime;
                            break;
                        }
                        en = en->next;
                    }
                }
                computedCost = routeCosts + layoverCosts;
                costComputed = true;
            }
            
            // 2. Total Time (sailing + waits)
            if (currentMode == MODE_DIJKSTRA_TIME || currentMode == MODE_ASTAR_TIME) {
                totalMinutes = static_cast<long long>(std::llround(lastResult.totalCost));
                timeComputed = true;
            } else {
                // Cost mode: calculate total time with waits (using weekly cycling)
                double sailTime = 0.0;
                double waitTime = 0.0;
                
                // Start time calculation
                double decemberStartMinutes = 482400.0;
                double currentArrival = decemberStartMinutes + (getSelectedDay() - 1) * 1440.0;
                
                for (int i = 0; i < lastResult.path.size() - 1; i++) {
                    int u = lastResult.path[i];
                    int v = lastResult.path[i + 1];
                    Node<Edge>* en = graph->adjList[u].head;
                    while (en != nullptr) {
                        if (en->data.destinationPortIndex == v) {
                            Route* r = en->data.routeData;
                            sailTime += r->durationMinutes;
                            
                            // Parse route times
                            Date depDate = parseDate(r->departureDate);
                            int depTimeMin = timeToMinutes(r->depTime);
                            int arrTimeMin = timeToMinutes(r->arrTime);
                            long long departureDatetime = dateTimeToMinutes(depDate, depTimeMin);
                            long long arrivalDatetime = dateTimeToMinutes(depDate, arrTimeMin);
                            
                            // Handle overnight routes
                            if (arrTimeMin < depTimeMin) {
                                arrivalDatetime += 1440;
                            }
                            
                            // Find next valid departure (weekly cycling)
                            while (departureDatetime < currentArrival) {
                                departureDatetime += 10080;
                                arrivalDatetime += 10080;
                            }
                            
                            // Calculate wait time for intermediate ports (not at start)
                            if (i > 0) {
                                double thisWait = departureDatetime - currentArrival;
                                waitTime += thisWait;
                            }
                            
                            // Update arrival time
                            currentArrival = (double)arrivalDatetime;
                            break;
                        }
                        en = en->next;
                    }
                }
                totalMinutes = static_cast<long long>(std::llround(sailTime + waitTime));
                timeComputed = true;
            }
        }

        // Stats section with background box
        // Gap from toggles (ends at 272) to Find Route (290) is 18px
        // Book Route ends at 340+40=380, so stats should start at 380+18=398
        float statX = 1070.f;
        float statY = 400.f;  // Moved down for proper spacing from Book Route
        float statsBoxWidth = 260.f;  // Expanded to align with buttons
        float statsBoxHeight = 130.f;  // Slightly taller for balanced padding
        
        // Draw stats background box
        sf::RectangleShape statsBox;
        statsBox.setSize(sf::Vector2f(statsBoxWidth, statsBoxHeight));
        statsBox.setPosition(statX - 5.f, statY - 5.f);
        statsBox.setFillColor(sf::Color(40, 40, 48));
        statsBox.setOutlineColor(sf::Color(100, 150, 255));  // Blue border
        statsBox.setOutlineThickness(1.f);
        window.draw(statsBox);
        
        auto addLine = [&](const string& label, const string& value, float offsetY) {
            sf::Text t;
            t.setFont(font);
            t.setCharacterSize(14);
            t.setFillColor(sf::Color::White);
            t.setString(label + value);
            t.setPosition(statX, statY + offsetY);
            window.draw(t);
        };

        // Path Found display: show "--" until a search has been run for the current selection
        std::string pfDisplay;
        if (!resultComputed) {
            pfDisplay = "--";
        } else {
            pfDisplay = pathFound ? "Yes" : "No";
        }
        addLine("Path Found: ", pfDisplay, 0.f);

        // Cost display
        std::ostringstream costStr;
        if (!resultComputed) {
            costStr << "--";
        } else if (resultComputed && pathFound && costComputed) {
            costStr << "$" << std::fixed << std::setprecision(2) << computedCost;
        } else {
            costStr << "N/A";
        }
        addLine("Cost: ", costStr.str(), 30.f);

        // Time display
        std::ostringstream timeStr;
        if (!resultComputed) {
            timeStr << "--";
        } else if (resultComputed && pathFound && timeComputed && totalMinutes >= 0) {
            long long hours = totalMinutes / 60;
            long long minutes = totalMinutes % 60;
            timeStr << totalMinutes << " mins (" << hours << "h " << minutes << "m)";
        } else {
            timeStr << "N/A";
        }
        addLine("Time: ", timeStr.str(), 60.f);

        // Hops display
        std::ostringstream hopsStr;
        if (!resultComputed) {
            hopsStr << "--";
        } else if (resultComputed && pathFound) {
            hopsStr << hops;
        } else {
            hopsStr << "N/A";
        }
        addLine("Hops: ", hopsStr.str(), 90.f);

        // Current selection info - centered between stats box bottom and live feed box top
        sf::Text sel;
        sel.setFont(font);
        sel.setCharacterSize(14);  // Slightly smaller for compact layout
        sel.setFillColor(sf::Color(255, 200, 100));
        std::string selStr = "Start: ";
        selStr += (startPortIdx != -1) ? graph->ports[startPortIdx].name : "None";
        selStr += " | End: ";
        selStr += (endPortIdx != -1) ? graph->ports[endPortIdx].name : "None";
        sel.setString(selStr);
        // Center horizontally on the panel (panel is 260 wide starting at statX-5)
        sf::FloatRect selBounds = sel.getLocalBounds();
        float selX = std::floor((statX - 5.f) + (260.f - selBounds.width) / 2.f - selBounds.left);
        sel.setPosition(selX, statY + 135.f);  // Moved up slightly
        window.draw(sel);
        
        // Real-time journey information panel
        if (journeyInfoVisible && pathFound && currentPath.size() >= 2) {
            // Determine which segment to display
            int displaySegment = currentSegment;
            bool showEndPort = false;
            
            if (!isAnimating && currentSegment >= currentPath.size() - 1) {
                // Journey finished, show end port info
                showEndPort = true;
            }
            
            // Only update if segment changed (or if we're now showing end port)
            if (showEndPort) {
                // Force update to show end port
                lastDisplayedSegment = currentPath.size() - 2;  // Use last segment for tracking
            } else if (displaySegment != lastDisplayedSegment && displaySegment >= 0 && displaySegment < currentPath.size() - 1) {
                lastDisplayedSegment = displaySegment;
            }
            
            if (lastDisplayedSegment >= 0 && lastDisplayedSegment < currentPath.size() - 1) {
                // Calculate panel position (below Start/End text)
                float panelX = statX - 5.f;
                float panelY = statY + 160.f;  // Moved up to avoid bottom panel
                float panelWidth = 260.f;
                float panelHeight = 130.f;  // Reduced height to fit above 700 (bottom panel starts at 700)
                
                // Draw background box
                sf::RectangleShape infoBox;
                infoBox.setSize(sf::Vector2f(panelWidth, panelHeight));
                infoBox.setPosition(panelX, panelY);
                infoBox.setFillColor(sf::Color(40, 40, 48));
                infoBox.setOutlineColor(sf::Color(100, 150, 255));  // Blue border
                infoBox.setOutlineThickness(1.f);
                window.draw(infoBox);
                
                // Determine if we're displaying the end port
                if (showEndPort) {
                    // Show end port information
                    int endPortIdx = currentPath[currentPath.size() - 1];
                    Port& endPort = graph->ports[endPortIdx];
                    
                    // Find the route that led to the end port
                    int prevPortIdx = currentPath[currentPath.size() - 2];
                    Route* lastRoute = nullptr;
                    double routeCharge = 0.0;
                    Node<Edge>* edge = graph->adjList[prevPortIdx].head;
                    while (edge != nullptr) {
                        if (edge->data.destinationPortIndex == endPortIdx) {
                            lastRoute = edge->data.routeData;
                            routeCharge = edge->data.weight;
                            break;
                        }
                        edge = edge->next;
                    }
                    
                    // Display information
                    float textX = panelX + 10.f;
                    float textY = panelY + 10.f;
                    float lineHeight = 25.f;
                    
                    auto drawInfoLine = [&](const string& label, const string& value, int lineNum) {
                        sf::Text text;
                        text.setFont(font);
                        text.setCharacterSize(14);
                        text.setFillColor(sf::Color::White);
                        text.setString(label + value);
                        text.setPosition(textX, textY + lineNum * lineHeight);
                        window.draw(text);
                    };
                    
                    drawInfoLine("Port: ", endPort.name, 0);
                    drawInfoLine("Heading To: ", "N/A - End of Journey", 1);
                    
                    std::ostringstream routeCostStr;
                    routeCostStr << "$" << std::fixed << std::setprecision(2) << routeCharge;
                    drawInfoLine("Route Charge: ", routeCostStr.str(), 2);
                    drawInfoLine("Port Wait: ", "N/A", 3);
                    drawInfoLine("Port Charge: ", "N/A", 4);
                    
                } else {
                    // Show intermediate port information
                    int currentPortIdx = currentPath[lastDisplayedSegment];
                    int nextPortIdx = currentPath[lastDisplayedSegment + 1];
                    Port& currentPort = graph->ports[currentPortIdx];
                    Port& nextPort = graph->ports[nextPortIdx];
                    
                    // Find the route edge from current to next port
                    Route* currentRoute = nullptr;
                    double routeCharge = 0.0;
                    Node<Edge>* edge = graph->adjList[currentPortIdx].head;
                    while (edge != nullptr) {
                        if (edge->data.destinationPortIndex == nextPortIdx) {
                            currentRoute = edge->data.routeData;
                            routeCharge = edge->data.weight;
                            break;
                        }
                        edge = edge->next;
                    }
                    
                    // Calculate wait time and port charge at this port
                    double waitMins = 0.0;
                    double portCharge = 0.0;
                    bool isStartPort = (lastDisplayedSegment == 0);
                    
                    if (!isStartPort && currentRoute) {
                        // Calculate actual wait time using weekly cycling
                        double decemberStartMinutes = 482400.0;
                        double currentArrival = decemberStartMinutes + (getSelectedDay() - 1) * 1440.0;
                        
                        // Trace through path to get actual arrival time at current port
                        for (int i = 0; i < lastDisplayedSegment; i++) {
                            int u = currentPath[i];
                            int v = currentPath[i + 1];
                            Node<Edge>* en = graph->adjList[u].head;
                            while (en != nullptr) {
                                if (en->data.destinationPortIndex == v) {
                                    Route* r = en->data.routeData;
                                    Date depDate = parseDate(r->departureDate);
                                    int depTimeMin = timeToMinutes(r->depTime);
                                    int arrTimeMin = timeToMinutes(r->arrTime);
                                    long long departureDatetime = dateTimeToMinutes(depDate, depTimeMin);
                                    long long arrivalDatetime = dateTimeToMinutes(depDate, arrTimeMin);
                                    
                                    if (arrTimeMin < depTimeMin) {
                                        arrivalDatetime += 1440;
                                    }
                                    
                                    // Apply cycling with 1-cycle limit
                                    int cycleCount = 0;
                                    while (departureDatetime < currentArrival && cycleCount < 1) {
                                        departureDatetime += 10080;
                                        arrivalDatetime += 10080;
                                        cycleCount++;
                                    }
                                    
                                    currentArrival = (double)arrivalDatetime;
                                    break;
                                }
                                en = en->next;
                            }
                        }
                        
                        // Now calculate wait for the current segment
                        Date depDate = parseDate(currentRoute->departureDate);
                        int depTimeMin = timeToMinutes(currentRoute->depTime);
                        long long departureDatetime = dateTimeToMinutes(depDate, depTimeMin);
                        
                        // Apply cycling
                        int cycleCount = 0;
                        while (departureDatetime < currentArrival && cycleCount < 1) {
                            departureDatetime += 10080;
                            cycleCount++;
                        }
                        
                        waitMins = departureDatetime - currentArrival;
                        
                        // Calculate port charge if wait > 12 hours
                        if (waitMins > 720.0) {
                            double days = std::ceil(waitMins / 1440.0);
                            portCharge = days * currentPort.dailyCharge;
                        }
                    }
                    
                    // Display information
                    float textX = panelX + 10.f;
                    float textY = panelY + 10.f;
                    float lineHeight = 25.f;
                    
                    auto drawInfoLine = [&](const string& label, const string& value, int lineNum) {
                        sf::Text text;
                        text.setFont(font);
                        text.setCharacterSize(14);
                        text.setFillColor(sf::Color::White);
                        text.setString(label + value);
                        text.setPosition(textX, textY + lineNum * lineHeight);
                        window.draw(text);
                    };
                    
                    // Port Name
                    drawInfoLine("Port: ", currentPort.name, 0);
                    
                    // Heading To
                    drawInfoLine("Heading To: ", nextPort.name, 1);
                    
                    // Route Charge
                    std::ostringstream routeCostStr;
                    routeCostStr << "$" << std::fixed << std::setprecision(2) << routeCharge;
                    drawInfoLine("Route Charge: ", routeCostStr.str(), 2);
                    
                    // Port Wait
                    string waitStr;
                    if (isStartPort) {
                        waitStr = "N/A";
                    } else {
                        long long hours = (long long)(waitMins / 60);
                        long long mins = (long long)waitMins % 60;
                        std::ostringstream oss;
                        oss << hours << "h " << mins << "m";
                        waitStr = oss.str();
                    }
                    drawInfoLine("Port Wait: ", waitStr, 3);
                    
                    // Port Charge
                    string chargeStr;
                    if (isStartPort) {
                        chargeStr = "N/A";
                    } else {
                        std::ostringstream oss;
                        oss << "$" << std::fixed << std::setprecision(2) << portCharge;
                        chargeStr = oss.str();
                    }
                    drawInfoLine("Port Charge: ", chargeStr, 4);
                }
            }
        }

        // Draw tooltip if hovering over a port (only map area)
        if (hoverPortIdx != -1) {
            Port& hoveredPort = graph->ports[hoverPortIdx];
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            float tooltipX = static_cast<float>(mousePos.x) + 15.0f;
            float tooltipY = static_cast<float>(mousePos.y) + 15.0f;

            // Build tooltip content
            string tooltipStr = "Port: " + hoveredPort.name + "\n";
            tooltipStr += "Daily Charge: $" + to_string(hoveredPort.dailyCharge) + "\n";
            tooltipStr += "--- Docking Queues ---\n";
            
            // Count total lines and add company queue info
            int totalLines = 4; // Base lines (Port, Daily Charge, separator, empty line at end)
            bool hasQueues = false;
            for (int i = 0; i < hoveredPort.companyNames.size(); i++) {
                int queueSize = hoveredPort.companyQueues[i].size();
                if (queueSize > 0) {
                    tooltipStr += hoveredPort.companyNames[i] + ": " + to_string(queueSize) + "\n";
                    totalLines++;
                    hasQueues = true;
                }
            }
            
            if (!hasQueues) {
                tooltipStr += "(No ships waiting)\n";
                totalLines++;
            }
            
            // Calculate tooltip size based on content
            float tooltipWidth = 200.0f;
            float lineHeight = 15.0f;
            float tooltipHeight = totalLines * lineHeight + 10.0f;

            // Adjust position to keep tooltip on screen
            if (tooltipX + tooltipWidth > 1050) tooltipX = mousePos.x - tooltipWidth - 15.0f;
            if (tooltipY + tooltipHeight > 700) tooltipY = mousePos.y - tooltipHeight - 15.0f;

            sf::RectangleShape tooltipBg;
            tooltipBg.setSize(sf::Vector2f(tooltipWidth, tooltipHeight));
            tooltipBg.setPosition(tooltipX, tooltipY);
            tooltipBg.setFillColor(sf::Color(40, 40, 40, 230));
            tooltipBg.setOutlineColor(sf::Color::White);
            tooltipBg.setOutlineThickness(1.5f);
            window.draw(tooltipBg);

            sf::Text tooltipText;
            tooltipText.setFont(font);
            tooltipText.setString(tooltipStr);
            tooltipText.setCharacterSize(12);
            tooltipText.setFillColor(sf::Color::White);
            tooltipText.setPosition(tooltipX + 5, tooltipY + 5);
            window.draw(tooltipText);
        }
    }

    // Update ship animation along the path
    void updateAnimation() {
        if (!isAnimating || currentPath.size() < 2) return;

        // Check if we've completed all segments
        if (currentSegment >= currentPath.size() - 1) {
            isAnimating = false;
            return;
        }

        // Get current and next port positions
        Port& fromPort = graph->ports[currentPath[currentSegment]];
        Port& toPort = graph->ports[currentPath[currentSegment + 1]];

        float segmentLength = distance(
            static_cast<float>(fromPort.x), static_cast<float>(fromPort.y),
            static_cast<float>(toPort.x), static_cast<float>(toPort.y)
        );

        // Update progress based on elapsed time
        float elapsed = animationClock.restart().asSeconds();
        float progressDelta = (shipSpeed * elapsed) / segmentLength;
        segmentProgress += progressDelta;

        // Move to next segment if current is complete
        if (segmentProgress >= 1.0f) {
            segmentProgress = 0.0f;
            currentSegment++;
        }
    }

    // Draw the ship during animation
    void drawShip() {
        if (!isAnimating || currentPath.size() < 2 || currentSegment >= currentPath.size() - 1) return;

        Port& fromPort = graph->ports[currentPath[currentSegment]];
        Port& toPort = graph->ports[currentPath[currentSegment + 1]];

        // Linear interpolation for ship position
        float shipX = fromPort.x + segmentProgress * (toPort.x - fromPort.x);
        float shipY = fromPort.y + segmentProgress * (toPort.y - fromPort.y);

        // Draw ship as a small circle
        sf::CircleShape ship(6.0f);
        ship.setFillColor(sf::Color::Yellow);
        ship.setOutlineColor(sf::Color::Black);
        ship.setOutlineThickness(1);
        ship.setPosition(shipX - 6.0f, shipY - 6.0f);
        window.draw(ship);
    }

    // Draw the company routes world: map with filtered routes
    void drawCompanyWorld() {
        window.clear(sf::Color::Black);
        window.draw(mapSprite);
        
        // Draw routes filtered by selected company
        for (int i = 0; i < graph->ports.size(); i++) {
            Node<Edge>* edge = graph->adjList[i].head;
            while (edge != nullptr) {
                Port& p1 = graph->ports[i];
                Port& p2 = graph->ports[edge->data.destinationPortIndex];
                
                // Filter logic
                bool shouldDraw = false;
                sf::Color routeColor;
                float thickness = 1.5f;
                
                if (selectedCompanyFilter == "ALL") {
                    shouldDraw = true;
                    // Use faint blue for "ALL"
                    routeColor = sf::Color(100, 200, 255, 150);  // Blue, more transparent
                    thickness = 1.5f;
                } else if (edge->data.routeData->company == selectedCompanyFilter) {
                    shouldDraw = true;
                    routeColor = sf::Color(100, 200, 255, 255);  // Blue, fully opaque
                    thickness = 3.0f;
                }
                
                if (shouldDraw) {
                    sf::Vertex line[] = {
                        sf::Vertex(sf::Vector2f(static_cast<float>(p1.x), static_cast<float>(p1.y)), routeColor),
                        sf::Vertex(sf::Vector2f(static_cast<float>(p2.x), static_cast<float>(p2.y)), routeColor)
                    };
                    
                    // Draw multiple times for thickness
                    for (int t = 0; t < static_cast<int>(thickness); t++) {
                        window.draw(line, 2, sf::Lines);
                    }
                }
                
                edge = edge->next;
            }
        }
        
        // Draw ports
        for (int i = 0; i < graph->ports.size(); i++) {
            Port& port = graph->ports[i];
            sf::CircleShape circle(3.0f);  // Same size as booking screen
            circle.setPosition(static_cast<float>(port.x) - 3.0f, static_cast<float>(port.y) - 3.0f);
            circle.setFillColor(sf::Color::Magenta);
            window.draw(circle);
        }
    }

    // Draw the entire world: map, routes, and ports
    void drawBookingWorld() {
        // Clear with dark ocean background
        window.clear(sf::Color(20, 20, 30));
        
        // Draw the map background
        window.draw(mapSprite);
        
        // Draw Routes (edges) with smart visibility
        for (int i = 0; i < graph->ports.size(); i++) {
            Port& srcPort = graph->ports[i];
            
            // Iterate through the adjacency list for this port
            Node<Edge>* current = graph->adjList[i].head;
            while (current != nullptr) {
                int destIndex = current->data.destinationPortIndex;
                Port& destPort = graph->ports[destIndex];
                
                // Determine line color based on selection
                sf::Color lineColor;
                if (i == startPortIdx) {
                    // Highlight routes from selected start port in Blue
                    lineColor = sf::Color(100, 200, 255);
                } else {
                    // Default: very faint/transparent white (ghost network)
                    lineColor = sf::Color(200, 200, 200, 30);
                }
                
                // Create a line from source to destination
                sf::VertexArray line(sf::Lines, 2);
                line[0].position = sf::Vector2f(static_cast<float>(srcPort.x), static_cast<float>(srcPort.y));
                line[0].color = lineColor;
                line[1].position = sf::Vector2f(static_cast<float>(destPort.x), static_cast<float>(destPort.y));
                line[1].color = lineColor;
                
                window.draw(line);
                
                current = current->next;
            }
        }

        // Highlight the found path
        if (pathFound && lastResult.path.size() >= 2) {
            for (int i = 0; i < lastResult.path.size() - 1; i++) {
                int u = lastResult.path[i];
                int v = lastResult.path[i + 1];
                Port& fromPort = graph->ports[u];
                Port& toPort = graph->ports[v];

                for (int offset = -2; offset <= 2; offset++) {
                    sf::VertexArray thickLine(sf::Lines, 2);
                    thickLine[0].position = sf::Vector2f(
                        static_cast<float>(fromPort.x) + offset * 0.3f, 
                        static_cast<float>(fromPort.y) + offset * 0.3f
                    );
                    thickLine[0].color = sf::Color(230, 130, 30);
                    thickLine[1].position = sf::Vector2f(
                        static_cast<float>(toPort.x) + offset * 0.3f, 
                        static_cast<float>(toPort.y) + offset * 0.3f
                    );
                    thickLine[1].color = sf::Color(230, 130, 30);
                    window.draw(thickLine);
                }
            }
        }
        
        // Draw the animated path if animating
        if (isAnimating && currentPath.size() >= 2) {
            for (int i = 0; i < currentPath.size() - 1; i++) {
                Port& from = graph->ports[currentPath[i]];
                Port& to = graph->ports[currentPath[i + 1]];
                
                sf::VertexArray pathLine(sf::Lines, 2);
                pathLine[0].position = sf::Vector2f(static_cast<float>(from.x), static_cast<float>(from.y));
                pathLine[0].color = sf::Color(230, 130, 30);  // Orange
                pathLine[1].position = sf::Vector2f(static_cast<float>(to.x), static_cast<float>(to.y));
                pathLine[1].color = sf::Color(230, 130, 30);  // Orange
                window.draw(pathLine);
            }
        }

        // Draw Ports
        for (int i = 0; i < graph->ports.size(); i++) {
            Port& port = graph->ports[i];

            // Draw port circle
            sf::CircleShape portCircle(3.0f);  // Radius of 3
            
            // Color logic based on selection
            if (i == startPortIdx) {
                portCircle.setFillColor(sf::Color::Green);
                portCircle.setRadius(5.0f);  // Slightly larger for selected
            } else if (i == endPortIdx) {
                portCircle.setFillColor(sf::Color::Red);
                portCircle.setRadius(5.0f);
            } else {
                portCircle.setFillColor(sf::Color::Magenta);
            }
            
            // Center the circle on the port coordinates
            float radius = portCircle.getRadius();
            portCircle.setPosition(static_cast<float>(port.x) - radius, static_cast<float>(port.y) - radius);
            
            window.draw(portCircle);
        }

        // Draw ship if animating
        drawShip();

        // Draw UI elements
        drawBookingUI();
        
        // Draw itinerary panel at bottom
        drawBookingItinerary();
        
        // Draw legend
        drawLegend(BOOKING);
        
        window.display();
    }
    
    // Draw menu screen
    void drawMenu() {
        window.clear(sf::Color(0, 20, 53));
        
        // Title
        sf::Text title;
        title.setFont(titleFont);
        title.setString("OCEAN ROUTE NAVIGATOR");
        title.setCharacterSize(48);
        title.setFillColor(sf::Color(100, 200, 255));
        title.setStyle(sf::Text::Bold);
        
        sf::FloatRect titleBounds = title.getLocalBounds();
        float titleX = std::floor((1350.f - titleBounds.width) / 2.f - titleBounds.left);
        float titleY = std::floor(60.f - titleBounds.top);
        title.setPosition(titleX, titleY);
        window.draw(title);
        
        // Draw icon centered between title and buttons
        float iconScaledWidth = iconSprite.getGlobalBounds().width;
        float iconScaledHeight = iconSprite.getGlobalBounds().height;
        float iconX = std::floor((1350.f - iconScaledWidth) / 2.f);
        float iconY = std::floor(120.f + (300.f - iconScaledHeight) / 2.f);
        iconSprite.setPosition(iconX, iconY);
        window.draw(iconSprite);
        
        // Apply hover/selection effects to menu buttons
        // Keyboard selection OR mouse hover triggers highlight
        bool highlightBooking = hoverMenuBooking || selectedMenuIndex == 0;
        bool highlightMultiLeg = hoverMenuMultiLeg || selectedMenuIndex == 1;
        bool highlightCompany = hoverMenuCompany || selectedMenuIndex == 2;
        bool highlightExit = hoverMenuExit || selectedMenuIndex == 3;
        
        menuBtnRouteBooking.setFillColor(highlightBooking ? sf::Color(80, 150, 220) : sf::Color(50, 100, 150));
        menuBtnMultiLeg.setFillColor(highlightMultiLeg ? sf::Color(160, 120, 200) : sf::Color(120, 80, 150));
        menuBtnCompanyRoutes.setFillColor(highlightCompany ? sf::Color(90, 170, 110) : sf::Color(60, 120, 80));
        menuBtnExit.setFillColor(highlightExit ? sf::Color(200, 80, 80) : sf::Color(150, 50, 50));
        
        // Draw menu buttons
        auto drawMenuButton = [&](sf::RectangleShape& btn, const string& label) {
            window.draw(btn);
            sf::Text btnText;
            btnText.setFont(font);
            btnText.setString(label);
            btnText.setCharacterSize(24);
            btnText.setFillColor(sf::Color::White);
            btnText.setStyle(sf::Text::Bold);
            
            sf::FloatRect textBounds = btnText.getLocalBounds();
            float textX = std::floor(btn.getPosition().x + (btn.getSize().x - textBounds.width) / 2.f - textBounds.left);
            float textY = std::floor(btn.getPosition().y + (btn.getSize().y - textBounds.height) / 2.f - textBounds.top);
            btnText.setPosition(textX, textY);
            window.draw(btnText);
        };
        
        drawMenuButton(menuBtnRouteBooking, "Route Booking");
        drawMenuButton(menuBtnMultiLeg, "Multi-leg Routes");
        drawMenuButton(menuBtnCompanyRoutes, "Company Routes");
        drawMenuButton(menuBtnExit, "Exit");
        
        window.display();
    }
    
    // Master draw function
    void draw() {
        if (currentScreen == MENU) {
            drawMenu();
        } else if (currentScreen == BOOKING) {
            drawBookingWorld();
        } else if (currentScreen == COMPANIES) {
            drawCompanyWorld();
            drawCompanyRoutesUI();
        } else if (currentScreen == MULTI_LEG) {
            drawMultiLegWorld();
            drawMultiLegUI();
        }
    }

    // Main SFML event loop
    void run() {
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                }
                handleInput(event);
            }
            
            // Update hover for all screens
            updateHover();
            
            // Update animation only in booking screen
            if (currentScreen == BOOKING) {
                updateAnimation();
            }
            
            draw();
        }
    }
};

#endif