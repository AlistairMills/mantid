#include "MantidQtCustomInterfaces/Fury.h"

#include "MantidQtMantidWidgets/RangeSelector.h"

#include <QFileInfo>

#include <qwt_plot.h>

namespace MantidQt
{
namespace CustomInterfaces
{
namespace IDA
{
  Fury::Fury(QWidget * parent) : IDATab(parent),
    m_furPlot(NULL), m_furRange(NULL), m_furCurve(NULL), m_furTree(NULL), 
    m_furProp(), m_furDblMng(NULL), m_furyResFileType()
  {}

  void Fury::setup()
  {
    m_furTree = new QtTreePropertyBrowser();
    uiForm().fury_TreeSpace->addWidget(m_furTree);

    m_furDblMng = new QtDoublePropertyManager();

    m_furPlot = new QwtPlot(this);
    uiForm().fury_PlotSpace->addWidget(m_furPlot);
    m_furPlot->setCanvasBackground(Qt::white);
    m_furPlot->setAxisFont(QwtPlot::xBottom, this->font());
    m_furPlot->setAxisFont(QwtPlot::yLeft, this->font());

    m_furProp["ELow"] = m_furDblMng->addProperty("ELow");
    m_furDblMng->setDecimals(m_furProp["ELow"], NUM_DECIMALS);
    m_furProp["EWidth"] = m_furDblMng->addProperty("EWidth");
    m_furDblMng->setDecimals(m_furProp["EWidth"], NUM_DECIMALS);
    m_furProp["EHigh"] = m_furDblMng->addProperty("EHigh");
    m_furDblMng->setDecimals(m_furProp["EHigh"], NUM_DECIMALS);

    m_furTree->addProperty(m_furProp["ELow"]);
    m_furTree->addProperty(m_furProp["EWidth"]);
    m_furTree->addProperty(m_furProp["EHigh"]);

    m_furTree->setFactoryForManager(m_furDblMng, doubleEditorFactory());

    m_furRange = new MantidQt::MantidWidgets::RangeSelector(m_furPlot);

    // signals / slots & validators
    connect(m_furRange, SIGNAL(minValueChanged(double)), this, SLOT(minChanged(double)));
    connect(m_furRange, SIGNAL(maxValueChanged(double)), this, SLOT(maxChanged(double)));
    connect(m_furDblMng, SIGNAL(valueChanged(QtProperty*, double)), this, SLOT(updateRS(QtProperty*, double)));
  
    connect(uiForm().fury_cbInputType, SIGNAL(currentIndexChanged(int)), uiForm().fury_swInput, SLOT(setCurrentIndex(int)));  
    connect(uiForm().fury_cbResType, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(resType(const QString&)));
    connect(uiForm().fury_pbPlotInput, SIGNAL(clicked()), this, SLOT(plotInput()));
  }

  void Fury::run()
  {
    QString filenames;
    switch ( uiForm().fury_cbInputType->currentIndex() )
    {
    case 0:
      filenames = uiForm().fury_iconFile->getFilenames().join("', r'");
      break;
    case 1:
      filenames = uiForm().fury_wsSample->currentText();
      break;
    }

    QString pyInput =
      "from IndirectDataAnalysis import fury\n"
      "samples = [r'" + filenames + "']\n"
      "resolution = r'" + uiForm().fury_resFile->getFirstFilename() + "'\n"
      "rebin = '" + m_furProp["ELow"]->valueText() +","+ m_furProp["EWidth"]->valueText() +","+m_furProp["EHigh"]->valueText()+"'\n";

    if ( uiForm().fury_ckVerbose->isChecked() ) pyInput += "verbose = True\n";
    else pyInput += "verbose = False\n";

    if ( uiForm().fury_ckPlot->isChecked() ) pyInput += "plot = True\n";
    else pyInput += "plot = False\n";

    if ( uiForm().fury_ckSave->isChecked() ) pyInput += "save = True\n";
    else pyInput += "save = False\n";

    pyInput +=
      "fury_ws = fury(samples, resolution, rebin, Save=save, Verbose=verbose, Plot=plot)\n";
    QString pyOutput = runPythonCode(pyInput).trimmed();
  }

  QString Fury::validate()
  {
    switch ( uiForm().fury_cbInputType->currentIndex() )
    {
    case 0: // File
      {
        if ( ! uiForm().fury_iconFile->isValid() )
          return "Empty or otherwise invalid reduction file field.";
      }
      break;
    case 1: // Workspace
      {
        if ( uiForm().fury_wsSample->currentText() == "" )
          return "No workspace selected.";
      }
      break;
    }

    if ( ! uiForm().fury_resFile->isValid()  )
      return "Invalid or empty resolution file field.";

    return "";
  }

  void Fury::loadSettings(const QSettings & settings)
  {
    uiForm().fury_iconFile->readSettings(settings.group());
    uiForm().fury_resFile->readSettings(settings.group());
  }

  void Fury::resType(const QString& type)
  {
    QStringList exts;
    if ( type == "RES File" )
    {
      exts.append("_res.nxs");
      m_furyResFileType = true;
    }
    else
    {
      exts.append("_red.nxs");
      m_furyResFileType = false;
    }
    uiForm().fury_resFile->setFileExtensions(exts);
  }

  void Fury::plotInput()
  {
    std::string workspace;
    if ( uiForm().fury_cbInputType->currentIndex() == 0 )
    {
      if ( uiForm().fury_iconFile->isValid() )
      {
        QString filename = uiForm().fury_iconFile->getFirstFilename();
        QFileInfo fi(filename);
        QString wsname = fi.baseName();

        QString pyInput = "LoadNexus(r'" + filename + "', '" + wsname + "')\n";
        QString pyOutput = runPythonCode(pyInput);

        workspace = wsname.toStdString();
      }
      else
      {
        showInformationBox("Selected input files are invalid.");
        return;
      }
    }
    else if ( uiForm().fury_cbInputType->currentIndex() == 1 )
    {
      workspace = uiForm().fury_wsSample->currentText().toStdString();
      if ( workspace.empty() )
      {
        showInformationBox("No workspace selected.");
        return;
      }
    }

    m_furCurve = plotMiniplot(m_furPlot, m_furCurve, workspace, 0);
    try
    {
      const std::pair<double, double> range = getCurveRange(m_furCurve);    
      m_furRange->setRange(range.first, range.second);
      m_furPlot->replot();
    }
    catch(std::invalid_argument & exc)
    {
      showInformationBox(exc.what());
    }
  }

  void Fury::maxChanged(double val)
  {
    m_furDblMng->setValue(m_furProp["EHigh"], val);
  }

  void Fury::minChanged(double val)
  {
    m_furDblMng->setValue(m_furProp["ELow"], val);
  }

  void Fury::updateRS(QtProperty* prop, double val)
  {
    if ( prop == m_furProp["ELow"] )
      m_furRange->setMinimum(val);
    else if ( prop == m_furProp["EHigh"] )
      m_furRange->setMaximum(val);
  }
} // namespace IDA
} // namespace CustomInterfaces
} // namespace MantidQt