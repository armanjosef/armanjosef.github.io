
#include <vector>
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

EM_JS(void, drawPoint, (int x, int y), {
    var ctx = document.getElementById("canvas").getContext("2d");
    ctx.beginPath();
    ctx.arc(x, y, 3, 0, Math.PI * 2, true);
    ctx.fillStyle = "red";
    ctx.fill();
});

EM_JS(void, drawLine, (int sx, int sy, int ex, int ey), {
    var ctx = document.getElementById("canvas").getContext("2d");
    console.log("asdf");
    ctx.beginPath();
    ctx.lineWidth = 1;
    ctx.strokeStyle = "black";

    ctx.moveTo(sx, sy);
    ctx.lineTo(ex, ey);

    ctx.stroke();
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
                if (det < 0)
                    valid = false;
                if (det == 0) {
                    int dist1 = pow(points[k].x - points[i].x, 2) + pow(points[k].y - points[i].y, 2);
                    int dist2 = pow(points[j].x - points[i].x, 2) + pow(points[j].y - points[i].y, 2);
                    if (dist2 <= dist1)
                        valid = false;
                }
            }
            if (valid) {
                edges[ points[i] ] = points[j];
                drawLine(points[i].x, points[i].y, points[j].x, points[j].y);
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
        drawPoint(x, y);
        addPoint(x, y);
    }
}

void clearPoints() {
    points.clear();
}

EMSCRIPTEN_BINDINGS(convex) {
    emscripten::function("slowConvex", &slowConvex);
    emscripten::function("addPoint", &addPoint);
    emscripten::function("randomPoints", &randomPoints);
    emscripten::function("clearPoints", &clearPoints);

    emscripten::value_object< Point > ("Point")
        .field("x", &Point::x)
        .field("y", &Point::y);
}