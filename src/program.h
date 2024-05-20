#ifndef PROGRAM_H
#define PROGRAM_H
// ****************************************************************************
//  program.h                                                     DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Implementation of RPL programs and blocks
//
//     Programs are lists with a special way to execute
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
#include "recorder.h"

GCP(program);
GCP(block);
RECORDER_DECLARE(program);

struct program : list
// ----------------------------------------------------------------------------
//   A program is a list with « and » as delimiters
// ----------------------------------------------------------------------------
{
    program(id type, gcbytes bytes, size_t len): list(type, bytes, len) {}


    result run(bool synchronous = true) const;
    INLINE result run_program() const               { return run(false); }
    static result run(object_p obj, bool sync = true);
    INLINE static result run_program(object_p obj)  { return run(obj, false); }
    static result run_loop(size_t depth);

    static bool      interrupted(); // Program interrupted e.g. by EXIT key
    static program_p parse(utf8 source, size_t size);

    static bool running, halted;
    static uint stepping;

  public:
    OBJECT_DECL(program);
    PARSE_DECL(program);
    RENDER_DECL(program);
    EVAL_DECL(program);
};
typedef const program *program_p;


struct block : program
// ----------------------------------------------------------------------------
//   A block inside a program, e.g. in loops
// ----------------------------------------------------------------------------
{
    block(id type, gcbytes bytes, size_t len): program(type, bytes, len) {}

public:
    OBJECT_DECL(block);
    PARSE_DECL(block);
    RENDER_DECL(block);
    EVAL_DECL(block);
};


COMMAND_DECLARE(Halt,-1);
COMMAND_DECLARE(Debug,1);
COMMAND_DECLARE(SingleStep,-1);
COMMAND_DECLARE(StepOver,-1);
COMMAND_DECLARE(StepOut,-1);
COMMAND_DECLARE(MultipleSteps,1);
COMMAND_DECLARE(Continue,-1);
COMMAND_DECLARE(Kill,-1);

#endif // PROGRAM_H
