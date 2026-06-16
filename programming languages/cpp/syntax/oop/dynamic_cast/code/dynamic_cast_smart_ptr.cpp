#include <iostream>
#include <memory>

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

void printEbook(std::shared_ptr<Ebook>);

int main(int argc, char const *argv[]) {
    std::shared_ptr<Book> book0 {std::make_shared<Ebook>("About C++", 420, 7)};
    std::shared_ptr<Ebook> ebook0 {std::dynamic_pointer_cast<Ebook>(book0)};
    printEbook(ebook0);

    std::shared_ptr<Book> book1 {std::make_shared<Book>("About go", 421)};
    std::shared_ptr<Ebook> ebook1 {std::dynamic_pointer_cast<Ebook>(book1)};
    printEbook(ebook1);

    return 0;
}

void printEbook(std::shared_ptr<Ebook> ebook) {
    if (ebook) {
        ebook->print();
    } else {
        std::cout << "The object is not Ebook" << std::endl;
    }
}
