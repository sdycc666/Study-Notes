// main.cxx

// #include "point.h"
#include "line.h"


int main(int argc, char const *argv[])
{
    // Point p1;
    // // Point p2(3,4);


    // p1.show();
    // p1.setX(999);
    // p1.setY(888);
    // p1.show();

    Point *p2 = new Point(3,4);
    p2->show();
    delete p2;
    p2 = nullptr;

;


    return 0;
}


