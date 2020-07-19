
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <stdlib.h>

#include <thread>
#include <chrono>

#include <emscripten/emscripten.h>
#include <emscripten/bind.h>

typedef struct Point {
    int x;
    int y;

    Point(): x(0), y(0) {}
    Point(int a, int b): x(a), y(b) {}
    bool operator== (const Point & p) const {
        return x == p.x && y == p.y;
    }
} Point;

std::vector< Point > points(0);
bool valid = false;

EM_JS(void, drawPoint, (int x, int y, int r, int g, int b), {
    var ctx = document.getElementById("canvas").getContext("2d");
    ctx.beginPath();
    ctx.arc(x, y, 3, 0, Math.PI * 2, true);
    ctx.fillStyle = 'rgb(' + r + ',' + g + ',' + b +')';
    ctx.fill();
});

EM_JS(void, drawLine, (int sx, int sy, int ex, int ey), {
    var ctx = document.getElementById("canvas").getContext("2d");
    ctx.beginPath();
    ctx.lineWidth = 1;
    ctx.strokeStyle = "blue";

    ctx.moveTo(sx, sy);
    ctx.lineTo(ex, ey);

    ctx.stroke();
});

EM_JS(void, animateLine, (int sx, int sy, int ex, int ey), {
    requestAnimationFrame(drawLine(sx, sy, ex, ey));
});

typedef struct hashFunc {
    std::size_t operator()(const Point & p) const {
        return std::hash<int>()(p.x) ^ std::hash<int>()(p.y);
    }
} hashFunc;

typedef struct Edge {
    Point a;
    Point b;

    Edge(Point p, Point q): a(p), b(q) {}
    bool operator== (const Edge & p) {
        if (a == p.a && b == p.b)
            return true;
        else if (a == p.b && b == p.a)
            return true;
        return false;
    }
} Edge;

void slowConvex() {
    std::unordered_map< Point, Point, hashFunc > edges;
    for (unsigned i = 0; i < points.size(); i++) {
        for (unsigned j = 0; j < points.size(); j++) {
            if (i == j)
                continue;
            bool valid = true;
            for (unsigned k = 0; k < points.size() && valid; k++) {
                if (k == i || k == j)
                    continue;
                int det = ( (points[k].x - points[i].x) * (points[j].y - points[i].y) - 
                            (points[k].y - points[i].y) * (points[j].x - points[i].x) );
                if (det > 0)
                    valid = false;
                if (det == 0) {
                    int dist1 = pow(points[k].x - points[i].x, 2) + pow(points[k].y - points[i].y, 2);
                    int dist2 = pow(points[j].x - points[i].x, 2) + pow(points[j].y - points[i].y, 2);
                    if (dist2 < dist1)
                        valid = false;
                }
            }
            if (valid) {
                edges[ points[i] ] = points[j];
                drawLine(points[i].x, points[i].y, points[j].x, points[j].y);
                //EM_ASM( requestAnimationFrame(function () {drawLine($0, $1, $2, $3);} ), points[i].x, points[i].y, points[j].x, points[j].y);
                //drawLine(points[i].x, points[i].y, points[j].x, points[j].y);
            }
        }
    }
/*
    // Return points in clockwise order
    std::vector< Point > convexHull(edges.size());

    for (auto i = 0; i < edges.size(); i++) {
        convexHull[i] = (i == 0) ? edges.begin()->first : edges[ convexHull[i - 1] ];
    }
    return convexHull;
*/
}

void addPoint(int x, int y) {
    points.push_back( Point(x, y) );
}

void randomPoints(int n) {
    for (int i = 0; i < n; i++) {
        int x = rand() % 480 + 10;
        int y = rand() % 480 + 10;
        addPoint(x, y);
        drawPoint(x, y, 255, 0, 0);
        //EM_ASM( requestAnimationFrame(function () {drawPoint($0, $1, 255, 0, 0);} ), points[i].x, points[i].y);
    }
}

void clearPoints() {
    points.clear();
}

bool getDeterminant(int i, int j, int k) {
    int det = ( (points[k].x - points[i].x) * (points[j].y - points[i].y) - 
                (points[k].y - points[i].y) * (points[j].x - points[i].x) );
    
    return det > 0;
}

std::vector<int> animateSlowConvex(int i, int j, int k) {
    if (j == i)
        j++;
    if (j >= points.size()) {
        j = 0;
        i++;
        return std::vector<int> { i, j, k };
    }

    if (k == 0) {
        EM_ASM(
            imageData = document.getElementById('canvas').getContext('2d').getImageData(0, 0, 500, 500);
        );
        drawLine(points[i].x, points[i].y, points[j].x, points[j].y);
        valid = false;
    }

    valid = getDeterminant(i, j, k);
    k++;
    
    if (valid) {
        EM_ASM(
            document.getElementById('canvas').getContext('2d').putImageData(imageData, 0, 0);
        );
    }

    if (valid || k >= points.size()) {
        k = 0;
        j++;
    }

    return std::vector<int> { i, j, k };
}

std::vector< Point > getPoints() {
    return points;
}

EMSCRIPTEN_BINDINGS(convex) {
    emscripten::value_object< Point > ("Point")
        .field("x", &Point::x)
        .field("y", &Point::y);

    emscripten::function("getDeterminant", &getDeterminant);
    emscripten::function("slowConvex", &slowConvex);
    emscripten::function("addPoint", &addPoint);
    emscripten::function("randomPoints", &randomPoints);
    emscripten::function("clearPoints", &clearPoints);
    emscripten::function("getPoints", &getPoints);
    emscripten::function("animateSlowConvex", &animateSlowConvex);

    emscripten::register_vector<int>("vector<int>");
    emscripten::register_vector<Point>("vector<Point>");
}