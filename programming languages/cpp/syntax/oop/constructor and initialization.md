---
tags:
  - programming-language
  - cpp
  - class
  - constructor
  - syntax
---
[[__cpp syntax oop__|<==]]

Конструкторы представляют специальную функцию, которая имеет то же имя, что и класс, которая не возвращает никакого значения и которая позволяют инициализировать объект класса во время его создания и таким образом гарантировать, что поля класса будут иметь определенные значения.

При каждом создании нового объекта класса вызывается конструктор класса.

```cpp
#include <iostream>

class Person {
public:
    std::string name;
    unsigned age;

    Person(std::string p_name, unsigned p_age) {
        name = p_name;
        age = p_age;
        std::cout
	        << "Person '" << name
	        << "'(" << age << ") has deen created." << std::endl;
    }

    void print() {
        std::cout
	        << "{name: " << name
	        << ", age: " << age << "}" << std::endl;
    }
};

int main(int argc, char const *argv[]) {
    Person tom("Tom", 42);
    Person bob{"Bob", 43};
    Person sam = Person("Sam", 44);

    tom.print();
    bob.print();
    sam.print();

    return 0;
}
```

```
Person 'Tom'(42) has deen created.
Person 'Bob'(43) has deen created.
Person 'Sam'(44) has deen created.
{name: Tom, age: 42}
{name: Bob, age: 43}
{name: Sam, age: 44}
```

Теперь в классе _Person_ определен конструктор.

По сути конструктор представляет функцию, которая может принимать параметры и которая __должна__ называться по имени класса. 

Если мы определяем свой конструктор, то компилятор больше __не создает__ конструктор по умолчанию. И при создании объекта нам надо обязательно вызвать определенный нами конструктор.

Как альтернативу можно вызывать конструктор с `{}`
```cpp
Person bob{"Bob", 43};
```

Также можно присвоить объекту результат вызова конструктора
```cpp
Person sam = Person("Sam", 44);
```

#### Определение нескольких конструкторов

```cpp
#include <iostream>

class Person0 {

public:
    std::string name;
    unsigned age;

    Person0(std::string p_name, unsigned p_age) {
        name = p_name;
        age = p_age;
        std::cout
	        << "[Person0, both] name: '" << name
	        << "', age: " << age << " - created" << std::endl;
    }

    Person0(std::string p_name) {
        name = p_name;
        age = 42;
        std::cout
	        << "[Person0, name] name: '" << name
	        << "', age: " << age << " - created" << std::endl;
    }

    Person0() {
        name = "NoName";
        age = 42;
        std::cout
	        << "[Person0, none] none: '" << name
	        << "', age: " << age << " - created" << std::endl;
    }

    void print() {
        std::cout
	        << "{name: " << name
	        << ", age: " << age << "}" << std::endl;
    }
};

class Person1 {

public:
    std::string name {};
    unsigned age {};

    Person1(std::string p_name, unsigned p_age) {
        name = p_name;
        age = p_age;
        std::cout
	        << "[Person1] name: '" << name
	        << "', age: " << age << " - created" << std::endl;
    }

    Person1(std::string p_name): Person1(p_name, 42) {
        std::cout << "[Person1] Second" << std::endl;
    }

    Person1(): Person1("NoName", 42) {
        std::cout << "[Person1] Third" << std::endl;
    }

    void print() {
        std::cout
	        << "{name: " << name
	        << ", age: " << age << "}" << std::endl;
    }
};

int main(int argc, char const *argv[]) {
    Person0 p00{"Tom", 18};
    Person0 p01("Bob");
    Person0 p02;
    Person1 p10{"Tommy", 19};
    Person1 p11("Bobby");
    Person1 p12;

    p00.print();
    p01.print();
    p02.print();
    p10.print();
    p11.print();
    p12.print();

    return 0;
}
```

```
[Person0, both] name: 'Tom', age: 18 - created
[Person0, name] name: 'Bob', age: 42 - created
[Person0, none] none: 'NoName', age: 42 - created
[Person1] name: 'Tommy', age: 19 - created
[Person1] name: 'Bobby', age: 42 - created
[Person1] Second
[Person1] name: 'NoName', age: 42 - created
[Person1] Third
{name: Tom, age: 18}
{name: Bob, age: 42}
{name: NoName, age: 42}
{name: Tommy, age: 19}
{name: Bobby, age: 42}
{name: NoName, age: 42}
```

В классе _Person0_ определено три конструктора, и в функции все эти конструкторы используются для создания объектов.

Хотя пример выше прекрасно работает, однако мы можем заметить, что все три конструктора выполняют фактически одни и те же действия - устанавливают значения переменных name и age. Можно сократить их определения, вызова из одного конструктора другой, как показано в _Person1_.

Данная техника называется __делегированием конструктора__, поскольку мы делегируем инициализацию другому конструктору.

#### Параметры по умолчанию

Как и другие функции, конструкторы могут иметь параметры по умолчанию.

```cpp
#include <iostream>

class Person {
public:
    std::string name;
    unsigned age;

    Person(std::string p_name = "NoName", unsigned p_age = 42) {
        name = p_name;
        age = p_age;
    }

    void print() {
        std::cout
	        << "{name: '" << name
	        << "', age: " << age << "}" << std::endl;
    }
};

int main(int argc, char const *argv[]) {
    Person tom {"Tom", 21};
    Person bob {"Bob"};
    Person sam;

    tom.print();
    bob.print();
    sam.print();

    return 0;
}
```

```
{name: 'Tom', age: 21}
{name: 'Bob', age: 42}
{name: 'NoName', age: 42}
```

#### Инициализация констант и списки инициализации

Константное поле не получится инициализировать обычным образом. Этот класс не будет компилироваться из-за отсутствия инициализации константы _name_.

```cpp
#include <iostream>

class Person {

public:
    const std::string name; // <=
    unsigned age;

    Person(std::string p_name, unsigned p_age) {
        name = p_name; // <= Error
        age = p_age;
    }
};

int main(int argc, char const *argv[]) {
    return 0;
}
```

```
.\constructor_init_const_bad.cpp:10:14: error: no viable overloaded '='
   10 |         name = p_name;
      |         ~~~~ ^ ~~~~~~
```

Хотя ее значение устанавливается в конструкторе, но к моменту, когда инструкции из тела конструктора начнут выполняться, константы уже должны быть инициализированы. И для этого необходимо использовать списки инициализации.

```cpp
#include <iostream>

class Person {

public:
    const std::string name;
    unsigned age;

    Person(std::string p_name, unsigned p_age): name(p_name) {
        age = p_age;
    }

    void print() {
        std::cout
	        << "{name: " << name
	        << ", age: " << age << "}" << std::endl;
    }
};

int main(int argc, char const *argv[]) {
    Person tom {"Tom", 42};
    tom.print();

    return 0;
}
```

```
{name: Tom, age: 42}
```

Списки инициализации представляют перечисления инициализаторов для каждой из переменных и констант через двоеточие после списка параметров конструктора.

Списки инициализации подобным образом можно использовать и для присвоения значений переменным.

```cpp
#include <iostream>

class Person {

public:
    const std::string name;
    unsigned age;

    Person(std::string p_name, unsigned p_age): name(p_name), age(p_age){}

    void print() {
        std::cout
	        << "{name: " << name
	        << ", age: " << age << "}" << std::endl;
    }
};

int main(int argc, char const *argv[]) {
    Person tom {"Tom", 42};
    tom.print();

    return 0;
}
```

```
{name: Tom, age: 42}
```

При использовании списков инициализации важно учитывать, что передача значений должна идти в том порядке, в котором константы и переменные определены в классе.

---
[Конструкторы и инициализация объектов](https://metanit.com/cpp/tutorial/5.2.php)