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

    eb.Book::print();
    eb.File::print();

    static_cast<Book&>(eb).print();
    static_cast<File&>(eb).print();

    return 0;
}
