#ifndef EQUATIONS_H
#define EQUATIONS_H
// ****************************************************************************
//  equations.h                                                   DB48X project
// ****************************************************************************
//
//   File Description:
//
//    Representation of equations from the equations library
//    This is defined by the file `config/equations.csv'
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

#include "constants.h"

#include <string.h>

GCP(equation);

struct equation : constant
// ----------------------------------------------------------------------------
//  An equation stored in `config/equations.csv` file
// ----------------------------------------------------------------------------
{
    equation(id type, uint index): constant(type, index) {}

    static equation_p make(uint index)
    {
        return rt.make<equation>(ID_equation, index);
    }

    static equation_p make(id type, uint index)
    {
        return rt.make<equation>(type, index);
    }

    static equation_p lookup(utf8 name, size_t len, bool error)
    {
        return equation_p(do_lookup(equations, name, len, error));
    }

    static equation_p lookup(cstring name, bool error = true)
    {
        return lookup(utf8(name), strlen(name), error);
    }

    uint        index() const
    {
        byte_p p = payload();
        return leb128<uint>(p);
    }

    utf8        name(size_t *size = nullptr) const
    {
        return do_name(equations, size);
    }
    algebraic_p value() const
    {
        return do_value(equations);
    }

    static const config equations;
    OBJECT_DECL(equation);
    PARSE_DECL(equation);
    EVAL_DECL(equation);
    RENDER_DECL(equation);
    GRAPH_DECL(equation);
    HELP_DECL(equation);
};


struct equation_menu : constant_menu
// ----------------------------------------------------------------------------
//   A equation menu is like a standard menu, but with equations
// ----------------------------------------------------------------------------
{
    equation_menu(id type) : constant_menu(type) { }
    static utf8 name(id type, size_t &len);
    MENU_DECL(equation_menu);
    HELP_DECL(equation_menu);
};



#define ID(i)
#define EQUATION_MENU(EquationMenu)     struct EquationMenu : equation_menu {};
#include "tbl/ids.tbl"

COMMAND_DECLARE_INSERT_HELP(EquationName,-1);
COMMAND_DECLARE_INSERT_HELP(EquationValue,-1);

#endif // EQUATIONS_H
