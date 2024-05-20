// ****************************************************************************
//  complex.cc                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//      Complex numbers
//
//      There are two representations for complex numbers:
//      - rectangular representation is one of X;Y, X+ⅈY, X-ⅈY, X+Yⅈ or X-Yⅈ
//      - polar representation is X∡Y, where X≥0 and Y is a ratio of π
//
//      Some settings control how complex numbers are rendered.
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

#include "complex.h"

#include "arithmetic.h"
#include "compare.h"
#include "functions.h"
#include "parser.h"
#include "renderer.h"
#include "runtime.h"
#include "tag.h"
#include "unit.h"


// ============================================================================
//
//   Generic complex operations
//
// ============================================================================
//
//  The generic operations optimize for the most efficient operation
//  if there is a difference between rectangular and polar
//

SIZE_BODY(complex)
// ----------------------------------------------------------------------------
//   Size of a complex number
// ----------------------------------------------------------------------------
{
    object_p p = object_p(payload(o));
    object_p e = p->skip()->skip();
    return byte_p(e) - byte_p(o);
}


HELP_BODY(complex)
// ----------------------------------------------------------------------------
//   Help topic for complex numbers
// ----------------------------------------------------------------------------
{
    return utf8("Complex numbers");
}


algebraic_g complex::re() const
// ----------------------------------------------------------------------------
//   Return real part in a format-independent way
// ----------------------------------------------------------------------------
{
    if (type() == ID_rectangular)
        return rectangular_p(this)->re();
    return polar_p(this)->re();
}


algebraic_g complex::im() const
// ----------------------------------------------------------------------------
//   Return imaginary part in a format-independent way
// ----------------------------------------------------------------------------
{
    if (type() == ID_rectangular)
        return rectangular_p(this)->im();
    return polar_p(this)->im();
}


algebraic_g complex::mod() const
// ----------------------------------------------------------------------------
//   Return modulus in a format-independant way
// ----------------------------------------------------------------------------
{
    if (type() == ID_polar)
        return polar_p(this)->mod();
    return rectangular_p(this)->mod();
}


algebraic_g complex::arg(complex::angle_unit unit) const
// ----------------------------------------------------------------------------
//   Return argument in a format-independant way
// ----------------------------------------------------------------------------
{
    if (type() == ID_polar)
        return polar_p(this)->arg(unit);
    return rectangular_p(this)->arg(unit);
}


algebraic_g complex::pifrac() const
// ----------------------------------------------------------------------------
//   Return argument as pi fraction in a format-independant way
// ----------------------------------------------------------------------------
{
    if (type() == ID_polar)
        return polar_p(this)->pifrac();
    return rectangular_p(this)->pifrac();
}


algebraic_p complex::is_real() const
// ----------------------------------------------------------------------------
//   Check if the complex is a purely real value
// ----------------------------------------------------------------------------
{
    if (type() == ID_polar)
        return polar_p(this)->is_real();
    return rectangular_p(this)->is_real();
}


complex_g complex::conjugate() const
// ----------------------------------------------------------------------------
//   Return complex conjugate in a format-independent way
// ----------------------------------------------------------------------------
{
    return make(type(), x(), -y(), ID_PiRadians);
}


complex_p complex::make(id type, algebraic_r x, algebraic_r y, angle_unit aunit)
// ----------------------------------------------------------------------------
//   Build a complex of the right type
// ----------------------------------------------------------------------------
{
    if (!x|| !y)
        return nullptr;
    if (type == ID_polar)
        return polar::make(x, y, aunit);
    else if (type == ID_unit)
        return unit::make(x, y);
    return rectangular::make(x, y);
}


rectangular_p complex::make(int re, int im)
// ----------------------------------------------------------------------------
//   Build a simple complex constant
// ----------------------------------------------------------------------------
{
    return rectangular_p(make(ID_rectangular,
                              integer::make(re), integer::make(im),
                              ID_PiRadians));
}


complex_g operator-(complex_r x)
// ----------------------------------------------------------------------------
//  Unary minus
// ----------------------------------------------------------------------------
{
    if (!x)
        return nullptr;
    if (x->type() == object::ID_polar)
    {
        polar_r p = polar_r(x);
        return polar::make(-p->mod(), p->pifrac(), object::ID_PiRadians);
    }
    rectangular_r r = rectangular_r(x);
    return rectangular::make(-r->re(), -r->im());
}


complex_g operator+(complex_r x, complex_r y)
// ----------------------------------------------------------------------------
//   Complex addition - In rectangular form, unless polar args are aligned
// ----------------------------------------------------------------------------
{
    if (!x|| !y)
        return nullptr;
    if (x->type() == object::ID_polar &&
        y->type() == object::ID_polar)
    {
        algebraic_g two = integer::make(2);
        algebraic_g angle_diff = (x->y() - y->y()) % two;
        if (angle_diff->is_zero(false))
            return polar::make(x->x() + y->x(), x->y(), object::ID_PiRadians);
        if (angle_diff->is_one(false))
            return polar::make(x->x() - y->x(), x->y(), object::ID_PiRadians);
    }
    return rectangular::make(x->re() + y->re(), x->im() + y->im());
}


complex_g operator-(complex_r x, complex_r y)
// ----------------------------------------------------------------------------
//   Complex subtraction - In rectangular form, unless polar args are aligned
// ----------------------------------------------------------------------------
{
    if (!x|| !y)
        return nullptr;
    if (x->is_zero())
        return -y;
    if (y->is_zero())
        return x;
    if (x->type() == object::ID_polar &&
        y->type() == object::ID_polar)
    {
        algebraic_g two = integer::make(2);
        algebraic_g angle_diff = (x->y() - y->y()) % two;
        if (angle_diff->is_zero(false))
            return polar::make(x->x() - y->x(), x->y(), object::ID_PiRadians);
        if (angle_diff->is_one(false))
            return polar::make(x->x() + y->x(), x->y(), object::ID_PiRadians);
    }
    return rectangular::make(x->re() - y->re(), x->im() - y->im());
}


complex_g operator*(complex_r x, complex_r y)
// ----------------------------------------------------------------------------
//   If both are in rectangular form, rectangular, otherwise polar
// ----------------------------------------------------------------------------
{
    if (!x|| !y)
        return nullptr;
    object::id xt = x->type();
    object::id yt = y->type();
    if (xt != object::ID_rectangular || yt != object::ID_rectangular)
        return polar::make(x->mod() * y->mod(),
                           x->pifrac() + y->pifrac(),
                           object::ID_PiRadians);

    rectangular_p xx = rectangular_p(complex_p(x));
    rectangular_p yy = rectangular_p(complex_p(y));
    algebraic_g xr = xx->re();
    algebraic_g xi = xx->im();
    algebraic_g yr = yy->re();
    algebraic_g yi = yy->im();
    return rectangular::make(xr*yr-xi*yi, xr*yi+xi*yr);
}


complex_g operator/(complex_r x, complex_r y)
// ----------------------------------------------------------------------------
//   Like for multiplication, it's slighly cheaper in polar form
// ----------------------------------------------------------------------------
{
    if (!x|| !y)
        return nullptr;
    object::id xt = x->type();
    object::id yt = y->type();
    if (xt != object::ID_rectangular || yt != object::ID_rectangular)
        return polar::make(x->mod() / y->mod(),
                           x->pifrac() - y->pifrac(),
                           object::ID_PiRadians);

    rectangular_p xx = rectangular_p(complex_p(x));
    rectangular_p yy = rectangular_p(complex_p(y));
    algebraic_g a = xx->re();
    algebraic_g b = xx->im();
    algebraic_g c = yy->re();
    algebraic_g d = yy->im();
    algebraic_g r = sq::run(c) + sq::run(d);
    return rectangular::make((a*c+b*d)/r, (b*c-a*d)/r);
}


polar_g complex::as_polar() const
// ----------------------------------------------------------------------------
//   Switch to polar form if preferred for computation
// ----------------------------------------------------------------------------
{
    if (type() == ID_rectangular)
    {
        rectangular_g r = rectangular_p(this);
        return polar::make(r->mod(), r->pifrac(), object::ID_PiRadians);
    }
    return polar_p(this);
}


rectangular_g complex::as_rectangular() const
// ----------------------------------------------------------------------------
//   Switch to rectangular form if preferred for computation
// ----------------------------------------------------------------------------
{
    if (type() == ID_polar)
    {
        polar_g r = polar_p(this);
        return rectangular::make(r->re(), r->im());
    }
    return rectangular_p(this);
}


PARSE_BODY(complex)
// ----------------------------------------------------------------------------
//   Parse the various forms of complex number
// ----------------------------------------------------------------------------
//   We accept the following formats:
//   a. (1;3)           Classic RPL
//   b. (1 3)           Classic RPL
//   c. 1ⅈ3             ⅈ as a separator
//   d. 1+ⅈ3            ⅈ as a prefix
//   e. 1-ⅈ3
//   f. 1+3ⅈ            ⅈ as a postfix
//   g. 1-3ⅈ
//   h. 1∡30            ∡ as a separator
//   i. ⅈ               Imaginary unit by itself
//   u. 1_km            _ as a separator for unit objects
//
//   Cases a-g generate a rectangular form, case i generates a polar form
//   Cases c-h can be surrounded by parentheses as well
//
//   In case (a), we do not accept (1,3) which classic RPL would accept,
//   because in DB48X 1,000.000 is a valid real number with thousands separator
{
    gcutf8      src    = p.source;
    size_t      max    = p.length;
    id          type   = ID_object;

    // Find the end of the possible complex number and check parentheses
    utf8        first  = src;
    utf8        last   = first;
    utf8        ybeg   = nullptr;
    size_t      xlen   = 0;
    size_t      ylen   = 0;
    uint        paren  = false;
    bool        signok = false;
    bool        ineq   = false;
    char        sign   = 0;
    unicode     angle  = 0;

    while (size_t(last - first) < max)
    {
        unicode cp = utf8_codepoint(last);

        // Check if we have an opening parenthese
        if (last == first && cp == '(')
        {
            paren = true;
            first++;
        }

        // Check if found a '+' or '-' (cases d-g)
        else if (signok && (cp == '+' || cp == '-'))
        {
            if (sign)
            {
                // Cannot have two signs
                return WARN;
            }
            sign = cp;
            ybeg = last + 1;
            if (type != ID_polar)
                xlen = last - first;
        }

        // Check if we have equations in our complex
        else if (cp == '\'')
        {
            if (p.precedence)
                break;
            ineq = !ineq;
        }
        else if (ineq)
        {
            // Skip the content of the equations
        }

        // Check if we found the ⅈ sign
        else if (cp == I_MARK)
        {
            // Can't have two complex signs
            if (type != ID_object)
            {
                rt.syntax_error().source(last);
                return WARN;
            }
            type = ID_rectangular;

            // Case of ⅈ as a separator (case c)
            if (!sign)
            {
                ybeg = last + utf8_size(cp);
                xlen = last - first;
            }
            // Case of prefix ⅈ (case d or e)
            else if (last == ybeg)
            {
                ybeg = last + utf8_size(cp);
            }
            // Case of postfix ⅈ (case f or g)
            else
            {
                ylen = last - ybeg;
            }
        }

        // Check if we found the ∡ sign
        else if (cp == ANGLE_MARK)
        {
            // Can't have two complex signs, or have that with a sign
            if (type != ID_object || sign)
            {
                rt.syntax_error().source(last);
                return WARN;
            }
            type = ID_polar;

            // Case of ∡ as a separator (case h)
            ybeg = last + utf8_size(cp);
            xlen = last - first;
        }

        // Check if we found the _ sign for units
        else if (cp == '_' || cp == settings::SPACE_UNIT)
        {
            // Can't have two complex signs, or have that with a sign
            if (type != ID_object || sign)
            {
                rt.syntax_error().source(last);
                return WARN;
            }
            type = ID_unit;

            // Case of ∡ as a separator (case h)
            ybeg = last + utf8_size(cp);
            xlen = last - first;
        }

        // Check parentheses inside units
        else if (type == ID_unit && cp == '(')
        {
            paren++;
        }
        else if (type == ID_unit && paren && cp == ')')
        {
            paren--;
        }

        // Check if we found a space or ';' inside parentheses
        else if (paren && (cp == ' ' || cp == ';'))
        {
            // Can't have two complex signs
            if (type != ID_object)
            {
                rt.syntax_error().source(last);
                return WARN;
            }
            type = ID_rectangular;
            ybeg = last + 1;
            xlen = last - first;
        }

        // Check if we found characters that we don't expect in a complex
        else if (cp == '"' || cp == '{' || cp == '[' || cp == L'«' || cp == ':')
        {
            return SKIP;
        }

        // Check if we have two parentheses
        else if (paren && cp == '(')
        {
            rt.syntax_error().source(last);
            return WARN;
        }

        // Check if we found the end of the complex number
        else if (cp == ' ' || cp == '\n' || cp == '\t' ||
                 cp == ')' || cp == '}'  || cp == ']'  || cp == L'»' ||
                 cp == '\'')
        {
            break;
        }

        // Check if we found an angle marker
        else if (cp == Settings.DEGREES_SYMBOL ||
                 cp == Settings.RADIANS_SYMBOL ||
                 cp == Settings.GRAD_SYMBOL    ||
                 cp == Settings.PI_RADIANS_SYMBOL)
        {
            // If parsing 1°_hms, need to accept degrees but not as angle marker
            bool deg_alone = (cp == Settings.DEGREES_SYMBOL &&
                              ((last != first && type == ID_object) ||
                               (last == ybeg && type == ID_unit)));

            // Just parsing π should be allowed
            if (!deg_alone)
            {
                if (last == first || type == ID_object)
                    return SKIP;

                if (angle || type != ID_polar)
                {
                    rt.syntax_error().source(last);
                    return WARN;
                }
                angle = cp;
                ylen = last - ybeg;
            }
        }

        // We can have a sign except after exponent markers
        signok = cp != 'e' && cp != 'E' && cp != L'⁳';

        // Loop on next characters
        last += utf8_size(cp);
    }

    // If we did not find the necessary structure, just skip
    if (type == ID_object || (!xlen && type != ID_rectangular) || !ybeg)
        return SKIP;

    // Check if we need to compute the length of y
    if (!ylen)
    {
        ylen = last - ybeg;
        if (!ylen && type != ID_rectangular)
        {
            rt.syntax_error().source(utf8(ybeg));
            return ERROR;
        }
    }

    // If we just have the imaginary unit, e.g. `3i`, `i3`, or just `i`.
    if (type == ID_rectangular && (!xlen || !ylen))
    {
        gcutf8 ysrc = ybeg;
        algebraic_g x = xlen
            ? algebraic_p(object::parse(first, xlen))
            : integer::make(0);
        algebraic_g y = ylen
            ? algebraic_p(object::parse(ysrc, ylen))
            : integer::make(1);
        if (xlen && !ylen)
        {
            // Case of `3i`
            y = x;
            x = integer::make(0);
        }
        rectangular_g result = rectangular::make(x, y);
        p.out = +result;
        p.end = last - first + 2*paren;
        return OK;
    }

    // Compute size that we parsed
    size_t parsed = last - first + 2*paren;

    // Parse the first object
    gcutf8 ysrc = ybeg;
    size_t xsz = xlen;
    algebraic_g x = algebraic_p(object::parse(first, xlen));
    if (!x)
        return ERROR;
    if (xlen != xsz)
    {
        rt.syntax_error().source(utf8(src) + xlen);
        return ERROR;
    }

    // Parse the second object
    size_t ysz = ylen;
    algebraic_g y   = type == ID_unit ? unit::parse_uexpr(ysrc, ylen)
                                      : algebraic_p(object::parse(ysrc, ylen));
    if (!y)
        return ERROR;
    if (ylen != ysz)
    {
        rt.syntax_error().source(utf8(ysrc) + ylen);
        return ERROR;
    }
    if (sign == '-')
    {
        y = neg::run(y);
        if (!y)
            return ERROR;
    }

    // Select forced angle mode if necessary
    angle_unit unit = Settings.AngleMode();
    switch(angle)
    {
    case Settings.DEGREES_SYMBOL:
        unit = ID_Deg;
        break;
    case Settings.RADIANS_SYMBOL:
        unit = ID_Rad;
        break;
    case Settings.GRAD_SYMBOL:
        unit = ID_Grad;
        break;
    case Settings.PI_RADIANS_SYMBOL:
        unit = ID_PiRadians;
        break;
    }

    // Build the resulting complex
    complex_g result = complex::make(type, x, y, unit);
    p.out = complex_p(result);
    p.end = parsed;

    return OK;
}



// ============================================================================
//
//   Specific code for rectangular form
//
// ============================================================================

algebraic_g rectangular::mod() const
// ----------------------------------------------------------------------------
//   Compute the modulus in rectangular form
// ----------------------------------------------------------------------------
{
    rectangular_g o = this;
    algebraic_g   r = o->re();
    algebraic_g   i = o->im();
    return hypot::evaluate(r, i);
}

algebraic_g rectangular::arg(angle_unit unit) const
// ----------------------------------------------------------------------------
//   Compute the argument in rectangular form
// ----------------------------------------------------------------------------
{
    algebraic_g a = pifrac();   // Compute "exact" angle
    a = convert_angle(a, object::ID_PiRadians, unit);
    return a;
}


algebraic_g rectangular::pifrac() const
// ----------------------------------------------------------------------------
//   Compute the argument as a fraction of pi
// ----------------------------------------------------------------------------
{
    rectangular_g o = this;
    algebraic_g   r = o->re();
    algebraic_g   i = o->im();
    if (!r || !i)
        return nullptr;

    angle_unit mode = Settings.AngleMode();
    Settings.AngleMode(object::ID_PiRadians); // Enable 'exact' optimizations
    algebraic_g a = atan2::evaluate(i, r);
    Settings.AngleMode(mode);
    return a;
}


bool rectangular::is_zero() const
// ----------------------------------------------------------------------------
//   A complex in rectangular form is zero iff both re and im are zero
// ----------------------------------------------------------------------------
{
    rectangular_g o = this;
    return o->re()->is_zero(false) && o->im()->is_zero(false);
}


bool rectangular::is_one() const
// ----------------------------------------------------------------------------
//   A complex in rectangular form is zero iff both re and im are zero
// ----------------------------------------------------------------------------
{
    rectangular_g o = this;
    return o->re()->is_one(false) && o->im()->is_zero(false);
}


algebraic_p rectangular::is_real() const
// ----------------------------------------------------------------------------
//   Check if the complex is a purely real value
// ----------------------------------------------------------------------------
{
    return y()->is_zero(false) ? x() : nullptr;
}


RENDER_BODY(rectangular)
// ----------------------------------------------------------------------------
//   Render a complex number in rectangular form
// ----------------------------------------------------------------------------
{
    rectangular_g go = o;
    algebraic_g re = go->re();
    algebraic_g im = go->im();
    if (!re || !im)
        return r.printf("Invalid rectangular");
    bool ifirst = r.editing() || Settings.ComplexIBeforeImaginary();
    bool neg  = im->is_negative(false);
    if (neg)
        im = -im;
    re->render(r);
    r.put(neg ? '-' : '+');
    if (ifirst)
        r.put(unicode(I_MARK));
    im->render(r);
    if (!ifirst)
        r.put(unicode(I_MARK));
    return r.size();
}



// ============================================================================
//
//   Polar-specific code
//
// ============================================================================
//
//   In the polar representation, the unit is always stored as a ratio of π.
//   For example, the internal representation of the imaginary unit is (1;1),
//   where the second 1 represents the angle π in radians.
//   This makes it possible to have an exact and compact representation of
//   common angles, like 1/4π, etc.
//   When the argument is symbolic, it is not transformed. The assumption is
//   that it represents an angle, irrespective of the angular unit.
//

algebraic_g polar::re() const
// ----------------------------------------------------------------------------
//   Compute the real part in polar form
// ----------------------------------------------------------------------------
{
    polar_g     o = this;
    algebraic_g m = o->mod();
    algebraic_g a = o->arg(Settings.AngleMode());
    return m * cos::run(a);
}

algebraic_g polar::im() const
// ----------------------------------------------------------------------------
//   Compute the imaginary part in polar form
// ----------------------------------------------------------------------------
{
    polar_g     o = this;
    algebraic_g m = o->mod();
    algebraic_g a = o->arg(Settings.AngleMode());
    return m * sin::run(a);
}


bool polar::is_zero() const
// ----------------------------------------------------------------------------
//   A complex in polar form is zero iff modulus is zero
// ----------------------------------------------------------------------------
{
    polar_g o = this;
    return o->mod()->is_zero(false);
}


bool polar::is_one() const
// ----------------------------------------------------------------------------
//   A complex in rectangular form is zero iff both re and im are zero
// ----------------------------------------------------------------------------
{
    polar_g o = this;
    return o->mod()->is_one(false) && o->pifrac()->is_zero();
}


algebraic_p polar::is_real() const
// ----------------------------------------------------------------------------
//   Check if the complex is a purely real value
// ----------------------------------------------------------------------------
{
    polar_g o = this;
    algebraic_g pifrac = o->pifrac();
    if (pifrac->is_zero(false))
        return o->mod();
    if (pifrac->is_one(false))
        return -o->mod();
    return nullptr;
}


polar_p polar::make(algebraic_r mr, algebraic_r ar, angle_unit unit)
// ----------------------------------------------------------------------------
//   Build a normalized polar from given modulus and argument
// ----------------------------------------------------------------------------
{
    if (!mr|| !ar)
        return nullptr;
    algebraic_g m = mr;
    algebraic_g a = ar;
    bool negmod = m->is_negative(false);
    a = convert_angle(a, unit, object::ID_PiRadians, negmod);
    if (negmod)
        m = neg::run(m);
    if (!a|| !m)
        return nullptr;
    return rt.make<polar>(m, a);
}


algebraic_g polar::mod() const
// ----------------------------------------------------------------------------
//   The modulus of a polar complex is always its first item
// ----------------------------------------------------------------------------
{
    return x();
}


algebraic_g polar::arg(angle_unit unit) const
// ----------------------------------------------------------------------------
//   Convert the argument to the current angle setting
// ----------------------------------------------------------------------------
{
    algebraic_g a = y();
    a = convert_angle(a, object::ID_PiRadians, unit);
    return a;
}


PARSE_BODY(polar)
// ----------------------------------------------------------------------------
//   Parse a complex number in polar form
// ----------------------------------------------------------------------------
{
    // This is really handled in the parser for 'rectangular'
    return SKIP;
}


RENDER_BODY(polar)
// ----------------------------------------------------------------------------
//   Render a complex number in rectangular form
// ----------------------------------------------------------------------------
{
    angle_unit  unit = Settings.AngleMode();
    polar_g     go   = o;
    algebraic_g m    = go->mod();
    algebraic_g a    = go->arg(unit);
    if (!m || !a)
        return r.printf("Invalid polar");
    m->render(r);
    r.put(unicode(ANGLE_MARK));
    a->render(r);
    if (!a->is_symbolic())
    {
        switch(unit)
        {
        case ID_Deg:
            r.put(unicode(Settings.DEGREES_SYMBOL));
            break;
        case ID_Grad:
            r.put(unicode(Settings.GRAD_SYMBOL));
            break;
        case ID_PiRadians:
            r.put(unicode(Settings.PI_RADIANS_SYMBOL));
            break;
        default:
        case ID_Rad:
            r.put(unicode(Settings.RADIANS_SYMBOL));
            break;
        }
    }
    return r.size();
}


COMMAND_BODY(RealToRectangular)
// ----------------------------------------------------------------------------
//   Take two values in x and y and turn them into a rectangular complex
// ----------------------------------------------------------------------------
{
    object_g re = tag::strip(rt.stack(1));
    object_g im = tag::strip(rt.stack(0));
    if (!re || !im)
        return ERROR;
    if (!(re->is_real() || re->is_symbolic()) ||
        !(im->is_real() || im->is_symbolic()))
    {
        rt.type_error();
        return ERROR;
    }
    complex_g z = rectangular::make(algebraic_p(+re),
                                    algebraic_p(+im));
    if (!z|| !rt.drop())
        return ERROR;
    if (!rt.top(z))
        return ERROR;
    return OK;
}


COMMAND_BODY(RealToPolar)
// ----------------------------------------------------------------------------
//   Take two values in x and y and turn them into a polar complex
// ----------------------------------------------------------------------------
{
    object_g mod = tag::strip(rt.stack(1));
    object_g arg = tag::strip(rt.stack(0));
    if (!mod || !arg)
        return ERROR;

    algebraic::angle_unit amode = Settings.AngleMode();
    if (algebraic_g arga = arg->as_algebraic())
    {
        if (algebraic::angle_unit givenmode = algebraic::adjust_angle(arga))
        {
            amode = givenmode;
            arg = +arga;
        }
    }

    if (!(mod->is_real() || mod->is_symbolic()) ||
        !(arg->is_real() || arg->is_symbolic()))
    {
        rt.type_error();
        return ERROR;
    }
    complex_g z = polar::make(algebraic_p(+mod), algebraic_p(+arg), amode);
    if (!z|| !rt.drop())
        return ERROR;
    if (!rt.top(z))
        return ERROR;
    return OK;
}


COMMAND_BODY(RectangularToReal)
// ----------------------------------------------------------------------------
//   Take a complex value and convert it into two real values
// ----------------------------------------------------------------------------
{
    object_g z = tag::strip(rt.top());
    if (!z)
        return ERROR;
    if (!z->is_complex())
    {
        rt.type_error();
        return ERROR;
    }
    complex_g zz = complex_p(+z);
    object_g re = +zz->re();
    object_g im = +zz->im();
    if (!re || !im)
        return ERROR;
    re = +tag::make("re", re);
    im = +tag::make("im", im);
    if (!re || !im || !rt.top(re) || !rt.push(im))
        return ERROR;
    return OK;
}


COMMAND_BODY(PolarToReal)
// ----------------------------------------------------------------------------
//   Take a complex value in polar form and convert it into two real values
// ----------------------------------------------------------------------------
{
    object_g z = tag::strip(rt.top());
    if (!z)
        return ERROR;
    if (!z->is_complex())
    {
        rt.type_error();
        return ERROR;
    }
    complex_g zz = complex_p(+z);
    algebraic_g mod = +zz->mod();
    algebraic_g arg = +zz->arg(Settings.AngleMode());
    if (!mod || !arg)
        return ERROR;
    if (!complex::add_angle(arg))
        return ERROR;
    object_g modobj = +tag::make("mod", +mod);
    object_g argobj = +tag::make("arg", +arg);
    if (!modobj || !argobj || !rt.top(modobj) || !rt.push(argobj))
        return ERROR;
    return OK;
}


COMMAND_BODY(ToRectangular)
// ----------------------------------------------------------------------------
//  Convert the top-level complex to rectangular form
// ----------------------------------------------------------------------------
{
    object_g x = tag::strip(rt.top());
    if (!x)
        return ERROR;
    if (!x->is_complex())
    {
        rt.type_error();
        return ERROR;
    }
    if (polar_p zp = x->as<polar>())
    {
        x = +zp->as_rectangular();
        if (!x|| !rt.top(x))
            return ERROR;
    }
    return OK;
}


COMMAND_BODY(ToPolar)
// ----------------------------------------------------------------------------
//  Convert the top-level complex to polar form
// ----------------------------------------------------------------------------
{
    object_g x = tag::strip(rt.top());
    if (!x)
        return ERROR;
    if (!x->is_complex())
    {
        rt.type_error();
        return ERROR;
    }
    if (rectangular_p zr = x->as<rectangular>())
    {
        x = +zr->as_polar();
        if (!x|| !rt.top(x))
            return ERROR;
    }
    return OK;
}



// ============================================================================
//
//   Implementation of complex functions
//
// ============================================================================

COMPLEX_BODY(sqrt)
// ----------------------------------------------------------------------------
//   Complex implementation of sqrt
// ----------------------------------------------------------------------------
{
    id zt = z->type();
    if (zt == ID_polar)
    {
        // Computation is a bit easier in polar form
        polar_r p = (polar_r) z;
        algebraic_g mod = p->mod();
        algebraic_g arg = p->pifrac(); // Want it in original form here
        algebraic_g two = integer::make(2);
        return polar::make(sqrt::run(mod), arg / two, object::ID_PiRadians);
    }

    rectangular_r r = (rectangular_r) z;
    algebraic_g a = r->re();
    algebraic_g b = r->im();
    if (b->is_zero(false))
    {
        if (!a->is_symbolic())
        {
            if (a->is_negative(false))
            {
                a = sqrt::run(-a);
                return rectangular::make(b, a);
            }
            else
            {
                a = sqrt::run(a);
                return rectangular::make(a, b);
            }
        }
    }

    algebraic_g znorm = abs::run(algebraic_p(z));
    algebraic_g two = algebraic_p(integer::make(2));
    algebraic_g re = sqrt::run((znorm + a) / two);
    algebraic_g im = sqrt::run((znorm - a) / two);
    if (b->is_negative(false))
        im = neg::run(im);
    else if (b->is_symbolic())
        im = sign::run(im) * im;
    return rectangular::make(re, im);
}


COMPLEX_BODY(cbrt)
// ----------------------------------------------------------------------------
//   Complex implementation of cbrt
// ----------------------------------------------------------------------------
{
    polar_g p = z->as_polar();
    if (!p)
        return nullptr;
    algebraic_g mod = p->mod();
    algebraic_g arg = p->pifrac();   // Want it in original form
    algebraic_g three = integer::make(3);
    return polar::make(cbrt::run(mod), arg / three, object::ID_PiRadians);
}


COMPLEX_BODY(sin)
// ----------------------------------------------------------------------------
//   Complex implementation of sin
// ----------------------------------------------------------------------------
{
    // sin(z) = (exp(iz) - exp(-iz)) / 2i
    complex_g i = complex::make(0,1);
    complex_g iz = i * z;
    complex_g niz = -iz;
    iz = complex::exp(iz);
    niz = complex::exp(niz);
    return (iz - niz) / complex::make(0,2);
}

COMPLEX_BODY(cos)
// ----------------------------------------------------------------------------
//   Complex implementation of cos
// ----------------------------------------------------------------------------
{
    // cos(z) = (exp(iz) + exp(-iz)) / 2
    complex_g i = complex::make(0,1);
    complex_g iz = i * z;
    complex_g niz = -iz;
    iz = complex::exp(iz);
    niz = complex::exp(niz);
    return (iz + niz) / complex::make(2,0);
}


COMPLEX_BODY(tan)
// ----------------------------------------------------------------------------
//   Complex implementation of tan
// ----------------------------------------------------------------------------
{
    // tan(z) = -i * (exp(iz) - exp(-iz)) / (exp(iz) + exp(-iz))
    complex_g i = complex::make(0,1);
    complex_g iz = i * z;
    complex_g niz = -iz;
    iz = complex::exp(iz);
    niz = complex::exp(niz);
    return complex::make(0,-1) * (iz - niz) / (iz + niz);
}


COMPLEX_BODY(asin)
// ----------------------------------------------------------------------------
//   Complex implementation of asin
// ----------------------------------------------------------------------------
{
    // asin(z) = i log(sqrt(1 - z^2) - iz)
    complex_g sq = z * z;
    complex_g one = complex::make(1,0);
    sq = complex::sqrt(one - sq);
    complex_g i = complex::make(0, 1);
    complex_g iz = i * z;
    return i * complex::log(sq - iz);
}


COMPLEX_BODY(acos)
// ----------------------------------------------------------------------------
//   Complex implementation of acos
// ----------------------------------------------------------------------------
{
    // acos(z) = -i log(z + i sqrt(1 - z^2))
    complex_g sq = z * z;
    complex_g one = complex::make(1,0);
    sq = complex::sqrt(one - sq);
    complex_g ni = complex::make(0,-1);
    return ni * complex::log(z - ni* sq);
}


COMPLEX_BODY(atan)
// ----------------------------------------------------------------------------
//   Complex implementation of atan
// ----------------------------------------------------------------------------
{
    // atan(z) = -i/2 ln((i-z) / (i + z))
    complex_g i = complex::make(0,1);
    return complex::log((i - z) / (i + z)) / complex_g(complex::make(0,2));
}


COMPLEX_BODY(sinh)
// ----------------------------------------------------------------------------
//   Complex implementation of sinh
// ----------------------------------------------------------------------------
{
    // sinh(z) = (exp(z) - exp(-z)) / 2
    return (complex::exp(z) - complex::exp(-z)) / complex_g(complex::make(2,0));
}

COMPLEX_BODY(cosh)
// ----------------------------------------------------------------------------
//   Complex implementation of cosh
// ----------------------------------------------------------------------------
{
    // cosh(z) = (exp(z) + exp(-z)) / 2
    complex_g two = complex::make(2,0);
    return (complex::exp(z) - complex::exp(-z)) / two;
}


COMPLEX_BODY(tanh)
// ----------------------------------------------------------------------------
//   Complex implementation of tanh
// ----------------------------------------------------------------------------
{
    // tanh(z) = (exp(2z) - 1) / (exp(2z) + 1)
    complex_g e2z = complex::exp(z + z);
    complex_g one = complex::make(1,0);
    return (e2z - one) /  (e2z + one);
}


COMPLEX_BODY(asinh)
// ----------------------------------------------------------------------------
//   Complex implementation of asinh
// ----------------------------------------------------------------------------
{
    // asinh(z) = ln(z + sqrt(z^2 + 1))
    complex_g one = complex::make(1, 0);
    return complex::log(z + complex::sqrt(z*z + one));
}


COMPLEX_BODY(acosh)
// ----------------------------------------------------------------------------
//   Complex implementation of acosh
// ----------------------------------------------------------------------------
{
    // asinh(z) = ln(z + sqrt(z^2 - 1))
    complex_g one = complex::make(1, 0);
    return complex::log(z + complex::sqrt(z*z - one));
}


COMPLEX_BODY(atanh)
// ----------------------------------------------------------------------------
//   Complex implementation of atanh
// ----------------------------------------------------------------------------
{
    // atanh(z) = 1/2 ln((1+z) / (1-z))
    complex_g one = complex::make(1, 0);
    complex_g two = complex::make(2, 0);
    return complex::log((one + z) / (one - z)) / two;
}


COMPLEX_BODY(log1p)
// ----------------------------------------------------------------------------
//   Complex implementation of log1p
// ----------------------------------------------------------------------------
{
    rt.type_error();
    return z;
}

COMPLEX_BODY(expm1)
// ----------------------------------------------------------------------------
//   Complex implementation of expm1
// ----------------------------------------------------------------------------
{
    rt.type_error();
    return z;
}


COMPLEX_BODY(log)
// ----------------------------------------------------------------------------
//   Complex implementation of log
// ----------------------------------------------------------------------------
{
    // log(a.exp(ib)) = log(a) + i b
    algebraic_g mod = z->mod();
    algebraic_g arg = z->arg(ID_Rad);
    return rectangular::make(log::run(mod), arg);
}

COMPLEX_BODY(log10)
// ----------------------------------------------------------------------------
//   Complex implementation of log10
// ----------------------------------------------------------------------------
{
    algebraic_g ten = integer::make(10);
    algebraic_g zero = integer::make(0);
    complex_g logten = rectangular::make(log::run(ten), zero);
    return complex::log(z) / logten;
}


COMPLEX_BODY(log2)
// ----------------------------------------------------------------------------
//   Complex implementation of log2
// ----------------------------------------------------------------------------
{
    algebraic_g two = integer::make(2);
    algebraic_g zero = integer::make(0);
    complex_g logtwo = rectangular::make(log::run(two), zero);
    return complex::log(z) / logtwo;
}


COMPLEX_BODY(exp)
// ----------------------------------------------------------------------------
//   Complex implementation of exp
// ----------------------------------------------------------------------------
{
    // exp(a+ib) = exp(a)*exp(ib)
    algebraic_g re = z->re();
    algebraic_g im = z->im();
    return polar::make(exp::run(re), im, ID_Rad);
}


COMPLEX_BODY(exp10)
// ----------------------------------------------------------------------------
//   Complex implementation of exp10
// ----------------------------------------------------------------------------
{
    algebraic_g ten = integer::make(10);
    algebraic_g zero = integer::make(0);
    complex_g logten = rectangular::make(log::run(ten), zero);
    return complex::exp(logten * z);
}


COMPLEX_BODY(exp2)
// ----------------------------------------------------------------------------
//   Complex implementation of exp2
// ----------------------------------------------------------------------------
{
    algebraic_g two = integer::make(2);
    algebraic_g zero = integer::make(0);
    complex_g logtwo = rectangular::make(log::run(two), zero);
    return complex::exp(logtwo * z);
}


COMPLEX_BODY(erf)
// ----------------------------------------------------------------------------
//   Complex implementation of erf
// ----------------------------------------------------------------------------
{
    rt.unimplemented_error();
    return z;
}

COMPLEX_BODY(erfc)
// ----------------------------------------------------------------------------
//   Complex implementation of erfc
// ----------------------------------------------------------------------------
{
    rt.unimplemented_error();
    return z;
}

COMPLEX_BODY(tgamma)
// ----------------------------------------------------------------------------
//   Complex implementation of tgamma
// ----------------------------------------------------------------------------
{
    rt.unimplemented_error();
    return z;
}

COMPLEX_BODY(lgamma)
// ----------------------------------------------------------------------------
//   Complex implementation of lgamma
// ----------------------------------------------------------------------------
{
    rt.unimplemented_error();
    return z;
}
