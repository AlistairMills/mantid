/***************************************************************************
    File                 : CurvesDialog.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Hoener zu Siederdissen
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Add/remove curves dialog

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include "CurvesDialog.h"
#include "Graph.h"
#include "Table.h"
#include "Matrix.h"
#include "FunctionCurve.h"
#include "PlotCurve.h"
#include "ApplicationWindow.h"
#include "Folder.h"
#include "pixmaps.h"
#include "Mantid/MantidMatrixCurve.h"

#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QLayout>
#include <QListWidget>
#include <QGroupBox>
#include <QPixmap>
#include <QShortcut>
#include <QKeySequence>
#include <QMenu>

#include <QMessageBox>

CurvesDialog::CurvesDialog( ApplicationWindow* app, Graph* g, Qt::WFlags fl )
  : QDialog( g, fl ),
    d_app(app),
    d_graph(g)
{
  setName( "CurvesDialog" );
  setWindowTitle( tr( "MantidPlot - Add/Remove curves" ) );
  setSizeGripEnabled(true);
  setFocus();

  QHBoxLayout *hl = new QHBoxLayout();

  hl->addWidget(new QLabel(tr("New curves style")));
  boxStyle = new QComboBox();
  boxStyle->addItem( getQPixmap("lPlot_xpm"), tr( " Line" ) );
  boxStyle->addItem( getQPixmap("pPlot_xpm"), tr( " Scatter" ) );
  boxStyle->addItem( getQPixmap("lpPlot_xpm"), tr( " Line + Symbol" ) );
  boxStyle->addItem( getQPixmap("dropLines_xpm"), tr( " Vertical drop lines" ) );
  boxStyle->addItem( getQPixmap("spline_xpm"), tr( " Spline" ) );
  boxStyle->addItem( getQPixmap("hor_steps_xpm"), tr( " Horizontal steps" ) );
  boxStyle->addItem( getQPixmap("vert_steps_xpm"), tr( " Vertical steps" ) );
  boxStyle->addItem( getQPixmap("area_xpm"), tr( " Area" ) );
  boxStyle->addItem( getQPixmap("vertBars_xpm"), tr( " Vertical Bars" ) );
  boxStyle->addItem( getQPixmap("hBars_xpm"), tr( " Horizontal Bars" ) );
  hl->addWidget(boxStyle);

  boxMatrixStyle = new QComboBox();
  boxMatrixStyle->addItem( getQPixmap("color_map_xpm"), tr("Contour - Color Fill"));
  boxMatrixStyle->addItem( getQPixmap("contour_map_xpm"), tr("Contour Lines"));
  boxMatrixStyle->addItem( getQPixmap("gray_map_xpm"), tr("Gray Scale Map"));
  boxMatrixStyle->addItem( getQPixmap("histogram_xpm"), tr("Histogram"));
  boxMatrixStyle->hide();
  hl->addWidget(boxMatrixStyle);
  hl->addStretch();

  QGridLayout *gl = new QGridLayout();
  gl->addWidget(new QLabel( tr( "Available data" )), 0, 0);
  gl->addWidget(new QLabel( tr( "Graph contents" )), 0, 2);

  available = new QListWidget();
  available->setSelectionMode (QAbstractItemView::ExtendedSelection);
  gl->addWidget(available, 1, 0);

  QVBoxLayout* vl1 = new QVBoxLayout();
  btnAdd = new QPushButton();
  btnAdd->setPixmap( getQPixmap("next_xpm") );
  btnAdd->setFixedWidth (35);
  btnAdd->setFixedHeight (30);
  vl1->addWidget(btnAdd);

  btnRemove = new QPushButton();
  btnRemove->setPixmap( getQPixmap("prev_xpm") );
  btnRemove->setFixedWidth (35);
  btnRemove->setFixedHeight(30);
  vl1->addWidget(btnRemove);
  vl1->addStretch();

  gl->addLayout(vl1, 1, 1);
  contents = new QListWidget();
  contents->setSelectionMode (QAbstractItemView::ExtendedSelection);
  gl->addWidget(contents, 1, 2);

  QVBoxLayout* vl2 = new QVBoxLayout();
  btnAssociations = new QPushButton(tr( "&Plot Associations..." ));
  btnAssociations->setEnabled(false);
  vl2->addWidget(btnAssociations);

  btnRange = new QPushButton(tr( "Edit &Range..." ));
  btnRange->setEnabled(false);
  vl2->addWidget(btnRange);

  btnEditFunction = new QPushButton(tr( "&Edit Function..." ));
  btnEditFunction->setEnabled(false);
  vl2->addWidget(btnEditFunction);

  btnOK = new QPushButton(tr( "OK" ));
  btnOK->setDefault( true );
  vl2->addWidget(btnOK);

  btnCancel = new QPushButton(tr( "Close" ));
  vl2->addWidget(btnCancel);

  boxShowRange = new QCheckBox(tr( "&Show Range" ));
  vl2->addWidget(boxShowRange);

  vl2->addStretch();
  gl->addLayout(vl2, 1, 3);

  QVBoxLayout* vl3 = new QVBoxLayout(this);
  vl3->addLayout(hl);
  vl3->addLayout(gl);

  boxShowCurrentFolder = new QCheckBox(tr("Show current &folder only" ));
  vl3->addWidget(boxShowCurrentFolder);

  init();

  connect(boxShowCurrentFolder, SIGNAL(toggled(bool)), this, SLOT(showCurrentFolder(bool)));
  connect(boxShowRange, SIGNAL(toggled(bool)), this, SLOT(showCurveRange(bool)));
  connect(btnRange, SIGNAL(clicked()),this, SLOT(showCurveRangeDialog()));
  connect(btnAssociations, SIGNAL(clicked()),this, SLOT(showPlotAssociations()));
  connect(btnEditFunction, SIGNAL(clicked()),this, SLOT(showFunctionDialog()));
  connect(btnAdd, SIGNAL(clicked()),this, SLOT(addCurves()));
  connect(btnRemove, SIGNAL(clicked()),this, SLOT(removeCurves()));
  connect(btnOK, SIGNAL(clicked()),this, SLOT(close()));
  connect(btnCancel, SIGNAL(clicked()),this, SLOT(close()));
  connect(contents, SIGNAL(currentRowChanged(int)), this, SLOT(showCurveBtn(int)));
  connect(contents, SIGNAL(itemSelectionChanged()), this, SLOT(enableRemoveBtn()));
  connect(available, SIGNAL(itemSelectionChanged()), this, SLOT(enableAddBtn()));

  QShortcut *shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
  connect(shortcut, SIGNAL(activated()), this, SLOT(removeCurves()));
  shortcut = new QShortcut(QKeySequence("-"), this);
  connect(shortcut, SIGNAL(activated()), this, SLOT(removeCurves()));
  shortcut = new QShortcut(QKeySequence("+"), this);
  connect(shortcut, SIGNAL(activated()), this, SLOT(addCurves()));

  setGraph(g);
}

CurvesDialog::~CurvesDialog()
{
  // Delete our local copies of the curves
  QMap<QString,PlotCurve*>::iterator it;
  for ( it = d_plotCurves.begin(); it != d_plotCurves.end() ; ++it)
  {
    delete it.value();
  }
}

void CurvesDialog::showCurveBtn(int)
{
  QwtPlotItem *it = d_graph->plotItem(contents->currentRow());
  if (!it)
    return;

  if ( it->rtti() == QwtPlotItem::Rtti_PlotSpectrogram ||
       it->rtti() == QwtPlotItem::Rtti_PlotUserItem )
  {
    btnEditFunction->setEnabled(false);
    btnAssociations->setEnabled(false);
    btnRange->setEnabled(false);
    return;
  }

  PlotCurve *c = dynamic_cast<PlotCurve *>(it);
  if (c->type() == Graph::Function)
  {
    btnEditFunction->setEnabled(true);
    btnAssociations->setEnabled(false);
    btnRange->setEnabled(false);
    return;
  }

  btnAssociations->setEnabled(true);

  btnRange->setEnabled(true);
  if (c->type() == Graph::ErrorBars)
    btnRange->setEnabled(false);
}

void CurvesDialog::showCurveRangeDialog()
{
  int curve = contents->currentRow();
  if (curve < 0)
    curve = 0;

  if (d_app)
  {
    d_app->showCurveRangeDialog(d_graph, curve);
    updateCurveRange();
  }
}

void CurvesDialog::showPlotAssociations()
{
  int curve = contents->currentRow();
  if (curve < 0)
    curve = 0;

  close();

  if (d_app)
    d_app->showPlotAssociations(curve);
}

void CurvesDialog::showFunctionDialog()
{
  int currentRow = contents->currentRow();
  close();

  if (d_app)
    d_app->showFunctionDialog(d_graph, currentRow);
}

QSize CurvesDialog::sizeHint() const
{
  return QSize(700, 400);
}

void CurvesDialog::contextMenuEvent(QContextMenuEvent *e)
{
  QPoint pos = available->viewport()->mapFromGlobal(QCursor::pos());
  QRect rect = available->visualItemRect(available->currentItem());
  if (rect.contains(pos))
  {
    QMenu contextMenu(this);
    QList<QListWidgetItem *> lst = available->selectedItems();
    if (lst.size() > 1)
      contextMenu.insertItem(tr("&Plot Selection"), this, SLOT(addCurves()));
    else if (lst.size() == 1)
      contextMenu.insertItem(tr("&Plot"), this, SLOT(addCurves()));
    contextMenu.exec(QCursor::pos());
  }

  pos = contents->viewport()->mapFromGlobal(QCursor::pos());
  rect = contents->visualItemRect(contents->currentItem());
  if (rect.contains(pos))
  {
    QMenu contextMenu(this);
    QList<QListWidgetItem *> lst = contents->selectedItems();
    if (lst.size() > 1)
      contextMenu.insertItem(tr("&Delete Selection"), this, SLOT(removeCurves()));
    else if (lst.size() == 1)
      contextMenu.insertItem(tr("&Delete Curve"), this, SLOT(removeCurves()));
    contextMenu.exec(QCursor::pos());
  }

  e->accept();
}

void CurvesDialog::init()
{
  if (d_app){
    bool currentFolderOnly = d_app->d_show_current_folder;
    boxShowCurrentFolder->setChecked(currentFolderOnly);
    showCurrentFolder(currentFolderOnly);

    QStringList matrices = d_app->matrixNames();
    if (!matrices.isEmpty ()){
      boxMatrixStyle->show();
      available->addItems(matrices);
    }

    int style = d_app->defaultCurveStyle;
    if (style == Graph::Line)
      boxStyle->setCurrentItem(0);
    else if (style == Graph::Scatter)
      boxStyle->setCurrentItem(1);
    else if (style == Graph::LineSymbols)
      boxStyle->setCurrentItem(2);
    else if (style == Graph::VerticalDropLines)
      boxStyle->setCurrentItem(3);
    else if (style == Graph::Spline)
      boxStyle->setCurrentItem(4);
    else if (style == Graph::VerticalSteps)
      boxStyle->setCurrentItem(5);
    else if (style == Graph::HorizontalSteps)
      boxStyle->setCurrentItem(6);
    else if (style == Graph::Area)
      boxStyle->setCurrentItem(7);
    else if (style == Graph::VerticalBars)
      boxStyle->setCurrentItem(8);
    else if (style == Graph::HorizontalBars)
      boxStyle->setCurrentItem(9);
  }

  QList<MdiSubWindow *> wList = d_app->windowsList();
  foreach(MdiSubWindow* w, wList)
  {
    MultiLayer* ml = dynamic_cast<MultiLayer*>(w);
    if (ml)// layers are numbered starting from 1
    {
      for(int i=1;i<=ml->layers();i++)
      {
        Graph* g = ml->layer(i);
        if (g)
        {
          for(int j=0;j<g->curves();j++)
          {
            MantidMatrixCurve* c = dynamic_cast<MantidMatrixCurve*>(g->curve(j));
            if (c)
            {
              available->addItem(c->title().text());
              // Store copies of the curves
              // Necessary because a curve is deleted when it's removed from a plot
              d_plotCurves[c->title().text()] = c->clone(g);
              ml->setCloseOnEmpty(false);
            }
          }
        }
      }
    }
  }

  if (!available->count())
    btnAdd->setDisabled(true);
}

void CurvesDialog::setGraph(Graph *graph)
{
  d_graph = graph;
  contents->addItems(d_graph->plotItemsList());
  enableRemoveBtn();
  enableAddBtn();
}

void CurvesDialog::addCurves()
{
  QStringList emptyColumns;
  QList<QListWidgetItem *> lst = available->selectedItems();
  for (int i = 0; i < lst.size(); ++i){
    QString text = lst.at(i)->text();
    if (contents->findItems(text, Qt::MatchExactly ).isEmpty ()){
      if (!addCurve(text))
        emptyColumns << text;
    }
  }
  d_graph->updatePlot();
  Graph::showPlotErrorMessage(this, emptyColumns);

  showCurveRange(boxShowRange->isChecked());
}

bool CurvesDialog::addCurve(const QString& name)
{
  if (!d_app)
    return false;

  QStringList matrices = d_app->matrixNames();
  if (matrices.contains(name)){
    Matrix *m = d_app->matrix(name);
    if (!m)
      return false;

    switch (boxMatrixStyle->currentIndex())
    {
    case 0:
      d_graph->plotSpectrogram(m, Graph::ColorMap);
      break;
    case 1:
      d_graph->plotSpectrogram(m, Graph::Contour);
      break;
    case 2:
      d_graph->plotSpectrogram(m, Graph::GrayScale);
      break;
    case 3:
      d_graph->addHistogram(m);
      break;
    }

    contents->addItem(name);
    return true;
  }

  int style = curveStyle();
  Table* t = d_app->table(name);
  if (t){
    PlotCurve *c = d_graph->insertCurve(t, name, style);
    CurveLayout cl = Graph::initCurveLayout();
    int color, symbol;
    d_graph->guessUniqueCurveLayout(color, symbol);

    cl.lCol = color;
    cl.symCol = color;
    cl.fillCol = color;
    cl.lWidth = float(d_app->defaultCurveLineWidth);
    cl.sSize = d_app->defaultSymbolSize;
    cl.sType = symbol;

    if (style == Graph::Line)
      cl.sType = 0;
    else if (style==Graph::VerticalBars || style==Graph::HorizontalBars ){
      cl.filledArea=1;
      cl.lCol=0;
      cl.aCol=color;
      cl.sType = 0;
    } else if (style==Graph::Area ){
      cl.filledArea=1;
      cl.aCol=color;
      cl.sType = 0;
    } else if (style == Graph::VerticalDropLines)
      cl.connectType=2;
    else if (style == Graph::VerticalSteps || style == Graph::HorizontalSteps){
      cl.connectType=3;
      cl.sType = 0;
    } else if (style == Graph::Spline)
      cl.connectType=5;

    d_graph->updateCurveLayout(c, &cl);
    contents->addItem(name);
    return true;
  }

  if (d_plotCurves.find(name) != d_plotCurves.end())
  {
    d_graph->insertCurve(d_plotCurves[name]->clone(d_graph));
    return true;
  }
  return false;
}

void CurvesDialog::removeCurves()
{
  QList<QListWidgetItem *> lst = contents->selectedItems();
  for (int i = 0; i < lst.size(); ++i){
    QListWidgetItem *it = lst.at(i);
    QString s = it->text();
    if (boxShowRange->isChecked()){
      QStringList lst = s.split("[");
      s = lst[0];
    }
    d_graph->removeCurve(s);
  }

  showCurveRange(boxShowRange->isChecked());
  d_graph->updatePlot();
}

void CurvesDialog::enableAddBtn()
{
  btnAdd->setEnabled (available->count()>0 && !available->selectedItems().isEmpty());
}

void CurvesDialog::enableRemoveBtn()
{
  btnRemove->setEnabled (contents->count()>0 && !contents->selectedItems().isEmpty());
}

int CurvesDialog::curveStyle()
{
  int style = 0;
  switch (boxStyle->currentItem())
  {
 case 0:
    style = Graph::Line;
    break;
 case 1:
    style = Graph::Scatter;
    break;
 case 2:
    style = Graph::LineSymbols;
    break;
 case 3:
    style = Graph::VerticalDropLines;
    break;
 case 4:
    style = Graph::Spline;
    break;
 case 5:
    style = Graph::VerticalSteps;
    break;
 case 6:
    style = Graph::HorizontalSteps;
    break;
 case 7:
    style = Graph::Area;
    break;
 case 8:
    style = Graph::VerticalBars;
    break;
 case 9:
    style = Graph::HorizontalBars;
    break;
  }
  return style;
}

void CurvesDialog::showCurveRange(bool on )
{
  int row = contents->currentRow();
  contents->clear();
  if (on){
    QStringList lst = QStringList();
    for (int i=0; i<d_graph->curves(); i++){
      QwtPlotItem *it = d_graph->plotItem(i);
      if (!it)
        continue;

      if (it->rtti() == QwtPlotItem::Rtti_PlotCurve && (dynamic_cast<PlotCurve *>(it))->type() != Graph::Function){
        DataCurve *c = dynamic_cast<DataCurve *>(it);
        lst << c->title().text() + "[" + QString::number(c->startRow()+1) + ":" + QString::number(c->endRow()+1) + "]";
      } else
        lst << it->title().text();
    }
    contents->addItems(lst);
  }
  else
    contents->addItems(d_graph->plotItemsList());

  contents->setCurrentRow(row);
  enableRemoveBtn();
}

void CurvesDialog::updateCurveRange()
{
  showCurveRange(boxShowRange->isChecked());
}

void CurvesDialog::showCurrentFolder(bool currentFolder)
{
  if (!d_app)
    return;

  d_app->d_show_current_folder = currentFolder;
  available->clear();

  if (currentFolder){
    Folder *f = d_app->currentFolder();
    if (f){
      QStringList columns;
      foreach (QWidget *w, f->windowsList()){
        if (!w->inherits("Table"))
          continue;

        Table *t = dynamic_cast<Table *>(w);
        for (int i=0; i < t->numCols(); i++){
          if(t->colPlotDesignation(i) == Table::Y)
            columns << QString(t->objectName()) + "_" + t->colLabel(i);
        }
      }
      available->addItems(columns);
    }
  }
  else
    available->addItems(d_app->columnsList(Table::Y));
}

void CurvesDialog::closeEvent(QCloseEvent* e)
{
  if (d_app)
  {
    d_app->d_add_curves_dialog_size = this->size();
    // Need to reenable close-on-empty behaviour so
    // that deleting workspaces causes the empty graphs to
    // disappear
    QList<MdiSubWindow *> wList = d_app->windowsList();
    foreach(MdiSubWindow* w, wList)
    {
      MultiLayer* ml = dynamic_cast<MultiLayer*>(w);
      if( ml ) ml->setCloseOnEmpty(true);
    }
  }

  e->accept();
}
