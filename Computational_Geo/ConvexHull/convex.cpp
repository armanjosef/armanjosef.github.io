
#include <vector>
#include <unordered_map>
#include <iostream>

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

EM_JS(void, drawPoint, (Point p, Point q), {
    var ctx = document.getElementById("canvas").getContext("2d");
    ctx.beginPath();
    ctx.arc(p.x + 1.5, p.y + 1.5, 3, 0, Math.PI * 2, true);
    ctx.fillStyle = "black";
    ctx.fill();
    ctx.arc(q.x + 1.5, q.y + 1.5, 3, 0, Math.PI * 2, true);
    ctx.fillStyle = "black";
    ctx.fill();
});

EM_JS(void, drawLine, (Point p, Point q), {
    var ctx = document.getElementById("canvas").getContext("2d");
    ctx.beginPath();
    ctx.lineWidth = 1;
    ctx.strokeStyle = "blue";

    ctx.moveTo(p.x, p.y);
    ctx.lineTo(q.x, q.y);

    ctx.stroke();
});

EM_JS(void, displayLog, (), {
    console.log("slowConvex() called");
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

void slowConvex() { //std::vector< Point > & points) {
    std::unordered_map< Point, Point, hashFunc > edges;

std::cout << points.size() << std::endl;
    displayLog();

    for (unsigned i = 0; i < points.size(); i++) {
        std::cout << points[i].x << ", " << points[i].y << std::endl;
        for (unsigned j = 0; j < points.size(); j++) {
            if (i == j)
                continue;
            bool valid = true;
            drawPoint(points[i], points[j]);
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
                drawLine(points[i], points[j]);
            }
        }
    }

    std::vector< Point > convexHull(edges.size());

    for (auto i = 0; i < edges.size(); i++) {
        convexHull[i] = (i == 0) ? edges.begin()->first : edges[ convexHull[i - 1] ];
    }

    for (auto p: convexHull)
        std::cout << "(" << p.x << ", " << p.y << ")" << std::endl;

    //return convexHull;
}

void addPoint(int x, int y) {
    points.push_back( Point(x, y) );
}

EMSCRIPTEN_BINDINGS(convex) {
    emscripten::function("slowConvex", &slowConvex);
    emscripten::function("addPoint", &addPoint);

    emscripten::value_object< Point > ("Point")
        .field("x", &Point::x)
        .field("y", &Point::y);
}