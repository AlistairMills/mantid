#include "vtkSQWReader.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkMath.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkTransform.h"
#include "vtkFloatArray.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "MantidMDAlgorithms/PlaneImplicitFunction.h"
#include "MantidVatesAPI/MultiDimensionalDbPresenter.h"
#include "MantidVatesAPI/vtkStructuredGridFactory.h"
#include "MantidVatesAPI/TimeToTimeStep.h"
#include "MantidMDAlgorithms/Load_MDWorkspace.h"

vtkStandardNewMacro(vtkSQWReader);

vtkSQWReader::vtkSQWReader() : m_presenter()
{
  this->FileName = NULL;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

vtkSQWReader::~vtkSQWReader()
{
  this->SetFileName(0);
}


int vtkSQWReader::RequestData(vtkInformation* vtkNotUsed(request), vtkInformationVector ** vtkNotUsed(inputVector), vtkInformationVector *outputVector)
{
  using namespace Mantid::VATES;
  //get the info objects
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  vtkStructuredGrid *output = vtkStructuredGrid::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  int time = 0;
  if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS()))
  {
     // usually only one actual step requested
     time = static_cast<int>(outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS())[0]);
  }

  RebinningKnowledgeSerializer serializer;
  vtkStructuredGridFactory<TimeToTimeStep> factory("signal", time);
  vtkStructuredGrid* structuredMesh = vtkStructuredGrid::SafeDownCast(m_presenter.getMesh(serializer, factory));
  structuredMesh->GetCellData()->AddArray(m_presenter.getScalarDataFromTime(factory));

  int subext[6];
  outInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), subext);

  output->SetExtent(subext);
  output->ShallowCopy(structuredMesh);

  return 1;
}

int vtkSQWReader::RequestInformation(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  vtkInformation *outInfo = outputVector->GetInformationObject(0);


  Mantid::MDAlgorithms::Load_MDWorkspace wsLoaderAlg;
  wsLoaderAlg.initialize();
  std::string wsId = "InputMDWs";
  wsLoaderAlg.setPropertyValue("inFilename", FileName);
  wsLoaderAlg.setPropertyValue("MDWorkspace",wsId);
  m_presenter.execute(wsLoaderAlg, wsId);
  int wholeExtent[6];

  Mantid::VATES::VecExtents extents = m_presenter.getExtents();
  wholeExtent[0] = extents[0];
  wholeExtent[1] = extents[1];
  wholeExtent[2] = extents[2];
  wholeExtent[3] = extents[3];
  wholeExtent[4] = extents[4];
  wholeExtent[5] = extents[5];

  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
               wholeExtent, 6);

  std::vector<double> timeStepValues = m_presenter.getTimesteps();

  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(), &timeStepValues[0], static_cast<int>(timeStepValues.size()));
  double timeRange[2];
  timeRange[0] = timeStepValues.front();
  timeRange[1] = timeStepValues.back();

  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), timeRange, 2);
  return 1;
}

void vtkSQWReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

int vtkSQWReader::CanReadFile(const char* vtkNotUsed(fname))
{
	return 1; //TODO: Apply checks here.
}
