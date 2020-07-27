
#include <vector>
#include <queue>
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

typedef struct Line {
    int sx, sy, ex, ey;
    bool draw;
    Line(): sx(0), sy(0), ex(0), ey(0), draw(false) {}
    Line(Point a, Point b, bool c): sx(a.x), sy(a.y), ex(b.x), ey(b.y), draw(c) {}
    Line(int a, int b, int c, int d, bool e): sx(a), sy(b), ex(c), ey(d), draw(e) {}
} Line;

std::vector< Point > points(0);
std::queue< Line > lines;
bool invalid = false;

EM_JS(void, drawPoint, (int x, int y, std::string color, std::string id), {
    var ctx = document.getElementById(id).getContext("2d");
    ctx.beginPath();
    ctx.arc(x, y, 3, 0, Math.PI * 2, true);
    ctx.fillStyle = color;
    ctx.fill();
});

EM_JS(void, drawLine, (int sx, int sy, int ex, int ey, std::string color, 
    std::string id), {
    var ctx = document.getElementById(id).getContext("2d");
    ctx.beginPath();
    ctx.lineWidth = 1;
    ctx.strokeStyle = color;

    ctx.moveTo(sx, sy);
    ctx.lineTo(ex, ey);

    ctx.stroke();
});

EM_JS(void, animateLine, (int sx, int sy, int ex, int ey), {
    drawLine(sx, sy, ex, ey, "blue", "incrementalConvex");
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

int getDeterminant(int i, int j, int k, std::vector< Point > & tpoints = points) {
    return ( (tpoints[k].x - tpoints[i].x) * (tpoints[j].y - tpoints[i].y) - 
                (tpoints[k].y - tpoints[i].y) * (tpoints[j].x - tpoints[i].x) );
}

double getDistant(Point a, Point b) {
    return pow(a.x - b.x, 2) + pow(a.y - b.y, 2);
}

void sortPointsByX(std::vector< Point > &pts = points) {
    std::sort(pts.begin(), pts.end(),
                [](const Point a, const Point b) { 
                    if (a.x == b.x)
                        return a.y < b.y;
                    return a.x < b.x;
                });
}

void sortPointsByRevX() {
    std::sort(points.begin(), points.end(),
                [](const Point a, const Point b) { return a.x > b.x; });
}

void sortPointsByY() {
    std::sort(points.begin(), points.end(),
                [](const Point a, const Point b) { return a.y < b.y; });
}

std::vector< Point > slowConvex() {
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
            }
        }
    }

    // Return points in clockwise order
    std::vector< Point > convexHull(edges.size());

    for (auto i = 0; i < edges.size(); i++) {
        convexHull[i] = (i == 0) ? edges.begin()->first : edges[ convexHull[i - 1] ];
    }



    return convexHull;
}

std::vector< Point > incrementalConvexHull() {
    if (points.size() < 2)
        return points;

    std::vector< Point > pts = points;
    sortPointsByX(pts);
    
    std::vector< Point > upper;
    upper.push_back(pts[0]);
    upper.push_back(pts[1]);
    lines.push(Line(upper[0], upper[1], true));

    for (unsigned i = 2; i < pts.size(); i++) {
        upper.push_back(pts[i]);
        lines.push(Line(upper[upper.size() - 2], upper.back(), true));
        while (upper.size() > 2) {
            int det = getDeterminant(upper.size() - 3, upper.size() - 2, upper.size() - 1, upper);
            if (det < 0)
                break;
            lines.push(Line(upper[upper.size() - 2], upper.back(), false));
            lines.push(Line(upper[upper.size() - 2], upper[upper.size() - 3], false));
            upper[upper.size() - 2] = upper.back();
            upper.pop_back();
        }
        lines.push(Line(upper[upper.size() - 2], upper.back(), true));
    }

    std::vector< Point > lower;
    lower.push_back(pts[ pts.size() - 1 ]);
    lower.push_back(pts[ pts.size() - 2 ]);

    lines.push(Line(lower[lower.size() - 2], lower.back(), true));
    for (int i = pts.size() - 3; i >= 0; i--) {
        lower.push_back(pts[i]);
        lines.push(Line(lower[lower.size() - 2], lower.back(), true));
        while (lower.size() > 2) {
            int det = getDeterminant(lower.size() - 3, lower.size() - 2, lower.size() - 1, lower);
            if (det < 0)
                break;
            lines.push(Line(lower[lower.size() - 2], lower.back(), false));
            lines.push(Line(lower[lower.size() - 2], lower[lower.size() - 3], false));
            lower[lower.size() - 2] = lower.back();
            lower.pop_back();
        }
        lines.push(Line(lower[lower.size() - 2], lower.back(), true));
    }
    lower.pop_back();
    upper.insert(upper.end(), lower.begin() + 1, lower.end());
    lines.push(Line(upper.back(), upper.front(), true));
    return upper;
}

bool animateIncrementalConvex() {
    if (lines.empty())
        return true;

    if (lines.front().draw) {
        EM_ASM({
            drawLine($0, $1, $2, $3, "blue", "incrementalConvex");
        }, lines.front().sx, lines.front().sy, lines.front().ex, lines.front().ey);
    }
    else {
        EM_ASM({
            clearLine($0, $1, $2, $3);
        }, lines.front().sx, lines.front().sy, lines.front().ex, lines.front().ey);
    }
    lines.pop();
    return false;
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
        EM_ASM({
            imageData = document.getElementById('slowConvex').getContext('2d').getImageData(0, 0, 500, 500);
            drawLine($0, $1, $2, $3, "blue", "slowConvex");
        }, points[i].x, points[i].y, points[j].x, points[j].y);

        invalid = false;
    }

    invalid = getDeterminant(i, j, k) > 0;
    k++;

    if (invalid) {
        EM_ASM(
            document.getElementById('slowConvex').getContext('2d').putImageData(imageData, 0, 0);
        );
    }

    if (invalid || k >= points.size()) {
        k = 0;
        j++;
    }

    return std::vector<int> { i, j, k };
}

void addPoint(int x, int y) {
    points.push_back( Point(x, y) );
}

void randomPoints(int n, int max) {
    max -= 20;
    for (int i = 0; i < n; i++) {
        int x = rand() % max + 10;
        int y = rand() % max + 10;
        addPoint(x, y);
        EM_ASM({drawPoint($0, $1, "red", "slowConvex");}, x, y);
        EM_ASM({drawPoint($0, $1, "red", "incrementalConvex");}, x, y);
    }
}

void clearPoints() {
    points.clear();
}

std::vector< Point > getPoints() {
    return points;
}

void redrawPoints() {
    for (auto p: points) {
        //EM_ASM({drawPoint($0, $1, "red", "slowConvex");}, p.x, p.y);
        EM_ASM({drawPoint($0, $1, "red", "incrementalConvex");}, p.x, p.y);
    }
}

std::vector< std::vector<int> > garbage() {
    return std::vector< std::vector<int> > { std::vector<int> {1, 2, 3},
            std::vector<int>{5, 6, 7} };
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
    emscripten::function("incrementalConvexHull", &incrementalConvexHull);
    emscripten::function("animateIncrementalConvex", &animateIncrementalConvex);
    emscripten::function("redrawPoints", &redrawPoints);
    emscripten::function("sortPointsByX", &sortPointsByX);
    emscripten::function("sortPointsByRevX", &sortPointsByRevX);
    emscripten::function("sortPointsByY", &sortPointsByY);

    emscripten::register_vector<int>("vector<int>");
    emscripten::register_vector<Point>("vector<Point>");
}