---
tags:
  - programming-language
  - cpp
  - syntax
  - exception
---
[[_cpp syntax - exception|<=]]

При возникновении исключения обработчики _catch_ проверяются в той последовательности, в которой они определены в коде. И если будет найден первый блок _catch_, параметр которого соответствует типу исключения, то он выбирается для обработки исключения. 

Для исключений, которые являются базовыми типами (а не типами классов), необходимо точное совпадение типа исключения с типом параметра в блоке _catch_. А для исключений-объектов классов при сопоставлении могут применяться неявные преобразования. В этом случае обработчик catch выбирается, если:

- Параметр в _catch_ имеет тот же самый тип, что и исключение (`const` игнорируется)
- Тип параметра в _catch_ представляет базовый класс для типа исключения или ссылку на базовый класс (`const` игнорируется)
- Исключение и параметр в _catch_ представляют указатели, соответственно объект исключения может быть неявно преобразован к типу параметра (`const` игнорируется)

Поскольку исключения производных классов неявно преобразуются в тип базового класса, то мы можем перехватывать все исключения, которые представляют базовый и производный типы, с помощью одного обработчика _catch_.

```cpp
#include <iostream>
#include <string>

class AgeException {

private:
    std::string message;

public:
    explicit AgeException(std::string) noexcept;
    virtual std::string getMessage() const noexcept;
};

AgeException::AgeException(std::string message) noexcept:
    message{message} {}

std::string AgeException::getMessage() const noexcept {
    return message;
}

class MaxAgeException: public AgeException {

private:
    unsigned maxAge;

public:
    explicit MaxAgeException(std::string, unsigned);
    std::string getMessage() const noexcept override;
};

MaxAgeException::MaxAgeException(std::string message, unsigned maxAge):
    AgeException{message},
    maxAge{maxAge} {}

std::string MaxAgeException::getMessage() const noexcept {
    return
	    AgeException::getMessage() +
	    " -- max age should be " +
	    std::to_string(maxAge);
}

class Person {

private:
    static const unsigned MAX_AGE {110};
    std::string name;
    unsigned age;

public:
    Person(std::string, unsigned);
    void print() const noexcept;
};

Person::Person(std::string name, unsigned age): name{name} {
    if (!age) {
        throw AgeException{"Invalid age"};
    }
    if (age > MAX_AGE) {
        throw MaxAgeException{"Invalid age", MAX_AGE};
    }
    this->age = age;
}

void Person::print() const noexcept {
    std::cout
        << "{name: " << name
        << ", age: " << age
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    try {
        Person tom {"Tom", 1234};
        tom.print();

    } catch(const AgeException& e) {
        std::cerr << e.getMessage() << '\n';
    }

    return 0;
}
```

```
Invalid age -- max age should be 110
```

Иногда может потребоваться выполнить раздельную обработку исключений базовых и производных классов, особенно когда необходимо вызвать какие-нибудь функции производных классов, которых нет в базовых. Поскольку объекты исключений могут сопоставляться с параметрами базовых классов в блоке _catch_, то обработку производных классов надо размещать перед обработкой базовых классов. 

```cpp
#include <iostream>
#include <string>

class AgeException {

private:
    std::string message;

public:
    explicit AgeException(std::string) noexcept;
    virtual std::string getMessage() const noexcept;
};

AgeException::AgeException(std::string message) noexcept:
    message{message} {}

std::string AgeException::getMessage() const noexcept {
    return message;
}

class MaxAgeException: public AgeException {

private:
    unsigned maxAge;

public:
    explicit MaxAgeException(std::string, unsigned);
    std::string getMessage() const noexcept override;
};

MaxAgeException::MaxAgeException(std::string message, unsigned maxAge):
    AgeException{message},
    maxAge{maxAge} {}

std::string MaxAgeException::getMessage() const noexcept {
    return
	    AgeException::getMessage() +
	    " -- max age should be " +
	    std::to_string(maxAge);
}

class Person {

private:
    static const unsigned MAX_AGE {110};
    std::string name;
    unsigned age;

public:
    Person(std::string, unsigned);
    void print() const noexcept;
};

Person::Person(std::string name, unsigned age): name{name} {
    if (!age) {
        throw AgeException{"Invalid age"};
    }
    if (age > MAX_AGE) {
        throw MaxAgeException{"Invalid age", MAX_AGE};
    }
    this->age = age;
}

void Person::print() const noexcept {
    std::cout
        << "{name: " << name
        << ", age: " << age
        << "}" << std::endl;
}

void test(std::string, unsigned);

int main(int argc, char const *argv[]) {
    ::test("Tom", 42);
    ::test("Bob", 0);
    ::test("Paul", 1234);

    return 0;
}

void test(std::string name, unsigned age) {
    try {
        Person person {name, age};
        person.print();

    } catch(const MaxAgeException& e) {
        std::cerr << "MaxAgeException <= " << e.getMessage() << std::endl;
    } catch(const AgeException& e) {
        std::cerr << "AgeException <= " << e.getMessage() << std::endl;
    }
}
```

```
{name: Tom, age: 42}
AgeException <= Invalid age
MaxAgeException <= Invalid age -- max age should be 110
```

---
[Создание своих типов исключений](https://metanit.com/cpp/tutorial/6.5.php)