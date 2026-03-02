#include <format>
#include <iostream>

#include "System Interface/SystemInterface.hpp"

using namespace std;
using namespace sjcs;

int main() {
    // Intentionally empty. All wiring is defined in header contracts and implemented elsewhere.
    cout<< format("The Application Works!!!");
    cin.get();
    return 0;
}