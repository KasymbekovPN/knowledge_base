---
tags:
  - programming-language
  - cpp
  - syntax
  - operator-overloading
---
[[__cpp syntax oop type conversion__|<=]]

```cpp
#include <iostream>

class PrintBook;

class Ebook {

private:
    std::string title;

public:
    Ebook(std::string);
    std::string getTitle() const;
    void print() const;
    operator PrintBook() const;
};

class PrintBook {

private:
    std::string title;

public:
    PrintBook(std::string);
    std::string getTitle() const;
    void print() const;
    operator Ebook() const;
};

Ebook::Ebook(std::string title): title{title} {}

std::string Ebook::getTitle() const {
    return title;
}

void Ebook::print() const {
    std::cout
        << "[Ebook] {title: " << getTitle()
        << "}" << std::endl;
}

Ebook::operator PrintBook() const {
    return PrintBook{getTitle()};
}

PrintBook::PrintBook(std::string title): title{title} {}

std::string PrintBook::getTitle() const {
    return title;
}

void PrintBook::print() const {
    std::cout
        << "[PrintBook] {title: " << getTitle()
        << "}" << std::endl;
}

PrintBook::operator Ebook() const {
    return Ebook{getTitle()};
}

int main(int argc, char const *argv[]) {
    PrintBook book0 {"About..."};
    book0.print();

    Ebook book1 {book0};
    book1.print();

    PrintBook book2 {book1};
    book2.print();

    return 0;
}
```

```
[PrintBook] {title: About...}
[Ebook] {title: About...}
[PrintBook] {title: About...}
```

---
[Операторы преобразования типов](https://metanit.com/cpp/tutorial/5.15.php)