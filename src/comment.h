#ifndef COMMENT_H
#define COMMENT_H
// ****************************************************************************
//  comment.h                                                     DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Comments in the source code
//
//     Comments in the source code begin with `@` and end with a newline
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

#include "text.h"

struct comment : text
// ----------------------------------------------------------------------------
//    A comment from the command-line
// ----------------------------------------------------------------------------
{
    comment(id type, gcutf8 source, size_t len): text(type, source, len) {}

    OBJECT_DECL(comment);
    PARSE_DECL(comment);
    RENDER_DECL(comment);
    EVAL_DECL(comment);
};

#endif // COMMENT_H
