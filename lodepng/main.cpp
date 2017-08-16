#include "lodepng.h"
#include <iostream>
#include <cmath>
#include <stack>

typedef unsigned char uchar;//0-255 store R\G\B\A

//Global variable: width, height, image
unsigned width, height;
std::vector<uchar> image;

//Structure of color: including red, green, blue and alpha(controlling transparency)
struct Color {
    uchar r, g, b, a;
    Color(uchar r = 255, uchar g = 255, uchar b = 255, uchar a = 255): r(r), g(g), b(b), a(a) { }
    Color seta(uchar a_) { return Color(r, g, b, a_); }
    bool operator ==(const Color& next) { return r == next.r && g == next.g && b == next.b && a == next.a; }
    bool operator !=(const Color& next) { return r != next.r || g != next.g || b != next.b || a != next.a; }
};

//Structure of point: including x, y
struct Point {
    int x, y;
    Point(int x = 0, int y = 0): x(x), y(y) { }
};

//Output image: using lodepng library(only in this part use lodepng library)
void Output(const char* filename) {
    unsigned error = lodepng::encode(filename, image, width, height);
    if(error) std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
}

//Initialize variables
void Init(int height_, int width_) {
    height = height_; width = width_;
    image.resize(width * height * 4);
    //Set background color: black
    for (int i = 0; i < width * height * 4; i++) {
        if ((i + 1) % 4) image[i] = 0;
        else image[i] = 255;
    }
}

//Draw pixel
void DrawPixel(int x, int y, Color& color) {
    if (x < 0 || y < 0 || x >= height || y >= width) return;
    image[4 * width * x + 4 * y + 0] = color.r;
    image[4 * width * x + 4 * y + 1] = color.g;
    image[4 * width * x + 4 * y + 2] = color.b;
    image[4 * width * x + 4 * y + 3] = color.a;
}

//Get pixel color
Color GetPixel(int x, int y) {
    int r = 255, g = 255, b = 255, a = 255;
    if (x >= 0 && y >= 0 && x < height && y < width) {
        r = image[4 * width * x + 4 * y + 0];
        g = image[4 * width * x + 4 * y + 1];
        b = image[4 * width * x + 4 * y + 2];
        a = image[4 * width * x + 4 * y + 3];
    }
    return Color(r, g, b, a);
}

//Draw line: integer Bresenham line algorithm
void DrawLine(int x0, int y0, int x1, int y1, Color& color) {
    int x, y, dx, dy, e;
    dx = x1 - x0, dy = y1 - y0, e = -dx;
    x = x0, y = y0;
    for (int i = 0; i <= dx; i++) {
        DrawPixel(x, y, color);
        x++, e = e + 2 * dy;
        if (e >= 0) { y++; e = e - 2 * dx; }
    }
}

//Circle points: based on symmetric characteristic
void CirclePoints(int x0, int y0, int x, int y, Color color) {
    DrawPixel(x + x0, y + y0, color); DrawPixel(y + x0, x + y0, color);
    DrawPixel(-x + x0, y + y0, color); DrawPixel(y + x0, -x + y0, color);
    DrawPixel(x + x0, -y + y0, color); DrawPixel(-y + x0, x + y0, color);
    DrawPixel(-x + x0, -y + y0, color); DrawPixel(-y + x0, -x + y0, color);
}

//Draw circle: mid-point circle algorithm
void AntiAliasingDrawCircle(int x0, int y0, int r, Color& color) {
    int x, y, d;
    float y_original, dy;
    x = 0; y = r;
    CirclePoints(x0, y0, x, y, color);
    for (x = 1; x <= y; x++) {
        y_original = sqrt(r * r - x * x);
        y = int(y_original);
        dy = y_original - y;//dy就是离下面的点的距离，介于0~1之间
        CirclePoints(x0, y0, x, y, Color((1.0 - dy) * color.r, (1.0 - dy) * color.g, (1.0 - dy) * color.b));
        CirclePoints(x0, y0, x, y + 1, Color(dy * color.r, dy * color.g, dy * color.b));
    }
}

//Draw circle: improved mid-point circle algorithm without float
void DrawCircle(int x0, int y0, int r, Color& color) {
    int x, y, d, dy;
    x = 0; y = r; d = 3 - 2 * r; dy = 0;
    CirclePoints(x0, y0, x, y, color);
    while (x <= y) {
        if (d < 0) {
            d += 4 * x + 6;
        }
        else {
            d += 4 * (x - y) + 10;
            y--;
        }
        x++;
		CirclePoints(x0, y0, x, y, color);
    }
}

//Fill algorithm: scan line algorithm
void Fill(int x, int y, Color& oldcolor, Color& newcolor) {
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
        while (GetPixel(x, y) == oldcolor) {
            DrawPixel(x, y, newcolor);
            x++;
        }
        xr = x - 1;
        x = pt.x - 1;
        //Fill left
        while (GetPixel(x, y) == oldcolor) {
            DrawPixel(x, y, newcolor);
            x--;
        }
        xl = x + 1;
        //Process up
        x = xl;
        y = y + 1;
        while (x <= xr) {
            spanNeedFill = false;
            while (GetPixel(x, y) == oldcolor) {
                spanNeedFill = true;
                x++;
            }
            if (spanNeedFill) {
                pt.x = x - 1;
                pt.y = y;
                s.push(pt);
                spanNeedFill = false;
            }
            while (GetPixel(x, y) != oldcolor && x <= xr) x++;
        }
        //Process down
        x = xl;
        y = y - 2;
        while (x <= xr) {
            spanNeedFill = false;
            while (GetPixel(x, y) == oldcolor) {
                spanNeedFill = true;
                x++;
            }
            if (spanNeedFill) {
                pt.x = x - 1;
                pt.y = y;
                s.push(pt);
                spanNeedFill = false;
            }
            while (GetPixel(x, y) != oldcolor && x <= xr) x++;
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
    int size = 500;
    Color red(255, 0, 0), green(0, 255, 0), blue(0, 0, 255), yellow(255, 255, 0), magenta(255, 0, 255), cyan(0, 255, 255), white(255, 255, 255), black(0, 0, 0);
    
    //If user input 3 arguments, then draw as user's input
    if (argc == 4) {
        x0 = atoi(argv[1]);
        y0 = atoi(argv[2]);
        r = atoi(argv[3]);
    }
    printf("x0 = %d, y0 = %d, r = %d\n", x0, y0, r);

    //Draw ordinary circle -> circle_1.png
    Init(size, size);
    DrawCircle(x0, y0, r, white);
    Output("circle_1.png");
    
    //Fill algorithm -> circle_2.png
    Init(size, size);
    DrawCircle(x0, y0, r, white);
    Fill(x0, y0, black, green);
    Output("circle_2.png");
    
    //Draw anti-aliasing circle use own algorithm -> circle_3.png
    Init(size, size);
    AntiAliasingDrawCircle(x0, y0, r, white);
    Output("circle_3.png");

    //Draw anti-aliasing circle use SSAA algorithm -> circle_4.png
    Init(size * 2, size * 2);
    DrawCircle(x0 * 2, y0 * 2, r * 2, white);
    Output("circle_4.png");

    //Show circles with various colors use own algorithm -> circle_5.png
    Init(600, 600);
    AntiAliasingDrawCircle(150, 150, 100, red);
    AntiAliasingDrawCircle(150, 300, 100, green);
    AntiAliasingDrawCircle(150, 450, 100, blue);
    AntiAliasingDrawCircle(450, 150, 100, yellow);
    AntiAliasingDrawCircle(450, 300, 100, magenta);
    AntiAliasingDrawCircle(450, 450, 100, cyan);
    AntiAliasingDrawCircle(300, 300, 150, white);
    Output("circle_5.png");

    return 0;
}
