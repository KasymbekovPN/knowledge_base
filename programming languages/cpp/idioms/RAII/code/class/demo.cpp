#include <iostream>

class DatabaseConnection {
public:
    DatabaseConnection(const std::string& host) {
        std::cout << "Connection to " << host << "..." << std::endl;
        // connection to DB
    }
    ~DatabaseConnection() {
        std::cout << "Disconnection..." << std::endl;
        // disconnection
    }

    void query(const std::string& sql) {
        std::cout << "Executing: " << sql << std::endl;
    }
};

void use_db();

int main() {
    use_db();

    return 0;
}

void use_db() {
    DatabaseConnection con("localhost");
    con.query("select * form users");
} // <- connection will closed automatically
