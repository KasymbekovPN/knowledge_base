#include "current_format.hpp"

#include <iomanip>
#include <sstream>

std::string formatCurrency(const double amount) {
    std::ostringstream oss;
    oss << "$" << std::fixed << std::setprecision(2) << amount;

    return oss.str();
}
