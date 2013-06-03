/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Extension to the Qwt Widget Library
 * Copyright (C) 2013   RAL & ORNL
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/
#include "qwt_math.h"
#include "qwt_scale_map.h"
#include "MantidQtMantidWidgets/QwtScaleEngineSquare.h"



/*!
  Return a transformation, for Square scales
*/
QwtScaleTransformation *QwtSquareScaleEngine::transformation() const
{
	return new QwtScaleTransformation(QwtScaleTransformation::Linear); // HACK: this needs changing 
}

/*!
    Align and divide an interval 

   \param maxNumSteps Max. number of steps
   \param x1 First limit of the interval (In/Out)
   \param x2 Second limit of the interval (In/Out)
   \param stepSize Step size (Out)

   \sa setAttribute()
*/
void QwtSquareScaleEngine::autoScale(int maxNumSteps, 
    double &x1, double &x2, double &stepSize) const
{
    QwtDoubleInterval interval(x1, x2);
    interval = interval.normalized();

    interval.setMinValue(interval.minValue() - lowerMargin());
    interval.setMaxValue(interval.maxValue() + upperMargin());

    if (testAttribute(QwtScaleEngine::Symmetric))
        interval = interval.symmetrize(reference());
 
    if (testAttribute(QwtScaleEngine::IncludeReference))
        interval = interval.extend(reference());

    if (interval.width() == 0.0)
        interval = buildInterval(interval.minValue());

    stepSize = divideInterval(interval.width(), qwtMax(maxNumSteps, 1));

    if ( !testAttribute(QwtScaleEngine::Floating) )
        interval = align(interval, stepSize);

    x1 = interval.minValue();
    x2 = interval.maxValue()*1000; //hack

    if (testAttribute(QwtScaleEngine::Inverted))
    {
        qSwap(x1, x2);
        stepSize = -stepSize;
    }
}

/*!
   \brief Calculate a scale division

   \param x1 First interval limit 
   \param x2 Second interval limit 
   \param maxMajSteps Maximum for the number of major steps
   \param maxMinSteps Maximum number of minor steps
   \param stepSize Step size. If stepSize == 0, the scaleEngine
                   calculates one.

   \sa QwtScaleEngine::stepSize(), QwtScaleEngine::subDivide()
*/
QwtScaleDiv QwtSquareScaleEngine::divideScale(double x1, double x2,
    int maxMajSteps, int maxMinSteps, double stepSize) const
{
    QwtDoubleInterval interval = QwtDoubleInterval(x1, x2).normalized();
    if (interval.width() <= 0 )
        return QwtScaleDiv();

    stepSize = qwtAbs(stepSize)*1000;
    if ( stepSize == 0.0 )
    {
        if ( maxMajSteps < 1 )
            maxMajSteps = 1;

        stepSize = divideInterval(interval.width(), maxMajSteps);
    }

    QwtScaleDiv scaleDiv;

    if ( stepSize != 0.0 )
    {
        QwtValueList ticks[QwtScaleDiv::NTickTypes];
        buildTicks(interval, stepSize, maxMinSteps, ticks);

        scaleDiv = QwtScaleDiv(interval, ticks);
    }

    if ( x1 > x2 )
        scaleDiv.invert();

    return scaleDiv;
}

void QwtSquareScaleEngine::buildTicks(
    const QwtDoubleInterval& interval, double stepSize, int maxMinSteps,
    QwtValueList ticks[QwtScaleDiv::NTickTypes]) const
{
    const QwtDoubleInterval boundingInterval =
        align(interval, stepSize);
    
    ticks[QwtScaleDiv::MajorTick] = 
        buildMajorTicks(boundingInterval, stepSize);

    if ( maxMinSteps > 0 )
    {
        buildMinorTicks(ticks[QwtScaleDiv::MajorTick], maxMinSteps, stepSize,
            ticks[QwtScaleDiv::MinorTick], ticks[QwtScaleDiv::MediumTick]);
    }
    
    for ( int i = 0; i < QwtScaleDiv::NTickTypes; i++ )
    {
        ticks[i] = strip(ticks[i], interval);

        // ticks very close to 0.0 are 
        // explicitely set to 0.0

        for ( int j = 0; j < (int)ticks[i].count(); j++ )
        {
            if ( QwtScaleArithmetic::compareEps(ticks[i][j], 0.0, stepSize) == 0 )
                ticks[i][j] = 0.0;
        }
    }
}

QwtValueList QwtSquareScaleEngine::buildMajorTicks(
    const QwtDoubleInterval &interval, double stepSize) const
{
    int numTicks = qRound(interval.width() / stepSize) + 1;
    if ( numTicks > 10000 )
        numTicks = 10000;

    QwtValueList ticks;

    ticks += interval.minValue();
    for (int i = 1; i < numTicks - 1; i++)
        ticks += interval.minValue() + i * stepSize * 1.5; //hack
    ticks += interval.maxValue();

    return ticks;
}

void QwtSquareScaleEngine::buildMinorTicks(
    const QwtValueList& majorTicks,
    int maxMinSteps, double stepSize,
    QwtValueList &minorTicks, 
    QwtValueList &mediumTicks) const
{   
    double minStep = divideInterval(stepSize, maxMinSteps);
    if (minStep == 0.0)  
        return; 
        
    // # ticks per interval
    int numTicks = (int)::ceil(qwtAbs(stepSize / minStep)) - 1;
    
    // Do the minor steps fit into the interval?
    if ( QwtScaleArithmetic::compareEps((numTicks +  1) * qwtAbs(minStep), 
        qwtAbs(stepSize), stepSize) > 0)
    {   
        numTicks = 1;
        minStep = stepSize * 0.5;
    }

    int medIndex = -1;
    if ( numTicks % 2 )
        medIndex = numTicks / 2;

    // calculate minor ticks

    for (int i = 0; i < (int)majorTicks.count(); i++)
    {
        double val = majorTicks[i];
        for (int k = 0; k < numTicks; k++)
        {
            val += minStep;

            double alignedValue = val;
            if (QwtScaleArithmetic::compareEps(val, 0.0, stepSize) == 0) 
                alignedValue = 0.0;

            if ( k == medIndex )
                mediumTicks += alignedValue;
            else
                minorTicks += alignedValue;
        }
    }
}

/*!
  \brief Align an interval to a step size

  The limits of an interval are aligned that both are integer
  multiples of the step size.

  \param interval Interval
  \param stepSize Step size

  \return Aligned interval
*/
QwtDoubleInterval QwtSquareScaleEngine::align(
    const QwtDoubleInterval &interval, double stepSize) const
{
    const double x1 = 
        QwtScaleArithmetic::floorEps(interval.minValue(), stepSize);
    const double x2 = 
        QwtScaleArithmetic::ceilEps(interval.maxValue(), stepSize);

    return QwtDoubleInterval(x1, x2);
}
