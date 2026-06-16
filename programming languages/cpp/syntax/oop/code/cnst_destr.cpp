#include <iostream>

class Book {

private:
    unsigned pages;

public:
    Book(unsigned);
    ~Book();
    void printPageCount() const;
};

Book::Book(unsigned pages): pages(pages) {
    std::cout << "The book created" << std::endl;
}

Book::~Book() {
    std::cout << "The book deleted" << std::endl;
}

void Book::printPageCount() const {
    std::cout << "Pages: " << pages << std::endl;
}


class File {

private:
    double size;

public:
    File(double size);
    ~File();
    void printSize() const;
};


File::File(double size): size(size) {
    std::cout << "The file created" << std::endl;
}

File::~File() {
    std::cout << "The file deleted" << std::endl;
}

void File::printSize() const {
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
    Book{pages}, File{size}, title{title} {
    
    std::cout << "The e-book created" << std::endl;
}

Ebook::~Ebook() {
    std::cout << "The e-book deleted" << std::endl;
}

void Ebook::printTitle() const {
    std::cout << "Title: " << title << std::endl;
}

int main(int argc, char const *argv[]) {
    {
        Ebook eb {"About ...", 555, 1.23};
        eb.printPageCount();
        eb.printSize();
        eb.printTitle();
    }

    return 0;
}
