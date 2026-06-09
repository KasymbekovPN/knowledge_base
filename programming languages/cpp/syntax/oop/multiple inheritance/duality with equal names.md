---
tags:
  - programming-language
  - cpp
  - syntax
  - inheritance
  - duality
---
[[__cpp syntax oop inheritance multiple__|<==]]

В случае, если в базовых класса есть методы с одинаковыми именами, то прямой вызов данного метода не даст скомпилировать код.

```cpp
#include <iostream>

class Book {

private:
    unsigned pages;

public:
    Book(unsigned);
    ~Book();
    void print() const;
};

Book::Book(unsigned pages):
    pages(pages) {}

Book::~Book() {}

void Book::print() const {
    std::cout << "Pages: " << pages << std::endl;
}

class File {

private:
    double size;

public:
    File(double size);
    ~File();
    void print() const;
};

File::File(double size):
    size(size) {}

File::~File() {}

void File::print() const {
    std::cout << "Size: " << size << std::endl;
}

class Ebook: public File, public Book {

private:
    std::string title;

public:
    Ebook(std::string, unsigned, double);
    ~Ebook();
    void printTitle() const;
};

Ebook::Ebook(std::string title, unsigned pages, double size):
    Book{pages},
    File{size},
    title{title} {}

Ebook::~Ebook() {}

void Ebook::printTitle() const {
    std::cout << "Title: " << title << std::endl;
}

int main(int argc, char const *argv[]) {
    Ebook eb {"About ...", 555, 1.23};
	// eb.print(); // <= Error 
    eb.Book::print();
    eb.File::print();

    static_cast<Book&>(eb).print();
    static_cast<File&>(eb).print();

    return 0;
}
```

```
Pages: 555
Size: 1.23
Pages: 555
Size: 1.23
```

```
.\duality_with_eq_names.cpp:80:12: error: member 'print' found in multiple base classes of different types
   80 |         eb.print();
      |            ^
.\duality_with_eq_names.cpp:47:12: note: member found by ambiguous name lookup
   47 | void File::print() const {
      |            ^
.\duality_with_eq_names.cpp:22:12: note: member found by ambiguous name lookup
   22 | void Book::print() const {
      |            ^
```

Чтобы решить проблему, мы можем указать, из какого конкретного класса мы хотим вызвать функцию _print_.

В качестве альтернативы мы можем выполнять операцию преобразования к нужному типу и затем вызывать функцию.

---
[Множественное наследование](https://metanit.com/cpp/tutorial/5.24.php)