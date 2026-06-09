---
tags:
  - programming-language
  - cpp
  - syntax
  - oop
  - dynamic-cast
---
[[__cpp syntax oop dynamic cast__|<=]]

Динамическое приведение типов, в отличие от статического, выполняется во время выполнения программы. Для этого применяется функция `dynamic_cast<>()`.

```cpp
dynamic_cast<some_type>(some_object);
```

Но эту функцию можно применять только к указателям и ссылкам на `полиморфные типы` классов, которые содержат хотя бы одну виртуальную функцию. Причина в том, что только указатели на типы `полиморфных классов` содержат информацию, которая необходима функции `dynamic_cast` для проверки правильности преобразования. Конечно, типы, между которыми выполняется преобразование, должны быть указателями или ссылками на классы в одной иерархии классов.

Есть два вида динамического приведения
- `нисходящее преобразование` или `downcast`преобразование указателя на базовый класс к указателю на произвольный класс.
- `кросскаст` или `crosscast` преобразование между базовыми типами в одной иерархии при множественном наследовании.

`downcast`
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
    Book* pbook {new Ebook {"Some book", 350, 6}};
    Ebook* ebook {dynamic_cast<Ebook*>(pbook)};
    ebook->print();
    std::cout << ebook->getSize() << std::endl;

    return 0;
}
```

Чтобы динамическое преобразование было возможно, базовые классы определяют виртуальную функцию _print_.

Стоит отметить, что в данном случае динамическое преобразование не имеет смысла для _print_, так как мы итак могли бы вызвать у указателя функцию _print_ и за счет виртуальности функции получили бы тот же самый результат. 

Преобразование нужно, если нам необходимо обратиться к каким-то членам производного класса, которые не определены в базовом. Например, класс _Book_ не имеет функции _getSize_, и чтобы обратиться к ней могло потребоваться преобразование.

`crosscast`
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

void printAsFile(const File*);

int main(int argc, char const *argv[]) {
    Book* go_book {new Ebook {"Golang", 420, 100}};
    File* go_file {dynamic_cast<File*>(go_book)};
    go_file->print();

    Book* java_book {new Ebook {"Java", 430, 110}};
    File* java_file {dynamic_cast<File*>(java_book)};
    printAsFile(java_file);

    Book* pbook {new Book{"", 123}};
    File* file {dynamic_cast<File*>(pbook)};
    printAsFile(file);

    return 0;
}

void printAsFile(const File* file) {
    if (file) {
        file->print();
    } else {
        std::cout << "The file is not book." << std::endl;
    }
}
```
Преобразование из указателя на _Book_ в указатель на _File_ является кросскастом и в первом случае возможно, потому что указатель хранит адрес объекта _Ebook_, который также наследуется от _File_.

Но подобные преобразования не всегда выполняются успешно. Во втором случае функция `dynamic_cast()` возвращает указатель _nullptr_.

---
[Динамическое преобразование](https://metanit.com/cpp/tutorial/5.26.php)