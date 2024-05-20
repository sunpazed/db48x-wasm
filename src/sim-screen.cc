// ****************************************************************************
//  screen.cpp                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//
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

#include "sim-screen.h"

#include "dmcp.h"
#include "sim-dmcp.h"

#include "target.h"

// A copy of the LCD buffer
pixword lcd_copy[sizeof(lcd_buffer) / sizeof(*lcd_buffer)];



uintptr_t updatePixmap()
// ----------------------------------------------------------------------------
//   Recompute the pixmap
// ----------------------------------------------------------------------------
//   This should be done on the RPL thread to get a consistent picture
{
    // Monochrome screen
    pixword mask = ~(~0U << color::BPP);
    surface s(lcd_buffer, LCD_W, LCD_H, LCD_SCANLINE);
    for (int y = 0; y < SIM_LCD_H; y++)
    {
        for (int xw = 0; xw < SIM_LCD_SCANLINE*color::BPP/32; xw++)
        {
            unsigned woffs = y * (SIM_LCD_SCANLINE*color::BPP/32) + xw;
            if (uint32_t diffs = lcd_copy[woffs] ^ lcd_buffer[woffs])
            {
                for (int bit = 0; bit < 32; bit += color::BPP)
                {
                    if ((diffs >> bit) & mask)
                    {
                        pixword bits = (lcd_buffer[woffs] >> bit) & mask;
                        color col(bits);
// #ifdef CONFIG_COLOR
//                         QColor qcol(col.red(), col.green(), col.blue());
// #else
//                         QColor &qcol = bits ? bgColor : fgColor;
// #endif
//                         pt.setPen(qcol);

                        coord xx = (xw * 32 + bit) / color::BPP;
                        coord yy = y;
                    }
                }
                lcd_copy[woffs] = lcd_buffer[woffs];
            }
        }
    }
    return uintptr_t(lcd_buffer);
}

