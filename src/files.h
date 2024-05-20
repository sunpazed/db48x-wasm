#ifndef FILES_H
#define FILES_H
// ****************************************************************************
//  files.h                                                       DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Operations on files
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

#include "array.h"
#include "list.h"
#include "text.h"

GCP(files);
GCP(grob);

struct files : text
// ----------------------------------------------------------------------------
//   Represents files at the given path location
// ----------------------------------------------------------------------------
{
    files(id type, gcutf8 source, size_t len): text(type, source, len) {}

    static files_p make(utf8 str, size_t len)
    {
        gcutf8 gcstr = str;
        return rt.make<files>(gcstr, len);
    }

    static files_p make(utf8 str)
    {
        return make(str, strlen(cstring(str)));
    }

    static files_p make(cstring str, size_t len)
    {
        return make(utf8(str), len);
    }

    static files_p make(cstring str)
    {
        return make(utf8(str), strlen(str));
    }

    // Store an object to disk
    bool     store(text_p name, object_p value, cstring ext = "48s") const;
    bool     store_binary(text_p name, object_p value) const;
    bool     store_source(text_p name, object_p value) const;
    bool     store_text(text_p name, text_p value) const;
    bool     store_list(text_p name, list_p value) const;
    bool     store_grob(text_p name, grob_p value) const;

    // Recall an object from disk
    object_p recall(text_p name, cstring ext = "48s") const;
    object_p recall_binary(text_p name) const;
    object_p recall_source(text_p name) const;
    text_p   recall_text(text_p name) const;
    list_p   recall_list(text_p name, bool as_array = false) const;
    grob_p   recall_grob(text_p name) const;

    // Purge (unlink) a file
    bool     purge(text_p name) const;

    // Build a file name from current path
    text_p   filename(text_p name, bool writing = false) const;
};

// Marker for valid binary files
#define DB48X_MAGIC     { 0xDB, 0x48, 0x17, 0x02 }
#define DB50X_MAGIC     { 0xDB, 0x50, 0x19, 0x69 }

#ifndef DM32
#  define FILE_MAGIC DB48X_MAGIC
#else
#  define FILE_MAGIC DB50X_MAGIC
#endif // DM32

#endif // FILES_H
