#include <iostream>
#include <string>  // 必须包含 string 头文件
using namespace std;

class mn {
private:
    string m;
    int n;        // 改名避免与类名冲突，且便于访问
public:
    string mo;

    // 构造函数用于初始化 private 成员
    mn(const string& init_m, int init_n, const string& init_mo) 
        : m(init_m), n(init_n), mo(init_mo) {}

    // const 成员函数正确写法
    void mp() const {
        cout << m << " " << n << " " << mo << endl;  // 输出 n 的值，不是字符串 "mn"
    }

    // 提供 setter 或保持构造函数初始化
};

int main() {
    mn po("popo", 90, "popoi");  // 通过构造函数初始化
    auto pl = po;                // 拷贝对象
    pl.mp();                     // 调用 const 函数

    return 0;
}
