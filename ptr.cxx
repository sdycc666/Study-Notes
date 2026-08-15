// c++智能指针的使用

#include <iostream>
#include <memory>
#include <string>



using namespace std;


struct Node
{
    // 成员属性默认公开
    int val;                    // 数据域
    weak_ptr <Node> next;     // 智能指针。特殊的共享指针，计数器不加一

    Node(int v):val(v)
    {
        cout << "Node()" << endl;
    }
    ~Node()
    {
        cout << "~Node()" << endl;
    }
};


int main(int argc, char const *argv[])
{


    // RAII
    auto n1 = make_shared<Node>(5);     // 创建共享指针 C++14
    shared_ptr<Node> n2(new Node(9));   // 创建共享指针 C++11
    // cout << n1.use_count() << endl;
    // cout << n2.use_count() << endl;

    // n1->next = n2;  // 弱指针
    // n2->next = n1;  // 弱指针
    // cout << n1.use_count() << endl;
    // cout << n2.use_count() << endl;

    weak_ptr<Node> n3 = n1;
    // cout << n3->val << endl;     // 弱指针不能解引用
    cout << n1->val << endl;
    cout << n3.lock()->val << endl;     // weak.lock()


    
    return 0;
}
