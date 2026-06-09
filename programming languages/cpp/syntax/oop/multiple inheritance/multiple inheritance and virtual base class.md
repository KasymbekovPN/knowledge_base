---
tags:
  - programming-language
  - cpp
  - syntax
  - oop
  - class
  - multiple-inheritance
  - vritual
---
[[__cpp syntax oop inheritance multiple__|<==]]

Одной из форм двойственности при наследовании может быть наследование от нескольких классов, которые косвенно или напрямую наследуются от одного и того же класса.

```cpp
#include <iostream>

class Person {

private:
    std::string name;

public:
    Person(std::string name);
    ~Person();
    void print() const;
};

Person::Person(std::string name): name{name} {
    std::cout << "Person '" << name << "' created" << std::endl;
}

Person::~Person() {
    std::cout << "Person '" << name << "' deleted" << std::endl;
}

void Person::print() const {
    std::cout << "{name: " << name << "}" << std::endl;
}

class Student: public Person {

public:
    Student(std::string name):
	    Person{name} {}
};

class Employee: public Person {

public:
    Employee(std::string name): Person{name} {}
};

class StudentEmployee: public Student, public Employee {

public:
    StudentEmployee(std::string name):
	    Student{name},
	    Employee{name} {}
};

class StudentV: public virtual Person {

public:
    StudentV(std::string name): Person{name} {}
};

class EmployeeV: public virtual Person {

public:
    EmployeeV(std::string name): Person{name} {}
};

class StudentEmployeeV: public StudentV, public EmployeeV {

public:
    StudentEmployeeV(std::string name):
	    Person{name},
	    StudentV{name},
	    EmployeeV{name} {}
};

int main(int argc, char const *argv[]) {
    std::cout << "### First part ###" << std::endl;
    {
        StudentEmployee se{"Bob"};
        // se.print(); // <= Error
    }

    std::cout << "### Second part ###" << std::endl;
    {
        StudentEmployeeV sev{"Tom"};
        sev.print();
    }

    return 0;
}
```

```
### First part ###
Person 'Bob' created
Person 'Bob' created
Person 'Bob' deleted
Person 'Bob' deleted
### Second part ###
Person 'Tom' created
{name: Tom}
Person 'Tom' deleted
```

```
.\virtual_base_class.cpp:66:12: error: non-static member 'print' found in multiple base-class subobjects of type 'Person':
    class StudentEmployee -> Student -> Person
    class StudentEmployee -> Employee -> Person
   66 |         se.print(); // <= Error
      |            ^
.\virtual_base_class.cpp:22:14: note: member found by ambiguous name lookup
   22 | void Person::print() const {
      |              ^
```

Если промежуточные классы наследуются ни как виртуальные, то экземпляр конечного класса имеет два набора от _Person_, что при водит к неоднозначности.

Базовый класс помечается виртуальным при помощи ключевого слова _virtual_.

```cpp
class Name: public virtual BaseName {}
```

---
[Множественное наследование](https://metanit.com/cpp/tutorial/5.24.php)