#include <ncurses.h>
#include <vector>
#include <cmath>
#include <chrono>
#include <thread>
#include <algorithm>

struct Vector3D {
    float x, y, z;
    Vector3D(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
};

struct Point2D {
    int x, y;
    char c;
    float depth;
    Point2D(int x = 0, int y = 0, char c = '#', float d = 0) : x(x), y(y), c(c), depth(d) {}
};

class Engine3D {
private:
    int width, height;
    std::vector<Vector3D> points;
    Vector3D camera{0, 0, -10};
    float angleX = 0, angleY = 0;

    float getDepth(const Vector3D& p) {
        return std::abs(p.z - camera.z);
    }

    char getLineChar(float depth) {
        const float maxDist = 30.0f;
        float intensity = 1.0f - (depth / maxDist);
        if (intensity < 0.2f) return '.';
        if (intensity < 0.4f) return '+';
        if (intensity < 0.6f) return '*';
        if (intensity < 0.8f) return '#';
        return '=';
    }

    Vector3D rotatePoint(const Vector3D& p, float ax, float ay) {
        float sx = std::sin(ax), cx = std::cos(ax);
        float sy = std::sin(ay), cy = std::cos(ay);
        
        Vector3D r = p;
        float tempX = r.x * cy + r.z * sy;
        r.z = -r.x * sy + r.z * cy;
        r.x = tempX;
        
        float tempY = r.y * cx - r.z * sx;
        r.z = r.y * sx + r.z * cx;
        r.y = tempY;
        
        return r;
    }

    Point2D project(const Vector3D& p) {
        const float VIEW_DISTANCE = 100;
        float scale = VIEW_DISTANCE / (p.z + VIEW_DISTANCE);
        int x = static_cast<int>(p.x * scale * 2 + width / 2);
        int y = static_cast<int>(p.y * scale + height / 2);
        float depth = getDepth(p);
        
        if (x >= 0 && x < width && y >= 0 && y < height) {
            return Point2D(x, y, getLineChar(depth), depth);
        }
        return Point2D(-1, -1);
    }

public:
    Engine3D() {
        initscr();
        noecho();
        curs_set(0);
        getmaxyx(stdscr, height, width);
        timeout(0);

        float size = 5.0f;
        float step = 0.3f;
        for (float x = -size; x <= size; x += step) {
            for (float y = -size; y <= size; y += step) {
                points.push_back(Vector3D(x, y, size));
                points.push_back(Vector3D(x, y, -size));
                points.push_back(Vector3D(x, size, y));
                points.push_back(Vector3D(x, -size, y));
                points.push_back(Vector3D(size, x, y));
                points.push_back(Vector3D(-size, x, y));
            }
        }
    }

    ~Engine3D() {
        endwin();
    }

    void render() {
        clear();
        std::vector<Point2D> projectedPoints;

        for (const auto& point : points) {
            Vector3D rotated = rotatePoint(point, angleX, angleY);
            Point2D projected = project(rotated);
            if (projected.x != -1) {
                projectedPoints.push_back(projected);
            }
        }

        std::sort(projectedPoints.begin(), projectedPoints.end(),
                 [](const Point2D& a, const Point2D& b) { return a.depth > b.depth; });

        for (const auto& p : projectedPoints) {
            mvaddch(p.y, p.x, p.c);
        }
        refresh();
    }

    void run() {
        while (true) {
            if (getch() == 'q') break;
            angleX += 0.03f;
            angleY += 0.02f;
            render();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
};

int main() {
    Engine3D engine;
    engine.run();
    return 0;
}