#include "app.h"
#include "log.h"

int main() {
    auto app = App::createApp();
    app->run();
    return 0;
}
