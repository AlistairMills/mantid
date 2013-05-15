/*WIKI*
The Stretched exponential fit function is defined by
:<math> Height \cdot e^{-(\frac{x}{Lifetime})^{Stretching} }</math>
 *WIKI*/

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidCurveFitting/StretchExp.h"
#include "MantidAPI/FunctionFactory.h"
#include <cmath>

namespace Mantid
{
namespace CurveFitting
{

using namespace Kernel;
using namespace API;

DECLARE_FUNCTION(StretchExp)

StretchExp::StretchExp()
{
  declareParameter("Height", 1.0, "Height at time zero");
  declareParameter("Lifetime", 1.0, "Relaxation time of the standard exponential");
  declareParameter("Stretching",1.0, "Stretching exponent");
}

/** \relates StretchExp
 * Implements the StretchExp function
 * @param out :: The result of evaluating the function
 * @param xValues :: function domain values
 * @param nData :: size of the function domain
 */
void StretchExp::function1D(double* out, const double* xValues, const size_t nData)const
{
    const double h = getParameter("Height");
    const double t = getParameter("Lifetime");
    const double b = getParameter("Stretching");

    for (size_t i = 0; i < nData; i++) 
    {
        out[i] = h*exp( -pow(xValues[i]/t,b) );
    }
}

/** \relates StretchExp
 * Calculates the derivatives of the StretchExp
 * @param out :: The resulting jacobian
 * @param xValues :: function domain values
 * @param nData :: size of the function domain
 */
void StretchExp::functionDeriv1D(API::Jacobian* out, const double* xValues, const size_t nData)
{
    const double h = getParameter("Height");
    const double t = getParameter("Lifetime");
    const double b = getParameter("Stretching");

    for (size_t i = 0; i < nData; i++) {
        double x = xValues[i];
        double a = pow( x/t, b);
        double e = exp(-a);
        out->set(i,0, e);              //derivative with respect to h
        out->set(i,1, h*a*b*e/t);      //derivative with respect to t
        out->set(i,2,-h*a*e*log(x/t)); //derivative with respect to b
    }

}


} // namespace CurveFitting
} // namespace Mantid
