#include <emscripten/bind.h>
using namespace emscripten;

EMSCRIPTEN_BINDINGS(my_module) {
    function("ui_battery", &ui_battery);
    function("updatePixmap", &updatePixmap);
    function("run_rpl", &run_rpl);
    function("ui_return_screen", &ui_return_screen);
    function("init_all_elements", &init_all_elements);
    function("ui_push_key", &key_push);
}