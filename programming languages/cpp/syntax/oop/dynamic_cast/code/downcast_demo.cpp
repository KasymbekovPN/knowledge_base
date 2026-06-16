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
