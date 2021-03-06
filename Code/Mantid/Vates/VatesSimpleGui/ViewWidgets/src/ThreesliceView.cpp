#include "MantidVatesSimpleGuiViewWidgets/ThreesliceView.h"
#include "MantidVatesSimpleGuiViewWidgets/LibHelper.h"

#if defined(__INTEL_COMPILER)
  #pragma warning disable 1170
#endif

#include <pqActiveObjects.h>
#include <pqApplicationCore.h>
#include <pqDataRepresentation.h>
#include <pqObjectBuilder.h>
#include <pqPipelineRepresentation.h>
#include <pqPipelineSource.h>
#include <pqPluginManager.h>
#include <pqRenderView.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMProxy.h>

#if defined(__INTEL_COMPILER)
  #pragma warning enable 1170
#endif

#include <QMessageBox>

#include <iostream>

namespace Mantid
{
namespace Vates
{
namespace SimpleGui
{

ThreeSliceView::ThreeSliceView(QWidget *parent) : ViewBase(parent)
{
  this->ui.setupUi(this);

  // Need to load plugin
  pqPluginManager* pm = pqApplicationCore::instance()->getPluginManager();
  QString error;
  pm->loadExtension(pqActiveObjects::instance().activeServer(),
                    QUADVIEW_LIBRARY, &error, false);

  this->mainView = this->createRenderView(this->ui.mainRenderFrame,
                                          QString("pqQuadView"));
  pqActiveObjects::instance().setActiveView(this->mainView);
}

ThreeSliceView::~ThreeSliceView()
{
}

void ThreeSliceView::destroyView()
{
  pqObjectBuilder *builder = pqApplicationCore::instance()->getObjectBuilder();
  // Active source disappears in only this view, so set it from the
  // internal source before destroying view.
  pqActiveObjects::instance().setActiveSource(this->origSrc);
  builder->destroy(this->mainView);
}

pqRenderView* ThreeSliceView::getView()
{
  return this->mainView.data();
}

void ThreeSliceView::render()
{
  this->makeThreeSlice();
  this->resetDisplay();
  emit this->triggerAccept();
}

void ThreeSliceView::makeThreeSlice()
{
  pqPipelineSource *src = NULL;
  src = pqActiveObjects::instance().activeSource();

  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();

  // Do not allow overplotting PeaksWorkspaces
  if (this->isPeaksWorkspace(src))
  {
    QMessageBox::warning(this, QApplication::tr("Overplotting Warning"),
                         QApplication::tr("Threeslice mode does not allow "\
                                          "overlay of PeaksWorkspaces"));
    // Need to destroy source since we tried to load it and set the active
    // back to something. In this case we'll choose the original source
    builder->destroy(src);
    pqActiveObjects::instance().setActiveSource(this->origSrc);
    return;
  }

  this->origSrc = src;

  pqDataRepresentation *drep = builder->createDataRepresentation(\
        this->origSrc->getOutputPort(0), this->mainView);
  vtkSMPropertyHelper(drep->getProxy(), "Representation").Set("Slices");
  drep->getProxy()->UpdateVTKObjects();
  this->origRep = qobject_cast<pqPipelineRepresentation*>(drep);
}

void ThreeSliceView::renderAll()
{
  this->mainView->render();
}

void ThreeSliceView::resetDisplay()
{
  this->mainView->resetDisplay();
}

/*
void ThreeSliceView::correctVisibility()
{
  //this->correctColorScaleRange();
}
*/
void ThreeSliceView::correctColorScaleRange()
{
  QPair<double, double> range = this->origRep->getColorFieldRange();
  emit this->dataRange(range.first, range.second);
}

void ThreeSliceView::resetCamera()
{
  this->mainView->resetCamera();
}

}
}
}
