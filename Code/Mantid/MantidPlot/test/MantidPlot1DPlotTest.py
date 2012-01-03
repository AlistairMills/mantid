""" 
Test of basic 1D plotting methods in MantidPlot
"""
import mantidplottests
from mantidplottests import *
import time
import numpy as np
from PyQt4 import QtGui, QtCore

# =============== Create a fake workspace to plot =======================
X1 = np.linspace(0,10, 100)
Y1 = 1000*(np.sin(X1)**2) + X1*10
X1 = np.append(X1, 10.1)

X2 = np.linspace(2,12, 100)
Y2 = 500*(np.cos(X2/2.)**2) + 20
X2 = np.append(X2, 12.10)

X = np.append(X1, X2)
Y = np.append(Y1, Y2)
E = np.sqrt(Y)

CreateWorkspace("fake", list(X), list(Y), list(E), NSpec=2, UnitX="TOF", YUnitLabel="Counts",  WorkspaceTitle="Faked data Workspace")
#CreateWorkspace("fake2", X, Y, E, NSpec=2, UnitX="TOF", YUnitLabel="Counts",  WorkspaceTitle="Faked data Workspace")

class MantidPlot1DPlotTest(unittest.TestCase):
    
    def setUp(self):
        pass
    
    def tearDown(self):
        """Clean up by closing the created window """
        self.g.confirmClose(False)
        self.g.close()
        QtCore.QCoreApplication.processEvents()

    def test_plotSpectrum_errorBars(self):
        g = plotSpectrum("fake", 0, error_bars=True)
        screenshot(g, "plotSpectrum_errorBars", "Call to plotSpectrum() of 1 spectrum, with error bars.")
        self.g = g

    def test_plotSpectrum_fromWorkspaceProxy(self):
        ws = mtd["fake"]
        g = plotSpectrum(ws, 0, error_bars=True)
        self.g = g
        
    def test_plotSpectrum_severalSpectra(self):
        g = plotSpectrum("fake", [0, 1])
        screenshot(g, "plotSpectrum_severalSpectra", "Call to plotSpectrum() of 2 spectra, no error bars.")
        self.g = g
        
    def test_Customized1DPlot(self):
        g = plotSpectrum("fake", 0, error_bars=True)
        l = g.activeLayer()
        l.setCurveLineColor(0, QtGui.QColor("orange") )
        l.setCurveLineWidth(0, 2 )
        l.setTitle('My customized plot of <font face="Symbol">D</font>q')
        l.setTitleFont(QtGui.QFont("Arial", 12))
        l.setTitleColor(QtGui.QColor("red"))
        l.setTitleAlignment(QtCore.Qt.AlignLeft)
        l.setScale(2, 0, 3.0)
        l.setAntialiasing(True)
        screenshot(g, "Customized1DPlot", "1D plot of a spectrum, with error bars, an orange line of width 2, a custom title in red Arial font, with X from 0 to 3")
        self.g = g


# Run the unit tests
mantidplottests.runTests(MantidPlot1DPlotTest)

