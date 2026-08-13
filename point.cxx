#include "point.h"

Point::Point()
{
    x = 0;
    y = 0;
    std::cout << "Point()构造函数" << std::endl;
}

Point::Point(int a, int b):x(a),y(b)
{
    // this->x = x;
    // this->y = y;
    std::cout << "Point(int a, int b)构造函数" << std::endl;

}

Point::~Point()
{
    std::cout << "~Point()析构函数" << std::endl;
}


void Point::setX(int x)
{
    this->x = x;
}

void Point::setY(int y)
{
    this->y = y;
}
void Point::show()
{
    std::cout << "{x = " << x  << ", y = " << y <<"}"<< std::endl;
}

int Point::getX()
{
    return x;
}

int Point::getY()
{
    return y;
}
