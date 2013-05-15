//----------------------
// Includes
//----------------------
#include "MantidQtCustomInterfaces/StepScan.h"
#include "MantidAPI/AlgorithmManager.h"
#include "MantidAPI/IEventWorkspace.h"
#include "MantidAPI/InstrumentDataService.h"
#include "MantidAPI/LiveListenerFactory.h"
#include "MantidKernel/InstrumentInfo.h"
#include "MantidKernel/TimeSeriesProperty.h"
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>

namespace MantidQt
{
namespace CustomInterfaces
{

//Register the class with the factory
DECLARE_SUBWINDOW(StepScan);

using namespace Mantid::Kernel;
using namespace Mantid::API;

/// Constructor
StepScan::StepScan(QWidget *parent)
  : UserSubWindow(parent), m_dataReloadNeeded(false),
    m_instrument(ConfigService::Instance().getInstrument().name()),
    m_addObserver(*this, &StepScan::handleAddEvent),
    m_replObserver(*this, &StepScan::handleReplEvent)
{
}

StepScan::~StepScan()
{
  // Stop live data collection, if running
  stopLiveListener();
  // Disconnect the observers for the mask workspace combobox
  AnalysisDataService::Instance().notificationCenter.removeObserver(m_addObserver);
  AnalysisDataService::Instance().notificationCenter.removeObserver(m_replObserver);
  // Clean up any hidden workspaces created
  cleanupWorkspaces();
}

/// Set up the dialog layout
void StepScan::initLayout()
{
  m_uiForm.setupUi(this);

  // I couldn't see a way to set a validator on a qlineedit in designer
  m_uiForm.xmin->setValidator(new QDoubleValidator(m_uiForm.xmin));
  m_uiForm.xmax->setValidator(new QDoubleValidator(m_uiForm.xmax));

  // Try to connect to live listener for default instrument to see if live button should be enabled
  // Enable the button if the connection is successful. Will be disabled otherwise.
  m_uiForm.liveButton->setEnabled(LiveListenerFactory::Instance().checkConnection(m_instrument));
  connect( m_uiForm.liveButton, SIGNAL(clicked(bool)), SLOT(triggerLiveListener(bool)), Qt::QueuedConnection );

  connect( m_uiForm.launchInstView, SIGNAL(clicked()), SLOT(launchInstrumentWindow()) );

  connect( m_uiForm.mWRunFiles, SIGNAL(filesFound()), SLOT(loadFile()) );
  connect( this, SIGNAL(logsAvailable(const Mantid::API::MatrixWorkspace_const_sptr &)),
           SLOT(fillPlotVarCombobox(const Mantid::API::MatrixWorkspace_const_sptr &)) );

  connect( m_uiForm.helpButton, SIGNAL(clicked()), SLOT(helpClicked()) );
  connect( m_uiForm.startButton, SIGNAL(clicked()), SLOT(runStepScanAlg()) );
  connect( m_uiForm.closeButton, SIGNAL(clicked()), this->parent(), SLOT(close()) );
}

void StepScan::cleanupWorkspaces()
{
  if ( ! m_inputWSName.empty() )
  {
    // Get a reference to the analysis data service
    auto& ADS = AnalysisDataService::Instance();
    // Clean up, checking first that those that may not exist do (to avoid a warning in the log)
    ADS.remove( m_inputWSName );
    const std::string monitorWSName = m_inputWSName + "_monitors";
    if ( ADS.doesExist( monitorWSName ) ) ADS.remove( monitorWSName );
    m_inputWSName.clear();
    if ( ADS.doesExist( m_plotWSName ) ) ADS.remove( m_plotWSName );
    m_plotWSName.clear();
  }

  m_uiForm.startButton->setEnabled(false);
  m_uiForm.launchInstView->setEnabled(false);
  m_uiForm.plotVariable->setEnabled(false);
  // Disconnect anything listening to the comboboxes
  m_uiForm.plotVariable->disconnect(SIGNAL(currentIndexChanged(const QString &)));
  m_uiForm.normalization->disconnect(SIGNAL(currentIndexChanged(const QString &)));
}

/** Slot that is called when the live data button is clicked
 *  @param checked Whether the button is being enabled (true) or disabled
 */
void StepScan::triggerLiveListener(bool checked)
{
  if ( checked )
  {
    startLiveListener();
  }
  else
  {
    stopLiveListener();
    cleanupWorkspaces();
  }
}

void StepScan::startLiveListener()
{
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

  // Remove any previously-loaded workspaces
  cleanupWorkspaces();

  // TODO: Run entirely asynchronously (see AlgorithmRunner)
  IAlgorithm_sptr startLiveData = AlgorithmManager::Instance().create("StartLiveData");
  startLiveData->setProperty("UpdateEvery",5.0);
  startLiveData->setProperty("Instrument",m_instrument);
  m_inputWSName = "__live";
  startLiveData->setProperty("OutputWorkspace",m_inputWSName);
  startLiveData->execute();

  // Keep track of the algorithm that's pulling in the live data
  m_monitorLiveData = startLiveData->getProperty("MonitorLiveData");

  setupOptionControls();

  QApplication::restoreOverrideCursor();
}

IAlgorithm_sptr StepScan::stopLiveListener()
{
  // TODO: Make return type IAlgorithm_const_sptr (requires ticket #6811)
  IAlgorithm_sptr theAlgorithmBeingCancelled = m_monitorLiveData;
  if (m_monitorLiveData)
  {
    m_monitorLiveData->cancel();
    m_monitorLiveData.reset();
  }
  return theAlgorithmBeingCancelled;
}

void StepScan::loadFile()
{
  const QString filename = m_uiForm.mWRunFiles->getFirstFilename();
  // This handles the fact that mwRunFiles emits the filesFound signal more than
  // we want (on some platforms). TODO: Consider dealing with this up in mwRunFiles.
  if ( filename != m_inputFilename || m_dataReloadNeeded )
  {
    m_inputFilename = filename;

    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

    // Remove any previously-loaded workspaces
    cleanupWorkspaces();

    // TODO: Run entirely asynchronously (see AlgorithmRunner or AlgorithmObserver)
    IAlgorithm_sptr alg = AlgorithmManager::Instance().create("LoadEventNexus");
    alg->setPropertyValue("Filename", filename.toStdString());
    m_inputWSName = "__" + QFileInfo(filename).baseName().toStdString();
    alg->setPropertyValue("OutputWorkspace", m_inputWSName);
    alg->setProperty("LoadMonitors", true);
    if ( alg->execute() )  // executeAsync???
    {
      QApplication::restoreOverrideCursor();
      m_dataReloadNeeded = false;
      setupOptionControls();
    }
    else
    {
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(this,"File loading failed","Is this an event nexus file?");
    }
  }
}

void StepScan::setupOptionControls()
{
  MatrixWorkspace_const_sptr outWS = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>(m_inputWSName);
  // Trigger population of the logs combobox
  emit logsAvailable( outWS );
  fillNormalizationCombobox();
  // Enable the button to launch the instrument view (for defining a mask)
  m_uiForm.launchInstView->setEnabled(true);
}

void StepScan::launchInstrumentWindow()
{
  // Gotta do this in python
  std::string pyCode = "instrument_view = getInstrumentView('" + m_inputWSName + "',2)\n"
                       "instrument_view.show()";

  runPythonCode( QString::fromStdString(pyCode) );

  // Attach the observers so that if a mask workspace is generated over in the instrument view,
  // it is automatically selected by the combobox over here
  AnalysisDataService::Instance().notificationCenter.addObserver(m_addObserver);
  AnalysisDataService::Instance().notificationCenter.addObserver(m_replObserver);
}

void StepScan::fillPlotVarCombobox(const MatrixWorkspace_const_sptr& ws)
{
  // Hold the name of the scan index log in a common place
  const std::string scan_index("scan_index");
  // Clear the combobox and immediately re-insert 'scan_index' (so it's the first entry)
  m_uiForm.plotVariable->clear();
  m_uiForm.plotVariable->addItem( QString::fromStdString(scan_index) );

  // First check that the provided workspace has the scan_index - complain if it doesn't
  try {
    auto scan_index_prop = ws->run().getTimeSeriesProperty<int>(scan_index);
    if ( !m_uiForm.liveButton->isChecked() && scan_index_prop->realSize() < 2 )
    {
      QMessageBox::warning(this,"scan_index log empty","This data does not appear to be an alignment scan");
      return;
    }
  } catch ( std::exception& ) {
    QMessageBox::warning(this,"scan_index log not found","Is this an ADARA-style dataset?");
    return;
  }

  expandPlotVarCombobox( ws );

  // Now that this has been populated, allow the user to select from it
  m_uiForm.plotVariable->setEnabled(true);
  // Now's the time to enable the start button as well
  m_uiForm.startButton->setEnabled(true);
}

void StepScan::expandPlotVarCombobox(const Mantid::API::MatrixWorkspace_const_sptr& ws)
{
  // This is unfortunately more or less a copy of SumEventsByLogValue::getNumberSeriesLogs
  // but I want to populate the box before running the algorithm
  const auto & logs = ws->run().getLogData();
  for ( auto log = logs.begin(); log != logs.end(); ++log )
  {
    const QString logName = QString::fromStdString( (*log)->name() );
    // Don't add scan_index - that's already there
    if ( logName == "scan_index" ) continue;
    // Try to cast to an ITimeSeriesProperty
    auto tsp = dynamic_cast<const ITimeSeriesProperty*>(*log);
    // Move on to the next one if this is not a TSP
    if ( tsp == NULL ) continue;
    // Don't keep ones with only one entry
    if ( tsp->realSize() < 2 ) continue;
    // Now make sure it's either an int or double tsp
    if ( dynamic_cast<TimeSeriesProperty<double>* >(*log) || dynamic_cast<TimeSeriesProperty<int>* >(*log))
    {
      // Add it to the list if it isn't already there
      if ( m_uiForm.plotVariable->findText( logName ) == -1 )
      {
        m_uiForm.plotVariable->addItem( logName );
      }
    }
  }
}

void StepScan::fillNormalizationCombobox()
{
  clearNormalizationCombobox();

  // Add the monitors to the normalization combobox
  try {
    MatrixWorkspace_const_sptr monWS = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>(m_inputWSName+"_monitors");
    for ( std::size_t i = 0; i < monWS->getNumberHistograms(); ++i )
    {
      const std::string monitorName = monWS->getDetector(i)->getName();
      m_uiForm.normalization->addItem( QString::fromStdString( monitorName ) );
    }
  } catch (Exception::NotFoundError&) {
    // No monitors workspace....carry on
  }
}

void StepScan::clearNormalizationCombobox()
{
  // If there are more than 3 entries in the combobox (nothing, time, proton_charge) then
  // remove any stale ones
  while ( m_uiForm.normalization->count() > 3 )
  {
    m_uiForm.normalization->removeItem(m_uiForm.normalization->count()-1);
  }
}

IAlgorithm_sptr StepScan::setupStepScanAlg()
{
  IAlgorithm_sptr stepScan = AlgorithmManager::Instance().create("StepScan");
  // The table should not be hidden, so leave off the prefix
  m_tableWSName = m_inputWSName.substr(2) + "_StepScan";
  stepScan->setPropertyValue("OutputWorkspace", m_tableWSName);

  const QString maskWS = m_uiForm.maskWorkspace->currentText();
  stepScan->setPropertyValue("MaskWorkspace",maskWS.toStdString());

  const QString xminStr = m_uiForm.xmin->text();
  const QString xmaxStr = m_uiForm.xmax->text();
  const double xmin = xminStr.toDouble();
  const double xmax = xmaxStr.toDouble();
  // If both set, check that xmax > xmin
  if ( !xminStr.isEmpty() && !xmaxStr.isEmpty() && xmin >= xmax )
  {
    QMessageBox::critical(this,"Invalid filtering range set","For the filtering range, min has to be less than max");
    return IAlgorithm_sptr();
  }
  if ( ! xminStr.isEmpty() ) stepScan->setProperty("XMin",xmin);
  if ( ! xmaxStr.isEmpty() ) stepScan->setProperty("XMax",xmax);
  // TODO: Update when entries added to rangeUnit combobox

  // If any of the filtering options were set, next time round we'll need to reload the data
  // as they cause the workspace to be changed
  if ( !maskWS.isEmpty() || !xminStr.isEmpty() || !xmaxStr.isEmpty() ) m_dataReloadNeeded = true;

  return stepScan;
}

void StepScan::runStepScanAlg()
{
  IAlgorithm_sptr stepScan = setupStepScanAlg();
  if ( !stepScan ) return;

  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

  if ( m_uiForm.liveButton->isChecked() )  // Live data
  {
    runStepScanAlgLive(stepScan->toString());
  }
  else  // Offline data
  {
    if ( m_dataReloadNeeded ) loadFile(); // Reload if workspace isn't fresh
    stepScan->setPropertyValue("InputWorkspace", m_inputWSName);
    stepScan->execute();
  }

  QApplication::restoreOverrideCursor();

  // Now that the algorithm's been run, connect up the signal to change the plot variable
  connect( m_uiForm.plotVariable, SIGNAL(currentIndexChanged(const QString &)),
           SLOT(generateCurve(const QString &)) );
  // and the one if the normalisation's been changed
  connect( m_uiForm.normalization, SIGNAL(currentIndexChanged(const QString &)),
           SLOT(updateForNormalizationChange()) );
  // Create the plot for the first time
  generateCurve( m_uiForm.plotVariable->currentText() );
}

void StepScan::runStepScanAlgLive(std::string stepScanProperties)
{
  // First stop the currently running live algorithm
  IAlgorithm_sptr oldMonitorLiveData = stopLiveListener();

  stepScanProperties.erase(0,stepScanProperties.find_first_of('(')+1);
  stepScanProperties.erase(stepScanProperties.find_last_of(')'));
  std::replace(stepScanProperties.begin(),stepScanProperties.end(),',',';');

  IAlgorithm_sptr startLiveData = AlgorithmManager::Instance().create("StartLiveData");
  startLiveData->setProperty("Instrument", m_instrument);
  // TODO: startLive->setProperty("FromStartOfRun",true);
  startLiveData->setProperty("UpdateEvery",10.0);
  startLiveData->setProperty("PreserveEvents",true);
  startLiveData->setProperty("PostProcessingAlgorithm","StepScan");
  startLiveData->setProperty("PostProcessingProperties",stepScanProperties);
  startLiveData->setProperty("EndRunBehavior","Stop");
  startLiveData->setProperty("AccumulationWorkspace",m_inputWSName);
  startLiveData->setProperty("OutputWorkspace",m_tableWSName);
  // The previous listener needs to finish before this one can start
  while ( oldMonitorLiveData->isRunning() ) // TODO: Can we get a signal for this?
  {
    Poco::Thread::sleep(200);
  }
  startLiveData->execute();
  // Keep track of the algorithm that's pulling in the live data
  m_monitorLiveData = startLiveData->getProperty("MonitorLiveData");

  AnalysisDataService::Instance().notificationCenter.addObserver(m_replObserver);
  connect( this, SIGNAL(logsUpdated(const Mantid::API::MatrixWorkspace_const_sptr &)),
           SLOT(expandPlotVarCombobox(const Mantid::API::MatrixWorkspace_const_sptr &)) );
  connect( this, SIGNAL(updatePlot(const QString&)), SLOT(generateCurve(const QString&)) );
}

void StepScan::updateForNormalizationChange()
{
  generateCurve( m_uiForm.plotVariable->currentText() );
}

void StepScan::generateCurve( const QString& var )
{
  // Create a matrix workspace out of the variable that's asked for
  IAlgorithm_sptr alg = AlgorithmManager::Instance().create("ConvertTableToMatrixWorkspace");
  alg->setLogging(false); // Don't log this algorithm
  alg->setPropertyValue("InputWorkspace", m_tableWSName);
  // TODO: Make workspace hidden once ticket #6803 is fixed
  m_plotWSName = "plot_" + m_tableWSName;
  alg->setPropertyValue("OutputWorkspace", m_plotWSName);
  alg->setPropertyValue("ColumnX", var.toStdString() );
  alg->setPropertyValue("ColumnY", "Counts" );
  if ( ! alg->execute() ) return;

  // Now create one for the normalisation, if required
  if ( m_uiForm.normalization->currentIndex() !=  0 )
  {
    IAlgorithm_sptr norm = AlgorithmManager::Instance().create("ConvertTableToMatrixWorkspace");
    norm->setChild(true);
    norm->setLogging(false); // Don't log this algorithm
    norm->setPropertyValue("InputWorkspace", m_tableWSName);
    norm->setPropertyValue("OutputWorkspace", "dummyName");
    norm->setPropertyValue("ColumnX", var.toStdString() );
    // TODO: Protect against column being missing (e.g. if monitor not found in data)
    norm->setPropertyValue("ColumnY", m_uiForm.normalization->currentText().toStdString() );
    if ( ! norm->execute() ) return;

    MatrixWorkspace_sptr top = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>(m_plotWSName);
    MatrixWorkspace_sptr bottom = norm->getProperty("OutputWorkspace");
    top /= bottom;
  }

  plotCurve();
}

void StepScan::plotCurve()
{
  // Get the name of the dataset to produce the plot title
  std::string title = m_inputWSName.substr(2);
  // qtiplot may unhelpfully change '_' to '-' so I need to as well
  std::replace(title.begin(), title.end(), '_', '-');

  // Figure out the axis titles
  const std::string xAxisTitle = m_uiForm.plotVariable->currentText().toStdString();
  std::string yAxisTitle = "Counts";
  const std::string normalization = m_uiForm.normalization->currentText().toStdString();
  if ( normalization == "nothing") /* Do nothing */;
  else if ( normalization == "time" ) yAxisTitle += " / second";
  else if ( normalization == "proton_charge" ) yAxisTitle += " / picocoulomb";
  else yAxisTitle += " / " + normalization;

  // Has to be done via python
  std::string pyCode = "g = graph('" + title + "')\n"
                       "if g is None:\n"
                       "    g = plotSpectrum('" + m_plotWSName + "',0,type=Layer.Scatter)\n"
                       "    l = g.activeLayer()\n"
                       "    l.legend().hide()\n"
                       "    l.removeTitle()\n"
                       "    setWindowName(g,'" + title + "')\n"
                       "    g.setWindowLabel('Step Scan')\n"
                       "l = g.activeLayer()\n"
                       "l.setAxisTitle(Layer.Bottom,'" + xAxisTitle + "')\n"
                       "l.setAxisTitle(Layer.Left,'" + yAxisTitle + "')";

  runPythonCode( QString::fromStdString(pyCode) );
}

void StepScan::handleAddEvent(Mantid::API::WorkspaceAddNotification_ptr pNf)
{
  checkForMaskWorkspace(pNf->object_name());
}

void StepScan::handleReplEvent(Mantid::API::WorkspaceAfterReplaceNotification_ptr pNf)
{
  checkForMaskWorkspace(pNf->object_name());
  checkForResultTableUpdate(pNf->object_name());
  checkForVaryingLogs(pNf->object_name());
}

void StepScan::checkForMaskWorkspace(const std::string & wsName)
{
  if ( wsName == "MaskWorkspace" )
  {
    // Make sure the combobox has picked up the new workspace
    m_uiForm.maskWorkspace->refresh();
    // Now set it to point at the mask workspace
    const int index = m_uiForm.maskWorkspace->findText("MaskWorkspace");
    if ( index != -1 ) m_uiForm.maskWorkspace->setCurrentIndex(index);
  }
}

void StepScan::checkForResultTableUpdate(const std::string& wsName)
{
  if ( wsName == m_tableWSName )
  {
    emit updatePlot( m_uiForm.plotVariable->currentText() );
  }
}

void StepScan::checkForVaryingLogs(const std::string& wsName)
{
  if ( wsName == m_inputWSName )
  {
    MatrixWorkspace_const_sptr ws = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>(m_inputWSName);
    emit logsUpdated( ws );
  }
}

void StepScan::helpClicked()
{
  QDesktopServices::openUrl(QUrl("http://www.mantidproject.org/Step_Scan_Interface"));
}

} // namespace CustomInterfaces
} // namespace MantidQt
