
#ifndef ARRAY_H
#define ARRAY_H
// ****************************************************************************
//  array.h                                                       DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Implementation of arrays (vectors, matrices and maybe tensors)
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

#include "list.h"
#include "runtime.h"


GCP(array);

struct array : list
// ----------------------------------------------------------------------------
//   An array is a list with [ and ] as delimiters
// ----------------------------------------------------------------------------
{
    array(id type, gcbytes bytes, size_t len): list(type, bytes, len) {}

    static array_g map(algebraic_fn fn, array_r x)
    {
        return x->map(fn);
    }

    array_p map(algebraic_fn fn) const
    {
        return array_p(list::map(fn));
    }

    array_p map(arithmetic_fn fn, algebraic_r y) const
    {
        return array_p(list::map(fn, y));
    }

    array_p map(algebraic_r x, arithmetic_fn fn) const
    {
        return array_p(list::map(x, fn));
    }

    // Append data
    array_p append(array_p a) const     { return array_p(list::append(a)); }
    array_p append(object_p o) const    { return array_p(list::append(o)); }
    static array_p wrap(object_p o);

    // Check if vector or matrix, and push all elements on stack
    bool is_vector(size_t *size, bool push = true) const;
    bool is_matrix(size_t *rows, size_t *columns, bool push = true) const;
    list_p dimensions(bool expand = false) const;
    bool expand() const;

    // Compute the result at row r column c from stack-exploded input
    typedef algebraic_g (*vector_fn)(size_t c, size_t cx, size_t cy);
    typedef algebraic_g (*matrix_fn)(size_t r, size_t c,
                                     size_t rx, size_t cx,
                                     size_t ry, size_t cy);
    typedef bool (*dimension_fn)(size_t rx, size_t cx, size_t ry, size_t cy,
                                 size_t *rr, size_t *cr);
    static array_g      do_matrix(array_r x, array_r y,
                                  dimension_fn d, vector_fn v, matrix_fn m);

    algebraic_g         determinant() const;
    algebraic_g         norm_square() const;
    algebraic_g         norm() const;
    array_g             invert() const;

  public:
    OBJECT_DECL(array);
    PARSE_DECL(array);
    RENDER_DECL(array);
    GRAPH_DECL(array);
    HELP_DECL(array);
};


array_g operator-(array_r x);
array_g operator+(array_r x, array_r y);
array_g operator-(array_r x, array_r y);
array_g operator*(array_r x, array_r y);
array_g operator/(array_r x, array_r y);

COMMAND_DECLARE(det,1);

#endif // ARRAY_H
