#ifndef H_CONVERT_TO_MDEVENTS_HISTO_WS
#define H_CONVERT_TO_MDEVENTS_HISTO_WS
/**TODO: FOR DEPRICATION */ 
//
#include "MantidKernel/System.h"
#include "MantidKernel/Exception.h"
#include "MantidAPI/Algorithm.h" 

#include "MantidDataObjects/EventWorkspace.h"
#include "MantidDataObjects/Workspace2D.h"

#include "MantidAPI/NumericAxis.h"
#include "MantidAPI/Progress.h"
#include "MantidKernel/PhysicalConstants.h"


#include "MantidMDEvents/MDEventWSWrapper.h"
#include "MantidMDEvents/MDEvent.h"

#include "MantidMDAlgorithms/ConvertToMDEventsWSInterface.h"

#include "MantidMDEvents/ConvToMDPreprocDet.h"
// coordinate transformation
#include "MantidMDAlgorithms/ConvertToMDEventsTransfInterface.h"
#include "MantidMDAlgorithms/ConvertToMDEventsTransfNoQ.h"
#include "MantidMDAlgorithms/ConvertToMDEventsTransfModQ.h"
#include "MantidMDAlgorithms/ConvertToMDEventsTransfQ3D.h"

namespace Mantid
{
namespace MDAlgorithms
{
/** The templated class to transform matrix workspace into MDEvent workspace when matrix workspace is ragged
   *
   * @date 11-10-2011

    Copyright &copy; 2010 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

        This file is part of Mantid.

        Mantid is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 3 of the License, or
        (at your option) any later version.

        Mantid is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program.  If not, see <http://www.gnu.org/licenses/>.

        File/ change history is stored at: <https://github.com/mantidproject/mantid>
        Code Documentation is available at: <http://doxygen.mantidproject.org>
*/

// service variable used for efficient filling of the MD event WS  -> should be moved to configuration;
#define SPLIT_LEVEL  8192

//-----------------------------------------------
// Method to process rugged Histogram workspace
template<ConvertToMD::QMode Q, ConvertToMD::AnalMode MODE, ConvertToMD::CnvrtUnits CONV,ConvertToMD::SampleType Sample>
class ConvertToMDEventsWS<ConvertToMD::Ws2DHistoType,Q,MODE,CONV,Sample>: public ConvertToMDEventsWSBase 
{
    /// shalow class which is invoked from processQND procedure and describes the transformation from workspace coordinates to target coordinates
    /// presumably will be completely inlined
   // the instanciation of the class which does the transformation itself
     CoordTransformer<Q,MODE,CONV,ConvertToMD::Histogram,Sample> QE_TRANSF; 
     // 

     // not yet parallel
     virtual size_t conversionChunk(size_t job_ID){UNUSED_ARG(job_ID); return 0;}
public:
    size_t  setUPConversion(const MDEvents::MDWSDescriptionDepricated &WSD, boost::shared_ptr<MDEvents::MDEventWSWrapper> inWSWrapper)
    {
        size_t numSpec=ConvertToMDEventsWSBase::setUPConversion(WSD,inWSWrapper);
        // initiate the templated class which does the conversion of workspace data into MD WS coordinates;
        QE_TRANSF.setUpTransf(this); 

        return numSpec;
    }

    void runConversion(API::Progress *pProg)
    {
       // counder for the number of events
        size_t n_added_events(0),n_buf_events(0);
        //
        Mantid::API::BoxController_sptr bc = pWSWrapper->pWorkspace()->getBoxController();
        size_t nEventsInWS                 = pWSWrapper->pWorkspace()->getNPoints();
        size_t lastNumBoxes                = bc->getTotalNumMDBoxes();

        //

        const size_t specSize = this->inWS2D->blocksize();    
        // preprocessed detectors associate each spectra with a detector (position)
        size_t nValidSpectra  = pDetLoc->nDetectors();

        // number of dimesnions
        std::vector<coord_t>  Coord(n_dims);             // coordinates for single event
     // if any property dimension is outside of the data range requested, the job is done;
        if(!QE_TRANSF.calcGenericVariables(Coord,n_dims))return; 

        // take at least bufSize amout of data in one run for efficiency
        size_t buf_size     = ((specSize>SPLIT_LEVEL)?specSize:SPLIT_LEVEL);
        // allocate temporary buffer for MD Events data 
        std::vector<float>    sig_err(2*buf_size);       // array for signal and error. 
        std::vector<uint16_t> run_index(buf_size);       // Buffer run index for each event 
        std::vector<uint32_t> det_ids(buf_size);         // Buffer of det Id-s for each event

        std::vector<coord_t>  allCoord(n_dims*buf_size); // MD events coordinates buffer
        size_t n_coordinates = 0;


  
        //External loop over the spectra:
        for (size_t i = 0; i < nValidSpectra; ++i)
        {
            size_t iSpctr             = pDetLoc->getDetSpectra(i);
            int32_t det_id            = pDetLoc->getDetID(i);

            const MantidVec& X        = inWS2D->readX(iSpctr);
            const MantidVec& Signal   = inWS2D->readY(iSpctr);
            const MantidVec& Error    = inWS2D->readE(iSpctr);

            // calculate the coordinates which depend on detector posision 
            if(!QE_TRANSF.calcYDepCoordinates(Coord,i))continue;   // skip y outsize of the range;

         //=> START INTERNAL LOOP OVER THE "TIME"
            for (size_t j = 0; j < specSize; ++j)
            {
                // drop NaN events
                if(isNaN(Signal[j]))continue;

                if(!QE_TRANSF.calcMatrixCoord(X,i,j,Coord))continue; // skip ND outside the range
                //  ADD RESULTING EVENTS TO THE BUFFER
                float ErrSq = float(Error[j]*Error[j]);

                // coppy all data into data buffer for future transformation into events;
                sig_err[2*n_buf_events+0]=float(Signal[j]);
                sig_err[2*n_buf_events+1]=ErrSq;
                run_index[n_buf_events]  = runIndex;
                det_ids[n_buf_events]    = det_id;
                for(size_t ii=0;ii<n_dims;ii++){
                    allCoord[n_coordinates++]=Coord[ii];
                }
                //allCoord.insert(itc,Coord.begin(),Coord.end());
                //std::advance(itc,n_dims);
#ifndef _DEBUG_EXCLUDE_ADD_TO_WORKSPACE
                n_buf_events++;
                if(n_buf_events>=buf_size){
                   pWSWrapper->addMDData(sig_err,run_index,det_ids,allCoord,n_buf_events);
                   n_added_events+=n_buf_events;
                   nEventsInWS   +=n_buf_events;

                   // reset buffer counts
                   n_coordinates=0;
                   n_buf_events=0;
                   if (bc->shouldSplitBoxes(nEventsInWS,n_added_events,lastNumBoxes))
                   {
                        // Do all the adding tasks
                        //   tp.joinAll();    
                        // Now do all the splitting tasks
                        //ws->splitAllIfNeeded(ts);
                        pWSWrapper->pWorkspace()->splitAllIfNeeded(NULL);
                        //if (ts->size() > 0)       tp.joinAll();
                        // Count the new # of boxes.
                        lastNumBoxes = pWSWrapper->pWorkspace()->getBoxController()->getTotalNumMDBoxes();
                        n_added_events=0;
                        pProg->report(i);
                    }
                }
#else
                n_coordinates=0;
                n_buf_events=0;
                n_added_events=0;
                UNUSED_ARG(lastNumBoxes);
#endif       
            } // end spectra loop
      
        } // end detectors loop;

       if(n_buf_events>0){
              pWSWrapper->addMDData(sig_err,run_index,det_ids,allCoord,n_buf_events);
              n_buf_events=0;
        }

        pWSWrapper->pWorkspace()->splitAllIfNeeded(NULL); 
        pWSWrapper->pWorkspace()->refreshCache();
        pProg->report();          
    }
};

 
} // endNamespace MDAlgorithms
} // endNamespace Mantid

#endif
