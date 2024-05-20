// ****************************************************************************
//  sim-window.cpp                                                DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Main window for the DM42 simulator
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2022 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the terms outlined in LICENSE.txt
// ****************************************************************************
//   This file is part of DB48X.
//
//   DB48X is free software: you can redistribute it and/or modify
//   it under the terms outlined in the LICENSE.txt file
//
//   DB48X is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// ****************************************************************************

#include "sim-window.h"

#include "dmcp.h"
#include "main.h"
#include "recorder.h"
#include "sim-dmcp.h"
// #include "sim-rpl.h"
#include "target.h"
// #include "ui_sim-window.h"

#include <iostream>

#include "emcc.h"

RECORDER(sim_keys, 16, "Recorder keys from the simulator");
RECORDER(sim_audio, 16, "Recorder keys from the simulator");

extern bool run_tests;
extern bool db48x_keyboard;
extern bool shift_held;
extern bool alt_held;
// MainWindow *MainWindow::mainWindow = nullptr;
// qreal MainWindow::devicePixelRatio = 1.0;


void ui_refresh()
// ----------------------------------------------------------------------------
//   Request a refresh of the LCD
// ----------------------------------------------------------------------------
{

}


uint ui_refresh_count()
// ----------------------------------------------------------------------------
//   Return the number of times the display was actually udpated
// ----------------------------------------------------------------------------
{
    return 0;
}


void ui_screenshot()
// ----------------------------------------------------------------------------
//   Take a screen snapshot
// ----------------------------------------------------------------------------
{
    
}


void ui_push_key(int k)
// ----------------------------------------------------------------------------
//   Update display when pushing a key
// ----------------------------------------------------------------------------
{
//    key_push(k);
}


void ui_ms_sleep(uint ms_delay)
// ----------------------------------------------------------------------------
//   Suspend the current thread for the given interval in milliseconds
// ----------------------------------------------------------------------------
{
    
}


int ui_file_selector(const char *title,
                     const char *base_dir,
                     const char *ext,
                     file_sel_fn callback,
                     void       *data,
                     int         disp_new,
                     int         overwrite_check)
// ----------------------------------------------------------------------------
//  File selector function
// ----------------------------------------------------------------------------
{
    return 0;
}


void ui_save_setting(const char *name, const char *value)
// ----------------------------------------------------------------------------
//  Save some settings
// ----------------------------------------------------------------------------
{
}


size_t ui_read_setting(const char *name, char *value, size_t maxlen)
// ----------------------------------------------------------------------------
//  Save some settings
// ----------------------------------------------------------------------------
{
    return 0;
}


uint last_battery_ms = 0;
uint battery = 1000;
bool charging = false;

uint ui_battery()
// ----------------------------------------------------------------------------
//   Return the battery voltage
// ----------------------------------------------------------------------------
{
    uint now = sys_current_ms();
    if (last_battery_ms < now - 1000)
        last_battery_ms = now - 1000;

    if (charging)
    {
        battery += (1000 - battery) * (now - last_battery_ms) / 6000;
        if (battery >= 990)
            charging = false;
    }
    else
    {
        battery -= (now - last_battery_ms) / 10;
        uint v = battery * (BATTERY_VMAX - BATTERY_VMIN) / 1000 + BATTERY_VMIN;
        if (v < BATTERY_VLOW)
            charging = true;
    }

    last_battery_ms = now;
    return battery;
}


bool ui_charging()
// ----------------------------------------------------------------------------
//   Return true if USB-powered or not
// ----------------------------------------------------------------------------
{
    return charging;
}


void ui_start_buzzer(uint frequency)
// ----------------------------------------------------------------------------
//   Start buzzer at given frequency
// ----------------------------------------------------------------------------
{
}


void ui_stop_buzzer()
// ----------------------------------------------------------------------------
//  Stop buzzer in simulator
// ----------------------------------------------------------------------------
{
}

uint32_t ui_return_screen() {
    return sizeof(lcd_buffer);
}

void run_rpl()
// ----------------------------------------------------------------------------
//   Thread entry point
// ----------------------------------------------------------------------------
{
    program_main();
}


// int ui_wrap_io(file_sel_fn callback, const char *path, void *data, bool)
// // ----------------------------------------------------------------------------
// //   Wrap I/Os into thread safety / file sync
// // ----------------------------------------------------------------------------
// {
//     cstring name = path;
//     for (cstring p = path; *p; p++)
//         if (*p == '/' || *p == '\\')
//             name = p + 1;
//     return callback(path, name, data);
// }


