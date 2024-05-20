// ****************************************************************************
//  solve.cc                                                      DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Numerical root finder
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2023 Christophe de Dinechin <christophe@dinechin.org>
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

#include "solve.h"

#include "algebraic.h"
#include "arithmetic.h"
#include "compare.h"
#include "equations.h"
#include "expression.h"
#include "functions.h"
#include "integer.h"
#include "recorder.h"
#include "settings.h"
#include "symbol.h"
#include "tag.h"

RECORDER(solve,         16, "Numerical solver");
RECORDER(solve_error,   16, "Numerical solver errors");


COMMAND_BODY(Root)
// ----------------------------------------------------------------------------
//   Numerical solver
// ----------------------------------------------------------------------------
{
    object_g eqobj    = rt.stack(2);
    object_g variable = rt.stack(1);
    object_g guess    = rt.stack(0);
    if (!eqobj || !variable || !guess)
        return ERROR;

    record(solve,
           "Solving %t for variable %t with guess %t",
           +eqobj, +variable, +guess);

    // Check that we have a variable name on stack level 1 and
    // a proram or equation on level 2
    symbol_g name = variable->as_quoted<symbol>();
    id eqty = eqobj->type();
    if (eqty == ID_equation)
    {
        eqobj = equation_p(+eqobj)->value();
        if (!eqobj)
            return ERROR;
        eqty = eqobj->type();
    }
    if (eqty != ID_program && eqty != ID_expression)
        name = nullptr;
    if (!name)
    {
        rt.type_error();
        return ERROR;
    }
    if (eqty == ID_expression)
        eqobj = expression_p(+eqobj)->as_difference_for_solve();

    // Drop input parameters
    rt.drop(3);

    if (!eqobj->is_program())
    {
        rt.invalid_equation_error();
        return ERROR;
    }

    // Actual solving
    program_g eq = program_p(+eqobj);
    if (algebraic_g x = solve(eq, name, guess))
    {
        size_t nlen = 0;
        gcutf8 ntxt = name->value(&nlen);
        object_g top = tag::make(ntxt, nlen, +x);
        if (rt.push(top))
            return rt.error() ? ERROR : OK;
    }

    return ERROR;
}



algebraic_p solve(program_g eq, symbol_g name, object_g guess)
// ----------------------------------------------------------------------------
//   The core of the solver
// ----------------------------------------------------------------------------
{
    // Check if the guess is an algebraic or if we need to extract one
    algebraic_g x, dx, lx, hx;
    algebraic_g y, dy, ly, hy;
    object::id gty = guess->type();
    if (object::is_real(gty) || object::is_complex(gty))
    {
        lx = algebraic_p(+guess);
        hx = algebraic_p(+guess);
        y = integer::make(1000);
        hx = hx->is_zero() ? inv::run(y) : hx + hx / y;
    }
    else if (gty == object::ID_list || gty == object::ID_array)
    {
        lx = guess->algebraic_child(0);
        hx = guess->algebraic_child(1);
        if (!lx || !hx)
            return nullptr;
    }
    x = lx;
    record(solve, "Initial range %t-%t", +lx, +hx);

    // We will run programs, do not save stack, etc.
    settings::PrepareForProgramEvaluation wilLRunPrograms;

    // Set independent variable
    save<symbol_g *> iref(expression::independent, &name);
    int              prec = Settings.SolverPrecision();
    algebraic_g      eps = decimal::make(1, -prec);

    bool is_constant = true;
    bool is_valid = false;
    uint max = Settings.SolverIterations();
    for (uint i = 0; i < max && !program::interrupted(); i++)
    {
        bool           jitter = false;

        // Evaluate equation
        y = algebraic::evaluate_function(eq, x);
        record(solve, "[%u] x=%t y=%t", i, +x, +y);
        if (!y)
        {
            // Error on last function evaluation, try again
            record(solve_error, "Got error %+s", rt.error());
            if (!ly || !hy)
            {
                rt.bad_guess_error();
                return nullptr;
            }
            jitter = true;
        }
        else
        {
            is_valid = true;
            if (y->is_zero() || smaller_magnitude(y, eps))
            {
                record(solve, "[%u] Solution=%t value=%t",
                       i, +x, +y);
                return x;
            }

            if (!ly)
            {
                record(solve, "Setting low");
                ly = y;
                lx = x;
                x = hx;
                continue;
            }
            else if (!hy)
            {
                record(solve, "Setting high");
                hy = y;
                hx = x;
            }
            else if (smaller_magnitude(y, ly))
            {
                // Smaller than the smallest
                record(solve, "Smallest");
                hx = lx;
                hy = ly;
                lx = x;
                ly = y;
            }
            else if (smaller_magnitude(y, hy))
            {
                record(solve, "Improvement");
                // Between smaller and biggest
                hx = x;
                hy = y;
            }
            else if (smaller_magnitude(hy, y))
            {
                // y became bigger, try to get closer to low
                bool crosses = (ly * hy)->is_negative(false);
                record(solve, "New value is worse");
                is_constant = false;

                // Try to bisect
                dx = integer::make(2);
                x = (lx + x) / dx;
                if (!x)
                    return nullptr;
                if (crosses)    // For positive and negative values, as is
                    continue;

                // Otherwise, try to jitter around
                jitter = true;
            }
            else
            {
                // y is constant - Try a random spot
                record(solve, "Unmoving");
                jitter = true;
            }

            if (!jitter)
            {
                dx = hx - lx;
                if (!dx)
                    return nullptr;
                if (dx->is_zero() ||
                    smaller_magnitude(abs::run(dx) /
                                      (abs::run(hx) + abs::run(lx)), eps))
                {
                    x = lx;
                    if ((ly * hy)->is_negative(false))
                    {
                        record(solve, "[%u] Cross solution=%t value=%t",
                               i, +x, +y);
                    }
                    else
                    {
                        record(solve, "[%u] Minimum=%t value=%t",
                               i, +x, +y);
                        rt.no_solution_error();
                    }
                    return x;
                }

                dy = hy - ly;
                if (!dy)
                    return nullptr;
                if (dy->is_zero())
                {
                    record(solve,
                           "[%u] unmoving %t between %t and %t",
                           +hy, +lx, +hx);
                    jitter = true;
                }
                else
                {
                    record(solve, "[%u] Moving to %t - %t / %t",
                           i, +lx, +dy, +dx);
                    is_constant = false;
                    x = lx - y * dx / dy;
                }
            }

            // Check if there are unresolved symbols
            if (x->is_symbolic())
            {
                rt.invalid_function_error();
                return x;
            }

            // If we are starting to use really big numbers, approximate
            if (!algebraic::to_decimal_if_big(x))
                return x;
        }

        // If we have some issue improving things, shake it a bit
        if (jitter)
        {
            int s = (i & 2)- 1;
            if (x->is_complex())
                dx = polar::make(integer::make(997 * s * i),
                                 integer::make(421 * s * i * i),
                                 object::ID_Deg);
            else
                dx = integer::make(0x1081 * s * i);
            dx = dx * eps;
            if (x->is_zero())
                x = dx;
            else
                x = x + x * dx;
            if (!x)
                return nullptr;
            record(solve, "Jitter x=%t", +x);
        }
    }

    record(solve, "Exited after too many loops, x=%t y=%t lx=%t ly=%t",
           +x, +y, +lx, +ly);

    if (!is_valid)
        rt.invalid_function_error();
    else if (is_constant)
        rt.constant_value_error();
    else
        rt.no_solution_error();
    return lx;
}
