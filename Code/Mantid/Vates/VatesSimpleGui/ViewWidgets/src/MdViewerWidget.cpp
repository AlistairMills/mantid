#include "MantidVatesSimpleGuiViewWidgets/MdViewerWidget.h"

#include "MantidVatesSimpleGuiQtWidgets/ModeControlWidget.h"
#include "MantidVatesSimpleGuiQtWidgets/RotationPointDialog.h"
#include "MantidVatesSimpleGuiViewWidgets/ColorSelectionWidget.h"
#include "MantidVatesSimpleGuiViewWidgets/MultisliceView.h"
#include "MantidVatesSimpleGuiViewWidgets/SaveScreenshotReaction.h"
#include "MantidVatesSimpleGuiViewWidgets/SplatterPlotView.h"
#include "MantidVatesSimpleGuiViewWidgets/StandardView.h"
#include "MantidVatesSimpleGuiViewWidgets/ThreesliceView.h"
#include "MantidVatesSimpleGuiViewWidgets/TimeControlWidget.h"

#include "MantidQtAPI/InterfaceManager.h"
#include "MantidKernel/DynamicFactory.h"

// Have to deal with ParaView warnings and Intel compiler the hard way.
#if defined(__INTEL_COMPILER)
  #pragma warning disable 1170
#endif

#include <pqActiveObjects.h>
#include <pqAnimationManager.h>
#include <pqAnimationScene.h>
#include <pqApplicationCore.h>
#include <pqLoadDataReaction.h>
#include <pqObjectBuilder.h>
#include <pqObjectInspectorWidget.h>
#include <pqParaViewBehaviors.h>
#include <pqPipelineSource.h>
#include <pqPVApplicationCore.h>
#include <pqRenderView.h>
#include <pqSettings.h>
#include <pqStatusBar.h>
#include <pqViewSettingsReaction.h>
#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMProxyManager.h>
#include <vtkSMProxy.h>
#include <vtkSMSourceProxy.h>
#include <vtkSMReaderFactory.h>
#include <vtksys/SystemTools.hxx>

#include <pqPipelineRepresentation.h>

// Used for plugin mode
#include <pqAlwaysConnectedBehavior.h>
#include <pqAutoLoadPluginXMLBehavior.h>
#include <pqCollaborationBehavior.h>
#include <pqCommandLineOptionsBehavior.h>
#include <pqCrashRecoveryBehavior.h>
#include <pqDataTimeStepBehavior.h>
#include <pqDefaultViewBehavior.h>
#include <pqDeleteBehavior.h>
#include <pqFixPathsInStateFilesBehavior.h>
#include <pqInterfaceTracker.h>
#include <pqMultiServerBehavior.h>
#include <pqObjectPickingBehavior.h>
//#include <pqPersistentMainWindowStateBehavior.h>
#include <pqPipelineContextMenuBehavior.h>
//#include <pqPluginActionGroupBehavior.h>
//#include <pqPluginDockWidgetsBehavior.h>
#include <pqPluginManager.h>
#include <pqPVNewSourceBehavior.h>
#include <pqQtMessageHandlerBehavior.h>
#include <pqSpreadSheetVisibilityBehavior.h>
#include <pqStandardPropertyWidgetInterface.h>
#include <pqStandardViewModules.h>
#include <pqUndoRedoBehavior.h>
#include <pqViewFrameActionsBehavior.h>
#include <pqViewStreamingBehavior.h>
#include <pqVerifyRequiredPluginBehavior.h>

#if defined(__INTEL_COMPILER)
  #pragma warning enable 1170
#endif

#include <QAction>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QMenuBar>
#include <QModelIndex>
#include <QUrl>
#include <QWidget>

#include <iostream>

namespace Mantid
{
namespace Vates
{
namespace SimpleGui
{
using namespace Mantid::API;
using namespace MantidQt::API;

REGISTER_VATESGUI(MdViewerWidget)

/**
 * This constructor is used in the plugin mode operation of the VSI.
 */
MdViewerWidget::MdViewerWidget() : VatesViewerInterface()
{
  // Calling workspace observer functions.
  observeAfterReplace();
  observePreDelete();
  observeADSClear();

  this->internalSetup(true);
}

/**
 * This constructor is used in the standalone mode operation of the VSI.
 * @param parent the parent widget for the main window
 */
MdViewerWidget::MdViewerWidget(QWidget *parent) : VatesViewerInterface(parent)
{
  this->checkEnvSetup();
  // We're in the standalone application mode
  this->internalSetup(false);
  this->setupUiAndConnections();
  // FIXME: This doesn't allow a clean split of the classes. I will need
  //        to investigate creating the individual behaviors to see if that
  //        eliminates the dependence on the QMainWindow.
  if (parent->inherits("QMainWindow"))
  {
    QMainWindow *mw = qobject_cast<QMainWindow *>(parent);
    new pqParaViewBehaviors(mw, mw);
  }
  this->setupMainView();
}

MdViewerWidget::~MdViewerWidget()
{
}

/**
 * This function consolidates setting up some of the internal members between
 * the standalone and plugin modes.
 * @param pMode flag to set the plugin mode
 */
void MdViewerWidget::internalSetup(bool pMode)
{
  this->isPluginInitialized = false;
  this->pluginMode = pMode;
  this->rotPointDialog = NULL;
  this->lodThreshold = 5.0;
  this->viewSwitched = false;
}

/**
 * This function uses VTK's system tools to check the environmental variables
 * to make sure PV_PLUGIN_PATH is available.
 */
void MdViewerWidget::checkEnvSetup()
{
  QString pv_plugin_path = vtksys::SystemTools::GetEnv("PV_PLUGIN_PATH");
  if (pv_plugin_path.isEmpty())
  {
    throw std::runtime_error("PV_PLUGIN_PATH not setup.\nVates plugins will not be available.\n"
                             "Further use will cause the program to crash.\nPlease exit and "
                             "set this variable.");
  }
}

/**
 * This function sets up the UI components and connects some of the main
 * window's control buttons.
 */
void MdViewerWidget::setupUiAndConnections()
{
  this->ui.setupUi(this);
  this->ui.splitter_2->setStretchFactor(1, 1);
  this->ui.statusBar->setSizeGripEnabled(false);

  // Unset the connections since the views aren't up yet.
  this->removeProxyTabWidgetConnections();

  QObject::connect(this->ui.modeControlWidget,
                   SIGNAL(executeSwitchViews(ModeControlWidget::Views)),
                   this, SLOT(switchViews(ModeControlWidget::Views)));

  // Setup rotation point button
  QObject::connect(this->ui.resetCenterToPointButton,
                   SIGNAL(clicked()),
                   this,
                   SLOT(onRotationPoint()));
}

/**
 * This function places the standard view to the main window, installs an
 * event filter, tweaks the UI layout for the view and calls the routine that
 * sets up connections between ParaView and the main window widgets.
 */
void MdViewerWidget::setupMainView()
{
  // Commented this out to only use Mantid supplied readers
  // Initialize all readers available to ParaView. Now our application can load
  // all types of datasets supported by ParaView.
  //vtkSMProxyManager::GetProxyManager()->GetReaderFactory()->RegisterPrototypes("sources");

  // Set the standard view as the default
  this->currentView = this->setMainViewWidget(this->ui.viewWidget,
                                              ModeControlWidget::STANDARD);
  this->currentView->installEventFilter(this);

  // Create a layout to manage the view properly
  this->viewLayout = new QHBoxLayout(this->ui.viewWidget);
  this->viewLayout->setMargin(0);
  this->viewLayout->setStretch(0, 1);
  this->viewLayout->addWidget(this->currentView);

  this->setParaViewComponentsForView();
}

/**
 * This function performs setup for the plugin mode of the Vates Simple
 * Interface. It calls a number of defined functions to complete the process.
 */
void MdViewerWidget::setupPluginMode()
{
  this->createAppCoreForPlugin();
  this->checkEnvSetup();
  this->setupUiAndConnections();
  if (!this->isPluginInitialized)
  {
    this->setupParaViewBehaviors();
    this->createMenus();
  }
  this->setupMainView();
}

/**
 * This function ensures that the main ParaView instance is only initialized
 * once. On the second call, it checks to make sure one doesn't exist. This is
 * only important for plugin mode operation of the VSI.
 */
void MdViewerWidget::createAppCoreForPlugin()
{
  if (!pqApplicationCore::instance())
  {
    int argc = 1;
    char *argv[] = {"/tmp/MantidPlot"};
    new pqPVApplicationCore(argc, argv);
  }
  else
  {
    this->isPluginInitialized = true;
  }
}

/**
 * This function duplicates the nearly identical call in ParaView for their
 * main program setup. This is necessary for the plugin mode since it does
 * not have access to the QMainWindow of MantidPlot.
 */
void MdViewerWidget::setupParaViewBehaviors()
{
  // Register ParaView interfaces.
  pqInterfaceTracker* pgm = pqApplicationCore::instance()->interfaceTracker();

  // * adds support for standard paraview views.
  pgm->addInterface(new pqStandardViewModules(pgm));

  pgm->addInterface(new pqStandardPropertyWidgetInterface(pgm));

  // Load plugins distributed with application.
  pqApplicationCore::instance()->loadDistributedPlugins();

  // Define application behaviors.
  new pqQtMessageHandlerBehavior(this);
  new pqDataTimeStepBehavior(this);
  new pqViewFrameActionsBehavior(this);
  new pqSpreadSheetVisibilityBehavior(this);
  new pqPipelineContextMenuBehavior(this);
  new pqDefaultViewBehavior(this);
  new pqAlwaysConnectedBehavior(this);
  new pqPVNewSourceBehavior(this);
  new pqDeleteBehavior(this);
  new pqUndoRedoBehavior(this);
  new pqCrashRecoveryBehavior(this);
  new pqAutoLoadPluginXMLBehavior(this);
  //new pqPluginDockWidgetsBehavior(mainWindow);
  new pqVerifyRequiredPluginBehavior(this);
  //new pqPluginActionGroupBehavior(mainWindow);
  new pqFixPathsInStateFilesBehavior(this);
  new pqCommandLineOptionsBehavior(this);
  //new pqPersistentMainWindowStateBehavior(mainWindow);
  new pqObjectPickingBehavior(this);
  new pqCollaborationBehavior(this);
  new pqMultiServerBehavior(this);
  new pqViewStreamingBehavior(this);
}

/**
 * This function connects ParaView's data loader the given action.
 * @param action the action to connect data loading to
 */
void MdViewerWidget::connectLoadDataReaction(QAction *action)
{
  // We want the actionLoad to result in the showing up the ParaView's OpenData
  // dialog letting the user pick from one of the supported file formats.
  this->dataLoader = new pqLoadDataReaction(action);
  QObject::connect(this->dataLoader, SIGNAL(loadedData(pqPipelineSource*)),
                   this, SLOT(onDataLoaded(pqPipelineSource*)));
}

/**
 * This function disconnects ParaView connections between pqActiveObjects
 * and the pqProxyTabWidget. This is necessary for clean view switching.
 */
void MdViewerWidget::removeProxyTabWidgetConnections()
{
  QObject::disconnect(&pqActiveObjects::instance(), 0,
                      this->ui.propertiesPanel, 0);
}

/**
 * This function creates the requested view on the main window.
 * @param container the UI widget to associate the view mode with
 * @param v the view mode to set on the main window
 * @return the requested view
 */
ViewBase* MdViewerWidget::setMainViewWidget(QWidget *container,
                                            ModeControlWidget::Views v)
{
  ViewBase *view;
  switch(v)
  {
  case ModeControlWidget::STANDARD:
  {
    view = new StandardView(container);
  }
  break;
  case ModeControlWidget::THREESLICE:
  {
    view = new ThreeSliceView(container);
  }
  break;
  case ModeControlWidget::MULTISLICE:
  {
    view = new MultiSliceView(container);
  }
  break;
  case ModeControlWidget::SPLATTERPLOT:
  {
    view = new SplatterPlotView(container);
  }
  break;
  default:
    view = NULL;
    break;
  }
  return view;
}

/**
 * This function is responsible for setting up all the connections between
 * ParaView's pqPipelineBrowser and pqProxyTabWidget and cetatin main window
 * widgets.
 */
void MdViewerWidget::setParaViewComponentsForView()
{
  // Extra setup stuff to hook up view to other items
  this->ui.propertiesPanel->setView(this->currentView->getView());
  this->ui.pipelineBrowser->setActiveView(this->currentView->getView());

  pqActiveObjects *activeObjects = &pqActiveObjects::instance();
  QObject::connect(activeObjects, SIGNAL(portChanged(pqOutputPort*)),
                   this->ui.propertiesPanel, SLOT(setOutputPort(pqOutputPort*)));
  QObject::connect(activeObjects, SIGNAL(representationChanged(pqRepresentation*)),
                   this->ui.propertiesPanel, SLOT(setRepresentation(pqRepresentation*)));
  QObject::connect(activeObjects, SIGNAL(viewChanged(pqView*)),
                   this->ui.propertiesPanel, SLOT(setView(pqView*)));

  QObject::connect(this->currentView,
                   SIGNAL(triggerAccept()),
                   this->ui.propertiesPanel,
                   SLOT(apply()));
  QObject::connect(this->ui.propertiesPanel,
                   SIGNAL(applied()),
                   this, SLOT(checkForUpdates()));

  QObject::connect(this->currentView,
                   SIGNAL(renderingDone()),
                   this,
                   SLOT(renderingDone()));

  SplatterPlotView *spv = dynamic_cast<SplatterPlotView *>(this->currentView);
  if (spv)
  {
    QObject::connect(this->ui.propertiesPanel,
                     SIGNAL(applied()),
                     spv,
                     SLOT(checkPeaksCoordinates()));
    QObject::connect(spv,
                     SIGNAL(toggleOrthographicProjection(bool)),
                     this->ui.parallelProjButton,
                     SLOT(setChecked(bool)));
    QObject::connect(spv,
                     SIGNAL(resetToStandardView()),
                     this->ui.modeControlWidget,
                     SLOT(setToStandardView()));
  }

  QObject::connect(this->currentView, SIGNAL(setViewsStatus(bool)),
                   this->ui.modeControlWidget, SLOT(enableViewButtons(bool)));
  QObject::connect(this->currentView,
                   SIGNAL(setViewStatus(ModeControlWidget::Views, bool)),
                   this->ui.modeControlWidget,
                   SLOT(enableViewButton(ModeControlWidget::Views, bool)));

  this->connectColorSelectionWidget();

  // Set animation (time) control widget <-> view signals/slots.
  QObject::connect(this->currentView,
                   SIGNAL(setAnimationControlState(bool)),
                   this->ui.timeControlWidget,
                   SLOT(enableAnimationControls(bool)));
  QObject::connect(this->currentView,
                   SIGNAL(setAnimationControlInfo(double, double, int)),
                   this->ui.timeControlWidget,
                   SLOT(updateAnimationControls(double, double, int)));

  // Set the connection for the parallel projection button
  QObject::connect(this->ui.parallelProjButton,
                   SIGNAL(toggled(bool)),
                   this->currentView,
                   SLOT(onParallelProjection(bool)));
}

/**
 * This function loads and renders data from the given source for the
 * standalone mode.
 * @param source a ParaView compatible source
 */
void MdViewerWidget::onDataLoaded(pqPipelineSource* source)
{
  source->updatePipeline();
  this->renderAndFinalSetup();
}

/**
 * This function is responsible for carrying out actions when ParaView
 * says the rendering is completed. It currently handles making sure the
 * color selection widget state is passed between views.
 */
void MdViewerWidget::renderingDone()
{
  if (this->viewSwitched)
  {
    this->viewSwitched = false;
    this->currentView->setColorsForView();
  }
}

/**
 * This function determines the type of source plugin and sets the workspace
 * name so that the data can be retrieved and rendered.
 * @param wsname the workspace name for the data
 * @param wstype a numeric indicator of the workspace type
 */
void MdViewerWidget::renderWorkspace(QString wsname, int wstype)
{
  QString sourcePlugin = "";
  if (VatesViewerInterface::PEAKS == wstype)
  {
    sourcePlugin = "Peaks Source";
  }
  else if (VatesViewerInterface::MDHW == wstype)
  {
    sourcePlugin = "MDHW Source";
  }
  else
  {
    sourcePlugin = "MDEW Source";
  }

  this->currentView->setPluginSource(sourcePlugin, wsname);
  this->renderAndFinalSetup();
}

/**
 * This function tells the current view to render the data, perform any
 * necessary checks on the view given the workspace type and update the
 * animation controls if necessary.
 */
void MdViewerWidget::renderAndFinalSetup()
{
  this->currentView->render();
  this->currentView->setColorsForView();
  this->currentView->checkView();
  this->currentView->updateAnimationControls();
}

/**
 * This function is used during the post-apply process of particular pipeline
 * filters to check for updates to anything that relies on information from the
 * rendered data.
 */
void MdViewerWidget::checkForUpdates()
{
  pqPipelineSource *src = pqActiveObjects::instance().activeSource();
  if (NULL == src)
  {
    return;
  }
  vtkSMProxy *proxy = src->getProxy();

  if (strcmp(proxy->GetXMLName(), "MDEWRebinningCutter") == 0)
  {
    this->currentView->onAutoScale();
    this->currentView->updateAnimationControls();
    this->currentView->updateView();
    this->currentView->updateUI();
  }
  if (QString(proxy->GetXMLName()).contains("Threshold"))
  {
    this->ui.colorSelectionWidget->enableControls(true);
    vtkSMDoubleVectorProperty *range = \
        vtkSMDoubleVectorProperty::SafeDownCast(\
          proxy->GetProperty("ThresholdBetween"));
    this->ui.colorSelectionWidget->setColorScaleRange(range->GetElement(0),
                                                      range->GetElement(1));
  }
  if (QString(proxy->GetXMLName()).contains("ScaleWorkspace"))
  {
    this->currentView->resetDisplay();
  }
}

/**
 * This function executes the logic for switching views on the main level
 * window.
 * @param v the view mode to switch to
 */
void MdViewerWidget::switchViews(ModeControlWidget::Views v)
{
  this->viewSwitched = true;
  this->currentView->closeSubWindows();
  this->disconnectDialogs();
  this->removeProxyTabWidgetConnections();
  this->hiddenView = this->setMainViewWidget(this->ui.viewWidget, v);
  this->hiddenView->setColorScaleState(this->ui.colorSelectionWidget);
  this->hiddenView->hide();
  this->viewLayout->removeWidget(this->currentView);
  this->swapViews();
  this->viewLayout->addWidget(this->currentView);
  this->currentView->installEventFilter(this);
  this->currentView->show();
  this->hiddenView->hide();
  this->setParaViewComponentsForView();
  this->connectDialogs();
  this->hiddenView->close();
  this->hiddenView->destroyView();
  delete this->hiddenView;
  this->currentView->render();
  this->currentView->setColorsForView();
  this->currentView->checkViewOnSwitch();
  this->updateAppState();
}

/**
 * This function performs a standard pointer swap for the view switching.
 */
void MdViewerWidget::swapViews()
{
  ViewBase *temp;
  temp = this->currentView;
  this->currentView = this->hiddenView;
  this->hiddenView = temp;
}

/**
 * This function allows one to filter the Qt events and look for a hide
 * event. As long as the event does not come from the system (minimize VSI
 * window or switch virtual desktops), it then executes source cleanup and
 * view mode switch if the viewer is in plugin mode.
 * @param obj the subject of the event
 * @param ev the actual event
 * @return true if the event was handled
 */
bool MdViewerWidget::eventFilter(QObject *obj, QEvent *ev)
{
  if (this->currentView == obj)
  {
    if (this->pluginMode && QEvent::Hide == ev->type() &&
        !ev->spontaneous())
    {
      if (this->ui.parallelProjButton->isChecked())
      {
        this->ui.parallelProjButton->toggle();
      }
      this->ui.colorSelectionWidget->reset();
      this->currentView->setColorScaleState(this->ui.colorSelectionWidget);
      pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
      builder->destroySources();
      this->ui.modeControlWidget->setToStandardView();
      return true;
    }
  }
  return VatesViewerInterface::eventFilter(obj, ev);
}

/**
 * This function performs shutdown procedures when MantidPlot is shut down,
 */
void MdViewerWidget::shutdown()
{
  // This seems to cure a XInitThreads error.
  pqPVApplicationCore::instance()->deleteLater();
}

/**
 * This function creates the main view widget specific menu items.
 */
void MdViewerWidget::createMenus()
{
  QMenuBar *menubar;
  if (this->pluginMode)
  {
    menubar = new QMenuBar(this->parentWidget());
    QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    menubar->setSizePolicy(policy);
  }
  else
  {
    menubar = qobject_cast<QMainWindow *>(this->parentWidget())->menuBar();
  }

  QMenu *viewMenu = menubar->addMenu(QApplication::tr("&View"));

  this->lodAction = new QAction(QApplication::tr("Level-of-Detail (LOD...)"), this);
  this->lodAction->setShortcut(QKeySequence::fromString("Ctrl+Shift+L"));
  this->lodAction->setStatusTip(QApplication::tr("Enable/disable level-of-detail threshold."));
  this->lodAction->setCheckable(true);
  this->lodAction->setChecked(true);
  QObject::connect(this->lodAction, SIGNAL(toggled(bool)),
                   this, SLOT(onLodToggled(bool)));
  viewMenu->addAction(this->lodAction);

  QAction *screenShotAction = new QAction(QApplication::tr("Save Screenshot"), this);
  screenShotAction->setShortcut(QKeySequence::fromString("Ctrl+Shift+R"));
  screenShotAction->setStatusTip(QApplication::tr("Save a screenshot of the current view."));
  this->screenShot = new SaveScreenshotReaction(screenShotAction);
  viewMenu->addAction(screenShotAction);

  QAction *settingsAction = new QAction(QApplication::tr("View Settings..."), this);
  settingsAction->setShortcut(QKeySequence::fromString("Ctrl+Shift+S"));
  settingsAction->setStatusTip(QApplication::tr("Show the settings for the current view."));
  this->viewSettings = new pqViewSettingsReaction(settingsAction);
  viewMenu->addAction(settingsAction);

  QMenu *helpMenu = menubar->addMenu(QApplication::tr("&Help"));

  QAction *wikiHelpAction = new QAction(QApplication::tr("Show Wiki Help"), this);
  wikiHelpAction->setShortcut(QKeySequence::fromString("Ctrl+Shift+H"));
  wikiHelpAction->setStatusTip(QApplication::tr("Show the wiki help page in a browser."));
  QObject::connect(wikiHelpAction, SIGNAL(triggered()),
                   this, SLOT(onWikiHelp()));
  helpMenu->addAction(wikiHelpAction);

  if (this->pluginMode)
  {
    this->ui.verticalLayout_4->insertWidget(0, menubar);
  }
}

/**
 * This function adds the menus defined here to a QMainWindow menu bar.
 * This must be done after the setup of the standalone application so that
 * the MdViewerWidget menus aren't added before the standalone ones.
 */
void MdViewerWidget::addMenus()
{
  this->createMenus();
}

/**
 * This function intercepts the LOD menu action checking and calls the
 * correct slot on the current view.
 * @param state : whether the action is checked or not
 */
void MdViewerWidget::onLodToggled(bool state)
{
  this->currentView->onLodThresholdChange(state, this->lodThreshold);
}

/**
 * This function handles creating the rotation point input dialog box and
 * setting the communication between it and the current view.
 */
void MdViewerWidget::onRotationPoint()
{
  if (NULL == this->rotPointDialog)
  {
    this->rotPointDialog = new RotationPointDialog(this);
    this->connectRotationPointDialog();
  }
  this->rotPointDialog->show();
  this->rotPointDialog->raise();
  this->rotPointDialog->activateWindow();
}

/**
 * This function shows the wiki help page for the simple interface in a
 * browser.
 */
void MdViewerWidget::onWikiHelp()
{
  QDesktopServices::openUrl(QUrl(QString("http://www.mantidproject.org/") +
                                 "VatesSimpleInterface_v2"));
}

/**
 * This function disconnects the present instances of the color options and the
 * point rotation dialog boxes from the current view. This is necessary on
 * switch view since the connection to the current view is destroyed.
 */
void MdViewerWidget::disconnectDialogs()
{
  if (NULL != this->rotPointDialog)
  {
    this->rotPointDialog->close();
    QObject::disconnect(this->rotPointDialog, 0, this->currentView, 0);
  }
}

/**
 * This function sets up the connections between the color selection widget
 * items and the current view.
 */
void MdViewerWidget::connectColorSelectionWidget()
{
  // Set color selection widget <-> view signals/slots
  QObject::connect(this->ui.colorSelectionWidget,
                   SIGNAL(colorMapChanged(const pqColorMapModel *)),
                   this->currentView,
                   SLOT(onColorMapChange(const pqColorMapModel *)));
  QObject::connect(this->ui.colorSelectionWidget,
                   SIGNAL(colorScaleChanged(double, double)),
                   this->currentView,
                   SLOT(onColorScaleChange(double, double)));
  QObject::connect(this->currentView, SIGNAL(dataRange(double, double)),
                   this->ui.colorSelectionWidget,
                   SLOT(setColorScaleRange(double, double)));
  QObject::connect(this->ui.colorSelectionWidget, SIGNAL(autoScale()),
                   this->currentView, SLOT(onAutoScale()));
  QObject::connect(this->ui.colorSelectionWidget, SIGNAL(logScale(int)),
                   this->currentView, SLOT(onLogScale(int)));
  QObject::connect(this->currentView, SIGNAL(lockColorControls(bool)),
                   this->ui.colorSelectionWidget,
                   SLOT(enableControls(bool)));
}

/**
 * This function sets up the connections between the rotation point dialog and
 * the current view.
 */
void MdViewerWidget::connectRotationPointDialog()
{
  if (NULL != this->rotPointDialog)
  {
    QObject::connect(this->rotPointDialog,
                     SIGNAL(sendCoordinates(double,double,double)),
                     this->currentView,
                     SLOT(onResetCenterToPoint(double,double,double)));
  }
}

/**
 * This function sets up the connections for all the dialogs associated with
 * the MdViewerWidget.
 */
void MdViewerWidget::connectDialogs()
{
  this->connectRotationPointDialog();
}

/**
 * This function handles any update to the state of application components
 * like menus, menu items, buttons, views etc.
 */
void MdViewerWidget::updateAppState()
{
  this->viewSettings->updateEnableState();

  ThreeSliceView *tsv = dynamic_cast<ThreeSliceView *>(this->currentView);
  SplatterPlotView *spv = dynamic_cast<SplatterPlotView *>(this->currentView);
  if (NULL != tsv || NULL != spv)
  {
    this->currentView->onLodThresholdChange(false, this->lodThreshold);
    this->lodAction->setChecked(false);
  }
  else
  {
    this->currentView->onLodThresholdChange(true, this->lodThreshold);
    this->lodAction->setChecked(true);
  }
}

/**
 * This function responds to the replacement of a workspace. It does not
 * handle workspace renaming. Also, by default it replaces the original
 * representation with a new one, deleting the old one first.
 * @param wsName : Name of workspace changing
 * @param ws : Pointer to changing workspace
 */
void MdViewerWidget::afterReplaceHandle(const std::string &wsName,
                                        const boost::shared_ptr<Mantid::API::Workspace> ws)
{
  UNUSED_ARG(ws);
  pqPipelineSource *src = this->currentView->hasWorkspace(wsName.c_str());
  if (NULL != src)
  {
    // Have to mark the filter as modified to get it to update. Do this by
    // changing the requested workspace name to a dummy name and then change
    // back. However, push the change all the way down for it to work.
    vtkSMPropertyHelper(src->getProxy(),
                        "Mantid Workspace Name").Set("ChangeMe!");
    vtkSMSourceProxy *srcProxy = vtkSMSourceProxy::SafeDownCast(src->getProxy());
    srcProxy->UpdateVTKObjects();
    srcProxy->Modified();
    srcProxy->UpdatePipelineInformation();
    src->updatePipeline();

    vtkSMPropertyHelper(src->getProxy(),
                        "Mantid Workspace Name").Set(wsName.c_str());
    // Update the source so that it retrieves the data from the Mantid workspace
    srcProxy = vtkSMSourceProxy::SafeDownCast(src->getProxy());
    srcProxy->UpdateVTKObjects();
    srcProxy->Modified();
    srcProxy->UpdatePipelineInformation();
    src->updatePipeline();

    this->currentView->setColorsForView();
    this->currentView->renderAll();;
  }
}

/**
 * This function responds to a workspace being deleted. If there are one or
 * more PeaksWorkspaces present, the requested one will be deleted. Otherwise,
 * if it is an IMDWorkspace, everything goes!
 * @param wsName : Name of workspace being deleted
 * @param ws : Pointer to workspace being deleted
 */
void MdViewerWidget::preDeleteHandle(const std::string &wsName,
                                     const boost::shared_ptr<Workspace> ws)
{
  UNUSED_ARG(ws);
  pqPipelineSource *src = this->currentView->hasWorkspace(wsName.c_str());
  if (NULL != src)
  {
    unsigned int numSources = this->currentView->getNumSources();
    if (numSources > 1)
    {
      pqObjectBuilder *builder = pqApplicationCore::instance()->getObjectBuilder();
      if (this->currentView->isPeaksWorkspace(src))
      {
        builder->destroy(src);
        return;
      }
    }
    emit this->requestClose();
  }
}

} // namespace SimpleGui
} // namespace Vates
} // namespace Mantid
