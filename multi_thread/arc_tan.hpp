/* $Id: arc_tan.hpp 2178 2011-10-30 06:52:58Z bradbell $ */
# ifndef CPPAD_ARC_TAN_INCLUDED
# define CPPAD_ARC_TAN_INCLUDED
/* --------------------------------------------------------------------------
CppAD: C++ Algorithmic Differentiation: Copyright (C) 2003-11 Bradley M. Bell

CppAD is distributed under multiple licenses. This distribution is under
the terms of the 
                    Common Public License Version 1.0.

A copy of this license is included in the COPYING file of this distribution.
Please visit http://www.coin-or.org/CppAD/ for information on other licenses.
-------------------------------------------------------------------------- */

# include <cppad/cppad.hpp>

extern CppAD::AD<double> 
arc_tan(const CppAD::AD<double>& x, const CppAD::AD<double>& y);

# endif
