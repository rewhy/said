# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ValidFuzzy.ui'
#
# Created by: PyQt5 UI code generator 5.15.1
#
# WARNING: Any manual changes made to this file will be lost when pyuic5 is
# run again.  Do not edit this file unless you know what you are doing.


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_ValidFuzzy(object):
    def setupUi(self, ValidFuzzy):
        ValidFuzzy.setObjectName("ValidFuzzy")
        ValidFuzzy.resize(1100, 528)
        self.tableShowMsg = QtWidgets.QTableWidget(ValidFuzzy)
        self.tableShowMsg.setGeometry(QtCore.QRect(0, 250, 1100, 281))
        self.tableShowMsg.setObjectName("tableShowMsg")
        self.tableShowMsg.setColumnCount(7)
        self.tableShowMsg.setRowCount(0)
        item = QtWidgets.QTableWidgetItem()
        item.setTextAlignment(QtCore.Qt.AlignCenter)
        self.tableShowMsg.setHorizontalHeaderItem(0, item)
        item = QtWidgets.QTableWidgetItem()
        item.setTextAlignment(QtCore.Qt.AlignCenter)
        self.tableShowMsg.setHorizontalHeaderItem(1, item)
        item = QtWidgets.QTableWidgetItem()
        item.setTextAlignment(QtCore.Qt.AlignCenter)
        self.tableShowMsg.setHorizontalHeaderItem(2, item)
        item = QtWidgets.QTableWidgetItem()
        item.setTextAlignment(QtCore.Qt.AlignCenter)
        self.tableShowMsg.setHorizontalHeaderItem(3, item)
        item = QtWidgets.QTableWidgetItem()
        item.setTextAlignment(QtCore.Qt.AlignCenter)
        self.tableShowMsg.setHorizontalHeaderItem(4, item)
        item = QtWidgets.QTableWidgetItem()
        item.setTextAlignment(QtCore.Qt.AlignCenter)
        self.tableShowMsg.setHorizontalHeaderItem(5, item)
        item = QtWidgets.QTableWidgetItem()
        item.setTextAlignment(QtCore.Qt.AlignCenter)
        font = QtGui.QFont()
        font.setPointSize(11)
        item.setFont(font)
        self.tableShowMsg.setHorizontalHeaderItem(6, item)
        self.lineEdit_times = QtWidgets.QLineEdit(ValidFuzzy)
        self.lineEdit_times.setGeometry(QtCore.QRect(720, 150, 51, 25))
        self.lineEdit_times.setObjectName("lineEdit_times")
        self.label_number_2 = QtWidgets.QLabel(ValidFuzzy)
        self.label_number_2.setGeometry(QtCore.QRect(650, 152, 67, 17))
        self.label_number_2.setObjectName("label_number_2")
        self.label_5 = QtWidgets.QLabel(ValidFuzzy)
        self.label_5.setGeometry(QtCore.QRect(650, 107, 67, 17))
        self.label_5.setObjectName("label_5")
        self.label_ms = QtWidgets.QLabel(ValidFuzzy)
        self.label_ms.setGeometry(QtCore.QRect(780, 107, 21, 17))
        self.label_ms.setObjectName("label_ms")
        self.lineEdit_interval = QtWidgets.QLineEdit(ValidFuzzy)
        self.lineEdit_interval.setGeometry(QtCore.QRect(720, 104, 51, 25))
        self.lineEdit_interval.setObjectName("lineEdit_interval")
        self.label_number = QtWidgets.QLabel(ValidFuzzy)
        self.label_number.setGeometry(QtCore.QRect(650, 57, 67, 17))
        self.label_number.setObjectName("label_number")
        self.lineEdit_number = QtWidgets.QLineEdit(ValidFuzzy)
        self.lineEdit_number.setGeometry(QtCore.QRect(720, 55, 51, 25))
        self.lineEdit_number.setObjectName("lineEdit_number")
        self.label_6 = QtWidgets.QLabel(ValidFuzzy)
        self.label_6.setGeometry(QtCore.QRect(650, 200, 67, 17))
        self.label_6.setObjectName("label_6")
        self.lineEdit_interval2 = QtWidgets.QLineEdit(ValidFuzzy)
        self.lineEdit_interval2.setGeometry(QtCore.QRect(720, 197, 51, 25))
        self.lineEdit_interval2.setObjectName("lineEdit_interval2")
        self.label_ms_2 = QtWidgets.QLabel(ValidFuzzy)
        self.label_ms_2.setGeometry(QtCore.QRect(780, 200, 16, 17))
        self.label_ms_2.setObjectName("label_ms_2")
        self.button_attack = QtWidgets.QPushButton(ValidFuzzy)
        self.button_attack.setGeometry(QtCore.QRect(890, 160, 131, 51))
        self.button_attack.setObjectName("button_attack")
        self.button_clear = QtWidgets.QPushButton(ValidFuzzy)
        self.button_clear.setGeometry(QtCore.QRect(890, 60, 131, 51))
        self.button_clear.setObjectName("button_clear")
        self.tableAllID = QtWidgets.QTableWidget(ValidFuzzy)
        self.tableAllID.setGeometry(QtCore.QRect(430, 40, 155, 192))
        self.tableAllID.setObjectName("tableAllID")
        self.tableAllID.setColumnCount(2)
        self.tableAllID.setRowCount(0)
        item = QtWidgets.QTableWidgetItem()
        item.setTextAlignment(QtCore.Qt.AlignCenter)
        self.tableAllID.setHorizontalHeaderItem(0, item)
        item = QtWidgets.QTableWidgetItem()
        item.setTextAlignment(QtCore.Qt.AlignCenter)
        self.tableAllID.setHorizontalHeaderItem(1, item)
        self.PB_selectAll = QtWidgets.QPushButton(ValidFuzzy)
        self.PB_selectAll.setGeometry(QtCore.QRect(220, 110, 121, 41))
        self.PB_selectAll.setObjectName("PB_selectAll")
        self.button_stoplisten = QtWidgets.QPushButton(ValidFuzzy)
        self.button_stoplisten.setGeometry(QtCore.QRect(20, 180, 111, 41))
        self.button_stoplisten.setObjectName("button_stoplisten")
        self.PB_load = QtWidgets.QPushButton(ValidFuzzy)
        self.PB_load.setGeometry(QtCore.QRect(20, 40, 111, 41))
        self.PB_load.setObjectName("PB_load")
        self.buttom_listen = QtWidgets.QPushButton(ValidFuzzy)
        self.buttom_listen.setGeometry(QtCore.QRect(20, 110, 111, 41))
        self.buttom_listen.setObjectName("buttom_listen")
        self.PB_validID = QtWidgets.QPushButton(ValidFuzzy)
        self.PB_validID.setGeometry(QtCore.QRect(220, 40, 121, 41))
        self.PB_validID.setObjectName("PB_validID")
        self.PB_Deselect = QtWidgets.QPushButton(ValidFuzzy)
        self.PB_Deselect.setGeometry(QtCore.QRect(220, 180, 121, 41))
        self.PB_Deselect.setObjectName("PB_Deselect")
        self.label_AttackName = QtWidgets.QLabel(ValidFuzzy)
        self.label_AttackName.setGeometry(QtCore.QRect(480, 0, 111, 21))
        font = QtGui.QFont()
        font.setPointSize(15)
        font.setBold(True)
        font.setItalic(True)
        font.setUnderline(True)
        font.setWeight(75)
        font.setStrikeOut(False)
        self.label_AttackName.setFont(font)
        self.label_AttackName.setObjectName("label_AttackName")

        self.retranslateUi(ValidFuzzy)
        QtCore.QMetaObject.connectSlotsByName(ValidFuzzy)

    def retranslateUi(self, ValidFuzzy):
        _translate = QtCore.QCoreApplication.translate
        ValidFuzzy.setWindowTitle(_translate("ValidFuzzy", "Form"))
        item = self.tableShowMsg.horizontalHeaderItem(0)
        item.setText(_translate("ValidFuzzy", "Index"))
        item = self.tableShowMsg.horizontalHeaderItem(1)
        item.setText(_translate("ValidFuzzy", "Time stamp"))
        item = self.tableShowMsg.horizontalHeaderItem(2)
        item.setText(_translate("ValidFuzzy", "Direction"))
        item = self.tableShowMsg.horizontalHeaderItem(3)
        item.setText(_translate("ValidFuzzy", "Type"))
        item = self.tableShowMsg.horizontalHeaderItem(4)
        item.setText(_translate("ValidFuzzy", "Frame ID"))
        item = self.tableShowMsg.horizontalHeaderItem(5)
        item.setText(_translate("ValidFuzzy", "DLC"))
        item = self.tableShowMsg.horizontalHeaderItem(6)
        item.setText(_translate("ValidFuzzy", "Data"))
        self.lineEdit_times.setText(_translate("ValidFuzzy", "1"))
        self.label_number_2.setText(_translate("ValidFuzzy", "Times:"))
        self.label_5.setText(_translate("ValidFuzzy", "Interval1:"))
        self.label_ms.setText(_translate("ValidFuzzy", "ms"))
        self.lineEdit_interval.setText(_translate("ValidFuzzy", "10"))
        self.label_number.setText(_translate("ValidFuzzy", "Number:"))
        self.lineEdit_number.setText(_translate("ValidFuzzy", "100"))
        self.label_6.setText(_translate("ValidFuzzy", "Interval2:"))
        self.lineEdit_interval2.setText(_translate("ValidFuzzy", "1"))
        self.label_ms_2.setText(_translate("ValidFuzzy", "s"))
        self.button_attack.setText(_translate("ValidFuzzy", "Attack"))
        self.button_clear.setText(_translate("ValidFuzzy", "Clear"))
        item = self.tableAllID.horizontalHeaderItem(0)
        item.setText(_translate("ValidFuzzy", "Index"))
        item = self.tableAllID.horizontalHeaderItem(1)
        item.setText(_translate("ValidFuzzy", "Identifiers"))
        self.PB_selectAll.setText(_translate("ValidFuzzy", "Select all"))
        self.button_stoplisten.setText(_translate("ValidFuzzy", "Stop Listen"))
        self.PB_load.setText(_translate("ValidFuzzy", "Load Data"))
        self.buttom_listen.setText(_translate("ValidFuzzy", "Recollection"))
        self.PB_validID.setText(_translate("ValidFuzzy", "Valid IDs"))
        self.PB_Deselect.setText(_translate("ValidFuzzy", "Deselect"))
        self.label_AttackName.setText(_translate("ValidFuzzy", "TextLabel"))
