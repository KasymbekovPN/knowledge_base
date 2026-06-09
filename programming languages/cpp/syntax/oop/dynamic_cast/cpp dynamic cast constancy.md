---
tags:
  - programming-language
  - cpp
  - syntax
  - oop
  - dynamic-cast
  - constants
---
[[__cpp syntax oop dynamic cast__|<=]]

```cpp
#include <iostream>

class Book {

private:
    std::string title;
    unsigned pages;

public:
    Book(std::string, unsigned);
    std::string getTitle() const;
    unsigned getPages() const;
    virtual void print() const;
};

Book::Book(std::string title, unsigned pages):
    title{title},
    pages{pages} {}

std::string Book::getTitle() const {
    return title;
}

unsigned Book::getPages() const {
    return pages;
}

void Book::print() const {
    std::cout
        << "{title: " << getTitle()
        << ", pages: " << getPages()
        << "}" << std::endl;
}

class File {

private:
    unsigned size;

public:
    File(unsigned);
    unsigned getSize() const;
    virtual void print() const;
};

File::File(unsigned size): size{size} {}

unsigned File::getSize() const {
    return size;
}

void File::print() const {
    std::cout
        << "{size: " << getSize()
        << "}" << std::endl;
}

class Ebook: public Book, public File {

public:
    Ebook(std::string, unsigned, unsigned);
    void print() const override;
};

Ebook::Ebook(std::string title, unsigned pages, unsigned size):
    Book{title, pages},
    File{size} {}

void Ebook::print() const {
    std::cout
        << "{title: " << getTitle()
        << ", pages: " << getPages()
        << ", size: " << getSize()
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    const Ebook ebook0 {"book 0", 350, 6};
    const Book* book0 {&ebook0};
    const Ebook* file0 {dynamic_cast<const Ebook*>(book0)};
    file0->print();

    const Ebook ebook1 {"book 1", 360, 7};
    const Book* const_book = &ebook1;
    Book* book {const_cast<Book*>(const_book)};
    Ebook* file {dynamic_cast<Ebook*>(book)};
    file->print();

    return 0;
}
```

Если преобразуемый указатель является указателем на константу, то тип указателя, к которому выполняется приведение, также должен представлять указатель на константу.

Если необходимо выполнить приведение из указателя на константу в обычный указатель (не на константу), то сначала надо выполнить приведение к указателю того же типа, что и исходный, с помощью функции `const_cast<T>`.

---
[Динамическое преобразование](https://metanit.com/cpp/tutorial/5.26.php)