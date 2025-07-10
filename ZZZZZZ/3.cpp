#include <iostream>
#include <string>
#include <nlohmann/json.hpp>  // 这是唯一需要的 JSON 库头文件

using json = nlohmann::json;
using namespace std;

// 定义结构体
struct Person {
    string name;
    int age;
};

// 声明 JSON 自动转换宏（非侵入式）
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Person, name, age)

int main() {
    Person p{"Tom", 28};

    // 转成 JSON
    json j = p;
    cout << j.dump(4) << endl;

    // 从 JSON 还原
    Person p2 = j.get<Person>();
    cout << "Name: " << p2.name << ", Age: " << p2.age << endl;

    return 0;
}
