# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'MulReplay.ui'
#
# Created by: PyQt5 UI code generator 5.15.1
#
# WARNING: Any manual changes made to this file will be lost when pyuic5 is
# run again.  Do not edit this file unless you know what you are doing.


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_MReplay_model(object):
    def setupUi(self, MReplay_model):
        MReplay_model.setObjectName("MReplay_model")
        MReplay_model.resize(1088, 526)
        self.tableShowMsg = QtWidgets.QTableWidget(MReplay_model)
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
        self.button_stoplisten = QtWidgets.QPushButton(MReplay_model)
        self.button_stoplisten.setGeometry(QtCore.QRect(20, 190, 111, 41))
        self.button_stoplisten.setObjectName("button_stoplisten")
        self.PB_load = QtWidgets.QPushButton(MReplay_model)
        self.PB_load.setGeometry(QtCore.QRect(20, 50, 111, 41))
        self.PB_load.setObjectName("PB_load")
        self.buttom_listen = QtWidgets.QPushButton(MReplay_model)
        self.buttom_listen.setGeometry(QtCore.QRect(20, 120, 111, 41))
        self.buttom_listen.setObjectName("buttom_listen")
        self.lineEdit_times = QtWidgets.QLineEdit(MReplay_model)
        self.lineEdit_times.setGeometry(QtCore.QRect(830, 127, 51, 25))
        self.lineEdit_times.setObjectName("lineEdit_times")
        self.label_number_2 = QtWidgets.QLabel(MReplay_model)
        self.label_number_2.setGeometry(QtCore.QRect(760, 130, 67, 17))
        self.label_number_2.setObjectName("label_number_2")
        self.label_number = QtWidgets.QLabel(MReplay_model)
        self.label_number.setGeometry(QtCore.QRect(760, 60, 67, 17))
        self.label_number.setObjectName("label_number")
        self.lineEdit_number = QtWidgets.QLineEdit(MReplay_model)
        self.lineEdit_number.setGeometry(QtCore.QRect(830, 58, 51, 25))
        self.lineEdit_number.setObjectName("lineEdit_number")
        self.label_6 = QtWidgets.QLabel(MReplay_model)
        self.label_6.setGeometry(QtCore.QRect(760, 203, 67, 17))
        self.label_6.setObjectName("label_6")
        self.lineEdit_interval2 = QtWidgets.QLineEdit(MReplay_model)
        self.lineEdit_interval2.setGeometry(QtCore.QRect(830, 200, 51, 25))
        self.lineEdit_interval2.setObjectName("lineEdit_interval2")
        self.button_clear = QtWidgets.QPushButton(MReplay_model)
        self.button_clear.setGeometry(QtCore.QRect(930, 70, 131, 51))
        self.button_clear.setObjectName("button_clear")
        self.button_attack = QtWidgets.QPushButton(MReplay_model)
        self.button_attack.setGeometry(QtCore.QRect(930, 170, 131, 51))
        self.button_attack.setObjectName("button_attack")
        self.label_FIndex = QtWidgets.QLabel(MReplay_model)
        self.label_FIndex.setGeometry(QtCore.QRect(180, 70, 81, 20))
        self.label_FIndex.setObjectName("label_FIndex")
        self.label_RIndex = QtWidgets.QLabel(MReplay_model)
        self.label_RIndex.setGeometry(QtCore.QRect(180, 120, 81, 20))
        self.label_RIndex.setObjectName("label_RIndex")
        self.PB_select = QtWidgets.QPushButton(MReplay_model)
        self.PB_select.setGeometry(QtCore.QRect(210, 180, 91, 41))
        self.PB_select.setObjectName("PB_select")
        self.lineEdit_FIndex = QtWidgets.QLineEdit(MReplay_model)
        self.lineEdit_FIndex.setGeometry(QtCore.QRect(270, 70, 113, 25))
        self.lineEdit_FIndex.setObjectName("lineEdit_FIndex")
        self.lineEdit_RIndex = QtWidgets.QLineEdit(MReplay_model)
        self.lineEdit_RIndex.setGeometry(QtCore.QRect(270, 120, 113, 25))
        self.lineEdit_RIndex.setObjectName("lineEdit_RIndex")
        self.tableSelectedMsg = QtWidgets.QTableWidget(MReplay_model)
        self.tableSelectedMsg.setGeometry(QtCore.QRect(420, 40, 300, 192))
        self.tableSelectedMsg.setObjectName("tableSelectedMsg")
        self.tableSelectedMsg.setColumnCount(2)
        self.tableSelectedMsg.setRowCount(0)
        item = QtWidgets.QTableWidgetItem()
        item.setTextAlignment(QtCore.Qt.AlignCenter)
        self.tableSelectedMsg.setHorizontalHeaderItem(0, item)
        item = QtWidgets.QTableWidgetItem()
        item.setTextAlignment(QtCore.Qt.AlignCenter)
        self.tableSelectedMsg.setHorizontalHeaderItem(1, item)
        self.label_AttackName = QtWidgets.QLabel(MReplay_model)
        self.label_AttackName.setGeometry(QtCore.QRect(500, 0, 111, 21))
        font = QtGui.QFont()
        font.setPointSize(15)
        font.setBold(True)
        font.setItalic(True)
        font.setUnderline(True)
        font.setWeight(75)
        font.setStrikeOut(False)
        self.label_AttackName.setFont(font)
        self.label_AttackName.setObjectName("label_AttackName")

        self.retranslateUi(MReplay_model)
        QtCore.QMetaObject.connectSlotsByName(MReplay_model)

    def retranslateUi(self, MReplay_model):
        _translate = QtCore.QCoreApplication.translate
        MReplay_model.setWindowTitle(_translate("MReplay_model", "Form"))
        item = self.tableShowMsg.horizontalHeaderItem(0)
        item.setText(_translate("MReplay_model", "Index"))
        item = self.tableShowMsg.horizontalHeaderItem(1)
        item.setText(_translate("MReplay_model", "Time stamp"))
        item = self.tableShowMsg.horizontalHeaderItem(2)
        item.setText(_translate("MReplay_model", "Direction"))
        item = self.tableShowMsg.horizontalHeaderItem(3)
        item.setText(_translate("MReplay_model", "Type"))
        item = self.tableShowMsg.horizontalHeaderItem(4)
        item.setText(_translate("MReplay_model", "Frame ID"))
        item = self.tableShowMsg.horizontalHeaderItem(5)
        item.setText(_translate("MReplay_model", "DLC"))
        item = self.tableShowMsg.horizontalHeaderItem(6)
        item.setText(_translate("MReplay_model", "Data"))
        self.button_stoplisten.setText(_translate("MReplay_model", "Stop Listen"))
        self.PB_load.setText(_translate("MReplay_model", "Load Data"))
        self.buttom_listen.setText(_translate("MReplay_model", "Recollection"))
        self.lineEdit_times.setText(_translate("MReplay_model", "1"))
        self.label_number_2.setText(_translate("MReplay_model", "Times:"))
        self.label_number.setText(_translate("MReplay_model", "Number:"))
        self.lineEdit_number.setText(_translate("MReplay_model", "100"))
        self.label_6.setText(_translate("MReplay_model", "Interval2:"))
        self.lineEdit_interval2.setText(_translate("MReplay_model", "1"))
        self.button_clear.setText(_translate("MReplay_model", "Clear"))
        self.button_attack.setText(_translate("MReplay_model", "Attack"))
        self.label_FIndex.setText(_translate("MReplay_model", "Front Index:"))
        self.label_RIndex.setText(_translate("MReplay_model", "Rear Index:"))
        self.PB_select.setText(_translate("MReplay_model", "Select"))
        item = self.tableSelectedMsg.horizontalHeaderItem(0)
        item.setText(_translate("MReplay_model", "Index"))
        item = self.tableSelectedMsg.horizontalHeaderItem(1)
        item.setText(_translate("MReplay_model", "TimeStamp"))
        self.label_AttackName.setText(_translate("MReplay_model", "TextLabel"))
