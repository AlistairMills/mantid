#include "vtkSQWEventReader.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkTransform.h"
#include "vtkFloatArray.h"
#include "vtkUnstructuredGrid.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkPVClipDataSet.h"
#include "vtkBox.h"

#include "MantidVatesAPI/vtkMDHexFactory.h"
#include "MantidVatesAPI/vtkMDQuadFactory.h"
#include "MantidVatesAPI/vtkMDLineFactory.h"
#include "MantidVatesAPI/IgnoreZerosThresholdRange.h"
#include "MantidVatesAPI/FilteringUpdateProgressAction.h"
#include "MantidVatesAPI/MDLoadingViewAdapter.h"

vtkStandardNewMacro(vtkSQWEventReader);

using namespace Mantid::VATES;
using Mantid::Geometry::IMDDimension_sptr;
using Mantid::Geometry::IMDDimension_sptr;


vtkSQWEventReader::vtkSQWEventReader() : 
  m_presenter(NULL),
  m_loadInMemory(false),
  m_depth(1),
  m_time(0)
{
  this->FileName = NULL;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);

}

vtkSQWEventReader::~vtkSQWEventReader()
{
  delete m_presenter;
  this->SetFileName(0);
}

void vtkSQWEventReader::SetDepth(int depth)
{
  size_t temp = depth;
  if(m_depth != temp)
  {
   this->m_depth = temp;
   this->Modified();
  }
}

size_t vtkSQWEventReader::getRecursionDepth() const
{
  return this->m_depth;
}

bool vtkSQWEventReader::getLoadInMemory() const
{
  return m_loadInMemory;
}

double vtkSQWEventReader::getTime() const
{
  return m_time;
}

/**
  Sets algorithm in-memory property. If this is changed, the file is reloaded.
  @param inMemory : true if the entire file should be loaded into memory.
*/
void vtkSQWEventReader::SetInMemory(bool inMemory)
{
  if(m_loadInMemory != inMemory)
  {
    this->Modified(); 
  }
  m_loadInMemory = inMemory;
}


/**
  Gets the geometry xml from the workspace. Allows object panels to configure themeselves.
  @return geometry xml const * char reference.
*/
const char* vtkSQWEventReader::GetInputGeometryXML()
{
  if(m_presenter == NULL)
  {
    return "";
  }
  try
  {
    return m_presenter->getGeometryXML().c_str();
  }
  catch(std::runtime_error&)
  {
    return "";
  }
}

int vtkSQWEventReader::RequestData(vtkInformation * vtkNotUsed(request), vtkInformationVector ** vtkNotUsed(inputVector), vtkInformationVector *outputVector)
{
  using namespace Mantid::VATES;
  //get the info objects
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  FilterUpdateProgressAction<vtkSQWEventReader> loadingProgressUpdate(this, "Loading...");
  FilterUpdateProgressAction<vtkSQWEventReader> drawingProgressUpdate(this, "Drawing...");

  if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()))
  {
    // usually only one actual step requested
    m_time =outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());
  }

  ThresholdRange_scptr thresholdRange(new IgnoreZerosThresholdRange());
  vtkMDHexFactory* hexahedronFactory = new vtkMDHexFactory(thresholdRange, "signal");
  vtkMDQuadFactory* quadFactory = new vtkMDQuadFactory(thresholdRange, "signal");
  vtkMDLineFactory* lineFactory = new vtkMDLineFactory(thresholdRange, "signal");

  hexahedronFactory->SetSuccessor(quadFactory);
  quadFactory->SetSuccessor(lineFactory);

  hexahedronFactory->setTime(m_time);
  vtkDataSet* product = m_presenter->execute(hexahedronFactory, loadingProgressUpdate, drawingProgressUpdate);

  //-------------------------------------------------------- Corrects problem whereby boundaries not set propertly in PV.
  vtkBox* box = vtkBox::New();
  box->SetBounds(product->GetBounds());
  vtkPVClipDataSet* clipper = vtkPVClipDataSet::New();
  clipper->SetInputData(product);
  clipper->SetClipFunction(box);
  clipper->SetInsideOut(true);
  clipper->Update();
  vtkDataSet* clipperOutput = clipper->GetOutput();
   //--------------------------------------------------------

  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  output->ShallowCopy(clipperOutput);

  m_presenter->setAxisLabels(output);

  clipper->Delete();
  
  return 1;
}

int vtkSQWEventReader::RequestInformation(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  if(m_presenter == NULL)
  {
    m_presenter = new SQWLoadingPresenter(new MDLoadingViewAdapter<vtkSQWEventReader>(this), FileName);
    m_presenter->executeLoadMetadata();
    setTimeRange(outputVector);
  }
  return 1;
}

void vtkSQWEventReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

int vtkSQWEventReader::CanReadFile(const char* fname)
{
  SQWLoadingPresenter temp(new MDLoadingViewAdapter<vtkSQWEventReader>(this), fname);
  return temp.canReadFile();
}

unsigned long vtkSQWEventReader::GetMTime()
{
  return Superclass::GetMTime();
}

/**
  Update/Set the progress.
  @param progress : progress increment.
  @param message : progress message
*/
void vtkSQWEventReader::updateAlgorithmProgress(double progress, const std::string& message)
{
  progressMutex.lock();
  this->SetProgressText(message.c_str());
  this->UpdateProgress(progress);
  progressMutex.unlock();
}

/** Helper function to setup the time range.
@param outputVector : vector onto which the time range will be set.
*/
void vtkSQWEventReader::setTimeRange(vtkInformationVector* outputVector)
{
  if(m_presenter->hasTDimensionAvailable())
  {
    vtkInformation *outInfo = outputVector->GetInformationObject(0);
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_LABEL_ANNOTATION(),
                 m_presenter->getTimeStepLabel().c_str());
    std::vector<double> timeStepValues = m_presenter->getTimeStepValues();
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(), &timeStepValues[0],
      static_cast<int> (timeStepValues.size()));
    double timeRange[2];
    timeRange[0] = timeStepValues.front();
    timeRange[1] = timeStepValues.back();

    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), timeRange, 2);
  }
}

/*
Getter for the workspace type name.
*/
char* vtkSQWEventReader::GetWorkspaceTypeName()
{
  //Forward request on to MVP presenter
  typeName = m_presenter->getWorkspaceTypeName();
  return const_cast<char*>(typeName.c_str());

}

