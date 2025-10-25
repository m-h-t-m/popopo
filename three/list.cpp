#include <iostream>
using namespace std;

class List{
	private:
	    struct Node{
	        int data;
	        Node* next;
	        Node(int d) : data(d), next(nullptr) {}
	    };
	Node* head;    
	public:
	    List() : head(nullptr) {}
	    ~List();
	    
	    void push_front(int value);
	    void print() const;
};
void List::push_front(int value){
    Node* newNode = new Node(value);
    newNode->next = head;
    head = newNode;
}
void List::print() const {
    Node* current = head;
    while (current) {
        cout << current->data <<"->";
        current = current->next;
    }
    cout << "nullptr" << endl;
}
List::~List() {
    Node* current = head;
    while (current) {
        Node *next = current->next;
        delete current;
        current = next;
    }
    head = nullptr;
}
int main() {
    List list;
    list.push_front(3);
    list.push_front(2);
    list.push_front(1);
    list.print();  // 输出: 1 -> 2 -> 3 -> nullptr
    return 0;
}
