// ****************************************************************************
//  symbol.cc                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Implementation of RPL symbols
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

#include "symbol.h"

#include "algebraic.h"
#include "command.h"
#include "constants.h"
#include "equations.h"
#include "library.h"
#include "expression.h"
#include "grob.h"
#include "parser.h"
#include "renderer.h"
#include "runtime.h"
#include "unit.h"
#include "utf8.h"
#include "variables.h"

#include <stdio.h>


EVAL_BODY(symbol)
// ----------------------------------------------------------------------------
//   Evaluate a symbol by looking it up
// ----------------------------------------------------------------------------
{
    if (object_p found = directory::recall_all(o, false))
        return program::run_program(found);
    if (unit::mode)
        if (unit_p u = unit::lookup(o))
            if (rt.push(u))
                return OK;
    if (object_g eq = expression::make(o))
        if (rt.push(eq))
            return OK;
    return ERROR;
}


PARSE_BODY(symbol)
// ----------------------------------------------------------------------------
//    Try to parse this as a symbol
// ----------------------------------------------------------------------------
{
    utf8    source = p.source;
    size_t  max    = p.length;
    size_t  parsed = 0;

    // First character must be alphabetic
    unicode cp = utf8_codepoint(source);
    if (!is_valid_as_name_initial(cp))
        return SKIP;
    parsed = utf8_next(source, parsed, max);

    // Other characters must be alphabetic
    while (parsed < max && is_valid_in_name(source + parsed))
        parsed = utf8_next(source, parsed, max);

    // Build the resulting symbol
    gcutf8 text   = source;
    p.end         = parsed;
    p.out = rt.make<symbol>(ID_symbol, text, parsed);

    return OK;
}


RENDER_BODY(symbol)
// ----------------------------------------------------------------------------
//   Render the symbol into the given symbol buffer
// ----------------------------------------------------------------------------
{
    size_t len    = 0;
    utf8   txt    = o->value(&len);
    auto   format = r.editing() ? ID_LongFormNames : Settings.NameDisplayMode();
    r.put(format, txt, len);
    return r.size();
}


symbol_g operator+(symbol_r x, symbol_r y)
// ----------------------------------------------------------------------------
//   Concatenate the text in two symbols
// ----------------------------------------------------------------------------
{
    if (!x)
        return y;
    if (!y)
        return x;
    size_t sx = 0, sy = 0;
    utf8 tx = x->value(&sx);
    utf8 ty = y->value(&sy);
    symbol_g concat = rt.make<symbol>(symbol::ID_symbol, tx, sx + sy);
    if (concat)
    {
        utf8 tc = concat->value();
        memcpy((byte *) tc + sx, (byte *) ty, sy);
    }
    return concat;
}


object_p symbol::recall(bool noerror) const
// ----------------------------------------------------------------------------
//   Recall the value associated with the symbol
// ----------------------------------------------------------------------------
{
    directory *dir = rt.variables(0);
    if (object_p found = dir->recall(this))
        return found;
    return noerror ? this : nullptr;
}


bool symbol::store(object_g value) const
// ----------------------------------------------------------------------------
//   Store something in the value associated with the symbol
// ----------------------------------------------------------------------------
{
    directory *dir = rt.variables(0);
    object_g name = this;
    return dir->store(name, value);
}


bool symbol::is_same_as(symbol_p other) const
// ----------------------------------------------------------------------------
//   Return true of two symbols represent the same thing
// ----------------------------------------------------------------------------
{
    size_t sz, osz;
    utf8 txt = value(&sz);
    utf8 otxt = other->value(&osz);
    if (sz != osz)
        return false;
    return compare(txt, otxt, sz) == 0;
}


bool symbol::matches(utf8 otxt, size_t osz) const
// ----------------------------------------------------------------------------
//   Check if the symbol matches the prefix
// ----------------------------------------------------------------------------
{
    size_t sz;
    utf8 txt = value(&sz);
    if (sz != osz)
        return false;
    return compare(txt, otxt, sz) == 0;
}


bool symbol::starts_with(utf8 otxt, size_t osz) const
// ----------------------------------------------------------------------------
//   Check if the symbol begins with the given symbols
// ----------------------------------------------------------------------------
{
    size_t sz;
    utf8 txt = value(&sz);
    if (sz < osz)
        return false;
    return compare(txt, otxt, osz) == 0;
}


int symbol::compare(utf8 x, utf8 y, size_t len)
// ----------------------------------------------------------------------------
//   Compare two names, ignoring case or not depending on settings
// ----------------------------------------------------------------------------
{
    return Settings.IgnoreSymbolCase()
        ? strncasecmp(cstring(x), cstring(y), len)
        : strncmp(cstring(x), cstring(y), len);
}


GRAPH_BODY(symbol)
// ----------------------------------------------------------------------------
//   Render the symbol as italics
// ----------------------------------------------------------------------------
{
    using pixsize = grob::pixsize;

    grob_g sym = object::do_graph(o, g);
    if (!sym)
        return nullptr;

    pixsize sw    = sym->width();
    pixsize sh    = sym->height();
    uint    slant = 8;
    pixsize xw    = (sh + (slant - 1)) / slant;
    pixsize rw    = sw + xw;
    pixsize rh    = sh;
    grob_g result = g.grob(rw, rh);
    if (!result)
        return nullptr;

    grob::surface ss = sym->pixels();
    grob::surface rs = result->pixels();

    rs.fill(0, 0, rw, rh, g.background);
    for (coord y = 0; y < coord(rh); y++)
    {
        coord x = xw - y / slant;
        rs.copy(ss, rect(x, y, x + sw - 1, y), point(0, y));
    }

    return result;

}
