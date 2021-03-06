#include "pqAutoGeneratedObjectPanel.h"
#include "MantidVatesAPI/DimensionView.h"
#include "MantidKernel/System.h"

/**

 Adds and removes from Paraview's autogenerated object panel for the Rebinning Cutting filter.

 @author Owen Arnold, Tessella plc
 @date 17/03/2011

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

 File change history is stored at: <https://github.com/mantidproject/mantid>
 Code Documentation is available at: <http://doxygen.mantidproject.org>
 */

//Forward declarations
class GeometryWidget;
class ThresholdRangeWidget;

// cppcheck-suppress class_X_Y
class DLLExport RebinningCutterObjectPanel: public pqAutoGeneratedObjectPanel
{
Q_OBJECT
private:
  /// cached min threshold.
  double m_cachedMinThreshold;
  /// cached max threshold.
  double m_cachedMaxThreshold;
  /// cached geometry xml string.
  std::string m_geometryXMLString;
  /// Pointer to custom geometry widget.
  GeometryWidget* m_geometryWidget;
  /// Pointer to custom threshold range widget.
  ThresholdRangeWidget* m_thresholdWidget;
  /// Cached bin display mode from the geometry widget.
  Mantid::VATES::BinDisplay m_geomBinDisplayMode;

private slots:

  void onGeometryChanged();

public:

  /// Constructor
  RebinningCutterObjectPanel(pqProxy* pxy, QWidget* p);

  /// Framework overriden method.
  void updateInformationAndDomains();

  /// Remove selected auto-generated widgets
  void removeAutoGeneratedWidgets();

  /// Pop the widget off the layout
  void popWidget();

  /// Construct threshold ranges and link-up with properties
  void constructThresholdRanges(QGridLayout* gLayout);

  /// Construct geometry widgets and link-up with properties
  void constructGeometry(QGridLayout* gLayout);

};
