#This is an extension of refl_gui.py as that is a auto-generated script form pyqt and shouldn't be edited
#so this file provides any extra GUI tweaks not easily doable in the designer
#for the time being this also includes non-GUI behaviour
import refl_window
import refl_save
import refl_choose_col
import refl_live_options
import csv
import string
import os
import re
from PyQt4 import QtCore, QtGui
from mantid.simpleapi import *
from isis_reflectometry.quick import *
from isis_reflectometry import load_live_runs
from isis_reflectometry.combineMulti import *
from latest_isis_runs import *
from mantid.api import Workspace, WorkspaceGroup

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

canMantidPlot = True

try:
    from mantidplot import *
except ImportError:
    canMantidPlot = False

class ReflGui(QtGui.QMainWindow, refl_window.Ui_windowRefl):


    __instrumentRuns = None

    def __init__(self):
        """
        Initialise the interface
        """
        super(QtGui.QMainWindow, self).__init__()
        self.setupUi(self)
        self.loading = False
        self.clip = QtGui.QApplication.clipboard()
        self.shown_cols = {}
        self.mod_flag = False
        self.run_cols = [0,5,10]
        self.angle_cols = [1,6,11]
        self.scale_col = 16
        self.stitch_col = 17
        self.plot_col = 18
        self._last_trans = ""
        #Setup instrument options with defaults assigned.
        self.instrument_list = ['INTER', 'SURF', 'CRISP', 'POLREF']
        self.polarisation_instruments = ['CRISP', 'POLREF']
        self.polarisation_options = {'None' : PolarisationCorrection.NONE, '1-PNR' : PolarisationCorrection.PNR, '2-PA' : PolarisationCorrection.PA }

        #Set the live data settings, use defualt if none have been set before
        settings = QtCore.QSettings()
        settings.beginGroup("Mantid/ISISReflGui/LiveData")
        self.live_method = settings.value("method", "", type=str)
        self.live_freq = settings.value("frequency", 0, type=float)
        if not (self.live_freq and self.live_method):
            logger.information("No settings were found for loading live data, Loading defaults (Update frequency: 60 seconds and Accumulation Method: Add)")
            self.live_freq = float(60)
            self.live_method = "Add"
            settings.setValue("frequency", self.live_freq)
            settings.setValue("method", self.live_method)
        settings.endGroup()
        del settings

    def __del__(self):
        """
        Save the contents of the table if the modified flag was still set
        """
        if self.mod_flag:
            self._save(true)

    def _save_check(self):
        """
        Show a standard message box asking if the user wants to save, or discard their changes or cancel back to the interface
        """
        msgBox = QtGui.QMessageBox()
        msgBox.setText("The table has been modified. Do you want to save your changes?")
        msgBox.setStandardButtons(QtGui.QMessageBox.Save | QtGui.QMessageBox.Discard | QtGui.QMessageBox.Cancel)
        msgBox.setIcon(QtGui.QMessageBox.Question)
        msgBox.setDefaultButton(QtGui.QMessageBox.Save)
        msgBox.setEscapeButton(QtGui.QMessageBox.Cancel)
        ret = msgBox.exec_()
        saved = None
        if ret == QtGui.QMessageBox.Save:
            saved = self._save()
        return ret, saved

    def closeEvent(self, event):
        """
        Close the window. but check if the user wants to save
        """
        self.buttonProcess.setFocus()
        if self.mod_flag:
            event.ignore()
            ret, saved = self._save_check()
            if ret == QtGui.QMessageBox.Save:
                if saved:
                    event.accept()
            elif ret == QtGui.QMessageBox.Discard:
                self.mod_flag = False
                event.accept()

    def _instrument_selected(self, instrument):
        """
        Change the default instrument to the selected one
        """
        config['default.instrument'] = self.instrument_list[instrument]
        logger.notice( "Instrument is now: " + str(config['default.instrument']))
        self.textRB.clear()
        self._populate_runs_list()
        self.current_instrument = self.instrument_list[instrument]
        self.comboPolarCorrect.setEnabled(self.current_instrument in self.polarisation_instruments) # Enable as appropriate
        self.comboPolarCorrect.setCurrentIndex(self.comboPolarCorrect.findText('None')) # Reset to None

    def _table_modified(self, row, column):
        """
        sets the modified flag when the table is altered
        """
        if not self.loading:
            self.mod_flag = True
            plotbutton = self.tableMain.cellWidget(row, self.plot_col).children()[1]
            self._reset_plot_button(plotbutton)

    def _plot_row(self):
        """
        handler for the plot buttons
        """
        plotbutton = self.sender()
        self._plot(plotbutton)

    def _polar_corr_selected(self):
        """
        Event handler for polarisation correction selection.
        """
        if self.current_instrument in self.polarisation_instruments:
            chosen_method = self.comboPolarCorrect.currentText()
            self.current_polarisation_method = self.polarisation_options[chosen_method]
        else:
            logger.notice("Polarisation correction is not supported on " + str(self.current_instrument))

    def setup_layout(self):
        """
        Do further setup layout that couldn't be done in the designer
        """
        self.comboInstrument.addItems(self.instrument_list)
        current_instrument = config['default.instrument'].upper()
        if current_instrument in self.instrument_list:
            self.comboInstrument.setCurrentIndex(self.instrument_list.index(current_instrument))
        else:
            self.comboInstrument.setCurrentIndex(0)
            config['default.instrument'] = 'INTER'
        self.current_instrument = config['default.instrument'].upper()

        #Setup polarisation options with default assigned
        self.comboPolarCorrect.clear()
        self.comboPolarCorrect.addItems(self.polarisation_options.keys())
        self.comboPolarCorrect.setCurrentIndex(self.comboPolarCorrect.findText('None'))
        self.current_polarisation_method = self.polarisation_options['None']
        self.comboPolarCorrect.setEnabled(self.current_instrument in self.polarisation_instruments)
        self.splitterList.setSizes([200, 800])
        self.labelStatus = QtGui.QLabel("Ready")
        self.statusMain.addWidget(self.labelStatus)
        self._initialise_table()
        self._populate_runs_list()
        self._connect_slots()
        return True

    def _reset_table(self):
        """
        Reset the plot buttons and stitch checkboxes back to thier defualt state
        """
        #switches from current to true, to false to make sure stateChanged fires
        self.checkTickAll.setCheckState(2)
        self.checkTickAll.setCheckState(0)
        for row in range(self.tableMain.rowCount()):
            plotbutton = self.tableMain.cellWidget(row, self.plot_col).children()[1]
            self._reset_plot_button(plotbutton)

    def _reset_plot_button(self, plotbutton):
        """
        Reset the provided plot button to ti's default state: disabled and with no cache
        """
        plotbutton.setDisabled(True)
        plotbutton.setProperty('runno', None)
        plotbutton.setProperty('overlapLow', None)
        plotbutton.setProperty('overlapHigh', None)
        plotbutton.setProperty('wksp', None)

    def _initialise_table(self):
        """
        Initialise the table. Clearing all data and adding the checkboxes and plot buttons
        """
        #first check if the table has been changed before clearing it
        if self.mod_flag:
            ret, saved = self._save_check()
            if ret == QtGui.QMessageBox.Cancel:
                return
        self.current_table = None

        settings = QtCore.QSettings()
        settings.beginGroup("Mantid/ISISReflGui/Columns")

        for column in range(self.tableMain.columnCount()):
            for row in range(self.tableMain.rowCount()):
                if (column in self.run_cols):
                    item = QtGui.QTableWidgetItem()
                    item.setText('')
                    item.setToolTip('Runs can be colon delimited to coadd them')
                    self.tableMain.setItem(row, column, item)
                elif (column in self.angle_cols):
                    item = QtGui.QTableWidgetItem()
                    item.setText('')
                    item.setToolTip('Angles are in degrees')
                    self.tableMain.setItem(row, column, item)
                elif column == self.stitch_col:
                    check = QtGui.QCheckBox()
                    check.setCheckState(False)
                    check.setToolTip('If checked, the runs in this row will be stitched together')
                    item = QtGui.QWidget()
                    layout = QtGui.QHBoxLayout(item)
                    layout.addWidget(check)
                    layout.setAlignment(QtCore.Qt.AlignCenter)
                    layout.setSpacing(0)
                    layout.setContentsMargins(0, 0, 0, 0)
                    item.setLayout(layout)
                    item.setContentsMargins(0, 0, 0, 0)
                    self.tableMain.setCellWidget(row, self.stitch_col, item)
                elif column == self.plot_col:
                    button = QtGui.QPushButton('Plot')
                    button.setProperty("row", row)
                    self._reset_plot_button(button)
                    button.setToolTip('Plot the workspaces produced by processing this row.')
                    button.clicked.connect(self._plot_row)
                    item = QtGui.QWidget()
                    layout = QtGui.QHBoxLayout(item)
                    layout.addWidget(button)
                    layout.setAlignment(QtCore.Qt.AlignCenter)
                    layout.setSpacing(0)
                    layout.setContentsMargins(0, 0, 0, 0)
                    item.setLayout(layout)
                    item.setContentsMargins(0, 0, 0, 0)
                    self.tableMain.setCellWidget(row, self.plot_col, item)
                else:
                    item = QtGui.QTableWidgetItem()
                    item.setText('')
                    self.tableMain.setItem(row, column, item)
            vis_state = settings.value(str(column), True, type=bool)
            self.shown_cols[column] = vis_state
            if vis_state:
                self.tableMain.showColumn(column)
            else:
                self.tableMain.hideColumn(column)
        settings.endGroup()
        del settings
        self.tableMain.resizeColumnsToContents()
        self.mod_flag = False

    def _connect_slots(self):
        """
        Connect the signals to the corresponding methods
        """
        self.checkTickAll.stateChanged.connect(self._set_all_stitch)
        self.comboInstrument.activated[int].connect(self._instrument_selected)
        self.comboPolarCorrect.activated.connect(self._polar_corr_selected)
        self.textRB.returnPressed.connect(self._populate_runs_list)
        self.buttonAuto.clicked.connect(self._autofill)
        self.buttonSearch.clicked.connect(self._populate_runs_list)
        self.buttonClear.clicked.connect(self._initialise_table)
        self.buttonProcess.clicked.connect(self._process)
        self.buttonTransfer.clicked.connect(self._transfer)
        self.buttonColumns.clicked.connect(self._choose_columns)
        self.actionOpen_Table.triggered.connect(self._load_table)
        self.actionReload_from_Disk.triggered.connect(self._reload_table)
        self.actionSave.triggered.connect(self._save)
        self.actionSave_As.triggered.connect(self._save_as)
        self.actionSave_Workspaces.triggered.connect(self._save_workspaces)
        self.actionClose_Refl_Gui.triggered.connect(self.close)
        self.actionMantid_Help.triggered.connect(self._show_help)
        self.actionAutofill.triggered.connect(self._autofill)
        self.actionSearch_RB.triggered.connect(self._populate_runs_list)
        self.actionClear_Table.triggered.connect(self._initialise_table)
        self.actionProcess.triggered.connect(self._process)
        self.actionTransfer.triggered.connect(self._transfer)
        self.tableMain.cellChanged.connect(self._table_modified)
        self.actionClear.triggered.connect(self._clear_cells)
        self.actionPaste.triggered.connect(self._paste_cells)
        self.actionCut.triggered.connect(self._cut_cells)
        self.actionCopy.triggered.connect(self._copy_cells)
        self.actionChoose_Columns.triggered.connect(self._choose_columns)
        self.actionLive_Data.triggered.connect(self._live_data_options)

    def _populate_runs_list(self):
        """
        Populate the list at the right with names of runs and workspaces form the ADS and archives
        """
        # Clear existing
        self.listMain.clear()
        # Fill with ADS workspaces
        self._populate_runs_listADSWorkspaces()
        try:
            selectedInstrument = config['default.instrument'].strip().upper()
            if not self.__instrumentRuns:
                self.__instrumentRuns =  LatestISISRuns(instrument=selectedInstrument)
                self.spinDepth.setMaximum(self.__instrumentRuns.getNumCycles())
            elif not self.__instrumentRuns.getInstrument() == selectedInstrument:
                self.__instrumentRuns =  LatestISISRuns(selectedInstrument)
                self.spinDepth.setMaximum(self.__instrumentRuns.getNumCycles())
            if self.textRB.text():
                runs = []
                self.statusMain.showMessage("Searching Journals for RB number: " + self.textRB.text())
                try:
                    runs = self.__instrumentRuns.getJournalRuns(self.textRB.text(),self.spinDepth.value())
                except:
                    logger.error( "Problem encountered when listing archive runs. Please check your network connection and that you have access to the journal archives.")
                    QtGui.QMessageBox.critical(self.tableMain, 'Error Retrieving Archive Runs',"Problem encountered when listing archive runs. Please check your network connection and that you have access to the journal archives.")
                    runs = []
                self.statusMain.clearMessage()
                for run in runs:
                    self.listMain.addItem(run)
                self.splitterList.setSizes([self.listMain.sizeHintForColumn(0), (1000 - self.listMain.sizeHintForColumn(0))])
        except Exception as ex:
            logger.notice("Could not list archive runs")
            logger.notice(str(ex))

    def _populate_runs_listADSWorkspaces(self):
        """
        get the workspaces from the ADS and add them to the list
        """
        names = mtd.getObjectNames()
        for ws in names:
            self.listMain.addItem(ws)

    def _autofill(self):
        """
        copy the contents of the selected cells to the row below as long as the row below contains a run number in the first cell
        """
        col = 0
        # make sure all selected cells are in the same row
        sum = 0
        howMany = len(self.tableMain.selectedItems())
        for cell in self.tableMain.selectedItems():
            sum = sum + self.tableMain.row(cell)
        if (howMany):
            selectedrow = self.tableMain.row(self.tableMain.selectedItems()[0])
            if (sum / howMany == selectedrow):
                startrow = selectedrow + 1
                filled = 0
                for cell in self.tableMain.selectedItems():
                    row = startrow
                    txt = cell.text()
                    while (self.tableMain.item(row, 0).text() != ''):
                        item = QtGui.QTableWidgetItem()
                        item.setText(txt)
                        self.tableMain.setItem(row, self.tableMain.column(cell), item)
                        row = row + 1
                        filled = filled + 1
                if not filled:
                    QtGui.QMessageBox.critical(self.tableMain, 'Cannot perform Autofill',"No target cells to autofill. Rows to be filled should contain a run number in their first cell, and start from directly below the selected line.")
            else:
                QtGui.QMessageBox.critical(self.tableMain, 'Cannot perform Autofill',"Selected cells must all be in the same row.")
        else:
            QtGui.QMessageBox.critical(self.tableMain, 'Cannot perform Autofill',"There are no source cells selected.")

    def _create_workspace_display_name(self, candidate):
        """
        Create a display name from a workspace.
        """
        if isinstance(mtd[candidate], WorkspaceGroup):
            todisplay = candidate # No single run number for a group of workspaces.
        else:
            todisplay = groupGet(mtd[candidate], "samp", "run_number")
        return todisplay

    def _clear_cells(self):
        """
        Clear the selected area of data
        """
        cells = self.tableMain.selectedItems()
        for cell in cells:
            column = cell.column()
            if column < self.stitch_col:
                cell.setText('')

    def _cut_cells(self):
        """
        copy the selected cells then clear the area
        """
        self._copy_cells()
        self._clear_cells()

    def _copy_cells(self):
        """
        Copy the selected ranage of cells to the clipboard
        """
        cells = self.tableMain.selectedItems()
        if not cells:
            print 'nothing to copy'
            return
        #first discover the size of the selection and initialise a list
        mincol = cells[0].column()
        if mincol > self.scale_col:
            logger.error("Cannot copy, all cells out of range")
            return
        maxrow = -1
        maxcol = -1
        minrow = cells[0].row()
        for cell in reversed(range(len(cells))):
            col = cells[cell].column()
            if col < self.stitch_col:
                maxcol = col
                maxrow = cells[cell].row()
                break
        colsize = maxcol - mincol + 1
        rowsize = maxrow - minrow + 1
        selection = [['' for x in range(colsize)] for y in range(rowsize)]
        #now fill that list
        for cell in cells:
            row = cell.row()
            col = cell.column()
            if col < self.stitch_col:
                selection[row - minrow][col - mincol] = str(cell.text())
        tocopy = ''
        for y in range(rowsize):
            for x in range(colsize):
                if x > 0:
                    tocopy += '\t'
                tocopy += selection[y][x]
            if y < (rowsize - 1):
                tocopy += '\n'
        self.clip.setText(str(tocopy))

    def _paste_cells(self):
        """
        Paste the contents of the clipboard to the table at the selected position
        """
        pastedtext = self.clip.text()
        if not pastedtext:
            logger.warning("Nothing to Paste")
            return
        selected = self.tableMain.selectedItems()
        if not selected:
            logger.warning("Cannot paste, no editable cells selected")
            return
        pasted = pastedtext.splitlines()
        pastedcells = []
        for row in pasted:
            pastedcells.append(row.split('\t'))
        pastedcols = len(pastedcells[0])
        pastedrows = len(pastedcells)
        if len(selected) > 1:
            #discover the size of the selection
            mincol = selected[0].column()
            if mincol > self.scale_col:
                logger.error("Cannot copy, all cells out of range")
                return
            minrow = selected[0].row()
            #now fill that list
            for cell in selected:
                row = cell.row()
                col = cell.column()
                if col < self.stitch_col and (col - mincol) < pastedcols and (row - minrow) < pastedrows and len(pastedcells[row - minrow]):
                    cell.setText(pastedcells[row - minrow][col - mincol])
        elif selected:
            #when only a single cell is selected, paste all the copied item up until the table limits
            cell = selected[0]
            currow = cell.row()
            homecol = cell.column()
            tablerows = self.tableMain.rowCount()
            for row in pastedcells:
                if len(row):
                    curcol = homecol
                    if currow < tablerows:
                        for col in row:
                            if curcol < self.stitch_col:
                                curcell = self.tableMain.item(currow, curcol)
                                curcell.setText(col)
                                curcol += 1
                            else:
                                #the row has hit the end of the editable cells
                                break
                        currow += 1
                    else:
                        #it's dropped off the bottom of the table
                        break
        else:
            logger.warning("Cannot paste, no editable cells selected")

    def _transfer(self):
        """
        Transfer run numbers to the table
        """
        col = 0
        row = 0
        while (self.tableMain.item(row, 0).text() != ''):
            row = row + 1
        for idx in self.listMain.selectedItems():
            contents = str(idx.text()).strip()
            first_contents = contents.split(':')[0]
            runnumber = None
            if mtd.doesExist(first_contents):
                runnumber = self._create_workspace_display_name(first_contents)
            else:
                try:
                    temp = Load(Filename=first_contents, OutputWorkspace="_tempforrunnumber")
                    runnumber = groupGet("_tempforrunnumber", "samp", "run_number")
                    DeleteWorkspace(temp)
                except:
                    logger.error("Unable to load file. Please check your managed user directories.")
                    QtGui.QMessageBox.critical(self.tableMain, 'Error Loading File',"Unable to load file. Please check your managed user directories.")
            item = QtGui.QTableWidgetItem()
            item.setText(runnumber)
            self.tableMain.setItem(row, col, item)
            item = QtGui.QTableWidgetItem()
            item.setText(self.textRuns.text())
            self.tableMain.setItem(row, col + 2, item)
            col = col + 5
            if col >= 11:
                col = 0
                row = row + 1

    def _set_all_stitch(self,state):
        """
        Set the checkboxes in the Stitch? column to the same
        """
        for row in range(self.tableMain.rowCount()):
            self.tableMain.cellWidget(row, self.stitch_col).children()[1].setCheckState(state)

    def _process(self):
        """
        Process has been pressed, check what has been selected then pass the selection (or whole table) to quick
        """
#--------- If "Process" button pressed, convert raw files to IvsLam and IvsQ and combine if checkbox ticked -------------
        try:
            willProcess = True
            rows = self.tableMain.selectionModel().selectedRows()
            rowIndexes=[]
            for idx in rows:
                rowIndexes.append(idx.row())
            if not len(rowIndexes):
                reply = QtGui.QMessageBox.question(self.tableMain, 'Process all rows?',"This will process all rows in the table. Continue?", QtGui.QMessageBox.Yes, QtGui.QMessageBox.No)
                if reply == QtGui.QMessageBox.No:
                    logger.notice("Cancelled!")
                    willProcess = False
                else:
                    rowIndexes = range(self.tableMain.rowCount())
            if willProcess:
                self._last_trans = ""
                for row in rowIndexes:  # range(self.tableMain.rowCount()):
                    runno = []
                    loadedRuns = []
                    wksp = []
                    overlapLow = []
                    overlapHigh = []
                    theta = [0, 0, 0]
                    if (self.tableMain.item(row, 0).text() != ''):
                        self.statusMain.showMessage("Processing row: " + str(row + 1))
                        logger.debug("Processing row: " + str(row + 1))
                        for i in range(3):
                            r = str(self.tableMain.item(row, i * 5).text())
                            if (r != ''):
                                runno.append(r)
                            ovLow = str(self.tableMain.item(row, i * 5 + 3).text())
                            if (ovLow != ''):
                                overlapLow.append(float(ovLow))
                            ovHigh = str(self.tableMain.item(row, i * 5 + 4).text())
                            if (ovHigh != ''):
                                overlapHigh.append(float(ovHigh))
                        # Determine resolution
                        if (self.tableMain.item(row, 15).text() == ''):
                            loadedRun = None
                            if load_live_runs.is_live_run(runno[0]):
                                loadedRun = load_live_runs.get_live_data(config['default.instrument'], frequency = self.live_freq, accumulation = self.live_method)
                            else:
                                Load(Filename=runno[0], OutputWorkspace="run")
                                loadedRun = mtd["run"]
                            try:
                                dqq = calcRes(loadedRun)
                                item = QtGui.QTableWidgetItem()
                                item.setText(str(dqq))
                                self.tableMain.setItem(row, 15, item)
                                logger.notice("Calculated resolution: " + str(dqq))
                            except IndexError:
                                self.statusMain.clearMessage()
                                logger.error("Cannot calculate resolution owing to unknown log properties. dq/q will need to be manually entered.")
                                return
                        else:
                            dqq = float(self.tableMain.item(row, 15).text())
                        # Populate runlist
                        first_wq = None
                        for i in range(len(runno)):
                            theta, qmin, qmax, wlam, wq = self._do_run(runno[i], row, i)
                            if not first_wq:
                                first_wq = wq # Cache the first Q workspace
                            theta = round(theta, 3)
                            qmin = round(qmin, 3)
                            qmax = round(qmax, 3)
                            wksp.append(wq.name())
                            if (self.tableMain.item(row, i * 5 + 1).text() == ''):
                                item = QtGui.QTableWidgetItem()
                                item.setText(str(theta))
                                self.tableMain.setItem(row, i * 5 + 1, item)
                            if (self.tableMain.item(row, i * 5 + 3).text() == ''):
                                item = QtGui.QTableWidgetItem()
                                item.setText(str(qmin))
                                self.tableMain.setItem(row, i * 5 + 3, item)
                                overlapLow.append(qmin)
                            if (self.tableMain.item(row, i * 5 + 4).text() == ''):
                                item = QtGui.QTableWidgetItem()
                                if i == len(runno) - 1:
                                # allow full high q-range for last angle
                                    qmax = 4 * math.pi / ((4 * math.pi / qmax * math.sin(theta * math.pi / 180)) - 0.5) * math.sin(theta * math.pi / 180)
                                item.setText(str(qmax))
                                self.tableMain.setItem(row, i * 5 + 4, item)
                                overlapHigh.append(qmax)
                            if wksp[i].find(',') > 0 or wksp[i].find(':') > 0:
                                wksp[i] = first_wq.name()
                        plotbutton = self.tableMain.cellWidget(row, self.plot_col).children()[1]
                        plotbutton.setProperty('runno',runno)
                        plotbutton.setProperty('overlapLow', overlapLow)
                        plotbutton.setProperty('overlapHigh', overlapHigh)
                        plotbutton.setProperty('wksp', wksp)
                        plotbutton.setEnabled(True)
                        self.statusMain.clearMessage()
            self.accMethod = None
            self.statusMain.clearMessage()
            self._last_trans = ""
            if mtd.doesExist("transWS"):
                DeleteWorkspace("transWS")
        except:
            self.statusMain.clearMessage()
            raise

    def _plot(self, plotbutton):
        """
        Plot the row belonging to the selected button
        """
        if not isinstance(plotbutton, QtGui.QPushButton):
            logger.error("Problem accessing cached data: Wrong data type passed, expected QtGui.QPushbutton")
            return
        import unicodedata

        #make sure the required data can be retrieved properly
        try:
            runno_u = plotbutton.property('runno')
            runno = []
            for uni in runno_u:
                runno.append(unicodedata.normalize('NFKD', uni).encode('ascii','ignore'))
            wksp_u = plotbutton.property('wksp')
            wksp = []
            for uni in wksp_u:
                wksp.append(unicodedata.normalize('NFKD', uni).encode('ascii','ignore'))
            overlapLow = plotbutton.property('overlapLow')
            overlapHigh = plotbutton.property('overlapHigh')
            row = plotbutton.property('row')
            g = ['g1', 'g2', 'g3']
            wkspBinned = []
            w1 = getWorkspace(wksp[0])
            w2 = getWorkspace(wksp[len(wksp) - 1])
            dqq = float(self.tableMain.item(row, 15).text())
        except:
            logger.error("Unable to plot row, required data couldn't be retrieved")
            _reset_plot_button(plotbutton)
            return
        for i in range(len(runno)):
            ws_name_binned = wksp[i] + '_binned'
            ws = getWorkspace(wksp[i])
            if len(overlapLow):
                Qmin = overlapLow[0]
            else:
                Qmin = w1.readX(0)[0]
            if len(overlapHigh):
                Qmax = overlapHigh[len(overlapHigh) - 1]
            else:
                Qmax = max(w2.readX(0))
            Rebin(InputWorkspace=str(wksp[i]), Params=str(overlapLow[i]) + ',' + str(-dqq) + ',' + str(overlapHigh[i]), OutputWorkspace=ws_name_binned)
            wkspBinned.append(ws_name_binned)
            wsb = getWorkspace(ws_name_binned)
            Imin = min(wsb.readY(0))
            Imax = max(wsb.readY(0))
            if canMantidPlot:
                g[i] = plotSpectrum(ws_name_binned, 0, True)
                titl = groupGet(ws_name_binned, 'samp', 'run_title')
                if (i > 0):
                    mergePlots(g[0], g[i])
                if (type(titl) == str):
                    g[0].activeLayer().setTitle(titl)
                g[0].activeLayer().setAxisScale(Layer.Left, Imin * 0.1, Imax * 10, Layer.Log10)
                g[0].activeLayer().setAxisScale(Layer.Bottom, Qmin * 0.9, Qmax * 1.1, Layer.Log10)
                g[0].activeLayer().setAutoScale()
        if (self.tableMain.cellWidget(row, self.stitch_col).children()[1].checkState() > 0):
            if (len(runno) == 1):
                logger.notice("Nothing to combine!")
            elif (len(runno) == 2):
                outputwksp = runno[0] + '_' + runno[1][3:5]
            else:
                outputwksp = runno[0] + '_' + runno[2][3:5]
            begoverlap = w2.readX(0)[0]
            # get Qmax
            if (self.tableMain.item(row, i * 5 + 4).text() == ''):
                overlapHigh = 0.3 * max(w1.readX(0))
            wcomb = combineDataMulti(wkspBinned, outputwksp, overlapLow, overlapHigh, Qmin, Qmax, -dqq, 1)
            if self.tableMain.item(row, self.scale_col).text():
                Scale(InputWorkspace=outputwksp, OutputWorkspace=outputwksp, Factor=1 / float(self.tableMain.item(row, self.scale_col).text()))
            Qmin = getWorkspace(outputwksp).readX(0)[0]
            Qmax = max(getWorkspace(outputwksp).readX(0))
            if canMantidPlot:
                gcomb = plotSpectrum(outputwksp, 0, True)
                titl = groupGet(outputwksp, 'samp', 'run_title')
                gcomb.activeLayer().setTitle(titl)
                gcomb.activeLayer().setAxisScale(Layer.Left, 1e-8, 100.0, Layer.Log10)
                gcomb.activeLayer().setAxisScale(Layer.Bottom, Qmin * 0.9, Qmax * 1.1, Layer.Log10)

    def _do_run(self, runno, row, which):
        """
        Run quick on the given run and row
        """
        g = ['g1', 'g2', 'g3']
        transrun = str(self.tableMain.item(row, which * 5 + 2).text())
        if mtd.doesExist("transWS") and mtd["transWS"].getAxis(0).getUnit().unitID() == "Wavelength" and self._check_trans_run(transrun):
            self._last_trans = transrun
            transrun = mtd["transWS"]
        else:
            self._last_trans = transrun
        angle = str(self.tableMain.item(row, which * 5 + 1).text())
        loadedRun = runno
        if load_live_runs.is_live_run(runno):
            load_live_runs.get_live_data(config['default.instrument'], frequency = self.live_freq, accumulation = self.live_method)
        wlam, wq, th = quick(loadedRun, trans=transrun, theta=angle)
        if ':' in runno:
            runno = runno.split(':')[0]
        if ',' in runno:
            runno = runno.split(',')[0]
        inst = groupGet(wq, 'inst')
        lmin = inst.getNumberParameter('LambdaMin')[0] + 1
        lmax = inst.getNumberParameter('LambdaMax')[0] - 2
        qmin = 4 * math.pi / lmax * math.sin(th * math.pi / 180)
        qmax = 4 * math.pi / lmin * math.sin(th * math.pi / 180)
        return th, qmin, qmax, wlam, wq
    def _check_trans_run(self, transrun):
        if self._last_trans == transrun:
            return True
        translist = [word.strip() for word in re.split(',|:', transrun)]
        lastlist = [word.strip() for word in re.split(',|:', self._last_trans)]
        if len(translist) == len(lastlist):
            for i in range(len(lastlist)):
                if not translist[i] == lastlist[i]:
                    return False
        else:
            return False
        return True
    def _save_table_contents(self, filename):
        """
        Save the contents of the table
        """
        try:
            writer = csv.writer(open(filename, "wb"))
            for row in range(self.tableMain.rowCount()):
                rowtext = []
                for column in range(self.tableMain.columnCount() - 2):
                    rowtext.append(self.tableMain.item(row, column).text())
                if (len(rowtext) > 0):
                    writer.writerow(rowtext)
            self.current_table = filename
            logger.notice("Saved file to " + filename)
            self.mod_flag = False
        except:
            return False
        self.mod_flag = False
        return True

    def _save(self, failsave = False):
        """
        Save the table, showing no interface if not necessary. This also provides the failing save functionality.
        """
        filename = ''
        if failsave:
            #this is an emergency autosave as the program is failing
            logger.error("The ISIS Reflectonomy GUI has encountered an error, it will now attempt to save a copy of your work.")
            msgBox = QtGui.QMessageBox()
            msgBox.setText("The ISIS Reflectonomy GUI has encountered an error, it will now attempt to save a copy of your work.\nPlease check the log for details.")
            msgBox.setStandardButtons(QtGui.QMessageBox.Ok)
            msgBox.setIcon(QtGui.QMessageBox.Critical)
            msgBox.setDefaultButton(QtGui.QMessageBox.Ok)
            msgBox.setEscapeButton(QtGui.QMessageBox.Ok)
            msgBox.exec_()
            import datetime
            failtime = datetime.datetime.today().strftime('%Y-%m-%d_%H-%M-%S')
            if self.current_table:
                filename = self.current_table.rsplit('.',1)[0] + "_recovered_" + failtime + ".tbl"
            else:
                mantidDefault = config['defaultsave.directory']
                if os.path.exists(mantidDefault):
                    filename = os.path.join(mantidDefault,"mantid_reflectometry_recovered_" + failtime + ".tbl")
                else:
                    import tempfile
                    tempDir = tempfile.gettempdir()
                    filename = os.path.join(tempDir,"mantid_reflectometry_recovered_" + failtime + ".tbl")
        else:
            #this is a save-on-quit or file->save
            if self.current_table:
                filename = self.current_table
            else:
                saveDialog = QtGui.QFileDialog(self.widgetMainRow.parent(), "Save Table")
                saveDialog.setFileMode(QtGui.QFileDialog.AnyFile)
                saveDialog.setNameFilter("Table Files (*.tbl);;All files (*.*)")
                saveDialog.setDefaultSuffix("tbl")
                saveDialog.setAcceptMode(QtGui.QFileDialog.AcceptSave)
                if saveDialog.exec_():
                    filename = saveDialog.selectedFiles()[0]
                else:
                    return False
        return self._save_table_contents(filename)

    def _save_as(self):
        """
        show the save as dialog and save to a .tbl file with that name
        """
        saveDialog = QtGui.QFileDialog(self.widgetMainRow.parent(), "Save Table")
        saveDialog.setFileMode(QtGui.QFileDialog.AnyFile)
        saveDialog.setNameFilter("Table Files (*.tbl);;All files (*.*)")
        saveDialog.setDefaultSuffix("tbl")
        saveDialog.setAcceptMode(QtGui.QFileDialog.AcceptSave)
        if saveDialog.exec_():
            filename = saveDialog.selectedFiles()[0]
            self._save_table_contents(filename)

    def _load_table(self):
        """
        Load a .tbl file from disk
        """
        self.loading = True
        loadDialog = QtGui.QFileDialog(self.widgetMainRow.parent(), "Open Table")
        loadDialog.setFileMode(QtGui.QFileDialog.ExistingFile)
        loadDialog.setNameFilter("Table Files (*.tbl);;All files (*.*)")
        if loadDialog.exec_():
            try:
                #before loading make sure you give them a chance to save
                if self.mod_flag:
                    ret, saved = self._save_check()
                    if ret == QtGui.QMessageBox.Cancel:
                        #if they hit cancel abort the load
                        self.loading = False
                        return
                self._reset_table()
                filename = loadDialog.selectedFiles()[0]
                self.current_table = filename
                reader = csv.reader(open(filename, "rb"))
                row = 0
                for line in reader:
                    if (row < 100):
                        for column in range(self.tableMain.columnCount() - 2):
                            item = QtGui.QTableWidgetItem()
                            item.setText(line[column])
                            self.tableMain.setItem(row, column, item)
                        row = row + 1
            except:
                logger.error('Could not load file: ' + str(filename) + '. File not found or unable to read from file.')
        self.loading = False
        self.mod_flag = False

    def _reload_table(self):
        """
        Reload the last loaded file from disk, replacing anything in the table already
        """
        self.loading = True
        filename = self.current_table
        if filename:
            if self.mod_flag:
                msgBox = QtGui.QMessageBox()
                msgBox.setText("The table has been modified. Are you sure you want to reload the table and lose your changes?")
                msgBox.setStandardButtons(QtGui.QMessageBox.Yes | QtGui.QMessageBox.No)
                msgBox.setIcon(QtGui.QMessageBox.Question)
                msgBox.setDefaultButton(QtGui.QMessageBox.Yes)
                msgBox.setEscapeButton(QtGui.QMessageBox.No)
                ret = msgBox.exec_()
                if ret == QtGui.QMessageBox.No:
                    #if they hit No abort the reload
                    self.loading = False
                    return
            try:
                self._reset_table()
                reader = csv.reader(open(filename, "rb"))
                row = 0
                for line in reader:
                    if (row < 100):
                        for column in range(self.tableMain.columnCount() - 2):
                            item = QtGui.QTableWidgetItem()
                            item.setText(line[column])
                            self.tableMain.setItem(row, column, item)
                        row = row + 1
                self.mod_flag = False
            except:
                logger.error('Could not load file: ' + str(filename) + '. File not found or unable to read from file.')
        else:
            logger.notice('No file in table to reload.')
        self.loading = False

    def _save_workspaces(self):
        """
        Shows the export dialog for saving workspaces to non mantid formats
        """
        try:
            Dialog = QtGui.QDialog()
            u = refl_save.Ui_SaveWindow()
            u.setupUi(Dialog)
            Dialog.exec_()
        except Exception as ex:
            logger.notice("Could not open save workspace dialog")
            logger.notice(str(ex))

    def _live_data_options(self):
        """
        Shows the dialog for setting options regarding live data
        """
        try:
            dialog = refl_live_options.ReflLiveOptions(def_meth = self.live_method, def_freq = self.live_freq)
            if dialog.exec_():
                self.live_freq = dialog.frequency
                self.live_method = dialog.get_method()
                settings = QtCore.QSettings()
                settings.beginGroup("Mantid/ISISReflGui/LiveData")
                settings.setValue("frequency", self.live_freq)
                settings.setValue("method", self.live_method)
                settings.endGroup()
                del settings
        except Exception as ex:
            logger.notice("Could not open live data options dialog")
            logger.notice(str(ex))

    def _choose_columns(self):
        """
        shows the choose columns dialog for hiding and revealing of columns
        """
        try:
            dialog = refl_choose_col.ReflChoose(self.shown_cols, self.tableMain)
            if dialog.exec_():
                settings = QtCore.QSettings()
                settings.beginGroup("Mantid/ISISReflGui/Columns")
                for key, value in dialog.visiblestates.iteritems():
                    self.shown_cols[key] = value
                    settings.setValue(str(key), value)
                    if value:
                        self.tableMain.showColumn(key)
                    else:
                        self.tableMain.hideColumn(key)
                settings.endGroup()
                del settings
        except Exception as ex:
            logger.notice("Could not open choose columns dialog")
            logger.notice(str(ex))

    def _show_help(self):
        """
        Launches the wiki page for this interface
        """
        import webbrowser
        webbrowser.open('http://www.mantidproject.org/ISIS_Reflectometry_GUI')

def get_representative_workspace(run):
    """
    Get a representative workspace from the input workspace.
    """
    if isinstance(run, WorkspaceGroup):
        run_number = groupGet(run[0], "samp", "run_number")
        _runno = Load(Filename=str(run_number))
    elif isinstance(run, Workspace):
        _runno = run
    elif isinstance(run, int):
        _runno = Load(Filename=run, OutputWorkspace=runno)
    elif isinstance(run, str) and mtd.doesExist(run):
        ws = mtd[run]
        if isinstance(ws, WorkspaceGroup):
            run_number = groupGet(ws[0], "samp", "run_number")
            _runno = Load(Filename=str(run_number))
    elif isinstance(run, str):
        _runno = Load(Filename=run.replace("raw", "nxs", 1), OutputWorkspace=runno)
    else:
        raise TypeError("Must be a workspace, int or str")
    return _runno

def calcRes(run):
    """
    Calculate the resolution from the slits.
    """
    runno = get_representative_workspace(run)
    # Get slits and detector angle theta from NeXuS
    th = groupGet(runno, 'samp', 'THETA')
    inst = groupGet(runno, 'inst')
    s1z = inst.getComponentByName('slit1').getPos().getZ() * 1000.0  # distance in mm
    s2z = inst.getComponentByName('slit2').getPos().getZ() * 1000.0  # distance in mm
    s1vg = inst.getComponentByName('slit1')
    s1vg = s1vg.getNumberParameter('vertical gap')[0]
    s2vg = inst.getComponentByName('slit2')
    s2vg = s2vg.getNumberParameter('vertical gap')[0]
    logger.notice( "s1vg=" + str(s1vg) + " s2vg=" + str(s2vg) + " theta=" + str(th))
    #1500.0 is the S1-S2 distance in mm for SURF!!!
    resolution = math.atan((s1vg + s2vg) / (2 * (s2z - s1z))) * 180 / math.pi / th
    logger.notice( "dq/q=" + str(resolution))
    if not type(run) == type(Workspace):
        DeleteWorkspace(runno)
    return resolution

def groupGet(wksp, whattoget, field=''):
    """
    returns information about instrument or sample details for a given workspace wksp,
    also if the workspace is a group (info from first group element)
    """
    if (whattoget == 'inst'):
        if isinstance(wksp, str):
            at = getattr(mtd[wksp],'size',None)
            if callable(at):
                return mtd[wksp][0].getInstrument()
            else:
                return mtd[wksp].getInstrument()
        elif isinstance(wksp, Workspace):
            at = getattr(wksp,'size',None)
            if callable(at):
                return wksp[0].getInstrument()
            else:
                return wksp.getInstrument()
        else:
            return 0
    elif (whattoget == 'samp' and field != ''):
        if isinstance(wksp, str):
            at = getattr(mtd[wksp],'size',None)
            if callable(at):
                try:
                    log = mtd[wksp][0].getRun().getLogData(field).value
                    if (type(log) is int or type(log) is str):
                        res = log
                    else:
                        res = log[-1]
                except RuntimeError:
                    res = 0
                    logger.error( "Block " + str(field) + " not found.")
            else:
                try:
                    log = mtd[wksp].getRun().getLogData(field).value
                    if (type(log) is int or type(log) is str):
                        res = log
                    else:
                        res = log[-1]
                except RuntimeError:
                    res = 0
                    logger.error( "Block " + str(field) + " not found.")
        elif isinstance(wksp, Workspace):
            at = getattr(wksp,'size',None)
            if callable(at):
                try:
                    log = wksp[0].getRun().getLogData(field).value
                    if (type(log) is int or type(log) is str):
                        res = log
                    else:
                        res = log[-1]
                except RuntimeError:
                    res = 0
                    logger.error( "Block " + str(field) + " not found.")
            else:
                try:
                    log = wksp.getRun().getLogData(field).value
                    if (type(log) is int or type(log) is str):
                        res = log
                    else:
                        res = log[-1]
                except RuntimeError:
                    res = 0
                    logger.error( "Block " + str(field) + " not found.")
        else:
            res = 0
        return res
    elif (whattoget == 'wksp'):
        if isinstance(wksp, str):
            at = getattr(mtd[wksp],'size',None)
            if callable(at):
                return mtd[wksp][0].getNumberHistograms()
            else:
                return mtd[wksp].getNumberHistograms()
        elif isinstance(wksp, Workspace):
            at = getattr(wksp,'size',None)
            if callable(at):
                return mtd[wksp][0].getNumberHistograms()
            else:
                return wksp.getNumberHistograms()
        else:
            return 0

def getWorkspace(wksp):
    """
    Gets the first workspace associated with the given string. Does not load.
    """
    if isinstance(wksp, Workspace):
        return wksp
    elif isinstance(wksp, str):
        if isinstance(mtd[wksp], WorkspaceGroup):
            wout = mtd[wksp][0]
        else:
            wout = mtd[wksp]
        return wout
    else:
        logger.error( "Unable to get workspace: " + str(wksp))
        return 0
