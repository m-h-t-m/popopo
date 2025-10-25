#include <iostream>
#include <string>  // 用于测试 string
using namespace std;

// 模板类定义
template <typename T>
class List {
private:
    // 节点定义也变成模板
    struct Node {
        T data;
        Node* next;
        Node(const T& d) : data(d), next(nullptr) {}
    };

    Node* head;

public:
    List() : head(nullptr) {}
    ~List();

    void push_front(const T& value);  // 接受 const 引用，提高效率
    void print() const;
};

// 模板成员函数必须在头文件或同一文件中定义（不能分离成 .cpp）

// push_front 模板函数
template <typename T>
void List<T>::push_front(const T& value) {
    Node* newNode = new Node(value);
    newNode->next = head;
    head = newNode;
}

// print 模板函数
template <typename T>
void List<T>::print() const {
    Node* current = head;
    while (current) {
        cout << current->data << " -> ";
        current = current->next;
    }
    cout << "nullptr" << endl;
}

// 析构函数模板
template <typename T>
List<T>::~List() {
    Node* current = head;
    while (current) {
        Node* next = current->next;
        delete current;
        current = next;
    }
    head = nullptr;
}

// ==================== 测试 main 函数 ====================
int main() {
    // 测试 1: int 类型
    cout << "List<int>:" << endl;
    List<int> intList;
    intList.push_front(3);
    intList.push_front(2);
    intList.push_front(1);
    intList.print();  // 输出: 1 -> 2 -> 3 -> nullptr

    // 测试 2: double 类型
    cout << "\nList<double>:" << endl;
    List<double> doubleList;
    doubleList.push_front(3.14);
    doubleList.push_front(2.71);
    doubleList.push_front(1.0);
    doubleList.print();  // 输出: 1 -> 2.71 -> 3.14 -> nullptr

    // 测试 3: string 类型
    cout << "\nList<string>:" << endl;
    List<string> stringList;
    stringList.push_front("World");
    stringList.push_front("Hello");
    stringList.push_front("Hi");
    stringList.print();  // 输出: Hi -> Hello -> World -> nullptr

    return 0;
}
