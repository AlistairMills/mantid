#ifndef MANTIDQTCUSTOMINTERFACESIDA_FURYFIT_H_
#define MANTIDQTCUSTOMINTERFACESIDA_FURYFIT_H_

#include "MantidQtCustomInterfaces/IDATab.h"
#include "MantidAPI/MatrixWorkspace.h"

#include "boost/shared_ptr.hpp"

namespace Mantid
{
  namespace API
  {
    class IFunction;
    class CompositeFunction;
  }
}

namespace MantidQt
{
namespace CustomInterfaces
{
namespace IDA
{
  class FuryFit : public IDATab
  {
    Q_OBJECT

  public:
    FuryFit(QWidget * parent = 0);

  private:
    virtual void setup();
    virtual void run();
    virtual QString validate();
    virtual void loadSettings(const QSettings & settings);
    virtual QString helpURL() {return "FuryFit";}

  private slots:
    void typeSelection(int index);
    void plotInput();
    void xMinSelected(double val);
    void xMaxSelected(double val);
    void backgroundSelected(double val);
    void propertyChanged(QtProperty*, double);
    void sequential();
    void plotGuess(QtProperty*);
    void fitContextMenu(const QPoint &);
    void fixItem();
    void unFixItem();

  private:
    boost::shared_ptr<Mantid::API::CompositeFunction> createFunction(bool tie=false);
    boost::shared_ptr<Mantid::API::IFunction> createUserFunction(const QString & name, bool tie=false);
    QtProperty* createExponential(const QString &);
    QtProperty* createStretchedExp(const QString &);
    void setDefaultParameters(const QString& name);
    QString fitTypeString() const;
    
    QIntValidator * m_intVal;
    QtStringPropertyManager* m_stringManager;
    QtTreePropertyBrowser* m_ffTree; ///< FuryFit Property Browser
    QtGroupPropertyManager* m_groupManager;
    QtDoublePropertyManager* m_ffDblMng;
    QtDoublePropertyManager* m_ffRangeManager; ///< StartX and EndX for FuryFit
    QMap<QString, QtProperty*> m_ffProp;
    QMap<QtProperty*, QtProperty*> m_fixedProps;
    QwtPlot* m_ffPlot;
    QwtPlotCurve* m_ffDataCurve;
    QwtPlotCurve* m_ffFitCurve;
    MantidQt::MantidWidgets::RangeSelector* m_ffRangeS;
    MantidQt::MantidWidgets::RangeSelector* m_ffBackRangeS;
    boost::shared_ptr<const Mantid::API::MatrixWorkspace> m_ffInputWS;
    boost::shared_ptr<const Mantid::API::MatrixWorkspace> m_ffOutputWS;
    QString m_ffInputWSName;
    QString m_ties;
  };
} // namespace IDA
} // namespace CustomInterfaces
} // namespace MantidQt

#endif /* MANTIDQTCUSTOMINTERFACESIDA_FURYFIT_H_ */
