#ifndef CATALOG_H
#define CATALOG_H
// ****************************************************************************
//  catalog.h                                                  DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Auto-completion menu (Catalog)
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

#include "menu.h"
#include "command.h"

struct Catalog : menu
// ----------------------------------------------------------------------------
//   The catalog of functions, as shown by the 'Catalog' menu
// ----------------------------------------------------------------------------
{
    Catalog(id type = ID_Catalog): menu(type) {}

    static uint count_commands();
    static void list_commands(info &mi);

public:
    OBJECT_DECL(Catalog);
    MENU_DECL(Catalog);
};

#endif // CATALOG_H
