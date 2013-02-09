/* --------------------------------------------------------------------------
 *  CppADCodeGen: C++ Algorithmic Differentiation with Source Code Generation:
 *    Copyright (C) 2012 Ciengis
 *
 *  CppADCodeGen is distributed under multiple licenses:
 *
 *   - Common Public License Version 1.0 (CPL1), and
 *   - GNU General Public License Version 2 (GPL2).
 *
 * CPL1 terms and conditions can be found in the file "epl-v10.txt", while
 * terms and conditions for the GPL2 can be found in the file "gpl2.txt".
 * ----------------------------------------------------------------------------
 * Author: Joao Leal
 */
#include "CppADCGSolveTest.hpp"

using namespace CppAD;

TEST_F(CppADCGSolveTest, SolveDivMul) {
    using namespace CppAD;

    // independent variable vector
    std::vector<ADCGD> u(3);
    u[0] = 4.0;
    u[1] = 2.0;
    u[2] = 2.5;

    Independent(u);

    // dependent variable vector
    std::vector< ADCGD> Z(4);

    // model
    Z[0] = u[0] / u[1];
    Z[1] = Z[0] * 4.;
    Z[2] = 2. / Z[1];
    Z[3] = Z[2] * u[2] - 0.625;

    // create f: U -> Z
    ADFun<CGD> fun(u, Z);

    test_solve(fun, 3, 0, u);
    test_solve(fun, 3, 1, u);
    test_solve(fun, 3, 2, u);
}
