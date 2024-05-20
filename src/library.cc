// ****************************************************************************
//  library.cc                                                  DB48X project
// ****************************************************************************
//
//   File Description:
//
//    Implementation of the Library, defined by the config/library.csv file
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2024 Christophe de Dinechin <christophe@dinechin.org>
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

#include "library.h"

#include "program.h"
#include "renderer.h"


RECORDER(library,         16, "Xlib objects");
RECORDER(library_error,   16, "Error on xlib objects");



// ============================================================================
//
//   Xlib definitions
//
// ============================================================================

static const cstring basic_library[] =
// ----------------------------------------------------------------------------
//   List of basic entries in the library
// ----------------------------------------------------------------------------
//   clang-format off
{

    // ------------------------------------------------------------------------
    //   Physics
    // ------------------------------------------------------------------------

    "Secrets",          nullptr,

    "Dedicace", "\"À tous ceux qui se souviennent de Maubert électronique\"",
    "LibraryHelp",  "\"To modify the library, edit the config/library.csv file\"",
};
//   clang-format on


static runtime &invalid_xlib_error()
// ----------------------------------------------------------------------------
//    Return the error message for invalid library
// ----------------------------------------------------------------------------
{
    return rt.invalid_xlib_error();
}


const xlib::config xlib::library =
// ----------------------------------------------------------------------------
//  Define the configuration for the library
// ----------------------------------------------------------------------------
{
    .menu_help      = "Library",
    .help           = "",
    .prefix         = L'Ⓛ',
    .type           = ID_xlib,
    .first_menu     = ID_LibraryMenu00,
    .last_menu      = ID_LibraryMenu99,
    .name           = ID_XlibName,
    .value          = ID_XlibValue,
    .file           = "config/library.csv",
    .builtins       = basic_library,
    .nbuiltins      = sizeof(basic_library) / sizeof(*basic_library),
    .error          = invalid_xlib_error
};



// ============================================================================
//
//   Menu implementation
//
// ============================================================================

PARSE_BODY(xlib)
// ----------------------------------------------------------------------------
//   Parse library entries
// ----------------------------------------------------------------------------
{
    return do_parsing(library, p);
}


EVAL_BODY(xlib)
// ----------------------------------------------------------------------------
//   Library entries evaluate like a program entry
// ----------------------------------------------------------------------------
{
    object_p value = o->value();
    if (!value)
    {
        rt.invalid_xlib_error();
        return ERROR;
    }
    if (program::running)
        return rt.push(value) ? OK : ERROR;
    return program::run_program(value);
}


RENDER_BODY(xlib)
// ----------------------------------------------------------------------------
//   Render the xlib into the given buffer
// ----------------------------------------------------------------------------
{
    return do_rendering(library, o, r);
}


GRAPH_BODY(xlib)
// ----------------------------------------------------------------------------
//   Render "normally"
// ----------------------------------------------------------------------------
{
    return object::do_graph(o, g);
}


HELP_BODY(xlib)
// ----------------------------------------------------------------------------
//   Help topic for libraries
// ----------------------------------------------------------------------------
{
    return o->do_instance_help(xlib::library);
}


MENU_BODY(library_menu)
// ----------------------------------------------------------------------------
//   Build a library menu
// ----------------------------------------------------------------------------
{
    return o->do_submenu(xlib::library, mi);
}


HELP_BODY(library_menu)
// ----------------------------------------------------------------------------
//   Show the help for the given library menu
// ----------------------------------------------------------------------------
{
    return o->do_menu_help(xlib::library, o);
}


MENU_BODY(Library)
// ----------------------------------------------------------------------------
//   The library menu is dynamically populated
// ----------------------------------------------------------------------------
{
    return xlib::do_collection_menu(xlib::library, mi);
}


utf8 library_menu::name(id type, size_t &len)
// ----------------------------------------------------------------------------
//   Return the name for a menu entry
// ----------------------------------------------------------------------------
{
    return do_name(xlib::library, type, len);
}


COMMAND_BODY(XlibName)
// ----------------------------------------------------------------------------
//   Put the name of a xlib on the stack
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (object_p cstobj = xlib::do_key(xlib::library, key))
        if (xlib_p cst = cstobj->as<xlib>())
            if (rt.push(cst))
                return OK;
    if (!rt.error())
        rt.type_error();
    return ERROR;
}


INSERT_BODY(XlibName)
// ----------------------------------------------------------------------------
//   Put the name of a xlib in the editor
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    return ui.insert_softkey(key, " Ⓛ", " ", false);
}


HELP_BODY(XlibName)
// ----------------------------------------------------------------------------
//   Put the help for a given xlib function key
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (object_p cstobj = xlib::do_key(xlib::library, key))
        if (xlib_p cst = cstobj->as<xlib>())
            return cst->help();
    return utf8("Library");
}


COMMAND_BODY(XlibValue)
// ----------------------------------------------------------------------------
//   Put the value of a xlib on the stack
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (constant_p cst = xlib::do_key(xlib::library, key))
        if (xlib_p xl = cst->as<xlib>())
            if (object_p value = xl->value())
                if (rt.push(value))
                    return OK;
    if (!rt.error())
        rt.type_error();
    return ERROR;
}


INSERT_BODY(XlibValue)
// ----------------------------------------------------------------------------
//   Insert the value of a xlib
// ----------------------------------------------------------------------------
{
    int key = ui.evaluating;
    if (object_p cstobj = xlib::do_key(xlib::library, key))
        if (xlib_p xl = cstobj->as<xlib>())
            if (object_p value = xl->value())
                return ui.insert_object(value, " ", " ");
    return ERROR;
}


HELP_BODY(XlibValue)
// ----------------------------------------------------------------------------
//   Put the help for a given library function ley
// ----------------------------------------------------------------------------
{
    return XlibName::do_help(nullptr);
}
