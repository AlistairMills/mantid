# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui/sans/hfir_background.ui'
#
# Created: Wed Nov 16 13:57:35 2011
#      by: PyQt4 UI code generator 4.7.4
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_Frame(object):
    def setupUi(self, Frame):
        Frame.setObjectName("Frame")
        Frame.setEnabled(True)
        Frame.resize(961, 721)
        Frame.setFrameShape(QtGui.QFrame.NoFrame)
        Frame.setFrameShadow(QtGui.QFrame.Raised)
        self.verticalLayout = QtGui.QVBoxLayout(Frame)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setObjectName("verticalLayout")
        self.scrollArea = QtGui.QScrollArea(Frame)
        self.scrollArea.setFrameShape(QtGui.QFrame.NoFrame)
        self.scrollArea.setWidgetResizable(True)
        self.scrollArea.setObjectName("scrollArea")
        self.scrollAreaWidgetContents = QtGui.QWidget(self.scrollArea)
        self.scrollAreaWidgetContents.setGeometry(QtCore.QRect(0, 0, 961, 721))
        self.scrollAreaWidgetContents.setObjectName("scrollAreaWidgetContents")
        self.verticalLayout_2 = QtGui.QVBoxLayout(self.scrollAreaWidgetContents)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.horizontalLayout_4 = QtGui.QHBoxLayout()
        self.horizontalLayout_4.setObjectName("horizontalLayout_4")
        self.background_chk = QtGui.QCheckBox(self.scrollAreaWidgetContents)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.background_chk.sizePolicy().hasHeightForWidth())
        self.background_chk.setSizePolicy(sizePolicy)
        self.background_chk.setMinimumSize(QtCore.QSize(175, 0))
        self.background_chk.setMaximumSize(QtCore.QSize(175, 16777215))
        self.background_chk.setObjectName("background_chk")
        self.horizontalLayout_4.addWidget(self.background_chk)
        spacerItem = QtGui.QSpacerItem(78, 20, QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_4.addItem(spacerItem)
        self.background_edit = QtGui.QLineEdit(self.scrollAreaWidgetContents)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.background_edit.sizePolicy().hasHeightForWidth())
        self.background_edit.setSizePolicy(sizePolicy)
        self.background_edit.setMinimumSize(QtCore.QSize(300, 0))
        self.background_edit.setMaximumSize(QtCore.QSize(16777215, 16777215))
        self.background_edit.setObjectName("background_edit")
        self.horizontalLayout_4.addWidget(self.background_edit)
        self.background_browse = QtGui.QPushButton(self.scrollAreaWidgetContents)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.background_browse.sizePolicy().hasHeightForWidth())
        self.background_browse.setSizePolicy(sizePolicy)
        self.background_browse.setMinimumSize(QtCore.QSize(0, 0))
        self.background_browse.setMaximumSize(QtCore.QSize(16777215, 16777215))
        self.background_browse.setObjectName("background_browse")
        self.horizontalLayout_4.addWidget(self.background_browse)
        self.background_plot_button = QtGui.QPushButton(self.scrollAreaWidgetContents)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.background_plot_button.sizePolicy().hasHeightForWidth())
        self.background_plot_button.setSizePolicy(sizePolicy)
        self.background_plot_button.setMinimumSize(QtCore.QSize(0, 0))
        self.background_plot_button.setMaximumSize(QtCore.QSize(16777215, 16777215))
        self.background_plot_button.setObjectName("background_plot_button")
        self.horizontalLayout_4.addWidget(self.background_plot_button)
        spacerItem1 = QtGui.QSpacerItem(49, 20, QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_4.addItem(spacerItem1)
        self.verticalLayout_2.addLayout(self.horizontalLayout_4)
        spacerItem2 = QtGui.QSpacerItem(20, 10, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Fixed)
        self.verticalLayout_2.addItem(spacerItem2)
        self.geometry_options_groupbox = QtGui.QGroupBox(self.scrollAreaWidgetContents)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.geometry_options_groupbox.sizePolicy().hasHeightForWidth())
        self.geometry_options_groupbox.setSizePolicy(sizePolicy)
        self.geometry_options_groupbox.setMinimumSize(QtCore.QSize(0, 0))
        self.geometry_options_groupbox.setMaximumSize(QtCore.QSize(16777215, 16777215))
        self.geometry_options_groupbox.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignTop)
        self.geometry_options_groupbox.setObjectName("geometry_options_groupbox")
        self.verticalLayout_4 = QtGui.QVBoxLayout(self.geometry_options_groupbox)
        self.verticalLayout_4.setObjectName("verticalLayout_4")
        self.formLayout_3 = QtGui.QFormLayout()
        self.formLayout_3.setSizeConstraint(QtGui.QLayout.SetDefaultConstraint)
        self.formLayout_3.setFieldGrowthPolicy(QtGui.QFormLayout.AllNonFixedFieldsGrow)
        self.formLayout_3.setLabelAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.formLayout_3.setFormAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignTop)
        self.formLayout_3.setHorizontalSpacing(0)
        self.formLayout_3.setObjectName("formLayout_3")
        self.label_4 = QtGui.QLabel(self.geometry_options_groupbox)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_4.sizePolicy().hasHeightForWidth())
        self.label_4.setSizePolicy(sizePolicy)
        self.label_4.setMinimumSize(QtCore.QSize(249, 0))
        self.label_4.setMaximumSize(QtCore.QSize(249, 16777215))
        self.label_4.setObjectName("label_4")
        self.formLayout_3.setWidget(0, QtGui.QFormLayout.LabelRole, self.label_4)
        self.sample_dist_edit = QtGui.QLineEdit(self.geometry_options_groupbox)
        self.sample_dist_edit.setEnabled(False)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.sample_dist_edit.sizePolicy().hasHeightForWidth())
        self.sample_dist_edit.setSizePolicy(sizePolicy)
        self.sample_dist_edit.setMinimumSize(QtCore.QSize(80, 0))
        self.sample_dist_edit.setMaximumSize(QtCore.QSize(80, 16777215))
        self.sample_dist_edit.setObjectName("sample_dist_edit")
        self.formLayout_3.setWidget(0, QtGui.QFormLayout.FieldRole, self.sample_dist_edit)
        self.label_5 = QtGui.QLabel(self.geometry_options_groupbox)
        self.label_5.setObjectName("label_5")
        self.formLayout_3.setWidget(1, QtGui.QFormLayout.LabelRole, self.label_5)
        self.wavelength_edit = QtGui.QLineEdit(self.geometry_options_groupbox)
        self.wavelength_edit.setEnabled(False)
        self.wavelength_edit.setMinimumSize(QtCore.QSize(80, 0))
        self.wavelength_edit.setMaximumSize(QtCore.QSize(80, 16777215))
        self.wavelength_edit.setObjectName("wavelength_edit")
        self.formLayout_3.setWidget(1, QtGui.QFormLayout.FieldRole, self.wavelength_edit)
        self.label_10 = QtGui.QLabel(self.geometry_options_groupbox)
        self.label_10.setIndent(0)
        self.label_10.setObjectName("label_10")
        self.formLayout_3.setWidget(2, QtGui.QFormLayout.LabelRole, self.label_10)
        self.wavelength_spread_edit = QtGui.QLineEdit(self.geometry_options_groupbox)
        self.wavelength_spread_edit.setEnabled(False)
        self.wavelength_spread_edit.setMinimumSize(QtCore.QSize(80, 0))
        self.wavelength_spread_edit.setMaximumSize(QtCore.QSize(80, 16777215))
        self.wavelength_spread_edit.setObjectName("wavelength_spread_edit")
        self.formLayout_3.setWidget(2, QtGui.QFormLayout.FieldRole, self.wavelength_spread_edit)
        self.verticalLayout_4.addLayout(self.formLayout_3)
        self.verticalLayout_2.addWidget(self.geometry_options_groupbox)
        spacerItem3 = QtGui.QSpacerItem(20, 10, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Fixed)
        self.verticalLayout_2.addItem(spacerItem3)
        self.transmission_grpbox = QtGui.QGroupBox(self.scrollAreaWidgetContents)
        self.transmission_grpbox.setFlat(False)
        self.transmission_grpbox.setCheckable(False)
        self.transmission_grpbox.setObjectName("transmission_grpbox")
        self.verticalLayout_3 = QtGui.QVBoxLayout(self.transmission_grpbox)
        self.verticalLayout_3.setObjectName("verticalLayout_3")
        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.bck_trans_label = QtGui.QLabel(self.transmission_grpbox)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.bck_trans_label.sizePolicy().hasHeightForWidth())
        self.bck_trans_label.setSizePolicy(sizePolicy)
        self.bck_trans_label.setMinimumSize(QtCore.QSize(180, 0))
        self.bck_trans_label.setMaximumSize(QtCore.QSize(180, 16777215))
        self.bck_trans_label.setObjectName("bck_trans_label")
        self.horizontalLayout.addWidget(self.bck_trans_label)
        spacerItem4 = QtGui.QSpacerItem(63, 20, QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem4)
        self.transmission_edit = QtGui.QLineEdit(self.transmission_grpbox)
        self.transmission_edit.setEnabled(False)
        self.transmission_edit.setMinimumSize(QtCore.QSize(80, 0))
        self.transmission_edit.setMaximumSize(QtCore.QSize(80, 30))
        self.transmission_edit.setObjectName("transmission_edit")
        self.horizontalLayout.addWidget(self.transmission_edit)
        self.bck_trans_err_label = QtGui.QLabel(self.transmission_grpbox)
        self.bck_trans_err_label.setMaximumSize(QtCore.QSize(16777215, 30))
        self.bck_trans_err_label.setObjectName("bck_trans_err_label")
        self.horizontalLayout.addWidget(self.bck_trans_err_label)
        self.dtransmission_edit = QtGui.QLineEdit(self.transmission_grpbox)
        self.dtransmission_edit.setEnabled(True)
        self.dtransmission_edit.setMinimumSize(QtCore.QSize(80, 0))
        self.dtransmission_edit.setMaximumSize(QtCore.QSize(80, 30))
        self.dtransmission_edit.setObjectName("dtransmission_edit")
        self.horizontalLayout.addWidget(self.dtransmission_edit)
        spacerItem5 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem5)
        self.verticalLayout_3.addLayout(self.horizontalLayout)
        self.theta_dep_chk = QtGui.QCheckBox(self.transmission_grpbox)
        self.theta_dep_chk.setObjectName("theta_dep_chk")
        self.verticalLayout_3.addWidget(self.theta_dep_chk)
        self.calculate_trans_chk = QtGui.QCheckBox(self.transmission_grpbox)
        self.calculate_trans_chk.setObjectName("calculate_trans_chk")
        self.verticalLayout_3.addWidget(self.calculate_trans_chk)
        self.horizontalLayout_3 = QtGui.QHBoxLayout()
        self.horizontalLayout_3.setObjectName("horizontalLayout_3")
        spacerItem6 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_3.addItem(spacerItem6)
        self.trans_dark_current_label = QtGui.QLabel(self.transmission_grpbox)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.trans_dark_current_label.sizePolicy().hasHeightForWidth())
        self.trans_dark_current_label.setSizePolicy(sizePolicy)
        self.trans_dark_current_label.setMinimumSize(QtCore.QSize(203, 27))
        self.trans_dark_current_label.setMaximumSize(QtCore.QSize(203, 27))
        self.trans_dark_current_label.setObjectName("trans_dark_current_label")
        self.horizontalLayout_3.addWidget(self.trans_dark_current_label)
        self.trans_dark_current_edit = QtGui.QLineEdit(self.transmission_grpbox)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.trans_dark_current_edit.sizePolicy().hasHeightForWidth())
        self.trans_dark_current_edit.setSizePolicy(sizePolicy)
        self.trans_dark_current_edit.setMinimumSize(QtCore.QSize(300, 0))
        self.trans_dark_current_edit.setMaximumSize(QtCore.QSize(16777215, 16777215))
        self.trans_dark_current_edit.setObjectName("trans_dark_current_edit")
        self.horizontalLayout_3.addWidget(self.trans_dark_current_edit)
        self.trans_dark_current_button = QtGui.QPushButton(self.transmission_grpbox)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.trans_dark_current_button.sizePolicy().hasHeightForWidth())
        self.trans_dark_current_button.setSizePolicy(sizePolicy)
        self.trans_dark_current_button.setMinimumSize(QtCore.QSize(0, 0))
        self.trans_dark_current_button.setMaximumSize(QtCore.QSize(16777215, 16777215))
        self.trans_dark_current_button.setObjectName("trans_dark_current_button")
        self.horizontalLayout_3.addWidget(self.trans_dark_current_button)
        self.trans_dark_current_plot_button = QtGui.QPushButton(self.transmission_grpbox)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.trans_dark_current_plot_button.sizePolicy().hasHeightForWidth())
        self.trans_dark_current_plot_button.setSizePolicy(sizePolicy)
        self.trans_dark_current_plot_button.setMinimumSize(QtCore.QSize(0, 0))
        self.trans_dark_current_plot_button.setMaximumSize(QtCore.QSize(16777215, 16777215))
        self.trans_dark_current_plot_button.setObjectName("trans_dark_current_plot_button")
        self.horizontalLayout_3.addWidget(self.trans_dark_current_plot_button)
        spacerItem7 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_3.addItem(spacerItem7)
        self.verticalLayout_3.addLayout(self.horizontalLayout_3)
        self.horizontalLayout_2 = QtGui.QHBoxLayout()
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        spacerItem8 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_2.addItem(spacerItem8)
        self.trans_direct_chk = QtGui.QRadioButton(self.transmission_grpbox)
        self.trans_direct_chk.setObjectName("trans_direct_chk")
        self.horizontalLayout_2.addWidget(self.trans_direct_chk)
        self.trans_spreader_chk = QtGui.QRadioButton(self.transmission_grpbox)
        self.trans_spreader_chk.setObjectName("trans_spreader_chk")
        self.horizontalLayout_2.addWidget(self.trans_spreader_chk)
        spacerItem9 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_2.addItem(spacerItem9)
        self.verticalLayout_3.addLayout(self.horizontalLayout_2)
        self.widget_placeholder = QtGui.QVBoxLayout()
        self.widget_placeholder.setObjectName("widget_placeholder")
        self.verticalLayout_3.addLayout(self.widget_placeholder)
        self.verticalLayout_2.addWidget(self.transmission_grpbox)
        spacerItem10 = QtGui.QSpacerItem(20, 40, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.verticalLayout_2.addItem(spacerItem10)
        self.scrollArea.setWidget(self.scrollAreaWidgetContents)
        self.verticalLayout.addWidget(self.scrollArea)

        self.retranslateUi(Frame)
        QtCore.QMetaObject.connectSlotsByName(Frame)

    def retranslateUi(self, Frame):
        Frame.setWindowTitle(QtGui.QApplication.translate("Frame", "Frame", None, QtGui.QApplication.UnicodeUTF8))
        self.background_chk.setToolTip(QtGui.QApplication.translate("Frame", "Select to apply a background subtraction.", None, QtGui.QApplication.UnicodeUTF8))
        self.background_chk.setText(QtGui.QApplication.translate("Frame", "Background data file:", None, QtGui.QApplication.UnicodeUTF8))
        self.background_edit.setToolTip(QtGui.QApplication.translate("Frame", "Enter a valid data file path.", None, QtGui.QApplication.UnicodeUTF8))
        self.background_browse.setText(QtGui.QApplication.translate("Frame", "Browse", None, QtGui.QApplication.UnicodeUTF8))
        self.background_plot_button.setToolTip(QtGui.QApplication.translate("Frame", "Click to plot 2D data.", None, QtGui.QApplication.UnicodeUTF8))
        self.background_plot_button.setText(QtGui.QApplication.translate("Frame", "Plot", None, QtGui.QApplication.UnicodeUTF8))
        self.geometry_options_groupbox.setTitle(QtGui.QApplication.translate("Frame", "Experiment Parameters from Data File (for information only)", None, QtGui.QApplication.UnicodeUTF8))
        self.label_4.setText(QtGui.QApplication.translate("Frame", "Sample-dectector distance [mm]", None, QtGui.QApplication.UnicodeUTF8))
        self.sample_dist_edit.setToolTip(QtGui.QApplication.translate("Frame", "Sample-to-detector distance read from the data file, in mm.", None, QtGui.QApplication.UnicodeUTF8))
        self.label_5.setText(QtGui.QApplication.translate("Frame", "Wavelength [Angstrom]", None, QtGui.QApplication.UnicodeUTF8))
        self.wavelength_edit.setToolTip(QtGui.QApplication.translate("Frame", "Neutron wavelength read from the data file.", None, QtGui.QApplication.UnicodeUTF8))
        self.label_10.setText(QtGui.QApplication.translate("Frame", "Wavelength spread [Angstrom]", None, QtGui.QApplication.UnicodeUTF8))
        self.wavelength_spread_edit.setToolTip(QtGui.QApplication.translate("Frame", "Neutron wavelength spread read from the data file.", None, QtGui.QApplication.UnicodeUTF8))
        self.transmission_grpbox.setTitle(QtGui.QApplication.translate("Frame", "Transmission", None, QtGui.QApplication.UnicodeUTF8))
        self.bck_trans_label.setText(QtGui.QApplication.translate("Frame", "Background transmission:", None, QtGui.QApplication.UnicodeUTF8))
        self.transmission_edit.setToolTip(QtGui.QApplication.translate("Frame", "Transmission value for the background in %.", None, QtGui.QApplication.UnicodeUTF8))
        self.bck_trans_err_label.setText(QtGui.QApplication.translate("Frame", "+/-", None, QtGui.QApplication.UnicodeUTF8))
        self.dtransmission_edit.setToolTip(QtGui.QApplication.translate("Frame", "Uncertainty on the background transmission.", None, QtGui.QApplication.UnicodeUTF8))
        self.theta_dep_chk.setToolTip(QtGui.QApplication.translate("Frame", "Select to apply a theta-dependent transmission correction.", None, QtGui.QApplication.UnicodeUTF8))
        self.theta_dep_chk.setText(QtGui.QApplication.translate("Frame", "Theta-dependent correction", None, QtGui.QApplication.UnicodeUTF8))
        self.calculate_trans_chk.setToolTip(QtGui.QApplication.translate("Frame", "Select to let the reduction software calculate the background transmission.", None, QtGui.QApplication.UnicodeUTF8))
        self.calculate_trans_chk.setText(QtGui.QApplication.translate("Frame", "Calculate background transmission", None, QtGui.QApplication.UnicodeUTF8))
        self.trans_dark_current_label.setText(QtGui.QApplication.translate("Frame", "Dark current for transmission:", None, QtGui.QApplication.UnicodeUTF8))
        self.trans_dark_current_edit.setToolTip(QtGui.QApplication.translate("Frame", "Enter a valid file path to be used for the dark current data.", None, QtGui.QApplication.UnicodeUTF8))
        self.trans_dark_current_button.setText(QtGui.QApplication.translate("Frame", "Browse", None, QtGui.QApplication.UnicodeUTF8))
        self.trans_dark_current_plot_button.setToolTip(QtGui.QApplication.translate("Frame", "Click to plot 2D data.", None, QtGui.QApplication.UnicodeUTF8))
        self.trans_dark_current_plot_button.setText(QtGui.QApplication.translate("Frame", "Plot", None, QtGui.QApplication.UnicodeUTF8))
        self.trans_direct_chk.setToolTip(QtGui.QApplication.translate("Frame", "Select to use the direct beam method for transmission calculation.", None, QtGui.QApplication.UnicodeUTF8))
        self.trans_direct_chk.setText(QtGui.QApplication.translate("Frame", "Direct beam", None, QtGui.QApplication.UnicodeUTF8))
        self.trans_spreader_chk.setToolTip(QtGui.QApplication.translate("Frame", "Select to use the beam spreader (glassy carbon) method for transmission calculation.", None, QtGui.QApplication.UnicodeUTF8))
        self.trans_spreader_chk.setText(QtGui.QApplication.translate("Frame", "Beam spreader", None, QtGui.QApplication.UnicodeUTF8))

