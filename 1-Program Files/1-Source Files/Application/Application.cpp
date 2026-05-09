#include <System Interface/SystemInterface.hpp>
using namespace sjcs;

int main() {
    auto app = Application::create(8000);
    return app ? app->run() : 1;
}
