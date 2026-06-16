#include <iostream>

class Camera {

public:
    void makePhoto() const;
};

void Camera::makePhoto() const {
    std::cout << "Making photo" << std::endl;
}

class Phone {

public:
    void makeCall() const;
};

void Phone::makeCall() const {
    std::cout << "Making call" << std::endl;
}

class Smartpone: public Phone, public Camera{};

int main(int argc, char const *argv[]) {
    Smartpone sp;
    sp.makeCall();
    sp.makePhoto();

    return 0;
}
