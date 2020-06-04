//
//  main.cpp
//  Road Generator
//
//  Created by Michael Schuff on 6/3/20.
//  Copyright © 2020 Michael Schuff. All rights reserved.
//

#include <iostream>
#include <vector>
#include <cmath>
#include <stdio.h>
#include <SFML/Graphics.hpp>
#include "SplinePoint2D.hpp"
#include "Vector2D.hpp"
#include "MathFunctions.hpp"
#include "Spline2D.hpp"
#include "Hamiltonian.hpp"

using namespace std;
using namespace sf;
using namespace MathFunctions;

double degrees(double);
double radians(double);
RectangleShape VectorLine(Vector2D, Vector2D);
RectangleShape Line(double, double, double, double, Color);
void DrawSpline(RenderWindow&, Spline2D, double);
vector<vector<Vector2D>> GeneratePoints(int, int, int, int, double);

int main() {
    int width = 800;
    int height = 800;
    
    enum State {leftDown, rightDown, up};
    State mouseState = up;
    
    srand(time(NULL));
    int rows = RandomRange((int) 3, (int)  5);
    int columns = RandomRange((int) 3, (int) 5);
    double randomness = 0.5;
    
    double mx, my, controlPointRadius = 5;
    int splinePointIndex = -1, controlPointIndex = -1;
    
    vector<double> multipliers = {1, 5};
    
    
    vector<vector<Vector2D>> pointLocations = GeneratePoints(rows, columns, width, height, randomness);
    vector<vector<Vector2D>> path = Hamiltonian::ManhattanCycleDiagonal(rows, columns);
    
    
    
    vector<SplinePoint2D> controlPoints;
    
    Spline2D spline(controlPoints, true);
    
    
    
    
    RenderWindow window(VideoMode(width, height), "Road Generator");
    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            } else if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape) {
               window.close();
            }

        }
        
        if (Mouse::isButtonPressed(Mouse::Left)) {
            mouseState = leftDown;
        } else if (Mouse::isButtonPressed(Mouse::Right)) {
            mouseState = rightDown;
        } else {
            mouseState = up;
        }
        
        if (mouseState != up) {
            mx = Mouse::getPosition(window).x;
            my = Mouse::getPosition(window).y;
            double minDistance = controlPointRadius;
            for (int i = 0; i < controlPoints.size(); i++) {
                double x = 0, y = 0;
                for (int j = 0; j < controlPoints[i].controlPoints.size(); j++) {
                    x += controlPoints[i].GetX(j);
                    y += controlPoints[i].GetY(j);
                    double distFromPoint = sqrt(pow(mx-x, 2) + pow(my-y, 2));
                    if (distFromPoint < minDistance) {
                        minDistance = distFromPoint;
                        splinePointIndex = i;
                        controlPointIndex = j;
                    }
                }
            }
            if (splinePointIndex != -1) {
                if (mouseState == leftDown) {
                    controlPoints[splinePointIndex].SetPositionKeepLocal(mx, my, controlPointIndex);
                } else if (mouseState == rightDown) {
                    controlPoints[splinePointIndex].SetPosition(mx, my, controlPointIndex);
                }
                spline.SetControlPoints(controlPoints);
            }
        } else {
            splinePointIndex = -1;
            controlPointIndex = -1;
        }
        window.clear();
        DrawSpline(window, spline, controlPointRadius);
        window.display();
    }
}

void DrawSpline(RenderWindow &window, Spline2D spline, double controlPointRadius = 5.0) {
    vector<Color> controlPointColors = {Color(255, 0, 0), Color(255, 255, 0), Color(0, 255, 0), Color(0, 255, 255), Color(0, 0, 255), Color(255, 0, 255)};
    Vector2D last = spline.Get(0), current(0, 0);
    double tInc = 0.001;
    for (double t = tInc; t < 1; t+=tInc) {
        current = spline.Get(t);
        window.draw(VectorLine(last, current));
        last = current;
    }
    
    for (int i = 0; i < spline.controlPoints.size(); i++) {
        double x = spline.controlPoints[i].GetX(0), y = spline.controlPoints[i].GetY(0);
        for (int j = 0; j < spline.controlPoints[i].controlPoints.size() - 1; j++) {
            CircleShape p(controlPointRadius);
            p.setOrigin(controlPointRadius / 2, controlPointRadius / 2);
            p.setFillColor(controlPointColors[j % controlPointColors.size()]);
            p.setPosition(x, y);
            window.draw(Line(x, y, x+spline.controlPoints[i].GetX(j+1), y+spline.controlPoints[i].GetY(j+1), Color(100, 100, 100)));
            window.draw(p);
            x += spline.controlPoints[i].GetX(j+1);
            y += spline.controlPoints[i].GetY(j+1);
            
        }
        CircleShape p(controlPointRadius);
        p.setOrigin(controlPointRadius / 2, controlPointRadius / 2);
        p.setFillColor(controlPointColors[spline.controlPoints[i].controlPoints.size() % controlPointColors.size()]);
        p.setPosition(x, y);
        window.draw(p);
    }
}

RectangleShape VectorLine(Vector2D v1, Vector2D v2) {
    return Line(v1.x, v1.y, v2.x, v2.y, Color(255, 255, 255));
}

RectangleShape Line(double x1, double y1, double x2, double y2, Color color) {
    double x = x2 - x1;
    double y = y2 - y1;
    RectangleShape line(Vector2f(sqrt(x*x+y*y),1));
    line.setOrigin(0.5, 0.5);
    line.rotate(degrees(atan2(y,x)));
    line.setPosition(x1, y1);
    line.setFillColor(color);
    return line;
}

double radians(double deg) {
    return deg * 3.1415626535 / 180;
}

double degrees(double rad) {
    return rad * 180 / 3.1415626535;
}

vector<vector<Vector2D>> GeneratePoints(int rows, int columns, int width, int height, double randomness) {
    double pxlPerRow = (double) height / rows;
    double pxlPerColumn = (double) width / columns;
    double maxRandomnessY = randomness * pxlPerRow;
    double maxRandomnessX = randomness * pxlPerColumn;
    
    vector<vector<Vector2D>> points;
    
    for (double y = 0.5; y < rows; y++) {
        points.push_back(vector<Vector2D> {});
        for (double x = 0.5; x < columns; x++) {
            double dx = RandomRange(-maxRandomnessX, maxRandomnessX);
            double dy = RandomRange(-maxRandomnessY, maxRandomnessY);
            points[(int) y].push_back(Vector2D((int) x * pxlPerColumn + dx, (int) x * pxlPerColumn + dy));
        }
    }
    
    return points;
}