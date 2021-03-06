#ifndef MANTID_MDEVENTS_MDHISTOWORKSPACEITERATORTEST_H_
#define MANTID_MDEVENTS_MDHISTOWORKSPACEITERATORTEST_H_

#include "MantidKernel/System.h"
#include "MantidKernel/Timer.h"
#include "MantidGeometry/MDGeometry/MDHistoDimension.h"
#include "MantidMDEvents/MDHistoWorkspace.h"
#include "MantidMDEvents/MDHistoWorkspaceIterator.h"
#include "MantidTestHelpers/MDEventsTestHelper.h"
#include <cxxtest/TestSuite.h>
#include <iomanip>
#include <iostream>
#include "MantidKernel/VMD.h"
#include "MantidGeometry/MDGeometry/MDImplicitFunction.h"
#include "MantidGeometry/MDGeometry/MDPlane.h"
#include <boost/assign/list_of.hpp>

using namespace Mantid;
using namespace Mantid::MDEvents;
using namespace Mantid::API;
using Mantid::Kernel::VMD;
using Mantid::Geometry::MDHistoDimension;
using Mantid::Geometry::MDImplicitFunction;
using Mantid::Geometry::MDHistoDimension_sptr;
using Mantid::Geometry::MDPlane;
using Mantid::Geometry::MDPlane;

class MDHistoWorkspaceIteratorTest: public CxxTest::TestSuite
{
private:

  /// Helper type allows masking to take place directly on MDHistoWorkspaces for testing purposes.
  class WritableHistoWorkspace: public Mantid::MDEvents::MDHistoWorkspace
  {
  public:
    WritableHistoWorkspace(MDHistoDimension_sptr x) :
        Mantid::MDEvents::MDHistoWorkspace(x)
    {
    }
    void setMaskValueAt(size_t at, bool value)
    {
      m_masks[at] = value;
    }
  };

public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static MDHistoWorkspaceIteratorTest *createSuite()
  {
    return new MDHistoWorkspaceIteratorTest();
  }
  static void destroySuite(MDHistoWorkspaceIteratorTest *suite)
  {
    delete suite;
  }

  void test_bad_constructor()
  {
    MDHistoWorkspace_sptr ws;
    TS_ASSERT_THROWS_ANYTHING( MDHistoWorkspaceIterator it(ws));
  }

  void do_test_iterator(size_t nd, size_t numPoints)
  {
    MDHistoWorkspace_sptr ws = MDEventsTestHelper::makeFakeMDHistoWorkspace(1.0, nd, 10);
    for (size_t i = 0; i < numPoints; i++)
      ws->setSignalAt(i, double(i));
    MDHistoWorkspaceIterator * it = new MDHistoWorkspaceIterator(ws);
    TSM_ASSERT( "This iterator is valid at the start.", it->valid());
    size_t i = 0;

    // Position of the first box
    for (size_t d = 0; d < nd; d++)
    {
      TS_ASSERT_DELTA( it->getInnerPosition(0,0), 0.5, 1e-6);
    }

    VMD compare(nd);
    for (size_t d = 0; d < nd; d++)
    {
      compare[d] = 0.5;
    }
    TS_ASSERT_EQUALS( it->getCenter(), compare);

    do
    {
      TS_ASSERT_DELTA( it->getNormalizedSignal(), double(i) / 1.0, 1e-5);
      TS_ASSERT_DELTA( it->getNormalizedError(), 1.0, 1e-5);
      coord_t * vertexes;
      size_t numVertices;
      vertexes = it->getVertexesArray(numVertices);
      TS_ASSERT( vertexes);
      TS_ASSERT_EQUALS( it->getNumEvents(), 1);
      TS_ASSERT_EQUALS( it->getInnerDetectorID(0), 0);
      TS_ASSERT_EQUALS( it->getInnerRunIndex(0), 0);
      TS_ASSERT_EQUALS( it->getInnerSignal(0), double(i));
      TS_ASSERT_EQUALS( it->getInnerError(0), 1.0);
      i++;
    } while (it->next());
    TS_ASSERT_EQUALS( i, numPoints);

    // Now use a for loop
    for (size_t i = 0; i < numPoints; i++)
    {
      it->jumpTo(i);
      TS_ASSERT_DELTA( it->getNormalizedSignal(), double(i) / 1.0, 1e-5);
    }
  }

  void test_iterator_1D()
  {
    do_test_iterator(1, 10);
  }

  void test_iterator_2D()
  {
    do_test_iterator(2, 100);
  }

  void test_iterator_3D()
  {
    do_test_iterator(3, 1000);
  }

  void test_iterator_4D()
  {
    do_test_iterator(4, 10000);
  }

  void test_iterator_2D_implicitFunction()
  {
    // Make an implicit function that will keep the points in a corner close to 0,0
    MDImplicitFunction * function = new MDImplicitFunction();
    function->addPlane(MDPlane(VMD(-1., -1.), VMD(4.5, 0.)));

    MDHistoWorkspace_sptr ws = MDEventsTestHelper::makeFakeMDHistoWorkspace(1.0, 2, 10);
    for (size_t i = 0; i < 100; i++)
      ws->setSignalAt(i, double(i));
    MDHistoWorkspaceIterator * it = new MDHistoWorkspaceIterator(ws, function);
    TSM_ASSERT( "This iterator is valid at the start.", it->valid());

    TS_ASSERT_EQUALS(it->getNormalizedSignal(), 0.);
    it->next();
    TS_ASSERT_EQUALS(it->getNormalizedSignal(), 1.);
    it->next();
    TS_ASSERT_EQUALS(it->getNormalizedSignal(), 2.);
    it->next();
    TS_ASSERT_EQUALS(it->getNormalizedSignal(), 3.);
    it->next();
    TS_ASSERT_EQUALS(it->getNormalizedSignal(), 10.);
    it->next();
    TS_ASSERT_EQUALS(it->getNormalizedSignal(), 11.);
    it->next();
    TS_ASSERT_EQUALS(it->getNormalizedSignal(), 12.);
    it->next();
    TS_ASSERT_EQUALS(it->getNormalizedSignal(), 20.);
    it->next();
    TS_ASSERT_EQUALS(it->getNormalizedSignal(), 21.);
    it->next();
    TS_ASSERT_EQUALS(it->getNormalizedSignal(), 30.);
    TS_ASSERT( !it->next());
  }

  void test_iterator_2D_implicitFunction_thatExcludesTheStart()
  {
    // Make an implicit function that will EXCLUDE the points in a corner close to 0,0
    MDImplicitFunction * function = new MDImplicitFunction();
    function->addPlane(MDPlane(VMD(+1., +1.), VMD(4.5, 0.)));

    MDHistoWorkspace_sptr ws = MDEventsTestHelper::makeFakeMDHistoWorkspace(1.0, 2, 10);
    for (size_t i = 0; i < 100; i++)
      ws->setSignalAt(i, double(i));
    MDHistoWorkspaceIterator * it = new MDHistoWorkspaceIterator(ws, function);
    TSM_ASSERT( "This iterator is valid at the start.", it->valid());

    TS_ASSERT_EQUALS(it->getNormalizedSignal(), 4.);
    it->next();
    TS_ASSERT_EQUALS(it->getNormalizedSignal(), 5.);
    it->next();
    TS_ASSERT_EQUALS(it->getNormalizedSignal(), 6.);
    it->next();
    TS_ASSERT_EQUALS(it->getNormalizedSignal(), 7.);
    it->next();
    TS_ASSERT_EQUALS(it->getNormalizedSignal(), 8.);
    it->next();
    TS_ASSERT_EQUALS(it->getNormalizedSignal(), 9.);
    it->next();
    TS_ASSERT_EQUALS(it->getNormalizedSignal(), 13.);
    it->next();
    // And so forth....
  }

  void test_iterator_2D_implicitFunction_thatExcludesEverything()
  {
    // Make an implicit function that will EXCLUDE all the points!
    MDImplicitFunction * function = new MDImplicitFunction();
    function->addPlane(MDPlane(VMD(-1., -1.), VMD(-4.5, 0.)));

    MDHistoWorkspace_sptr ws = MDEventsTestHelper::makeFakeMDHistoWorkspace(1.0, 2, 10);
    for (size_t i = 0; i < 100; i++)
      ws->setSignalAt(i, double(i));
    MDHistoWorkspaceIterator * it = new MDHistoWorkspaceIterator(ws, function);

    TSM_ASSERT( "This iterator is not valid at the start.", !it->valid());
  }

  /** Create several parallel iterators */
  void test_parallel_iterators()
  {
    size_t numPoints = 100;
    MDHistoWorkspace_sptr ws = MDEventsTestHelper::makeFakeMDHistoWorkspace(1.0, 2, 10);
    for (size_t i = 0; i < numPoints; i++)
      ws->setSignalAt(i, double(i));

    // Make 3 iterators
    std::vector<IMDIterator *> iterators = ws->createIterators(3);
    TS_ASSERT_EQUALS( iterators.size(), 3);

    IMDIterator *it;

    it = iterators[0];
    TS_ASSERT_DELTA( it->getSignal(), 0.0, 1e-5);
    TS_ASSERT_EQUALS( it->getDataSize(), 33);
    TS_ASSERT_DELTA( it->getInnerPosition(0,0), 0.5, 1e-5);
    TS_ASSERT_DELTA( it->getInnerPosition(0,1), 0.5, 1e-5);

    it = iterators[1];
    TS_ASSERT_DELTA( it->getSignal(), 33.0, 1e-5);
    TS_ASSERT_EQUALS( it->getDataSize(), 33);
    TS_ASSERT_DELTA( it->getInnerPosition(0,0), 3.5, 1e-5);
    TS_ASSERT_DELTA( it->getInnerPosition(0,1), 3.5, 1e-5);

    it = iterators[2];
    TS_ASSERT_DELTA( it->getSignal(), 66.0, 1e-5);
    TS_ASSERT_EQUALS( it->getDataSize(), 34);
    TS_ASSERT_DELTA( it->getInnerPosition(0,0), 6.5, 1e-5);
    TS_ASSERT_DELTA( it->getInnerPosition(0,1), 6.5, 1e-5);

  }

  void test_predictable_steps()
  {
    MDHistoWorkspace_sptr ws = MDEventsTestHelper::makeFakeMDHistoWorkspace(1.0, 2, 10);
    MDHistoWorkspaceIterator* histoIt = dynamic_cast<MDHistoWorkspaceIterator*>(ws->createIterator());
    size_t expected = 0;
    for (size_t i = 0; i < histoIt->getDataSize(); ++i)
    {
      size_t current = histoIt->getLinearIndex();
      TSM_ASSERT_EQUALS("Has not proceeded in a incremental manner.", expected, current);
      expected = current + 1;
      histoIt->next();
    }
  }

  void test_skip_masked_detectors()
  {
    WritableHistoWorkspace* ws = new WritableHistoWorkspace(
        MDHistoDimension_sptr(new MDHistoDimension("x", "x", "m", 0.0, 10, 100)));

    ws->setMaskValueAt(0, true);  //Mask the first bin
    ws->setMaskValueAt(1, true);  //Mask the second bin
    ws->setMaskValueAt(2, false);  //NOT MASKED
    ws->setMaskValueAt(3, true);  //Mask the second bin
    ws->setMaskValueAt(4, true);  //Mask the second bin
    ws->setMaskValueAt(5, false);  //NOT MASKED

    Mantid::MDEvents::MDHistoWorkspace_sptr ws_sptr(ws);

    MDHistoWorkspaceIterator* histoIt =
        dynamic_cast<MDHistoWorkspaceIterator*>(ws_sptr->createIterator());
    histoIt->next();
    TSM_ASSERT_EQUALS("The first index hit should be 2 since that is the first unmasked one", 2,
        histoIt->getLinearIndex());
    histoIt->next();
    TSM_ASSERT_EQUALS("The first index hit should be 2 since that is the first unmasked one", 5,
        histoIt->getLinearIndex());
  }

  bool doesContainIndex(const std::vector<size_t>& container, const size_t element)
  {
    return std::find(container.begin(), container.end(), element) != container.end();
  }

  void test_neighours_1d()
  {
    const size_t nd = 1;
    MDHistoWorkspace_sptr ws = MDEventsTestHelper::makeFakeMDHistoWorkspace(1.0, nd, 10);
    /*
     1D MDHistoWorkspace

     0 - 1 - 2 - 3 - 4 - 5 - 6 - 7 - 8 - 9

     */

    MDHistoWorkspaceIterator * it = new MDHistoWorkspaceIterator(ws);

    // At first position
    /*
     0 - 1 - 2 - 3 - 4 - 5 - 6 - 7 - 8 - 9
     ^
     |
     */
    std::vector<size_t> neighbourIndexes = it->findNeighbourIndexes();
    TS_ASSERT_EQUALS(1, neighbourIndexes.size());
    // should be on edge
    TSM_ASSERT( "Neighbour at index 0 is 1", doesContainIndex(neighbourIndexes, 1));

    // Go to intermediate position
    /*
     0 - 1 - 2 - 3 - 4 - 5 - 6 - 7 - 8 - 9
     ^
     |
     */
    it->next();
    neighbourIndexes = it->findNeighbourIndexes();
    TS_ASSERT_EQUALS(2, neighbourIndexes.size());
    // should be on edge
    TSM_ASSERT( "Neighbours at index 1 includes 0", doesContainIndex(neighbourIndexes, 0));
    TSM_ASSERT( "Neighbours at index 1 includes 2", doesContainIndex(neighbourIndexes, 2));

    // Go to last position
    /*
     0 - 1 - 2 - 3 - 4 - 5 - 6 - 7 - 8 - 9
     ^
     |
     */
    it->jumpTo(9);
    neighbourIndexes = it->findNeighbourIndexes();
    TSM_ASSERT( "Neighbour at index 9 is 8", doesContainIndex(neighbourIndexes, 8));

  }

  void test_neighbours_2d()
  {
    const size_t nd = 2;
    MDHistoWorkspace_sptr ws = MDEventsTestHelper::makeFakeMDHistoWorkspace(1.0, nd, 4);
    /*
     2D MDHistoWorkspace

     0 - 1 - 2 - 3
     4 - 5 - 6 - 7
     8 - 9 -10 -11
     12-13 -14 -15
     */
    MDHistoWorkspaceIterator * it = new MDHistoWorkspaceIterator(ws);

    // At initial position
    /*
     |0| - 1 - 2 - 3
     4 - 5 - 6 - 7
     8 - 9 -10 -11
     12-13 -14 -15
     */
    std::vector<size_t> neighbourIndexes = it->findNeighbourIndexes();
    TS_ASSERT_EQUALS(3, neighbourIndexes.size());
    // Is on an edge
    TSM_ASSERT( "Neighbour at index 0 is 1", doesContainIndex(neighbourIndexes, 1));
    TSM_ASSERT( "Neighbour at index 0 is 4", doesContainIndex(neighbourIndexes, 4));
    TSM_ASSERT( "Neighbour at index 0 is 5", doesContainIndex(neighbourIndexes, 5));

    // At first position
    /*
     0 -|1|- 2 - 3
     4 - 5 - 6 - 7
     8 - 9 -10 -11
     12-13 -14 -15
     */
    it->next();
    neighbourIndexes = it->findNeighbourIndexes();
    TS_ASSERT_EQUALS(5, neighbourIndexes.size());
    TSM_ASSERT( "Neighbour at index 1 is 0", doesContainIndex(neighbourIndexes, 0));
    TSM_ASSERT( "Neighbour at index 1 is 2", doesContainIndex(neighbourIndexes, 2));
    TSM_ASSERT( "Neighbour at index 1 is 4", doesContainIndex(neighbourIndexes, 4));
    TSM_ASSERT( "Neighbour at index 1 is 5", doesContainIndex(neighbourIndexes, 5));
    TSM_ASSERT( "Neighbour at index 1 is 6", doesContainIndex(neighbourIndexes, 6));

    // At index 9 position
    /*
     0 - 1 - 2 - 3
     4 - 5 - 6 - 7
     8 -|9|-10 -11
     12-13 -14 -15
     */
    it->jumpTo(9);
    neighbourIndexes = it->findNeighbourIndexes();
    TS_ASSERT_EQUALS(8, neighbourIndexes.size());

    TSM_ASSERT( "Neighbour at index 9 is 4", doesContainIndex(neighbourIndexes, 4));
    TSM_ASSERT( "Neighbour at index 9 is 5", doesContainIndex(neighbourIndexes, 5));
    TSM_ASSERT( "Neighbour at index 9 is 6", doesContainIndex(neighbourIndexes, 6));
    TSM_ASSERT( "Neighbour at index 9 is 8", doesContainIndex(neighbourIndexes, 8));
    TSM_ASSERT( "Neighbour at index 9 is 10", doesContainIndex(neighbourIndexes, 10));
    TSM_ASSERT( "Neighbour at index 9 is 12", doesContainIndex(neighbourIndexes, 12));
    TSM_ASSERT( "Neighbour at index 9 is 13", doesContainIndex(neighbourIndexes, 13));
    TSM_ASSERT( "Neighbour at index 9 is 14", doesContainIndex(neighbourIndexes, 14));

    // At last position
    /*
     0 - 1 - 2 - 3
     4 - 5 - 6 - 7
     8 - 9 -10 -11
     12-13 -14 -|15|
     */
    it->jumpTo(15);
    neighbourIndexes = it->findNeighbourIndexes();
    TS_ASSERT_EQUALS(3, neighbourIndexes.size());
    // Is on an edge
    TSM_ASSERT( "Neighbour at index 15 is 10", doesContainIndex(neighbourIndexes, 10));
    TSM_ASSERT( "Neighbour at index 15 is 11", doesContainIndex(neighbourIndexes, 11));
    TSM_ASSERT( "Neighbour at index 15 is 14", doesContainIndex(neighbourIndexes, 14));
  }

  void test_neighbours_3d()
  {
    const size_t nd = 3;
    MDHistoWorkspace_sptr ws = MDEventsTestHelper::makeFakeMDHistoWorkspace(1.0, nd, 4);
    /*
     3D MDHistoWorkspace

     [[[ 0  1  2  3]
     [ 4  5  6  7]
     [ 8  9 10 11]
     [12 13 14 15]]

     [[16 17 18 19]
     [20 21 22 23]
     [24 25 26 27]
     [28 29 30 31]]

     [[32 33 34 35]
     [36 37 38 39]
     [40 41 42 43]
     [44 45 46 47]]

     [[48 49 50 51]
     [52 53 54 55]
     [56 57 58 59]
     [60 61 62 63]]]
     */

    MDHistoWorkspaceIterator * it = new MDHistoWorkspaceIterator(ws);

    // Start at Index = 0
    std::vector<size_t> neighbourIndexes = it->findNeighbourIndexes();
    TS_ASSERT_EQUALS(7, neighbourIndexes.size());
    // Is on an edge
    TS_ASSERT(doesContainIndex(neighbourIndexes, 1));
    TS_ASSERT(doesContainIndex(neighbourIndexes, 4));
    TS_ASSERT(doesContainIndex(neighbourIndexes, 5));
    TS_ASSERT(doesContainIndex(neighbourIndexes, 16));
    TS_ASSERT(doesContainIndex(neighbourIndexes, 17));
    TS_ASSERT(doesContainIndex(neighbourIndexes, 20));
    TS_ASSERT(doesContainIndex(neighbourIndexes, 21));

    // Move to index 1
    it->jumpTo(1);
    neighbourIndexes = it->findNeighbourIndexes();
    TS_ASSERT_EQUALS(11, neighbourIndexes.size());
    std::vector<size_t> expected_neighbours = boost::assign::list_of(0)(2)(4)(5)(6)(16)(17)(18)(20)(21)(22)(22).convert_to_container<std::vector<size_t>>();
    for (auto i = expected_neighbours.begin(); i != expected_neighbours.end(); ++i)
    {
      TS_ASSERT(doesContainIndex(neighbourIndexes, *i));
    }

    // Move to index 21
    it->jumpTo(21);
    neighbourIndexes = it->findNeighbourIndexes();
    TSM_ASSERT_EQUALS("Should have 3^n-1 neighbours here", 26, neighbourIndexes.size());
    // Is completely enclosed
    expected_neighbours = boost::assign::list_of(0)(1)(2)(4)(5)(6)(8)(9)(10)(16)(17)(18)(22)(20)(24)(25)(26)(32)(33)(34)(37)(38)(36)(41)(40)(42).convert_to_container<std::vector<size_t>>();

    for (auto i = expected_neighbours.begin(); i != expected_neighbours.end(); ++i)
    {
      TS_ASSERT(doesContainIndex(neighbourIndexes, *i));
    }

    // Move to index 63. The last index.
    it->jumpTo(63);
    neighbourIndexes = it->findNeighbourIndexes();
    TS_ASSERT_EQUALS(7, neighbourIndexes.size());
    // Is completely enclosed
    expected_neighbours = boost::assign::list_of(42)(43)(46)(47)(58)(59)(62).convert_to_container<std::vector<size_t>>();

    for (auto i = expected_neighbours.begin(); i != expected_neighbours.end(); ++i)
    {
      TS_ASSERT(doesContainIndex(neighbourIndexes, *i));
    }

  }

};

class MDHistoWorkspaceIteratorTestPerformance: public CxxTest::TestSuite
{
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static MDHistoWorkspaceIteratorTestPerformance *createSuite()
  {
    return new MDHistoWorkspaceIteratorTestPerformance();
  }
  static void destroySuite(MDHistoWorkspaceIteratorTestPerformance *suite)
  {
    delete suite;
  }

  MDHistoWorkspace_sptr ws;
  MDHistoWorkspace_sptr small_ws;

  MDHistoWorkspaceIteratorTestPerformance()
  {
    // 125^3 workspace = about 2 million
    ws = MDEventsTestHelper::makeFakeMDHistoWorkspace(1.0, 3, 125);
    // 10^3 workspace = 21000
    small_ws = MDEventsTestHelper::makeFakeMDHistoWorkspace(1.0,3,30);
  }

  /** ~Two million iterations */
  void test_iterator_3D_signalAndErrorOnly()
  {
    MDHistoWorkspaceIterator * it = new MDHistoWorkspaceIterator(ws, new SkipNothing);
    do
    {
      signal_t sig = it->getNormalizedSignal();
      signal_t err = it->getNormalizedError();
      UNUSED_ARG(sig);
      UNUSED_ARG(err);
    } while (it->next());
  }

  /** ~Two million iterations */
  void test_iterator_3D_withGetVertexes()
  {
    MDHistoWorkspaceIterator * it = new MDHistoWorkspaceIterator(ws, new SkipNothing);
    size_t numVertices;
    do
    {
      signal_t sig = it->getNormalizedSignal();
      signal_t err = it->getNormalizedError();
      coord_t * vertexes = it->getVertexesArray(numVertices);
      delete[] vertexes;
      UNUSED_ARG(sig);
      UNUSED_ARG(err);
    } while (it->next());
  }

  /** ~Two million iterations */
  void test_iterator_3D_withGetCenter()
  {
    MDHistoWorkspaceIterator * it = new MDHistoWorkspaceIterator(ws, new SkipNothing);
    do
    {
      signal_t sig = it->getNormalizedSignal();
      signal_t err = it->getNormalizedError();
      VMD center = it->getCenter();
      UNUSED_ARG(sig);
      UNUSED_ARG(err);
    } while (it->next());
  }

  /** Use jumpTo() */
  void test_iterator_3D_withGetCenter_usingJumpTo()
  {
    MDHistoWorkspaceIterator * it = new MDHistoWorkspaceIterator(ws, new SkipNothing);
    int max = int(it->getDataSize());
    for (int i = 0; i < max; i++)
    {
      it->jumpTo(size_t(i));
      signal_t sig = it->getNormalizedSignal();
      signal_t err = it->getNormalizedError();
      VMD center = it->getCenter();
      UNUSED_ARG(sig);
      UNUSED_ARG(err);
    }
  }

  void test_masked_get_vertexes_call_throws()
  {
    boost::scoped_ptr<MDHistoWorkspaceIterator> it(new MDHistoWorkspaceIterator(ws, new SkipNothing));
    size_t numVertexes;
    size_t outDimensions = 1;
    bool maskDim[] =
    { true };
    TSM_ASSERT_THROWS("Not implemented yet, should throw",
        it->getVertexesArray(numVertexes, outDimensions, maskDim), std::runtime_error);
  }

  void test_getIsMasked()
  {
    //Characterisation test
    MDHistoWorkspaceIterator iterator(small_ws, new SkipNothing());
    for (size_t i = 0; i < small_ws->getNPoints(); ++i)
    {
      std::stringstream stream;
      stream << "Masking is different from the workspace at index: " << i;
      TSM_ASSERT_EQUALS(stream.str(), small_ws->getIsMaskedAt(i), iterator.getIsMasked());
      iterator.next();
    }
  }

  void test_findNeighbours()
  {
    MDHistoWorkspaceIterator iterator(small_ws, new SkipNothing());
    do
    {
      iterator.findNeighbourIndexes();
    } while (iterator.next());
  }

};

#endif /* MANTID_MDEVENTS_MDHISTOWORKSPACEITERATORTEST_H_ */

