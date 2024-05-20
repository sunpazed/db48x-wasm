// ****************************************************************************
//  comment.cc                                                    DB48X project
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

#include "comment.h"

#include "parser.h"
#include "renderer.h"


PARSE_BODY(comment)
// ----------------------------------------------------------------------------
//   Try to parse this as a comment
// ----------------------------------------------------------------------------
{
    utf8 source = p.source;
    utf8 s      = source;
    char open   = *s++;
    if (open != '@')
        return SKIP;

    bool remove = *s == '@';
    utf8 end = source + p.length;
    s += remove;

    while (s < end && *s != '\n' && (*s != '@' || (remove && s[-1] != '@')))
        s++;

    size_t parsed = s - source;
    size_t slen   = parsed - 1;
    gcutf8 txt    = source + 1;
    p.end         = parsed;
    if (!remove)
        p.out = rt.make<text>(ID_comment, txt, slen);

    return remove ? COMMENTED : OK;
}


RENDER_BODY(comment)
// ----------------------------------------------------------------------------
//   Render a comment by simply emitting the text as is
// ----------------------------------------------------------------------------
{
    size_t  len = 0;
    utf8 txt = o->value(&len);
    r.wantCR();
    settings::SaveEditorWrapColumn sew(0);
    r.put('@');
    r.put(txt, len);
    r.wantCR();
    return r.size();
}


EVAL_BODY(comment)
// ----------------------------------------------------------------------------
//   A comment is a no-operation
// ----------------------------------------------------------------------------
{
    return OK;
}
