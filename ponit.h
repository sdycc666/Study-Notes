#include <iostream>

class Point
{
    // 成员
private:
    // 属性
    int x;
    int y;

public:
    // 函数，方法
    Point();
    Point(int x,int y);
    ~Point();
    void show();
    void setX(int x);
    void setY(int y);
    int getX();
    int getY();
};