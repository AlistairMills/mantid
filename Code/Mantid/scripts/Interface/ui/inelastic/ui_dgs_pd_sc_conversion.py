# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui/inelastic/dgs_pd_sc_conversion.ui'
#
# Created: Tue Nov 13 14:25:59 2012
#      by: PyQt4 UI code generator 4.9.1
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_PdScConversionFrame(object):
    def setupUi(self, PdScConversionFrame):
        PdScConversionFrame.setObjectName(_fromUtf8("PdScConversionFrame"))
        PdScConversionFrame.resize(878, 586)
        PdScConversionFrame.setFrameShape(QtGui.QFrame.StyledPanel)
        PdScConversionFrame.setFrameShadow(QtGui.QFrame.Raised)
        self.verticalLayout_3 = QtGui.QVBoxLayout(PdScConversionFrame)
        self.verticalLayout_3.setObjectName(_fromUtf8("verticalLayout_3"))
        self.powder_gb = QtGui.QGroupBox(PdScConversionFrame)
        self.powder_gb.setCheckable(True)
        self.powder_gb.setChecked(False)
        self.powder_gb.setObjectName(_fromUtf8("powder_gb"))
        self.verticalLayout = QtGui.QVBoxLayout(self.powder_gb)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.q_binning_gb = QtGui.QGroupBox(self.powder_gb)
        self.q_binning_gb.setObjectName(_fromUtf8("q_binning_gb"))
        self.horizontalLayout_2 = QtGui.QHBoxLayout(self.q_binning_gb)
        self.horizontalLayout_2.setObjectName(_fromUtf8("horizontalLayout_2"))
        self.q_low_label = QtGui.QLabel(self.q_binning_gb)
        self.q_low_label.setObjectName(_fromUtf8("q_low_label"))
        self.horizontalLayout_2.addWidget(self.q_low_label)
        self.q_low_edit = QtGui.QLineEdit(self.q_binning_gb)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.q_low_edit.sizePolicy().hasHeightForWidth())
        self.q_low_edit.setSizePolicy(sizePolicy)
        self.q_low_edit.setObjectName(_fromUtf8("q_low_edit"))
        self.horizontalLayout_2.addWidget(self.q_low_edit)
        self.q_width_label = QtGui.QLabel(self.q_binning_gb)
        self.q_width_label.setObjectName(_fromUtf8("q_width_label"))
        self.horizontalLayout_2.addWidget(self.q_width_label)
        self.q_width_edit = QtGui.QLineEdit(self.q_binning_gb)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.q_width_edit.sizePolicy().hasHeightForWidth())
        self.q_width_edit.setSizePolicy(sizePolicy)
        self.q_width_edit.setObjectName(_fromUtf8("q_width_edit"))
        self.horizontalLayout_2.addWidget(self.q_width_edit)
        self.q_high_label = QtGui.QLabel(self.q_binning_gb)
        self.q_high_label.setObjectName(_fromUtf8("q_high_label"))
        self.horizontalLayout_2.addWidget(self.q_high_label)
        self.q_high_edit = QtGui.QLineEdit(self.q_binning_gb)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.q_high_edit.sizePolicy().hasHeightForWidth())
        self.q_high_edit.setSizePolicy(sizePolicy)
        self.q_high_edit.setObjectName(_fromUtf8("q_high_edit"))
        self.horizontalLayout_2.addWidget(self.q_high_edit)
        self.q_units_label = QtGui.QLabel(self.q_binning_gb)
        self.q_units_label.setObjectName(_fromUtf8("q_units_label"))
        self.horizontalLayout_2.addWidget(self.q_units_label)
        spacerItem = QtGui.QSpacerItem(189, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_2.addItem(spacerItem)
        self.verticalLayout.addWidget(self.q_binning_gb)
        self.horizontalLayout_3 = QtGui.QHBoxLayout()
        self.horizontalLayout_3.setObjectName(_fromUtf8("horizontalLayout_3"))
        self.save_procnexus_cb = QtGui.QCheckBox(self.powder_gb)
        self.save_procnexus_cb.setChecked(True)
        self.save_procnexus_cb.setObjectName(_fromUtf8("save_procnexus_cb"))
        self.horizontalLayout_3.addWidget(self.save_procnexus_cb)
        self.save_procnexus_edit = QtGui.QLineEdit(self.powder_gb)
        self.save_procnexus_edit.setObjectName(_fromUtf8("save_procnexus_edit"))
        self.horizontalLayout_3.addWidget(self.save_procnexus_edit)
        self.save_procnexus_save = QtGui.QPushButton(self.powder_gb)
        self.save_procnexus_save.setObjectName(_fromUtf8("save_procnexus_save"))
        self.horizontalLayout_3.addWidget(self.save_procnexus_save)
        spacerItem1 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_3.addItem(spacerItem1)
        self.verticalLayout.addLayout(self.horizontalLayout_3)
        self.verticalLayout_3.addWidget(self.powder_gb)
        self.single_crystal_gb = QtGui.QGroupBox(PdScConversionFrame)
        self.single_crystal_gb.setCheckable(True)
        self.single_crystal_gb.setChecked(False)
        self.single_crystal_gb.setObjectName(_fromUtf8("single_crystal_gb"))
        self.verticalLayout_3.addWidget(self.single_crystal_gb)
        self.save_nxspe_gb = QtGui.QGroupBox(PdScConversionFrame)
        self.save_nxspe_gb.setCheckable(True)
        self.save_nxspe_gb.setChecked(False)
        self.save_nxspe_gb.setObjectName(_fromUtf8("save_nxspe_gb"))
        self.verticalLayout_2 = QtGui.QVBoxLayout(self.save_nxspe_gb)
        self.verticalLayout_2.setObjectName(_fromUtf8("verticalLayout_2"))
        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.save_nxspe_label = QtGui.QLabel(self.save_nxspe_gb)
        self.save_nxspe_label.setObjectName(_fromUtf8("save_nxspe_label"))
        self.horizontalLayout.addWidget(self.save_nxspe_label)
        self.save_nxspe_edit = QtGui.QLineEdit(self.save_nxspe_gb)
        self.save_nxspe_edit.setObjectName(_fromUtf8("save_nxspe_edit"))
        self.horizontalLayout.addWidget(self.save_nxspe_edit)
        self.save_nxspe_save = QtGui.QPushButton(self.save_nxspe_gb)
        self.save_nxspe_save.setObjectName(_fromUtf8("save_nxspe_save"))
        self.horizontalLayout.addWidget(self.save_nxspe_save)
        spacerItem2 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem2)
        self.verticalLayout_2.addLayout(self.horizontalLayout)
        self.verticalLayout_3.addWidget(self.save_nxspe_gb)
        spacerItem3 = QtGui.QSpacerItem(20, 315, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.verticalLayout_3.addItem(spacerItem3)

        self.retranslateUi(PdScConversionFrame)
        QtCore.QMetaObject.connectSlotsByName(PdScConversionFrame)

    def retranslateUi(self, PdScConversionFrame):
        PdScConversionFrame.setWindowTitle(QtGui.QApplication.translate("PdScConversionFrame", "Frame", None, QtGui.QApplication.UnicodeUTF8))
        self.powder_gb.setTitle(QtGui.QApplication.translate("PdScConversionFrame", "Powder Conversion", None, QtGui.QApplication.UnicodeUTF8))
        self.q_binning_gb.setTitle(QtGui.QApplication.translate("PdScConversionFrame", "Momentum Transfer Range", None, QtGui.QApplication.UnicodeUTF8))
        self.q_low_label.setText(QtGui.QApplication.translate("PdScConversionFrame", "Low", None, QtGui.QApplication.UnicodeUTF8))
        self.q_width_label.setText(QtGui.QApplication.translate("PdScConversionFrame", "Width", None, QtGui.QApplication.UnicodeUTF8))
        self.q_high_label.setText(QtGui.QApplication.translate("PdScConversionFrame", "High", None, QtGui.QApplication.UnicodeUTF8))
        self.q_units_label.setText(QtGui.QApplication.translate("PdScConversionFrame", "Angstroms<sup>-1</sup>", None, QtGui.QApplication.UnicodeUTF8))
        self.save_procnexus_cb.setText(QtGui.QApplication.translate("PdScConversionFrame", "Save Processed NeXus", None, QtGui.QApplication.UnicodeUTF8))
        self.save_procnexus_save.setText(QtGui.QApplication.translate("PdScConversionFrame", "Save", None, QtGui.QApplication.UnicodeUTF8))
        self.single_crystal_gb.setTitle(QtGui.QApplication.translate("PdScConversionFrame", "Single Crystal Conversion", None, QtGui.QApplication.UnicodeUTF8))
        self.save_nxspe_gb.setTitle(QtGui.QApplication.translate("PdScConversionFrame", "Save NXSPE", None, QtGui.QApplication.UnicodeUTF8))
        self.save_nxspe_label.setText(QtGui.QApplication.translate("PdScConversionFrame", "Filename:", None, QtGui.QApplication.UnicodeUTF8))
        self.save_nxspe_save.setText(QtGui.QApplication.translate("PdScConversionFrame", "Save", None, QtGui.QApplication.UnicodeUTF8))
