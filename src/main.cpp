#include "first_app.hpp"

#include <cstdlib>
#include <iostream>

int main() {
    Glorp::FirstApp app{};

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
