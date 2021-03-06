#ifndef POLDIMOCKINSTRUMENTHELPERS_H
#define POLDIMOCKINSTRUMENTHELPERS_H

#include "MantidSINQ/DllConfig.h"
#include <gmock/gmock.h>
#include <algorithm>
#include "MantidSINQ/PoldiUtilities/PoldiAbstractDetector.h"
#include "MantidSINQ/PoldiUtilities/PoldiAbstractChopper.h"

#include "MantidSINQ/PoldiUtilities/PoldiHeliumDetector.h"

using namespace Mantid;
using namespace Mantid::Poldi;

namespace Mantid {
namespace Poldi {

typedef std::pair<double, double> DoublePair;

class  MockDetector : public PoldiAbstractDetector
{
protected:
    std::vector<int> m_availableElements;

public:
    MockDetector() : PoldiAbstractDetector()
    {
        m_availableElements.resize(400);
        for(int i = 0; i < static_cast<int>(m_availableElements.size()); ++i) {
            m_availableElements[i] = i;
        }
    }

    ~MockDetector() { }

    void loadConfiguration(Geometry::Instrument_const_sptr poldiInstrument)
    {
        UNUSED_ARG(poldiInstrument);
    }

    MOCK_METHOD1(twoTheta, double(int elementIndex));
    MOCK_METHOD1(distanceFromSample, double(int elementIndex));
    MOCK_METHOD0(elementCount, size_t());
    MOCK_METHOD0(centralElement, size_t());
    MOCK_METHOD2(qLimits, DoublePair(double lambdaMin, double lambdaMax));

    const std::vector<int> &availableElements()
    {
        return m_availableElements;
    }
};

class  ConfiguredHeliumDetector : public PoldiHeliumDetector
{
public:
    ConfiguredHeliumDetector() :
        PoldiHeliumDetector()
    {
        loadConfiguration(Geometry::Instrument_const_sptr());
    }

    void loadConfiguration(Geometry::Instrument_const_sptr poldiInstrument)
    {
        UNUSED_ARG(poldiInstrument);

        initializeFixedParameters(3000.0, static_cast<size_t>(400), 2.5);
        initializeCalibratedParameters(Mantid::Kernel::V2D(-931.47, -860.0), 90.41 / 180.0 * M_PI);
    }
};


class  MockChopper : public PoldiAbstractChopper
{
protected:
    std::vector<double> m_slitPositions;
    std::vector<double> m_slitTimes;

public:
    MockChopper() : PoldiAbstractChopper()
    {
        double slits [] = {0.000000, 0.162156};
        m_slitPositions = std::vector<double>(slits, slits + sizeof(slits) / sizeof(slits[0]));

        double times [] = {0.000000, 243.234};
        m_slitTimes = std::vector<double>(times, times + sizeof(times) / sizeof(times[0]));
    }

    ~MockChopper() { }

    void loadConfiguration(Geometry::Instrument_const_sptr poldiInstrument)
    {
        UNUSED_ARG(poldiInstrument)
    }

    MOCK_METHOD0(rotationSpeed, double());
    MOCK_METHOD0(cycleTime, double());
    MOCK_METHOD0(zeroOffset, double());
    MOCK_METHOD0(distanceFromSample, double());

    MOCK_METHOD1(setRotationSpeed, void(double rotationSpeed));

    const std::vector<double>& slitPositions() {
        return m_slitPositions;
    }
    const std::vector<double>& slitTimes() {
        return m_slitTimes;
    }
};
}
}
#endif // POLDIMOCKINSTRUMENTHELPERS_H
