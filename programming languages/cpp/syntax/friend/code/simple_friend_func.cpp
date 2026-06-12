#include <iostream>

class Auto {

friend void drive(const Auto&);
friend void setPrice(Auto&, unsigned);

private:
    std::string name;
    unsigned price;

public:
    Auto(std::string name, unsigned price);
    void print();
};

Auto::Auto(std::string name, unsigned price): name(name), price(price) {}

void Auto::print() {
    std::cout
        << "{name: " << name
        << ", " << price
        << "}" << std::endl;
}

void drive(const Auto& car) {
    std::cout <<  car.name << " is driven" << std::endl;
}

void setPrice(Auto& car, unsigned price) {
    car.price = price;
}

int main(int argc, char const *argv[]) {
    Auto tesla{"Tesla", 5000};
    tesla.print();

    drive(tesla);
    setPrice(tesla, 4000);
    tesla.print();

    return 0;
}
