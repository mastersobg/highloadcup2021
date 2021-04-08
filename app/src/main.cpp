#include "lib/app.h"

int main() {
    auto app = App::createApp();
    app->run();
    return 0;
}
