// ****************************************************************************
//  command.cc                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Shared code for all RPL commands
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

#include "command.h"

#include "arithmetic.h"
#include "bignum.h"
#include "decimal.h"
#include "dmcp.h"
#include "fraction.h"
#include "integer.h"
#include "parser.h"
#include "polynomial.h"
#include "program.h"
#include "renderer.h"
#include "runtime.h"
#include "settings.h"
#include "symbol.h"
#include "sysmenu.h"
#include "tag.h"
#include "unit.h"
#include "user_interface.h"
#include "utf8.h"
#include "util.h"
#include "version.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>


RECORDER(command,       16, "RPL Commands");
RECORDER(command_error, 16, "Errors processing a command");


PARSE_BODY(command)
// ----------------------------------------------------------------------------
//    Try to parse this as a command, using either short or long name
// ----------------------------------------------------------------------------
{
    // We scan all the commands in one loop under 'Drop'. Skip all other
    id i = p.candidate;
    if (i != ID_Drop)
        return SKIP;

    bool    eq     = p.precedence;
    id      type   = id(0);
    id      found  = id(0);
    cstring ref    = cstring(utf8(p.source));
    size_t  maxlen = p.length;
    size_t  len    = maxlen;

    for (size_t i = 0; i < spelling_count; i++)
    {
        if (!is_command(spellings[i].type))
            continue;
        if (cstring cmd = spellings[i].name)
        {
            if (type != spellings[i].type)
            {
                type = spellings[i].type;

                // When parsing an equation, parse x³ as cubed(x)
                if (eq && (type == ID_sq || type == ID_cubed || type == ID_inv))
                    continue;
            }

            // No function names like `min` while parsing units
            if (unit::mode && is_valid_as_name_initial(utf8(cmd)))
                continue;

            len = strlen(cmd);
            if (len <= maxlen
                && strncasecmp(ref, cmd, len) == 0
                && (len >= maxlen
                    || (eq && (!is_valid_as_name_initial(utf8(cmd)) ||
                               ((ref[len] < '0' || ref[len] > '9') &&
                                !is_valid_as_name_initial(utf8(ref + len)))))
                    || is_separator(utf8(ref + len))))
            {
                found = type;
                break;
            }
        }
    }

    record(command,
           "Parsing [%s] with id %u %+s (%+s), found %u len %u",
           ref, i, name(i), fancy(i), found, len);

    if (!found)
        return SKIP;

    // Record output - Dynamically generate ID for use in programs
    p.end = len;
    p.out = rt.make<command>(found);

    return OK;
}


RENDER_BODY(command)
// ----------------------------------------------------------------------------
//   Render the command into the given string buffer
// ----------------------------------------------------------------------------
{
    id ty = o->type();
    if (ty < NUM_IDS)
    {
        id format = Settings.CommandDisplayMode();

        // Ensure that we display + as `+` irrespective of mode
        utf8 fname = object::name(ty);

        while (unit::mode)
        {
            if (ty == ID_div)
                r.put('/');
            else if (ty == ID_mul)
                r.put(unicode(L'·'));
            else
                break;
            return r.size();
        }
        if (ty == ID_mul && format == ID_LongForm &&
            r.expression() && Settings.UseDotForMultiplication())
            fname = utf8("·");
        r.put(format, fname);
    }

    record(command, "Render %u as [%s]", ty, (cstring) r.text());
    return r.size();
}


uint32_t command::uint32_arg(uint level)
// ----------------------------------------------------------------------------
//   Get an unsigned value from the stack
// ----------------------------------------------------------------------------
{
    if (object_p d = rt.stack(level))
        return d->as_uint32(0, true);
    return 0;
}


int32_t command::int32_arg(uint level)
// ----------------------------------------------------------------------------
//   Get a signed value from the stack
// ----------------------------------------------------------------------------
{
    if (object_p d = rt.stack(level))
        return d->as_int32(0, true);
    return 0;
}



// ============================================================================
//
//   Command implementations
//
// ============================================================================

COMMAND_BODY(Eval)
// ----------------------------------------------------------------------------
//   Evaluate an object
// ----------------------------------------------------------------------------
{
    if (object_p x = rt.pop())
        return program::run(x);
    return ERROR;
}


COMMAND_BODY(ToText)
// ----------------------------------------------------------------------------
//   Convert an object to text
// ----------------------------------------------------------------------------
{
    if (object_g obj = rt.top())
        if (object_g txt = obj->as_text(false, false))
            if (rt.top(txt))
                return OK;
    return ERROR;
}


COMMAND_BODY(Compile)
// ----------------------------------------------------------------------------
//   Interpret the object as a command line and evaluate it
// ----------------------------------------------------------------------------
{
    if (object_p obj = rt.top())
    {
        if (text_p tobj = obj->as<text>())
        {
            if (tobj->compile_and_run())
                return OK;
        }
        else
        {
            rt.type_error();
        }
    }
    return ERROR;
}


COMMAND_BODY(Explode)
// ----------------------------------------------------------------------------
//  Implement the Obj→ command
// ----------------------------------------------------------------------------
{
    object_p obj = rt.top();
    id       oty = obj->type();
    switch (oty)
    {
    case ID_rectangular:
    case ID_polar:
    case ID_unit:
    {
        complex_p cplx = complex_p(obj);
        if (rt.top(cplx->x()) && rt.push(+cplx->y()))
            return OK;
        break;
    }
    case ID_program:
    case ID_expression:
    case ID_list:
        if (rt.drop())
        {
            if (list_p(obj)->expand())
                return OK;
            rt.push(obj);
        }
        break;
    case ID_array:
        if (rt.drop())
        {
            if (array_p(obj)->expand())
                return OK;
            rt.dimension_error();
            rt.push(obj);
        }
        break;
    case ID_polynomial:
        if (algebraic_p alg = polynomial_p(obj)->as_expression())
            if (rt.top(alg))
                return OK;
        break;
    case ID_text:
        if (rt.drop())
        {
            size_t depth = rt.depth();
            if (text_p(obj)->compile_and_run())
                return OK;
            // Try to undo the damage - Won't always work
            if (rt.depth() > depth)
                rt.drop(rt.depth() - depth);
            rt.push(obj);
        }
        break;
    case ID_fraction:
    case ID_neg_fraction:
    case ID_big_fraction:
    case ID_neg_big_fraction:
    {
        fraction_p frac = fraction_p(obj);
        bignum_g num = frac->numerator();
        bignum_g den = frac->denominator();
        if (num && den && rt.top(num) && rt.push(+den))
            return OK;
        break;
    }
    case ID_tag:
    {
        tag_p tobj = tag_p(obj);
        if (rt.top(tobj->tagged_object()) && rt.push(tobj->label()))
            return OK;
        break;
    }
    default:
        rt.type_error();
        break;
    }
    return ERROR;
}


COMMAND_BODY(ReplaceChar)
// ----------------------------------------------------------------------------
//   Find the label associated to the menu and enter it in the editor
// ----------------------------------------------------------------------------
{
    if (int key = ui.evaluating_function_key())
    {
        uint plane = ui.shift_plane();
        uint menu_idx = key - KEY_F1 + plane * ui.NUM_SOFTKEYS;
        if (symbol_p sym = ui.label(menu_idx))
            ui.replace_character_left_of_cursor(sym);
        else if (cstring lbl = ui.label_text(menu_idx))
            ui.replace_character_left_of_cursor(utf8(lbl), strlen(lbl));
    }
    return OK;
}


COMMAND_BODY(SelfInsert)
// ----------------------------------------------------------------------------
//   Find the label associated to the menu and enter it in the editor
// ----------------------------------------------------------------------------
{
    if (int key = ui.evaluating_function_key())
    {
        uint plane = ui.shift_plane();
        uint menu_idx = key - KEY_F1 + plane * ui.NUM_SOFTKEYS;
        if (symbol_p sym = ui.label(menu_idx))
        {
            size_t len = 0;
            utf8 txt = sym->value(&len);
            ui.edit(txt, len, ui.TEXT);
        }
        else if (cstring lbl = ui.label_text(menu_idx))
        {
            ui.edit(utf8(lbl), ui.TEXT);
        }
    }
    return OK;
}


EVAL_BODY(Unimplemented)
// ----------------------------------------------------------------------------
//   Display an unimplemented error
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating_function_key();
    rt.command(o);
    if (key)
    {
        uint plane = ui.shift_plane();
        uint menu_idx = key - KEY_F1 + plane * ui.NUM_SOFTKEYS;
        if (cstring lbl = ui.label_text(menu_idx))
            rt.command(symbol::make(lbl));
    }
    rt.unimplemented_error();
    return ERROR;
}


MARKER_BODY(Unimplemented)
// ----------------------------------------------------------------------------
//   We mark unimplemented features with a little gray mark
// ----------------------------------------------------------------------------
{
    return L'░';
}


COMMAND_BODY(Ticks)
// ----------------------------------------------------------------------------
//   Return number of ticks
// ----------------------------------------------------------------------------
{
    uint ticks = sys_current_ms();
    if (integer_p ti = rt.make<integer>(ID_integer, ticks))
        if (rt.push(ti))
            return OK;
    return ERROR;
}


COMMAND_BODY(Wait)
// ----------------------------------------------------------------------------
//   Wait the specified amount of seconds
// ----------------------------------------------------------------------------
{
    if (object_p obj = rt.top())
    {
        if (algebraic_g wtime = obj->as_algebraic())
        {
            rt.drop();
            algebraic_g scale = integer::make(1000);
            wtime = wtime * scale;
            if (wtime)
            {
                bool     negative = wtime->is_negative();
                uint32_t msec     = negative ? 0 : wtime->as_uint32(1000, true);
                uint32_t end      = sys_current_ms() + msec;
                bool     infinite = msec == 0 || negative;
                int      key      = 0;

                if (negative)
                    ui.draw_menus();
                while (!key)
                {
                    // Sleep in chunks of one minute
                    int remains = infinite ? 60000 : int(end - sys_current_ms());
                    if (remains <= 0)
                        break;
                    if (remains > 60000)
                        remains = 60000;

                    // Refresh screen moving elements after the requested period
                    sys_timer_disable(TIMER1);
                    sys_timer_start(TIMER1, remains);

                    // Do not switch off if on USB power
                    if (usb_powered())
                        reset_auto_off();

                    // Honor auto-off while waiting, do not erase drawn image
                    if (power_check(false))
                        continue;

                    if (!key_empty())
                    {
#if SIMULATOR
                        if (key_tail() != KEY_EXIT)
                        {
                            key = key_pop();
                        }
#else // SIMULATOR
                        key = key_pop();
#endif // SIMULATOR
                    }
                    if (key == KEY_EXIT)
                    {
                        program::halted = true;
                        program::stepping = 0;
                    }
                }
                if (infinite)
                {
                    if (integer_p ikey = integer::make(key))
                        if (rt.push(ikey))
                            return OK;
                    return ERROR;
                }
                return OK;
            }
        }
        else
        {
            rt.type_error();
        }
    }
    return ERROR;
}



COMMAND_BODY(Bytes)
// ----------------------------------------------------------------------------
//   Return the bytes and a binary represenetation of the object
// ----------------------------------------------------------------------------
{
    if (object_p top = rt.top())
    {
        size_t size = top->size();
        size_t maxsize = (Settings.WordSize() + 7) / 8;
        size_t hashsize = size > maxsize ? maxsize : size;
        gcbytes bytes = byte_p(top);
#if CONFIG_FIXED_BASED_OBJECTS
        // Force base 16 if we have that option
        const id type = ID_hex_bignum;
#else // !CONFIG_FIXED_BASED_OBJECTS
        const id type = ID_based_bignum;
#endif // CONFIG_FIXED_BASED_OBJECTS
        if (bignum_p bin = rt.make<bignum>(type, bytes, hashsize))
            if (rt.top(bin))
                if (rt.push(integer::make(size)))
                    return OK;

    }
    return ERROR;
}


static integer_p type_value(object_p obj)
// ----------------------------------------------------------------------------
//   Return a native or compatible type value
// ----------------------------------------------------------------------------
{
    uint type = obj->type();
    if (Settings.CompatibleTypes())
    {
        switch (type)
        {
        case object::ID_hwfloat:
        case object::ID_hwdouble:
        case object::ID_decimal:                type = 0; break; // Or 21
        case object::ID_rectangular:
        case object::ID_polar:                  type = 1; break;
        case object::ID_text:                   type = 2; break;
        // Treat as symbolic vector matrix on HP50G,
        // don't check inside to see if it's real (3) or complex (4) array
        case object::ID_array:                  type = 29; break;
        case object::ID_list:                   type = 5; break;
        case object::ID_symbol:                 type = 6; break;
        case object::ID_local:                  type = 7; break;
        case object::ID_block:
        case object::ID_locals:
        case object::ID_program:                type = 8; break;
        case object::ID_fraction:
        case object::ID_neg_fraction:
        case object::ID_big_fraction:
        case object::ID_neg_big_fraction:
        case object::ID_expression:             type = 9; break;

#ifdef CONFIG_FIXED_BASED_OBJECTS
        case object::ID_hex_integer:
        case object::ID_dec_integer:
        case object::ID_oct_integer:
        case object::ID_bin_integer:
        case object::ID_hex_bignum:
        case object::ID_dec_bignum:
        case object::ID_oct_bignum:
        case object::ID_bin_bignum:
#endif // CONFIG_FIXED_BASED_OBJECTS
        case object::ID_based_integer:
        case object::ID_based_bignum:           type = 10; break;
        case object::ID_grob:
        case object::ID_bitmap:                 type = 11; break;
        case object::ID_tag:                    type = 12; break;
        case object::ID_unit:                   type = 13; break;
        // No XLIB type 14 yet
        case object::ID_directory:              type = 15; break;
        // No Library type 16 yet
        // No Backup object type 17 yet
        case object::ID_integer:
        case object::ID_neg_integer:
        case object::ID_bignum:
        case object::ID_neg_bignum:             type = 28; break;
        case object::ID_dense_font:             type = 27; break;
        case object::ID_sparse_font:            type = 30; break;

        default:
            type = object::is_algebraic(object::id(type)) ? 18 : 19;
            break;
        }
        return integer::make(type);
    }
    return rt.make<neg_integer>(type + 1);
}



COMMAND_BODY(Type)
// ----------------------------------------------------------------------------
//   Return the type of the top of stack as a numerical value
// ----------------------------------------------------------------------------
{
    if (object_p top = rt.top())
        if (integer_p type = type_value(top))
            if (rt.top(type))
                return OK;
    return ERROR;
}


COMMAND_BODY(TypeName)
// ----------------------------------------------------------------------------
//   Return the type of the top of stack as text
// ----------------------------------------------------------------------------
{
    if (object_p top = rt.top())
        if (text_p type = text::make(top->fancy()))
            if (rt.top(type))
                return OK;
    return ERROR;
}


COMMAND_BODY(Off)
// ----------------------------------------------------------------------------
//   Switch the calculator off
// ----------------------------------------------------------------------------
{
    power_off();
    return OK;
}


COMMAND_BODY(SaveState)
// ----------------------------------------------------------------------------
//   Save the system state to disk
// ----------------------------------------------------------------------------
{
    save_system_state();
    return OK;
}


COMMAND_BODY(SystemSetup)
// ----------------------------------------------------------------------------
//   Select the system menu
// ----------------------------------------------------------------------------
{
    system_setup();
    return OK;
}


COMMAND_BODY(ScreenCapture)
// ----------------------------------------------------------------------------
//   Snapshot the screen and save to a file
// ----------------------------------------------------------------------------
{
    if (screenshot())
        return OK;

    rt.screenshot_capture_error();
    return ERROR;
}


COMMAND_BODY(Beep)
// ----------------------------------------------------------------------------
//   Emit a sound
// ----------------------------------------------------------------------------
{
    algebraic_g duration  = rt.stack(0)->as_real();
    if (!duration)
    {
        rt.type_error();
        return ERROR;
    }
    uint frequency = rt.stack(1)->as_uint32(4400, true);
    if (frequency < 1 || frequency > 18000)
    {
        rt.drop(2);
        return OK;
    }
    duration = duration * integer::make(1000);
    if (duration)
    {
        uint ms = duration->as_uint32(10, true);
        if (ms > 10000)
            ms = 10000;
        if (!rt.error())
        {
            rt.drop(2);
            beep(frequency, ms);
            return OK;
        }
    }
    return ERROR;
}


COMMAND_BODY(Version)
// ----------------------------------------------------------------------------
//   Return a version string
// ----------------------------------------------------------------------------
{
    const utf8 version_text = (utf8)
        "DB48X " DB48X_VERSION "\n"
        "A modern implementation of\n"
        "Reverse Polish Lisp (RPL)\n"
        "and a tribute to\n"
        "Bill Hewlett and Dave Packard\n"
        "© 2024 Christophe de Dinechin";
    if (text_g version = text::make(version_text))
        if (rt.push(object_p(version)))
            return OK;
    return ERROR;
}


COMMAND_BODY(Help)
// ----------------------------------------------------------------------------
//   Bring contextual help
// ----------------------------------------------------------------------------
{
    utf8   topic  = utf8("Overview");
    size_t length = 0;

    if (rt.depth())
    {
        if (object_p top = rt.top())
        {
            if (text_p index = top->as<text>())
            {
                utf8 what = index->value(&length);
                if (length)
                    topic = what;
            }
            else if (symbol_p index = top->as<symbol>())
            {
                topic = index->value(&length);
            }
            else
            {
                topic = top->help();
            }
        }
    }

    ui.load_help(topic, length);
    return OK;
}


COMMAND_BODY(Cycle)
// ----------------------------------------------------------------------------
//  Cycle object across multiple representations
// ----------------------------------------------------------------------------
{
    if (object_p top = rt.top())
    {
        id     cmd   = ID_object;
        id     type  = ID_object;
        id     ttype = top->type();
        switch(ttype)
        {
        case ID_hwfloat:
        case ID_hwdouble:
        case ID_decimal:
        case ID_neg_decimal:            cmd = ID_ToFraction; break;
        case ID_integer:
        case ID_bignum:
        case ID_neg_integer:
        case ID_neg_bignum:
        case ID_fraction:
        case ID_neg_fraction:
        case ID_big_fraction:
        case ID_neg_big_fraction:       cmd = ID_ToDecimal;     break;
        case ID_polar:                  cmd = ID_ToRectangular; break;
        case ID_rectangular:            cmd = ID_ToPolar;       break;
#if CONFIG_FIXED_BASED_OBJECTS
        case ID_based_integer:          type = ID_hex_integer;  break;
        case ID_hex_integer:            type = ID_dec_integer;  break;
        case ID_dec_integer:            type = ID_oct_integer;  break;
        case ID_oct_integer:            type = ID_bin_integer;  break;
        case ID_bin_integer:            type = ID_based_integer;break;

        case ID_based_bignum:           type = ID_hex_bignum;   break;
        case ID_hex_bignum:             type = ID_dec_bignum;   break;
        case ID_dec_bignum:             type = ID_oct_bignum;   break;
        case ID_oct_bignum:             type = ID_bin_bignum;   break;
        case ID_bin_bignum:             type = ID_based_bignum; break;
#else // ! CONFIG_FIXED_BASED_OBJECTS
        case ID_based_integer:
        case ID_based_bignum:
            switch(Settings.Base())
            {
            default:
            case 2:                     Settings.Base(16);     return OK;
            case 8:                     Settings.Base(2);      return OK;
            case 10:                    Settings.Base(8);      return OK;
            case 16:                    Settings.Base(10);     return OK;
            }
            break;
#endif // CONFIG_FIXED_BASED_OBJECTS
        case ID_expression:
            Settings.GraphicResultDisplay(!Settings.GraphicResultDisplay());
            if (!Settings.GraphicResultDisplay())
                Settings.GraphicStackDisplay(!Settings.GraphicStackDisplay());
            if (0)
                ui.draw_message("Stack rendering",
                                Settings.GraphicStackDisplay()
                                ? "Stack shown graphically"
                                : "Stack shown as text",
                                Settings.GraphicResultDisplay()
                                ? "Results shown graphically"
                                : "Results shown as text");
            break;
        case ID_list:                   type = ID_array;        break;
        case ID_array:                  type = ID_program;      break;
        case ID_program:                type = ID_list;         break;
        case ID_symbol:                 type = ID_text;         break;
        case ID_text:                   type = ID_symbol;       break;
        case ID_tag:                    cmd  = ID_dtag;         break;
        case ID_unit:
        {
            // Cycle prefix
            unit_p uobj = unit_p(top);
            uobj = uobj->cycle();
            if (uobj && rt.top(uobj))
                return OK;
            return ERROR;
        }

        default:
            rt.type_error();
            return ERROR;
        }

        // In-place retyping
        if (type != ID_object)
        {
            ASSERT(leb128size(type) == leb128size(top->type()));
            if (object_p clone = rt.clone(top))
            {
                byte *p = (byte *) clone;
                leb128(p, type);
                if (rt.top(clone))
                    return OK;
            }
            return ERROR;
        }

        // Evaluation of a command
        if (cmd != ID_object)
            return command::static_object(cmd)->evaluate();

        return OK;
    }
    return ERROR;
}


COMMAND_BODY(BinaryToReal)
// ----------------------------------------------------------------------------
//    Convert binary values to real (really integer)
// ----------------------------------------------------------------------------
{
    if (object_p top = rt.top())
    {
        id type = top->type();
        id to = ID_object;

        switch (type)
        {
#if CONFIG_FIXED_BASED_OBJECTS
        case ID_hex_integer:
        case ID_dec_integer:
        case ID_oct_integer:
        case ID_bin_integer:
            to = ID_integer;
            break;
        case ID_hex_bignum:
        case ID_dec_bignum:
        case ID_oct_bignum:
        case ID_bin_bignum:
            to = ID_bignum;
            break;
#endif // CONFIG_FIXED_BASED_OBJECTS
        case ID_based_integer:
            to = ID_integer;
            break;
        case ID_based_bignum:
            to = ID_bignum;
            break;
        default:
            rt.type_error();
            return ERROR;
        }

        ASSERT(leb128size(type) == leb128size(to));
        if (object_p clone = rt.clone(top))
        {
            byte *p = (byte *) clone;
            leb128(p, to);
            if (rt.top(clone))
                return OK;
        }
        return ERROR;

    }
    return ERROR;
}


COMMAND_BODY(RealToBinary)
// ----------------------------------------------------------------------------
//    Convert real and integer values to binary
// ----------------------------------------------------------------------------
{
    if (object_p top = rt.top())
    {
        id   type = top->type();
        id   to   = ID_object;
        bool neg  = type == ID_neg_integer || type == ID_neg_bignum;

        switch (type)
        {
        case ID_neg_integer:
        case ID_integer:
            to = ID_based_integer;
            break;
        case ID_neg_bignum:
        case ID_bignum:
            to = ID_based_bignum;
            break;
        default:
            rt.type_error();
            return ERROR;
        }

        ASSERT(leb128size(type) == leb128size(to));
        if (object_p clone = rt.clone(top))
        {
            byte *p = (byte *) clone;
            leb128(p, to);
            if (rt.top(clone))
            {
                if (neg)
                    return neg::evaluate();
                return OK;
            }
        }
        return ERROR;

    }
    return ERROR;
}



// ============================================================================
//
//   History and undo
//
// ============================================================================

COMMAND_BODY(LastArg)
// ----------------------------------------------------------------------------
//   Return the last arguments
// ----------------------------------------------------------------------------
{
    return rt.last() ? OK : ERROR;
}


COMMAND_BODY(LastX)
// ----------------------------------------------------------------------------
//   Return the last first argument
// ----------------------------------------------------------------------------
{
    return rt.last(0) ? OK : ERROR;
}


COMMAND_BODY(Undo)
// ----------------------------------------------------------------------------
//   Return the undo stack
// ----------------------------------------------------------------------------
{
    return rt.undo() ? OK : ERROR;
}



// ============================================================================
//
//   Editor commands
//
// ============================================================================

COMMAND_BODY(EditorSelect)
// ----------------------------------------------------------------------------
//   Select current cursor position
// ----------------------------------------------------------------------------
{
    return ui.editor_select() ? OK : ERROR;
}


COMMAND_BODY(EditorWordLeft)
// ----------------------------------------------------------------------------
//   Move cursor one word to the left
// ----------------------------------------------------------------------------
{
    return ui.editor_word_left() ? OK : ERROR;
}


COMMAND_BODY(EditorWordRight)
// ----------------------------------------------------------------------------
//   Move cursor one word to the right
// ----------------------------------------------------------------------------
{
    return ui.editor_word_right() ? OK : ERROR;
}


COMMAND_BODY(EditorBegin)
// ----------------------------------------------------------------------------
//   Move cursor to beginning of editor
// ----------------------------------------------------------------------------
{
    return ui.editor_begin() ? OK : ERROR;
}


COMMAND_BODY(EditorEnd)
// ----------------------------------------------------------------------------
//   Move cursor to end of editor
// ----------------------------------------------------------------------------
{
    return ui.editor_end() ? OK : ERROR;
}


COMMAND_BODY(EditorCut)
// ----------------------------------------------------------------------------
//   Cut selection
// ----------------------------------------------------------------------------
{
    return ui.editor_cut() ? OK : ERROR;
}


COMMAND_BODY(EditorCopy)
// ----------------------------------------------------------------------------
//   Copy selection
// ----------------------------------------------------------------------------
{
    return ui.editor_copy() ? OK : ERROR;
}


COMMAND_BODY(EditorPaste)
// ----------------------------------------------------------------------------
//   Paste selection
// ----------------------------------------------------------------------------
{
    return ui.editor_paste() ? OK : ERROR;
}


COMMAND_BODY(EditorSearch)
// ----------------------------------------------------------------------------
//   Search selection
// ----------------------------------------------------------------------------
{
    return ui.editor_search() ? OK : ERROR;
}


COMMAND_BODY(EditorReplace)
// ----------------------------------------------------------------------------
//   Replace selection
// ----------------------------------------------------------------------------
{
    return ui.editor_replace() ? OK : ERROR;
}


COMMAND_BODY(EditorClear)
// ----------------------------------------------------------------------------
//   Clear selection
// ----------------------------------------------------------------------------
{
    return ui.editor_clear() ? OK : ERROR;
}


COMMAND_BODY(EditorFlip)
// ----------------------------------------------------------------------------
//   Flip selection point and cursor
// ----------------------------------------------------------------------------
{
    return ui.editor_selection_flip() ? OK : ERROR;
}
