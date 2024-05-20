// ****************************************************************************
//  catalog.cc                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Auto-completion for commands (Catalog)
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

#include "catalog.h"

#include "characters.h"
#include "runtime.h"
#include "user_interface.h"

#include <stdlib.h>
#include <string.h>


RECORDER(catalog_error, 16, "Errors building the catalog");


MENU_BODY(Catalog)
// ----------------------------------------------------------------------------
//   Process the MENU command for Catalog
// ----------------------------------------------------------------------------
{
    if (ui.editing_mode() == ui.TEXT)
    {
        // Character catalog
        character_menu::build_at_cursor(mi);
    }
    else
    {
        // Command catalog
        uint  nitems = count_commands();
        items_init(mi, nitems);
        ui.menu_auto_complete();
        list_commands(mi);
        if (mi.page >= mi.pages)
            mi.page = 0;
    }
    return OK;
}


static uint16_t *sorted_ids       = nullptr;
static size_t    sorted_ids_count = 0;


#ifdef DEOPTIMIZE_CATALOG
// This is necessary on the DM32, where otherwise we access memory too
// fast and end up with bad data in the sorted array
#  pragma GCC push_options
#  pragma GCC optimize("-O2")
#endif // DEOPTIMIZE_CATALOG

static int sort_ids(const void *left, const void *right)
// ----------------------------------------------------------------------------
//   Sort the IDs alphabetically based on their fancy name
// ----------------------------------------------------------------------------
{
    uint16_t l = *((uint16_t *) left);
    uint16_t r = *((uint16_t *) right);
    if (!object::spellings[l].name || !object::spellings[r].name)
        return !!object::spellings[l].name - !!object::spellings[r].name;
    return strcasecmp(object::spellings[l].name, object::spellings[r].name);
}


static void initialize_sorted_ids()
// ----------------------------------------------------------------------------
//   Sort IDs alphabetically
// ----------------------------------------------------------------------------
{
    // Count number of items to put in the list
    uint count = 0;
    for (uint i = 0; i < object::spelling_count; i++)
        if (object::id ty = object::spellings[i].type)
            if (object::is_command(ty))
                if (object::spellings[i].name)
                    count++;

    sorted_ids = (uint16_t *) realloc(sorted_ids, count * sizeof(uint16_t));
    if (sorted_ids)
    {
        uint cmd = 0;
        for (uint i = 0; i < object::spelling_count; i++)
            if (object::id ty = object::spellings[i].type)
                if (object::is_command(ty))
                    if (object::spellings[i].name)
                        sorted_ids[cmd++] = i;
        qsort(sorted_ids, count, sizeof(sorted_ids[0]), sort_ids);

        // Make sure we have unique commands in the catalog
        cstring spelling = nullptr;
        cmd = 0;
        for (uint i = 0; i < count; i++)
        {
            uint16_t j = sorted_ids[i];
            auto &s = object::spellings[j];

            if (object::is_command(s.type))
            {
                if (cstring sp = s.name)
                {
                    if (!spelling ||
                        (spelling != sp && strcasecmp(sp, spelling) != 0))
                    {
                        sorted_ids[cmd++] = sorted_ids[i];
                        spelling = sp;
                    }
                    else if (cmd)
                    {
                        uint c = sorted_ids[cmd - 1];
                        auto &last = object::spellings[c];
                        if (s.type != last.type)
                        {
                            record(catalog_error,
                                   "Types %u and %u have same spelling "
                                   "%+s and %+s",
                                   s.type, last.type, spelling, sp);
                        }
                    }
                }
            }
            else
            {
                // Do not remove this code
                // It seems useless, but without it, the catalog is
                // badly broken on DM42. Apparently, the loop is a bit
                // too fast, and we end up adding a varying, but too small,
                // number of commands to the array
                debug_printf(5, "Not a command for %u, type %u[%s]",
                             i, s.type, object::name(s.type));
                debug_wait(-1);
            }
        }
        sorted_ids_count = cmd;
    }
}

#ifdef DEOPTIMIZE_CATALOG
#pragma GCC pop_options
#endif // DEOPTIMIZE_CATALOG


static bool matches(utf8 start, size_t size, utf8 name)
// ----------------------------------------------------------------------------
//   Check if what was typed matches the name
// ----------------------------------------------------------------------------
{
    size_t len   = strlen(cstring(name));
    bool   found = false;
    for (uint o = 0; !found && o + size <= len; o++)
    {
        found = true;
        for (uint i = 0; found && i < size; i++)
            found = tolower(start[i]) == tolower(name[i + o]);
    }
    return found;
}


uint Catalog::count_commands()
// ----------------------------------------------------------------------------
//    Count the commands to display in the catalog
// ----------------------------------------------------------------------------
{
    utf8   start  = 0;
    size_t size   = 0;
    bool   filter = ui.current_word(start, size);
    uint   count  = 0;

    for (size_t i = 0; i < spelling_count; i++)
    {
        object::id ty = object::spellings[i].type;
        if (!object::is_command(ty))
            continue;

        if (cstring name = spellings[i].name)
            if (!filter || matches(start, size, utf8(name)))
                count++;
    }

    return count;
}


void Catalog::list_commands(info &mi)
// ----------------------------------------------------------------------------
//   Fill the menu with all possible spellings of the command
// ----------------------------------------------------------------------------
{
    utf8   start  = nullptr;
    size_t size   = 0;
    bool   filter = ui.current_word(start, size);

    if (!sorted_ids)
        initialize_sorted_ids();

    if (sorted_ids)
    {
        for (size_t i = 0; i < sorted_ids_count; i++)
        {
            uint16_t j = sorted_ids[i];
            auto &s = object::spellings[j];
            id ty  = s.type;
            if (cstring name = s.name)
                if (!filter || matches(start, size, utf8(name)))
                    menu::items(mi, name, command::static_object(ty));
        }
    }
    else
    {
        // Fallback if we did not have enough memory for sorted_ids
        for (size_t i = 0; i < spelling_count; i++)
        {
            object::id ty = object::spellings[i].type;
            if (object::is_command(ty))
                if (cstring name = spellings[i].name)
                    if (!filter || matches(start, size, utf8(name)))
                        menu::items(mi, name, command::static_object(ty));
        }
    }
}
