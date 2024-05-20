# DB48X for WASM

[Try the simulator in your browser](https://sunpazed.github.io/db48x-wasm/bin/index.html)

## About
This is a fork/variation of the excellent [db48x](https://github.com/c3d/db48x) simulator written by 
Christophe de Dinechin for the [SwissMicro DM42](https://www.swissmicros.com/product/dm42) calculator.
This code is a proof-of-concept, is provided as-is, and is likely out of date. As such, this code is not 
supported by Christophe — please refer to the original repository.

(Update by Christophe: Yes it is! This is insanely great stuff!)

## WASM simulator
The simulator implementation of db48x utilises Qt which runs fairly sluggishly on low resource devices 
such as handhelds. The motivation is to port the simulator from Qt to something more portable such as
WASM which can run on mobile devices. 

A very basic simulator is wired up to a WASM compiled binary of db48x, which then runs on HTML and 
Javascript for use on mobile devices. Be warned that there are many bugs, with the simulator hanging 
when accessing system services (help, save state, system menu) which are not implemented .

## Building the simulator
Install and activate [emsdk](https://github.com/emscripten-core/emsdk) then ensure that it is available 
in the path. Run `make` within this repo to build `main.js` and `main.wasm`. Host the files within the `bin` 
directory via a webserver.

## Running the simulator
Point your web-browser to the `index.html` page to run the simiulator. The simulator can be run fullscreen 
by selecting "Add to Home Screen" on iOS or your Android device.

