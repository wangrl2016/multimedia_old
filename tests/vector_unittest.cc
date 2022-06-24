//
// Created by WangRuiLing on 2022/6/24.
//

// 对std::vector进行测试

#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <gtest/gtest.h>
#include <glog/logging.h>

namespace mm {
    class Student {
    public:
        Student(std::string name, bool male, int age) :
                name_(std::move(name)), male_(male), age_(age) {
            std::cout << name_ << "-" << male << "-" << age << std::endl;
        }

        Student(const Student& student) {
            std::cout << __FUNCTION__ << std::endl;
            name_ = student.name_;
            male_ = student.male_;
            age_ = student.age_;
        };

        Student& operator=(Student&) = delete;

        ~Student() {
            std::cout << "~Student" << std::endl;
        }

    private:
        std::string name_;
        bool male_;
        int age_;
    };

    class ClassRoom {
    public:
        ClassRoom() = default;

        ClassRoom(std::vector<std::shared_ptr<Student>> students, int level) : level_(level) {
            students_ = std::move(students);
        }

        Student CreateStudent() {
            Student s("bye", false, 17);
            // 没有多余的构造
            return s;
        }

        std::vector<std::shared_ptr<Student>> students() { return students_; }

    private:
        std::vector<std::shared_ptr<Student>> students_;
        int level_;
    };

    TEST(TEST, Construct) {
//        Student s1("hello", false, 10);
//        Student s2("world", true, 11);
//
//        std::vector<std::shared_ptr<Student>> students1;
//
//        // 避免重复构建
//        students1.reserve(100);
//        students1.emplace_back(std::make_shared<Student>("!", true, 12));
//        students1.emplace_back(std::make_shared<Student>("ok", false, 13));
//        students1.emplace_back(std::make_shared<Student>("how", true, 14));


        // ClassRoom room1(students1, 7);

//        std::vector<Student> students1;
//        students1.emplace_back(s1);
//        students1.emplace_back(s2);

        ClassRoom room3;
        // 只进行一次构造
        // 声明Student类
        Student s = room3.CreateStudent();

        // 没有进行重新构建
//        ClassRoom room1(std::move(students1), 7);

        // std::vector<Student>& students2 = room1.students();

        // ClassRoom room2 = room1;

        // Student s3 = students2[0];

        // students1.clear();
    }
}