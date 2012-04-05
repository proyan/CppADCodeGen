#ifndef CPPAD_CG_ARITHMETIC_ASSIGN_INCLUDED
#define	CPPAD_CG_ARITHMETIC_ASSIGN_INCLUDED
/* --------------------------------------------------------------------------
CppAD: C++ Algorithmic Differentiation: Copyright (C) 2012 Ciengis

CppAD is distributed under multiple licenses. This distribution is under
the terms of the
                    Common Public License Version 1.0.

A copy of this license is included in the COPYING file of this distribution.
Please visit http://www.coin-or.org/CppAD/ for information on other licenses.
-------------------------------------------------------------------------- */


namespace CppAD {

    template<class Base>
    inline CG<Base>& CG<Base>::operator+=(const CG<Base> &right) {
        if (isParameter() && right.isParameter()) {
            *value_ += *right.value_;

        } else {

            CodeHandler<Base>* handler;
            if (isParameter()) {
                handler = right.getCodeHandler();

                if (IdenticalZero()) {
                    *this = right;
                    return *this;
                }

                // must print the assignment to the parameter value
                const CG<Base> copy(*this); // make a copy
                makeVariable(*handler);
                handler->printOperationAssign(*this, copy);


            } else if (right.isParameter()) {
                if (right.IdenticalZero()) {
                    return *this; // nothing to do
                }
                handler = getCodeHandler();
            } else {
                assert(getCodeHandler() == right.getCodeHandler());
                handler = getCodeHandler();
            }

            if (isVariable()) {
                variableValueWillChange();
            } else {
                makeVariable(*handler);
            }

            handler->printOperationPlusAssign(*this, right);
        }

        return *this;
    }

    template<class Base>
    inline CG<Base>& CG<Base>::operator-=(const CG<Base> &right) {
        if (isParameter() && right.isParameter()) {
            *value_ -= *right.value_;

        } else {
            CodeHandler<Base>* handler;
            if (isParameter()) {
                handler = right.getCodeHandler();
                // must print the assignment to the parameter value
                const CG<Base> copy(*this); // make a copy
                makeVariable(*handler);
                handler->printOperationAssign(*this, copy.getParameterValue());

            } else if (right.isParameter()) {
                if (right.IdenticalZero()) {
                    return *this; // nothing to do
                }
                handler = getCodeHandler();
            } else {
                assert(getCodeHandler() == right.getCodeHandler());
                handler = getCodeHandler();
            }

            if (isVariable()) {
                variableValueWillChange();
            } else {
                makeVariable(*handler);
            }

            handler->printOperationMinusAssign(*this, right);
        }

        return *this;
    }

    template<class Base>
    inline CG<Base>& CG<Base>::operator*=(const CG<Base> &right) {
        if (isParameter() && right.isParameter()) {
            * value_ *= *right.value_;

        } else {
            CodeHandler<Base>* handler;

            if (isParameter()) {
                handler = right.getCodeHandler();
                if (IdenticalZero()) {
                    return *this; // nothing to do (does not consider that right might be infinite)
                } else if (IdenticalOne()) {
                    *this = right;
                    return *this;
                }
                // must print the assignment to the parameter value
                const CG<Base> copy(*this); // make a copy
                makeVariable(*handler);
                handler->printOperationAssign(*this, copy.getParameterValue());

            } else if (right.isParameter()) {
                if (right.IdenticalZero()) {
                    if (isVariable()) {
                        variableValueWillChange();
                    }
                    makeParameter(Base(0.0)); // does not consider that left might be infinite
                    return *this;
                } else if (right.IdenticalOne()) {
                    return *this; // nothing to do
                }
                handler = getCodeHandler();
            } else {
                assert(getCodeHandler() == right.getCodeHandler());
                handler = getCodeHandler();
            }

            if (isVariable()) {
                variableValueWillChange();
            } else {
                makeVariable(*handler);
            }

            handler->printOperationMultAssign(*this, right);
        }

        return *this;
    }

    template<class Base>
    inline CG<Base>& CG<Base>::operator/=(const CG<Base> &right) {
        if (isParameter() && right.isParameter()) {
            * value_ /= *right.value_;

        } else {
            CodeHandler<Base>* handler;

            if (isParameter()) {
                handler = right.getCodeHandler();
                if (IdenticalZero()) {
                    return *this; // does not consider the possibility of right being infinity or zero
                }
                // must print the assignment to the parameter value
                const CG<Base> copy(*this); // make a copy
                makeVariable(*handler);
                handler->printOperationAssign(*this, copy.getParameterValue());

            } else if (right.isParameter()) {
                if (right.IdenticalOne()) {
                    return *this; // nothing to do
                }
                handler = getCodeHandler();
            } else {
                assert(getCodeHandler() == right.getCodeHandler());
                handler = getCodeHandler();
            }

            if (isVariable()) {
                variableValueWillChange();
            } else {
                makeVariable(*handler);
            }

            handler->printOperationDivAssign(*this, right);
        }

        return *this;
    }

    template<class Base>
    inline CG<Base>& CG<Base>::operator+=(const Base &right) {
        return operator+=(CG<Base > (right));
    }

    template<class Base>
    inline CG<Base>& CG<Base>::operator-=(const Base &right) {
        return operator-=(CG<Base > (right));
    }

    template<class Base>
    inline CG<Base>& CG<Base>::operator/=(const Base &right) {
        return operator/=(CG<Base > (right));
    }

    template<class Base>
    inline CG<Base>& CG<Base>::operator*=(const Base &right) {
        return operator*=(CG<Base > (right));
    }

    template<class Base>
    template<class T>
    inline CG<Base>& CG<Base>::operator+=(const T &right) {
        return operator+=(CG<Base > (right));
    }

    template<class Base>
    template<class T>
    inline CG<Base>& CG<Base>::operator-=(const T &right) {
        return operator-=(CG<Base > (right));
    }

    template<class Base>
    template<class T>
    inline CG<Base>& CG<Base>::operator/=(const T &right) {
        return operator/=(CG<Base > (right));
    }

    template<class Base>
    template<class T>
    inline CG<Base>& CG<Base>::operator*=(const T &right) {
        return operator*=(CG<Base > (right));
    }

}

#endif

