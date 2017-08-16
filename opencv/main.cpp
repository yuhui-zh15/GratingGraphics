#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <cmath>
#include <cstring>
#include <stack>

typedef cv::Vec3b Color;
typedef cv::Mat_<Color> Image;

//Global variable: width, height
int height, width;

//Structure of point: including x, y
struct Point {
    int x, y;
    Point(int x = 0, int y = 0): x(x), y(y) { }
};

//Output image: using imwrite
void Output(const char* filename, Image& image) {
    cv::imwrite(filename, image);
}

//Draw pixel
void DrawPixel(int x, int y, Color& color, Image& image) {
    if (x < 0 || y < 0 || x >= height || y >= width) return;
    image(x, y) = color;
}

//Get pixel color
Color GetPixel(int x, int y, Image& image) {
    if (x < 0 || y < 0 || x >= height || y >= width) return Color(1, 0, 0);
    return image(x, y);
}

//Draw line: integer Bresenham line algorithm
void DrawLine(int x0, int y0, int x1, int y1, Color& color, Image& image) {
    int x, y, dx, dy, e;
    dx = x1 - x0, dy = y1 - y0, e = -dx;
    x = x0, y = y0;
    for (int i = 0; i <= dx; i++) {
        DrawPixel(x, y, color, image);
        x++, e = e + 2 * dy;
        if (e >= 0) { y++; e = e - 2 * dx; }
    }
}

//Circle points: based on symmetric characteristic
void CirclePoints(int x0, int y0, int x, int y, Color color, Image& image) {
    DrawPixel(x + x0, y + y0, color, image); DrawPixel(y + x0, x + y0, color, image);
    DrawPixel(-x + x0, y + y0, color, image); DrawPixel(y + x0, -x + y0, color, image);
    DrawPixel(x + x0, -y + y0, color, image); DrawPixel(-y + x0, x + y0, color, image);
    DrawPixel(-x + x0, -y + y0, color, image); DrawPixel(-y + x0, -x + y0, color, image);
}

//Draw circle: mid-point circle algorithm
void AntiAliasingDrawCircle(int x0, int y0, int r, Color& color, Image& image) {
    int x, y, d;
    float y_original, dy;
    x = 0; y = r;
    CirclePoints(x0, y0, x, y, color, image);
    for (x = 1; x <= y; x++) {
        y_original = sqrt(r * r - x * x);
        y = int(y_original);
        dy = y_original - y;//dy represents the distance between real point and integer point
        CirclePoints(x0, y0, x, y, Color((1.0 - dy) * color[0], (1.0 - dy) * color[1], (1.0 - dy) * color[2]), image);
        CirclePoints(x0, y0, x, y + 1, Color(dy * color[0], dy * color[1], dy * color[2]), image);
    }
}

//Draw circle: improved mid-point circle algorithm without float
void DrawCircle(int x0, int y0, int r, Color& color, Image& image) {
    int x, y, d, dy;
    x = 0; y = r; d = 3 - 2 * r; dy = 0;
    CirclePoints(x0, y0, x, y, color, image);
    while (x <= y) {
        if (d < 0) {
            d += 4 * x + 6;
        }
        else {
            d += 4 * (x - y) + 10;
            y--;
        }
        x++;
		CirclePoints(x0, y0, x, y, color, image);
    }
}

//Fill algorithm: scan line algorithm
void Fill(int x, int y, Color oldcolor, Color newcolor, Image image) {
    int xl, xr, i;
    bool spanNeedFill;
    Point pt;
    std::stack<Point> s;
    pt.x = x; pt.y = y;
    s.push(pt);
    while (!s.empty()) {
        pt = s.top();
        s.pop();
        y = pt.y;
        x = pt.x;
        //Fill right
        while (GetPixel(x, y, image) == oldcolor) {
            DrawPixel(x, y, newcolor, image);
            x++;
        }
        xr = x - 1;
        x = pt.x - 1;
        //Fill left
        while (GetPixel(x, y, image) == oldcolor) {
            DrawPixel(x, y, newcolor, image);
            x--;
        }
        xl = x + 1;
        //Process up
        x = xl;
        y = y + 1;
        while (x <= xr) {
            spanNeedFill = false;
            while (GetPixel(x, y, image) == oldcolor) {
                spanNeedFill = true;
                x++;
            }
            if (spanNeedFill) {
                pt.x = x - 1;
                pt.y = y;
                s.push(pt);
                spanNeedFill = false;
            }
            while (GetPixel(x, y, image) != oldcolor && x <= xr) x++;
        }
        //Process down
        x = xl;
        y = y - 2;
        while (x <= xr) {
            spanNeedFill = false;
            while (GetPixel(x, y, image) == oldcolor) {
                spanNeedFill = true;
                x++;
            }
            if (spanNeedFill) {
                pt.x = x - 1;
                pt.y = y;
                s.push(pt);
                spanNeedFill = false;
            }
            while (GetPixel(x, y, image) != oldcolor && x <= xr) x++;
        }
    }
}

//Array to int
int atoi(char *s) {
    int ret = 0, len = strlen(s);
    for (int i = 0; i < len; i++) {
    ret = ret * 10 + s[i] - '0';
    }
    return ret;
}

int main(int argc, char** argv) { 
    int x0 = 250, y0 = 250, r = 150;
    Color red(255, 0, 0), green(0, 255, 0), blue(0, 0, 255), yellow(255, 255, 0), magenta(255, 0, 255), cyan(0, 255, 255), white(255, 255, 255), black(0, 0, 0);
    
    //If user input 3 arguments, then draw as user's input
    if (argc == 4) {
        x0 = atoi(argv[1]);
        y0 = atoi(argv[2]);
        r = atoi(argv[3]);
    }
    printf("x0 = %d, y0 = %d, r = %d\n", x0, y0, r);

    //Draw ordinary circle -> circle_1.png
    height = 500, width = 500;
    Image image1(height, width, black);
    DrawCircle(x0, y0, r, white, image1);
    Output("circle_1.png", image1);

    //Fill algorithm -> circle_2.png
    height = 500, width = 500;
    Image image2(height, width, black);
    DrawCircle(x0, y0, r, white, image2);
    Fill(x0, y0, black, green, image2);
    Output("circle_2.png", image2);

    //Draw anti-aliasing circle use own algorithm -> circle_3.png
    height = 500, width = 500;
    Image image3(height, width, black);
    AntiAliasingDrawCircle(x0, y0, r, white, image3);
    Output("circle_3.png", image3);

    //Draw anti-aliasing circle use SSAA algorithm -> circle_4.png
    height = 500, width = 500;
    Image image4;
    Image image4_;
    cv::resize(image1, image4_, cv::Size(height * 2, width * 2));
    cv::resize(image4_, image4, cv::Size(height, width));
    Output("circle_4.png", image4);

    //Show circles with various colors use own algorithm -> circle_5.png
    height = 600; width = 600;
    Image image5(height, width, black);
    AntiAliasingDrawCircle(150, 150, 100, red, image5);
    AntiAliasingDrawCircle(150, 300, 100, green, image5);
    AntiAliasingDrawCircle(150, 450, 100, blue, image5);
    AntiAliasingDrawCircle(450, 150, 100, yellow, image5);
    AntiAliasingDrawCircle(450, 300, 100, magenta, image5);
    AntiAliasingDrawCircle(450, 450, 100, cyan, image5);
    AntiAliasingDrawCircle(300, 300, 150, white, image5);
    Output("circle_5.png", image5);

    return 0;
}
