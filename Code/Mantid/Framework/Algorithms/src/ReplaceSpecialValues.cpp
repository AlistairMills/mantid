/*WIKI* 

The algorithm searches over all of the values in a workspace and if it finds a value set to NaN (not a number), infinity or larger than the 'big' threshold given then that value and the associated error is replaced by the user provided values.

If no value is provided for either NaNValue, InfinityValue or BigValueThreshold then the algorithm will exit with an error, as in this case it would not be checking anything.

Algorithm is now event aware.

*WIKI*/
//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAlgorithms/ReplaceSpecialValues.h"
#include "MantidKernel/Exception.h"
#include "boost/math/special_functions/fpclassify.hpp"
#include <limits>
#include <cmath>

using namespace Mantid::API;
using namespace Mantid::Kernel;

namespace Mantid
{
namespace Algorithms
{

// Register the class into the algorithm factory
DECLARE_ALGORITHM(ReplaceSpecialValues)

/// Sets documentation strings for this algorithm
void ReplaceSpecialValues::initDocs()
{
  this->setWikiSummary("Replaces instances of NaN and infinity in the workspace with user defined numbers.<p>If a replacement value is not provided the check will not occur. This algorithm can also be used to replace numbers whose absolute value is larger than a user-defined threshold. ");
  this->setOptionalMessage("Replaces instances of NaN and infinity in the workspace with user defined numbers. If a replacement value is not provided the check will not occur. This algorithm can also be used to replace numbers whose absolute value is larger than a user-defined threshold.");
}


void ReplaceSpecialValues::defineProperties()
{
  declareProperty("NaNValue", Mantid::EMPTY_DBL(), "The value used to replace occurrances of NaN "
    "(default: do not check).");
  declareProperty("NaNError", 0.0, "The error value used when replacing a value of NaN ");
  declareProperty("InfinityValue", Mantid::EMPTY_DBL(),
    "The value used to replace occurrances of positive or negative infinity "
    "(default: do not check).");
  declareProperty("InfinityError", 0.0, "The error value used when replacing a value of infinity ");
  declareProperty("BigNumberThreshold", Mantid::EMPTY_DBL(),
    "The threshold above which a number (positive or negative) should be replaced. "
    "(default: do not check)");
  declareProperty("BigNumberValue", 0.0, "The value with which to replace occurrances of 'big' numbers.");
  declareProperty("BigNumberError", 0.0, "The error value used when replacing a 'big' number");
}

void ReplaceSpecialValues::retrieveProperties()
{
  m_NaNValue = getProperty("NaNValue");
  m_NaNError = getProperty("NaNError");
  m_InfiniteValue = getProperty("InfinityValue");
  m_InfiniteError = getProperty("InfinityError");
  m_bigThreshold = getProperty("BigNumberThreshold");
  m_bigValue = getProperty("BigNumberValue");
  m_bigError = getProperty("BigNumberError");

  m_performNaNCheck = !checkifPropertyEmpty(m_NaNValue);
  m_performInfiniteCheck = !checkifPropertyEmpty(m_InfiniteValue);
  m_performBigCheck = !checkifPropertyEmpty(m_bigThreshold);
  if (!(m_performNaNCheck || m_performInfiniteCheck || m_performBigCheck))
  {
    throw std::invalid_argument("No value was defined for NaN, infinity or BigValueThreshold");
  }
}

void ReplaceSpecialValues::performUnaryOperation(const double XIn, const double YIn,
    const double EIn, double& YOut, double& EOut)
{
  (void) XIn; //Avoid compiler warning
  YOut = YIn;
  EOut = EIn;

  if (m_performNaNCheck && checkIfNan(YIn))
  {
    YOut = m_NaNValue;
    EOut = m_NaNError;
  }
  else if (m_performInfiniteCheck && checkIfInfinite(YIn))
  {
    YOut = m_InfiniteValue;
    EOut = m_InfiniteError;
  }
  else if (m_performBigCheck && checkIfBig(YIn))
  {
    YOut = m_bigValue;
    EOut = m_bigError;
  }
}

bool ReplaceSpecialValues::checkIfNan(const double& value) const
{
  return (boost::math::isnan(value));
}

bool ReplaceSpecialValues::checkIfInfinite(const double& value) const
{
  return (std::abs(value) == std::numeric_limits<double>::infinity());
}

bool ReplaceSpecialValues::checkIfBig(const double& value) const
{
  return (std::abs(value) > m_bigThreshold);
}

bool ReplaceSpecialValues::checkifPropertyEmpty(const double& value) const
{
  return (std::abs(value - Mantid::EMPTY_DBL()) < 1e-08);
}

} // namespace Algorithms
} // namespace Mantid
