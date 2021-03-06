#include "MantidQtAPI/PropertyWidget.h"
#include "MantidKernel/System.h"
#include "MantidAPI/IWorkspaceProperty.h"

using namespace Mantid::Kernel;
using namespace Mantid::API;
using Mantid::API::IWorkspaceProperty;

namespace MantidQt
{
namespace API
{


  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  PropertyWidget::PropertyWidget(Mantid::Kernel::Property * prop, QWidget * parent, QGridLayout * layout, int row)
  : QWidget(parent),
    m_prop(prop), m_gridLayout(layout), m_row(row),
    m_replaceWSButton(NULL)
  {
    if (!prop)
      throw std::runtime_error("NULL Property passed to the PropertyWidget constructor.");

    if (!m_gridLayout)
    {
      // Create a LOCAL grid layout
      m_gridLayout = new QGridLayout(this, 1, 5);
      m_gridLayout->setSpacing(5);
      this->setLayout(m_gridLayout);
      // Will always go to row 0
      m_row = 0;
      // And the parent is THIS widget
      m_parent = this;
      //this->setStyleSheet(          "QWidget {  background-color: yellow;  }"          );
    }
    else
    {
      // Use the parent of the provided QGridLayout when adding widgets
      m_parent = parent;
    }

    // Create the validator label (that red star)
    m_validLbl = new QLabel("*");
    QPalette pal = m_validLbl->palette();
    pal.setColor(QPalette::WindowText, Qt::darkRed);
    m_validLbl->setPalette(pal);
    // Start off hidden
    m_validLbl->setVisible(false);
    // Put it in the 4th column.
    m_gridLayout->addWidget(m_validLbl, m_row, 4);

    /// Save the documentation tooltip
    m_doc = QString::fromStdString(prop->documentation());


  }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  PropertyWidget::~PropertyWidget()
  {
  }

  //----------------------------------------------------------------------------------------------
  /** Slot called when someone clicks the "replace ws button" */
  void PropertyWidget::replaceWSButtonClicked()
  {
    emit replaceWorkspaceName(QString::fromStdString(m_prop->name()));
  }

  //----------------------------------------------------------------------------------------------
  /** Create and show the "Replace WS" button.
   *
   * This only has an effect for Output WorkspaceProperty's.
   *
   */
  void PropertyWidget::addReplaceWSButton()
  {
    // Don't re-create it if it already exists
    if (m_replaceWSButton)
      return;

    IWorkspaceProperty * wsProp = dynamic_cast<IWorkspaceProperty*>(m_prop);
    if (wsProp && (m_prop->direction() == Direction::Output) )
    {
      m_replaceWSButton = new QPushButton(QIcon(":/data_replace.png"), "", m_parent);
      // MG: There is no way with the QIcon class to actually ask what size it is so I had to hard
      // code this number here to get it to a sensible size
      m_replaceWSButton->setMaximumWidth(32);
      //m_wsbtn_tracker[btn ] = 1;
      m_replaceWSButton->setToolTip("Replace input workspace");
      connect(m_replaceWSButton, SIGNAL(clicked()), this, SLOT(replaceWSButtonClicked()));
      m_widgets.push_back(m_replaceWSButton);
      // Place in the grid on column 2.
      m_gridLayout->addWidget(m_replaceWSButton, m_row, 2);
      m_replaceWSButton->setVisible(true);
    }
  }


  //----------------------------------------------------------------------------------------------
  /** Externally set an error string to display in the validator
   *
   * @param error :: string to show in the star, empty means no error
   */
  void PropertyWidget::setError(const QString & error)
  {
    m_error = error.trimmed();

    // Show the invalid star if there was an error
    if (m_error.isEmpty())
      m_validLbl->setVisible(false);
    else
    {
      m_validLbl->setVisible(true);
      m_validLbl->setToolTip(m_error);
    }
  }


  //----------------------------------------------------------------------------------------------
  /** Slot called when the value of the property had been changed.
   * This performs validation of the value and shows/hides that validator
   * star.
   *
   * It then emits a signal that the value of the property was changed.
   * */
  void PropertyWidget::valueChangedSlot()
  {
    // Try to set the value
    QString value = this->getValue().trimmed();
    // Use the default if empty
    if( value.isEmpty() )
      value = QString::fromStdString(m_prop->getDefault());

    std::string error("");
    try
    {
      error = m_prop->setValue(value.toStdString());
    }
    catch(std::exception & err_details)
    {
      error = err_details.what();
    }
    this->setError(QString::fromStdString(error).trimmed());

    // This will be caught by the GenericDialog.
    emit valueChanged( QString::fromStdString(m_prop->name()) ) ;
  }

  //----------------------------------------------------------------------------------------------
  /** Sets all widgets contained within to Enabled
   * @param val :: enabled or not   */
  void PropertyWidget::setEnabled(bool val)
  {
    for (int i=0; i < m_widgets.size(); i++)
      m_widgets[i]->setEnabled(val);
    QWidget::setEnabled(val);
  }

  //----------------------------------------------------------------------------------------------
  /** Sets all widgets contained within to Visible
   * @param val :: Visible or not   */
  void PropertyWidget::setVisible(bool val)
  {
    for (int i=0; i < m_widgets.size(); i++)
      m_widgets[i]->setVisible(val);
    QWidget::setVisible(val);
  }


} // namespace MantidQt
} // namespace API
