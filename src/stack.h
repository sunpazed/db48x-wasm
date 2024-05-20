#ifndef STACK_H
#define STACK_H
// ****************************************************************************
//  stack.h                                                       DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Rendering of objects on the stack
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

#include "object.h"

#include <string>

struct runtime;

struct stack
// ----------------------------------------------------------------------------
//   Rendering of the stack
// ----------------------------------------------------------------------------
{
    stack();

    void draw_stack();

#if SIMULATOR
public:
    struct data
    // ------------------------------------------------------------------------
    //   Record the output of the stack for testing purpose
    // ------------------------------------------------------------------------
    {
        int         key;
        object::id  type;
        std::string output;
    } history[8];

    enum { COUNT = sizeof(history) / sizeof(*history) };

    void output(int key, object::id type, utf8 stack0, size_t len)
    {
        data *ptr = history + writer % COUNT;
        std::string out = std::string(cstring(stack0), len);
        ptr->key = key;
        ptr->type = type;
        ptr->output = out;
        writer++;
    }

    uint available()
    {
        return writer - reader;
    }

    utf8 recorded()
    {
        if (reader >= writer)
            return nullptr;
        data *ptr = history + reader % (sizeof(history) / sizeof(*history));
        return utf8(ptr->output.c_str());
    }

    object::id type()
    {
        if (reader >= writer)
            return object::ID_object;
        data *ptr = history + reader % (sizeof(history) / sizeof(*history));
        return ptr->type;
    }

    int key()
    {
        if (reader >= writer)
            return -99999;
        data *ptr = history + reader % (sizeof(history) / sizeof(*history));
        return ptr->key;
    }

    void consume()
    {
        reader++;
    }

    void catch_up()
    {
        reader = writer;
    }

    uint writer;
    uint reader;
#endif
};

extern stack Stack;

#endif // STACK_H
