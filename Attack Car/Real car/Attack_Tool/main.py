import sys
import os
import can


from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtCore import Qt, QThread

from PyQt5.QtWidgets import QApplication, QMainWindow, QMessageBox, QCheckBox, QWidget
from PyQt5.QtWidgets import *

from connect import Ui_Setup
from MainWindow import Ui_MainWindow
from attack_model_Fabrication import Ui_Attack1
from DoS_model import Ui_DoS_model
from SingleReplay import Ui_SReplay_model
from MulReplay import Ui_MReplay_model
from RandomFuzzy import Ui_Fuzzy_randomID
from ValidFuzzy import Ui_ValidFuzzy
from Fuzzy_designed import Ui_Fuzzy_designed
from ControlModel import Ui_Control

#Tool function
from CAN_Action import *
from utils import *


class MainWindow(QMainWindow, Ui_MainWindow):
    _AttackSignal = QtCore.pyqtSignal(str, list)

    def __init__(self, parent=None):
        super(MainWindow, self).__init__(parent)
        self.setupUi(self)
        self.actionStart_S.triggered.connect(self.openConnect)
        self.actionStop_T.triggered.connect(self.closeConnect)
        self.actionLoadData_LD.triggered.connect(self.LoadFile)
        self.actionSaveData_SD.triggered.connect(self.SaveFile)
        self.actionResetData_RD.triggered.connect(self.ResetData)
        self.genWin = ConnectForm()
        self.genWin.setWindowModality(Qt.ApplicationModal)
        self.driver_name = ""
        self.bitrate = 0
        #self.LoadedData = []
        self.receivedDataList = []
        # self.index = 0
        self.menuOperation.show()

        # TODO Adjust to UI.py
        self.tableShowMsg.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self.tableShowMsg.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.tableShowMsg.verticalHeader().setVisible(False)
        self.tableShowMsg.horizontalHeader().resizeSection(0, 120)
        self.tableShowMsg.horizontalHeader().resizeSection(1, 200)
        self.tableShowMsg.horizontalHeader().resizeSection(2, 100)
        self.tableShowMsg.horizontalHeader().resizeSection(3, 110)
        self.tableShowMsg.horizontalHeader().resizeSection(4, 110)
        self.tableShowMsg.horizontalHeader().resizeSection(5, 50)
        self.tableShowMsg.horizontalHeader().resizeSection(6, 400)
        # END

        # msg = ["99999", "1234567", "send", "standard", "0x000", "1", "0x01"]
        # self.insertMsg(msg)
        # Listen
        self.buttom_listen.clicked.connect(self.listenCAN)
        self.button_stoplisten.clicked.connect(self.stoplistenCAN)
        self.button_stoplisten.setEnabled(False)
        self.button_clear.clicked.connect(self.clearTable)

        # Send
        self.button_send.clicked.connect(self.sendCAN)

        # Attack
        self.attack1 = ""
        self.attack2 = ""
        self.radioButton_IVN.toggled.connect(self.chooseAttakType)
        self.radioButton_DIA.toggled.connect(self.chooseAttakType)

        self.pushButton_create.clicked.connect(self.creatAttackUi)

        # received data
        self.receivedDataList = []
        self.genAttack1 = CreateAttack1()
        self.genAttack1.setWindowModality(Qt.ApplicationModal)
        self.genAttack2 = CreateDoS()
        self.genAttack2.setWindowModality(Qt.ApplicationModal)
        self.genAttack3 = CreateSingeReplay()
        self.genAttack3.setWindowModality(Qt.ApplicationModal)
        self.genAttack4 = CreateMultipleReplay()
        self.genAttack4.setWindowModality(Qt.ApplicationModal)
        self.genAttack5 = CreateRandomFuzzy()
        self.genAttack5.setWindowModality(Qt.ApplicationModal)
        self.genAttack6 = CreateValidFuzzy()
        self.genAttack6.setWindowModality(Qt.ApplicationModal)
        self.genAttack7 = CreateDesignedFyzzy()
        self.genAttack7.setWindowModality(Qt.ApplicationModal)
        self.genAttack8 = CreateControlModel()
        self.genAttack8.setWindowModality(Qt.ApplicationModal)
        GenerateDefaultAction()

    def openConnect(self):
        self.genWin.show()
        self.genWin._signal.connect(self.getName)

    def closeConnect(self):
        if self.driver_name == "":
            QMessageBox.critical(self, "Warning", "CAN port is not opened!", QMessageBox.Yes)
        else:
            command = "ip link set " + self.driver_name + " down"
            sudoPassword = self.password
            result = os.system(('echo %s|sudo -S %s' % (sudoPassword, command)))
            if result == 0:
                QMessageBox.information(self, "Prompt", "CAN is set down successfully")
                self.driver_name == ""
            else:
                QMessageBox.information(self, "Prompt", "cannot close device %s" % self.driver_name)

    def getName(self, parameter1, parameter2, parameter3):
        #print(parameter1, parameter2)
        self.driver_name = parameter1
        self.password = parameter2
        self.bitrate = parameter3

    #File
    def LoadFile(self):
        file_name = QFileDialog.getOpenFileName(None, "open file", ".", "Txt files(*.txt);;Csv files(*.csv);;All Files (*)")
        if file_name[0]:
            FileList = parseFile(file_name[0])
            if FileList:
                for msg in FileList:
                    self.insertMsg(msg)
                self.receivedDataList = self.receivedDataList + FileList

    def SaveFile(self):
        file_name = QFileDialog.getSaveFileName(None, 'save file', '.', "Txt files(*.txt);;Csv files(*.csv);;All Files (*)")
        if file_name[0]:
            with open(file_name[0], 'w') as fp:
                for tmpList in self.receivedDataList:
                    tmpString = ",".join(tmpList)
                    tmpString = tmpString.strip() + "\n"
                    fp.write(tmpString)

    def ResetData(self):
        self.receivedDataList.clear()

    # Table show
    def insertMsg(self, msg_noindex):
        msg = msg_noindex[:]
        row = self.tableShowMsg.rowCount()
        self.tableShowMsg.insertRow(row)
        msg.insert(0, str(row))
        for index in range(len(msg)):
            item = QTableWidgetItem(str(msg[index]))
            item.setTextAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
            self.tableShowMsg.setItem(row, index, item)

    # Listen
    def listenCAN(self):
        if self.driver_name == "" or self.bitrate == 0:
            QMessageBox.critical(self, "Warning", "CAN port is not opened!", QMessageBox.Yes)
        else:
            self.listen_Thread = listen_Thread(self.driver_name, self.bitrate)
            self.listen_Thread._signal.connect(self.updateTable)
            self.button_stoplisten.setEnabled(True)
            self.buttom_listen.setEnabled(False)
            self.listen_Thread.start()

            self.enableSend(False)
            self.enableAttack(False)

    def stoplistenCAN(self):
        self.listen_Thread.flag = 0
        self.buttom_listen.setEnabled(True)
        self.button_stoplisten.setEnabled(False)
        self.enableSend(True)
        self.ResetAttack()

    def updateTable(self, msg):
        if len(msg) == 1:
            if msg[0] == "send_stop":
                self.enableSend(True)
                self.buttom_listen.setEnabled(True)
                self.ResetAttack()
        else:
            if msg[1] == "send":
                pass
                #print("send")
            elif msg[1] == "receive":
                self.receivedDataList.append(msg)
                #print(msg)
            else:
                pass
                ##print("Error!")
            self.insertMsg(msg)

    def clearTable(self):
        self.tableShowMsg.setRowCount(0)

    # Send thread
    def sendCAN(self):
        if self.driver_name == "" or self.bitrate == 0:
            QMessageBox.critical(self, "Warning", "CAN port is not opened!", QMessageBox.Yes)
        else:
            can_msg = self.creatCANMsg()
            can_msgList = [can_msg]
            parameters = self.creatParameters()
            self.send_Thread = send_Thread(self.driver_name, self.bitrate, can_msgList, parameters=parameters)
            self.send_Thread._signal.connect(self.updateTable)
            self.send_Thread.start()
            self.enableSend(False)
            self.buttom_listen.setEnabled(False)
            self.enableAttack(False)

    def creatCANMsg(self):
        type = self.CB_type.currentText()
        str_id = self.lineEdit_id.text()
        str_data = self.lineEdit_data.text()
        can_msg = parseStr2CAN(type, str_id, str_data)
        return can_msg

    def creatParameters(self):
        num = self.lineEdit_number.text()
        interval = self.lineEdit_interval.text()
        int_num = int(num)
        ms_interval = float(interval) / 1000.0
        times = 1
        interval2 = 1
        parameters = [int_num, ms_interval, times, interval2]
        print(parameters)
        return parameters

    def enableSend(self, flag):
        self.CB_type.setEnabled(flag)
        self.lineEdit_id.setEnabled(flag)
        self.lineEdit_data.setEnabled(flag)
        self.button_send.setEnabled(flag)
        self.lineEdit_number.setEnabled(flag)
        self.lineEdit_interval.setEnabled(flag)

    def enableListen(self, flag):
        self.buttom_listen.setEnabled(flag)
        self.button_stoplisten.setEnabled(flag)

    # Attack function
    def chooseAttakType(self):
        sender = self.sender()
        if sender == self.radioButton_IVN:
            self.enableDIAattack(False)
        else:
            self.enableIVNattack(False)

    def enableIVNattack(self, flag):
        self.CB_IVN.setEnabled(flag)
        self.CB_DIA.setEnabled(not flag)

    def enableDIAattack(self, flag):
        self.CB_DIA.setEnabled(flag)
        self.CB_IVN.setEnabled(not flag)

    def enableAttack(self, flag):
        self.radioButton_IVN.setEnabled(flag)
        self.radioButton_DIA.setEnabled(flag)
        self.CB_IVN.setEnabled(flag)
        self.CB_DIA.setEnabled(flag)
        self.pushButton_create.setEnabled(flag)

    def ResetAttack(self):
        self.enableAttack(True)
        if self.radioButton_IVN.isChecked():
            self.CB_DIA.setEnabled(False)
        elif self.radioButton_DIA.isChecked():
            self.CB_IVN.setEnabled(False)

    def creatAttackUi(self):
        # make sure state
        if self.radioButton_IVN.isChecked():
            self.attack1 = "In-vehicle attack"
            self.attack2 = self.CB_IVN.currentText()
        elif self.radioButton_DIA.isChecked():
            self.attack1 = "Diagnostic attack"
            self.attack2 = self.CB_DIA.currentText()
        else:
            self.attack1 = ""

        if self.driver_name == "" or self.bitrate == 0:
            QMessageBox.critical(self, "Warning", "CAN port is not opened!", QMessageBox.Yes)
        elif self.attack1 == "" or self.attack2 == "":
            QMessageBox.information(self, "Which one?", "You haven't choose the attack type!")
        else:
            if self.attack1 == "In-vehicle attack" and self.attack2 == "Fabrication":
                self.genAttack1.transfer(self.attack2, self.receivedDataList, self.driver_name, self.bitrate)
                self.genAttack1.show()
            elif self.attack1 == "In-vehicle attack" and self.attack2 == "Surpass the capacity":
                self.genAttack1.transfer(self.attack2, self.receivedDataList, self.driver_name, self.bitrate)
                self.genAttack1.show()
            elif self.attack1 == "In-vehicle attack" and self.attack2 == "Dos with high priority ID":
                self.genAttack2.transfer(self.attack2, self.receivedDataList, self.driver_name, self.bitrate)
                self.genAttack2.show()
            elif self.attack1 == "In-vehicle attack" and self.attack2 == "Replay Single Frame":
                self.genAttack3.transfer(self.attack2, self.receivedDataList, self.driver_name, self.bitrate)
                self.genAttack3.show()
            elif self.attack1 == "In-vehicle attack" and self.attack2 == "Repaly Multiple Frame":
                self.genAttack4.transfer(self.attack2, self.receivedDataList, self.driver_name, self.bitrate)
                self.genAttack4.show()
            elif self.attack1 == "In-vehicle attack" and self.attack2 == "Fuzzy random ID":
                self.genAttack5.transfer(self.attack2, self.receivedDataList, self.driver_name, self.bitrate)
                self.genAttack5.show()
            elif self.attack1 == "In-vehicle attack" and self.attack2 == "Fuzzy valid ID":
                self.genAttack6.transfer(self.attack2, self.receivedDataList, self.driver_name, self.bitrate)
                self.genAttack6.show()
            elif self.attack1 == "Diagnostic attack" and self.attack2 == "Fuzzy randomly":
                self.genAttack5.transfer(self.attack2, self.receivedDataList, self.driver_name, self.bitrate)
                self.genAttack5.show()
            elif self.attack1 == "Diagnostic attack" and self.attack2 == "Fuzzy with designed part":
                self.genAttack7.transfer(self.attack2, self.receivedDataList, self.driver_name, self.bitrate)
                self.genAttack7.show()
            else:
                self.genAttack8.transfer(self.attack2, self.receivedDataList, self.driver_name, self.bitrate)
                self.genAttack8.show()

    # Close the connection while exit
    def closeEvent(self, QCloseEvent):
        if self.driver_name != "":
            command = "ip link set " + self.driver_name + " down"
            sudoPassword = self.password
            result = os.system(('echo %s|sudo -S %s' % (sudoPassword, command)))

        QCloseEvent.accept()


class listen_Thread(QThread):
    _signal = QtCore.pyqtSignal(list)

    def __init__(self, driver_name, bitrate):
        super().__init__()
        self.driver_name = driver_name
        self.bitrate = bitrate
        self.flag = 1

    def run(self):
        try:
            self.bus = creatCANbus(self.driver_name, self.bitrate)
            while (self.flag):
                msg = self.bus.recv(0.1)
                if msg:
                    str_msg = parseCAN2Str(msg)
                    self._signal.emit(str_msg)
            print("read is over!")
            self.bus.shutdown()
        except Exception as e:
            print(e)


class send_Thread(QThread):
    _signal = QtCore.pyqtSignal(list)

    def __init__(self, driver_name, bitrate, msgList, parameters=[1, 0.01, 1, 1]):
        super().__init__()
        self.driver_name = driver_name
        self.bitrate = bitrate
        self.flag = 1
        self.msgList = msgList
        self.number = parameters[0]
        self.interval = parameters[1]
        self.times = parameters[2]
        self.interval2 = parameters[3]

    def run(self):
        self.bus = creatCANbus(self.driver_name, self.bitrate)
        length = len(self.msgList)
        for one_time in range(self.times):  # times

            for number in range(self.number):
                for index in range(length):
                    msg = self.msgList[index]
                    str_msg = parseCAN2Str(msg, direction=2)
                    try:
                        self.bus.send(msg)
                        self._signal.emit(str_msg)
                        time.sleep(self.interval)
                    except can.CanError:
                        print("CAN message is not sent out!")
            if one_time < self.times - 1:
                time.sleep(self.interval2)

        self.bus.shutdown()
        self._signal.emit(["send_stop"])
        print("Send is over")


class Mulsend_Thread(QThread):
    _signal = QtCore.pyqtSignal(list)

    def __init__(self, driver_name, bitrate, msgList, parameters=[1, 1, 1]):
        super().__init__()
        self.driver_name = driver_name
        self.bitrate = bitrate
        self.flag = 1
        self.msgList = msgList
        self.number = parameters[0]
        self.times = parameters[1]
        self.interval2 = parameters[2]

    def run(self):
        self.bus = creatCANbus(self.driver_name, self.bitrate)
        length = len(self.msgList)
        for one_time in range(self.times):  # times

            for number in range(self.number):
                for index in range(length):
                    str_msg = self.msgList[index]
                    timestamp, msg = parseStrList2CAN(str_msg)
                    if index == 0:
                        beforetime = timestamp
                    interval = float(timestamp - beforetime) / 1000000.0
                    beforetime = timestamp
                    #print(interval)
                    try:
                        # add new timestamp
                        time.sleep(interval)
                        str_msg = parseCAN2Str(msg, direction=2)
                        self.bus.send(msg)
                        self._signal.emit(str_msg)
                    except can.CanError:
                        print("CAN message is not sent out!")
                time.sleep(0.1)
            if one_time < self.times - 1:
                time.sleep(self.interval2)

        self.bus.shutdown()
        self._signal.emit(["send_stop"])
        print("Send is over")


class DiagnosicSend_Thread(QThread):
    _signal = QtCore.pyqtSignal(list)

    def __init__(self, driver_name, bitrate, msgList, parameters=[1, 0.1, 1, 1]):
        super().__init__()
        self.driver_name = driver_name
        self.bitrate = bitrate
        self.flag = 1
        self.msgList = msgList
        self.number = parameters[0]
        self.interval1 = parameters[1]
        self.times = parameters[2]
        self.interval2 = parameters[3]

    # None:FC fail
    # (BS,ST):FC success
    # We can change the client_id to match the serve_id
    def wait_FC(self, bus, Iterations=0, client_id=0x700):
        # Won't iterate all the time
        if Iterations >= 3:
            return None

        msg = bus.recv(2)
        if msg == None:
            #print("No reply message!")
            return None
        str_msg = parseCAN2Str(msg)
        self._signal.emit(str_msg)

        if msg.arbitration_id < client_id:
            Iterations = Iterations + 0.5
            self.wait_FC(bus, Iterations)

        # msg_status = msg.data[0]
        msg_type = (msg.data[0] & 0xF0) >> 4
        msg_FS = msg.data[0] & 0x0F
        msg_BS = msg.data[1]
        msg_ST = msg.data[2]
        if msg_type != 3:
            return None
        if msg_FS == 0:
            return (msg_BS, msg_ST)
        elif msg_FS == 1:
            Iterations = Iterations + 1
            return self.wait_FC(bus, Iterations)
        elif msg_BS == 2:
            return None

    def SendMsg(self, bus, msg):
        bus.send(msg)
        str_msg = parseCAN2Str(msg, direction=2)
        self._signal.emit(str_msg)

    # 0:send fail
    # 1:send successfully
    def multiple_send(self, msgList, bus):
        msgLen = len(msgList)
        sendIndex = 0  # the index to send

        try:
            msg = msgList[sendIndex]
            self.SendMsg(bus, msg)
            if msgLen == 1:
                return 1

            sendIndex = sendIndex + 1
            res = self.wait_FC(bus)
            #print(res)
            if res == None:
                return 0
            BS = res[0]
            TS = res[1]
            send_no = 0
            while (sendIndex <= msgLen - 1):
                if BS == 0:
                    time.sleep(0.001)
                    msg = msgList[sendIndex]
                    self.SendMsg(bus, msg)
                    sendIndex = sendIndex + 1
                elif BS > 0:
                    if send_no < BS:
                        time.sleep(0.001)
                        msg = msgList[sendIndex]
                        self.SendMsg(bus, msg)
                        sendIndex = sendIndex + 1
                        send_no = send_no + 1
                    else:
                        res = self.wait_FC(bus)
                        if res == None:
                            return 0
                        BS = res[0]
                        send_no = 0

            return 1
        except can.CanError:
            print("Message not sent!")

    def run(self):
        self.bus = creatCANbus(self.driver_name, self.bitrate)
        length = len(self.msgList)
        for one_time in range(self.times):  # times

            for number in range(self.number):
                self.multiple_send(self.msgList, self.bus)
                time.sleep(self.interval1)
            if one_time < self.times - 1:
                time.sleep(self.interval2)

        self.bus.shutdown()
        self._signal.emit(["send_stop"])
        print("Send is over")


class ConnectForm(QWidget, Ui_Setup):
    _signal = QtCore.pyqtSignal(str, str, int)

    def __init__(self, parent=None):
        super(ConnectForm, self).__init__(parent)
        self.setupUi(self)
        self.button_ok.clicked.connect(self.createConnection)
        self.button_close.clicked.connect(self.close)
        self.Line_passport.setEchoMode(QLineEdit.Password)

    def createConnection(self):
        bit_rate = self.CB_baudrate.currentText()
        driver_name = self.Line_DriverName.text()
        sudoPassword = self.Line_passport.text()
        if driver_name == "" or sudoPassword == "":
            QMessageBox.critical(self, "Warning", "The Driver name or password is empty!", QMessageBox.Yes)
        else:
            speed = parseBitrate(bit_rate)
            command1 = "ip link set " + driver_name + " type can bitrate " + str(speed) + ";"

            command2 = "ip link set up " + driver_name + ";"

            result1 = os.system('echo %s|sudo -S %s' % (sudoPassword, command1))
            result2 = os.system(('echo %s|sudo -S %s' % (sudoPassword, command2)))

            if result1 == 0 and result2 == 0:
                QMessageBox.information(self, "Command", "CAN is set up successfully")
                self._signal.emit(driver_name, sudoPassword, speed)
                self.close()
            else:
                QMessageBox.information(self, "Command", "cannot find device %s" % driver_name)


class CreateAttack1(QWidget, Ui_Attack1):
    def __init__(self, parent=None):
        super(CreateAttack1, self).__init__(parent)
        self.setupUi(self)
        self.LastData = []
        self.LoadedData = []
        self.receivedDataList = []
        self.PB_load.clicked.connect(self.loadData)
        self.driver_name = ""
        self.bitrate = 0

        # set Table
        self.tableShowMsg.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self.tableShowMsg.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.tableShowMsg.verticalHeader().setVisible(False)
        self.tableShowMsg.horizontalHeader().resizeSection(0, 120)
        self.tableShowMsg.horizontalHeader().resizeSection(1, 200)
        self.tableShowMsg.horizontalHeader().resizeSection(2, 100)
        self.tableShowMsg.horizontalHeader().resizeSection(3, 110)
        self.tableShowMsg.horizontalHeader().resizeSection(4, 110)
        self.tableShowMsg.horizontalHeader().resizeSection(5, 50)
        self.tableShowMsg.horizontalHeader().resizeSection(6, 400)
        self.tableShowMsg.doubleClicked.connect(self.selectData)
        # end

        # select row
        self.lineEdit_index.returnPressed.connect(self.selectIndex)
        self.button_clear.clicked.connect(self.clearTable)

        # listening
        self.buttom_listen.clicked.connect(self.listenCAN)
        self.button_stoplisten.clicked.connect(self.stoplistenCAN)
        self.button_stoplisten.setEnabled(False)
        self.button_clear.clicked.connect(self.clearTable)

        # Send
        self.button_attack.setStyleSheet("color:rgb(255,0,0)")
        self.button_attack.clicked.connect(self.sendCAN)

    def transfer(self, attack_type, LoadData, driver_name, bit_rate):
        self.attack_type = attack_type
        self.LastData = LoadData
        self.label_AttackName.setText(self.attack_type)
        self.label_AttackName.adjustSize()
        self.driver_name = driver_name
        self.bitrate = bit_rate
        if self.attack_type == "Surpass the capacity":
            self.lineEdit_number.setText("1000")
            self.lineEdit_interval.setText("1")

    def loadData(self):
        length = len(self.LastData)
        if length == 0:
            QMessageBox.information(self, "Reminder", "No data has been collected yet!")
        else:
            self.LoadedData = self.LoadedData + self.LastData
            for msg in self.LoadedData:
                self.insertMsg(msg)
            self.receivedDataList = self.receivedDataList = self.LoadedData

    def selectData(self):
        row = self.tableShowMsg.currentRow()
        self.lineEdit_index.setText(str(row))
        str_id = self.tableShowMsg.item(row, 4).text()
        str_data = self.tableShowMsg.item(row, 6).text()
        can_id = ChangeDataFormat(str_id)
        self.lineEdit_id.setText(can_id)
        data = ChangeDataFormat(str_data)
        self.lineEdit_data.setText(data)

    def selectIndex(self):
        str_index = self.lineEdit_index.text()
        if is_Int(str_index):
            index = int(str_index)
            all_row = self.tableShowMsg.rowCount()
            if index < 0 or index >= all_row:
                QMessageBox.information(self, "Reminder", "Index is out of range!")
            else:
                self.tableShowMsg.selectRow(index)
                str_id = self.tableShowMsg.item(index, 4).text()
                str_data = self.tableShowMsg.item(index, 6).text()
                can_id = ChangeDataFormat(str_id)
                self.lineEdit_id.setText(can_id)
                data = ChangeDataFormat(str_data)
                self.lineEdit_data.setText(data)
        else:
            QMessageBox.information(self, "Reminder", "Input is wrong!")

    # Listen
    def listenCAN(self):
        if self.driver_name == "" or self.bitrate == 0:
            QMessageBox.critical(self, "Warning", "CAN port is not opened!", QMessageBox.Yes)
        else:
            self.listen_Thread = listen_Thread(self.driver_name, self.bitrate)
            self.listen_Thread._signal.connect(self.updateTable)
            self.button_stoplisten.setEnabled(True)
            self.buttom_listen.setEnabled(False)
            self.listen_Thread.start()

            self.enableSend(False)

    def stoplistenCAN(self):
        self.listen_Thread.flag = 0
        self.buttom_listen.setEnabled(True)
        self.button_stoplisten.setEnabled(False)
        self.enableSend(True)

    # Send thread
    def sendCAN(self):
        if self.driver_name == "" or self.bitrate == 0:
            QMessageBox.critical(self, "Warning", "CAN port is not opened!", QMessageBox.Yes)
        else:
            if self.attack_type == "Surpass the capacity":
                num = self.lineEdit_number.text()
                interval = self.lineEdit_interval.text()
                iNum = int(num)
                iInterval = float(interval)
                if iNum < 1000 or iInterval > 1:
                    QMessageBox.critical(self, "Warning",
                                         "Number should be more than 1000 and inetval1 should less than 1!",
                                         QMessageBox.Yes)
                    return
            can_msg = self.creatCANMsg()
            can_msgList = [can_msg]
            parameters = self.creatParameters()
            self.send_Thread = send_Thread(self.driver_name, self.bitrate, can_msgList, parameters=parameters)
            self.send_Thread._signal.connect(self.updateTable)
            self.send_Thread.start()
            self.enableSend(False)
            self.buttom_listen.setEnabled(False)

    def creatCANMsg(self):
        type = self.CB_type.currentText()
        str_id = self.lineEdit_id.text()
        str_data = self.lineEdit_data.text()
        can_msg = parseStr2CAN(type, str_id, str_data)
        return can_msg

    def creatParameters(self):
        num = self.lineEdit_number.text()
        interval = self.lineEdit_interval.text()
        int_num = int(num)
        ms_interval = float(interval) / 1000.0
        str_times = self.lineEdit_times.text()
        str_interval2 = self.lineEdit_interval2.text()
        times = int(str_times)
        interval2 = float(str_interval2)
        parameters = [int_num, ms_interval, times, interval2]
        #print(parameters)
        return parameters

    def updateTable(self, msg):
        if len(msg) == 1:
            if msg[0] == "send_stop":
                self.enableSend(True)
                self.buttom_listen.setEnabled(True)
        else:
            if msg[1] == "send":
                # print("send")
                pass
            elif msg[1] == "receive":
                self.receivedDataList.append(msg)
                #print("receive")
            else:
                pass
                #print("Error!")
            self.insertMsg(msg)

    def clearTable(self):
        self.tableShowMsg.setRowCount(0)

    def enableSend(self, flag):
        self.button_attack.setEnabled(flag)
        self.PB_load.setEnabled(flag)

    # Table show
    def insertMsg(self, msg_noindex):
        msg = msg_noindex[:]
        row = self.tableShowMsg.rowCount()
        self.tableShowMsg.insertRow(row)
        msg.insert(0, str(row))
        for index in range(len(msg)):
            item = QTableWidgetItem(str(msg[index]))
            item.setTextAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
            self.tableShowMsg.setItem(row, index, item)


class CreateDoS(QWidget, Ui_DoS_model):
    def __init__(self, parent=None):
        super(CreateDoS, self).__init__(parent)
        self.setupUi(self)
        self.LastData = []
        self.LoadedData = []
        self.receivedDataList = []
        self.PB_load.clicked.connect(self.loadData)
        self.driver_name = ""
        self.bitrate = 0

        # set Table
        self.tableShowMsg.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self.tableShowMsg.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.tableShowMsg.verticalHeader().setVisible(False)
        self.tableShowMsg.horizontalHeader().resizeSection(0, 120)
        self.tableShowMsg.horizontalHeader().resizeSection(1, 200)
        self.tableShowMsg.horizontalHeader().resizeSection(2, 100)
        self.tableShowMsg.horizontalHeader().resizeSection(3, 110)
        self.tableShowMsg.horizontalHeader().resizeSection(4, 110)
        self.tableShowMsg.horizontalHeader().resizeSection(5, 50)
        self.tableShowMsg.horizontalHeader().resizeSection(6, 400)
        self.tableShowMsg.doubleClicked.connect(self.selectData)
        # end

        self.pushButton_tradition.clicked.connect(self.SetTraditionDos)
        # select row
        self.PB_choose.clicked.connect(self.ChooseLowestID)
        self.button_clear.clicked.connect(self.clearTable)

        # listening
        self.buttom_listen.clicked.connect(self.listenCAN)
        self.button_stoplisten.clicked.connect(self.stoplistenCAN)
        self.button_stoplisten.setEnabled(False)
        self.button_clear.clicked.connect(self.clearTable)

        # Send
        self.button_attack.setStyleSheet("color:rgb(255,0,0)")
        self.button_attack.clicked.connect(self.sendCAN)

    def transfer(self, attack_type, LoadData, driver_name, bit_rate):
        self.attack_type = attack_type
        self.LastData = LoadData
        self.label_AttackName.setText(self.attack_type)
        self.label_AttackName.adjustSize()
        self.driver_name = driver_name
        self.bitrate = bit_rate

    def loadData(self):
        length = len(self.LastData)
        if length == 0:
            QMessageBox.information(self, "Reminder", "No data has been collected yet!")
        else:
            self.LoadedData = self.LoadedData + self.LastData
            ID_list = findLowestID(self.LoadedData)
            if ID_list is None:
                QMessageBox.information(self, "Reminder", "No data has been collected yet!")
            else:
                for msg in ID_list:
                    self.insertMsg(msg)
                self.receivedDataList = self.receivedDataList + self.LoadedData

    def selectData(self):
        row = self.tableShowMsg.currentRow()
        str_id = self.tableShowMsg.item(row, 4).text()
        str_data = self.tableShowMsg.item(row, 6).text()
        can_id = ChangeDataFormat(str_id)
        self.lineEdit_id.setText(can_id)
        data = ChangeDataFormat(str_data)
        self.lineEdit_data.setText(data)

    # Listen
    def listenCAN(self):
        if self.driver_name == "" or self.bitrate == 0:
            QMessageBox.critical(self, "Warning", "CAN port is not opened!", QMessageBox.Yes)
        else:
            self.listen_Thread = listen_Thread(self.driver_name, self.bitrate)
            self.listen_Thread._signal.connect(self.updateTable)
            self.button_stoplisten.setEnabled(True)
            self.buttom_listen.setEnabled(False)
            self.listen_Thread.start()

            self.enableSend(False)

    def stoplistenCAN(self):
        self.listen_Thread.flag = 0
        self.buttom_listen.setEnabled(True)
        self.button_stoplisten.setEnabled(False)
        self.enableSend(True)

    # Send thread
    def sendCAN(self):
        if self.driver_name == "" or self.bitrate == 0:
            QMessageBox.critical(self, "Warning", "CAN port is not opened!", QMessageBox.Yes)
        else:
            can_msg = self.creatCANMsg()
            can_msgList = [can_msg]
            parameters = self.creatParameters()
            self.send_Thread = send_Thread(self.driver_name, self.bitrate, can_msgList, parameters=parameters)
            self.send_Thread._signal.connect(self.updateTable)
            self.send_Thread.start()
            self.enableSend(False)
            self.buttom_listen.setEnabled(False)

    def creatCANMsg(self):
        type = self.CB_type.currentText()
        str_id = self.lineEdit_id.text()
        str_data = self.lineEdit_data.text()
        can_msg = parseStr2CAN(type, str_id, str_data)
        return can_msg

    def creatParameters(self):
        num = self.lineEdit_number.text()
        interval = self.lineEdit_interval.text()
        int_num = int(num)
        ms_interval = float(interval) / 1000.0
        str_times = self.lineEdit_times.text()
        str_interval2 = self.lineEdit_interval2.text()
        times = int(str_times)
        interval2 = float(str_interval2)
        parameters = [int_num, ms_interval, times, interval2]
        #print(parameters)
        return parameters

    def updateTable(self, msg):
        if len(msg) == 1:
            if msg[0] == "send_stop":
                self.enableSend(True)
                self.buttom_listen.setEnabled(True)
        else:
            if msg[1] == "send":
                # print("send")
                pass
            elif msg[1] == "receive":
                self.receivedDataList.append(msg)
                #print("receive")
            else:
                pass
                #print("Error!")
            self.insertMsg(msg)

    def clearTable(self):
        self.tableShowMsg.setRowCount(0)

    def enableSend(self, flag):
        self.button_attack.setEnabled(flag)
        self.PB_load.setEnabled(flag)

    # Table show
    def insertMsg(self, msg_noindex):
        msg = msg_noindex[:]
        row = self.tableShowMsg.rowCount()
        self.tableShowMsg.insertRow(row)
        msg.insert(0, str(row))
        for index in range(len(msg)):
            item = QTableWidgetItem(str(msg[index]))
            item.setTextAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
            self.tableShowMsg.setItem(row, index, item)

    def ChooseLowestID(self):
        new_list = self.receivedDataList
        ID_List = findLowestID(new_list)
        if ID_List is None:
            QMessageBox.information(self, "Reminder", "No data has been collected yet!")
        else:
            self.clearTable()
            for msg in ID_List:
                self.insertMsg(msg)

    def SetTraditionDos(self):
        self.lineEdit_id.setText("0000")
        self.lineEdit_data.setText("00 00 00 00 00 00 00 00")


class CreateSingeReplay(QWidget, Ui_SReplay_model):
    def __init__(self, parent=None):
        super(CreateSingeReplay, self).__init__(parent)
        self.setupUi(self)
        self.LastData = []
        self.LoadedData = []
        self.receivedDataList = []
        self.PB_load.clicked.connect(self.loadData)
        self.driver_name = ""
        self.bitrate = 0

        # set Table
        self.tableShowMsg.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self.tableShowMsg.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.tableShowMsg.verticalHeader().setVisible(False)
        self.tableShowMsg.horizontalHeader().resizeSection(0, 120)
        self.tableShowMsg.horizontalHeader().resizeSection(1, 200)
        self.tableShowMsg.horizontalHeader().resizeSection(2, 100)
        self.tableShowMsg.horizontalHeader().resizeSection(3, 110)
        self.tableShowMsg.horizontalHeader().resizeSection(4, 110)
        self.tableShowMsg.horizontalHeader().resizeSection(5, 50)
        self.tableShowMsg.horizontalHeader().resizeSection(6, 400)
        self.tableShowMsg.doubleClicked.connect(self.selectData)
        # end

        # select row
        self.lineEdit_index.returnPressed.connect(self.selectIndex)
        self.button_clear.clicked.connect(self.clearTable)

        # listening
        self.buttom_listen.clicked.connect(self.listenCAN)
        self.button_stoplisten.clicked.connect(self.stoplistenCAN)
        self.button_stoplisten.setEnabled(False)
        self.button_clear.clicked.connect(self.clearTable)

        # Send
        self.button_attack.setStyleSheet("color:rgb(255,0,0)")
        self.button_attack.clicked.connect(self.sendCAN)

        self.CB_type.setEditable(False)
        self.lineEdit_id.setFocusPolicy(Qt.NoFocus)
        self.lineEdit_data.setFocusPolicy(Qt.NoFocus)

    def transfer(self, attack_type, LoadData, driver_name, bit_rate):
        self.attack_type = attack_type
        self.LastData = LoadData
        self.label_AttackName.setText(self.attack_type)
        self.label_AttackName.adjustSize()
        self.driver_name = driver_name
        self.bitrate = bit_rate

    def loadData(self):
        length = len(self.LastData)
        if length == 0:
            QMessageBox.information(self, "Reminder", "No data has been collected yet!")
        else:
            self.LoadedData = self.LoadedData + self.LastData
            for msg in self.LoadedData:
                self.insertMsg(msg)
            self.receivedDataList = self.receivedDataList = self.LoadedData

    def selectData(self):
        row = self.tableShowMsg.currentRow()
        self.lineEdit_index.setText(str(row))
        str_id = self.tableShowMsg.item(row, 4).text()
        str_data = self.tableShowMsg.item(row, 6).text()
        can_id = ChangeDataFormat(str_id)
        self.lineEdit_id.setText(can_id)
        data = ChangeDataFormat(str_data)
        self.lineEdit_data.setText(data)

    def selectIndex(self):
        str_index = self.lineEdit_index.text()
        if is_Int(str_index):
            index = int(str_index)
            all_row = self.tableShowMsg.rowCount()
            if index < 0 or index >= all_row:
                QMessageBox.information(self, "Reminder", "Index is out of range!")
            else:
                self.tableShowMsg.selectRow(index)
                str_id = self.tableShowMsg.item(index, 4).text()
                str_data = self.tableShowMsg.item(index, 6).text()
                can_id = ChangeDataFormat(str_id)
                self.lineEdit_id.setText(can_id)
                data = ChangeDataFormat(str_data)
                self.lineEdit_data.setText(data)
        else:
            QMessageBox.information(self, "Reminder", "Input is wrong!")

    # Listen
    def listenCAN(self):
        if self.driver_name == "" or self.bitrate == 0:
            QMessageBox.critical(self, "Warning", "CAN port is not opened!", QMessageBox.Yes)
        else:
            self.listen_Thread = listen_Thread(self.driver_name, self.bitrate)
            self.listen_Thread._signal.connect(self.updateTable)
            self.button_stoplisten.setEnabled(True)
            self.buttom_listen.setEnabled(False)
            self.listen_Thread.start()

            self.enableSend(False)

    def stoplistenCAN(self):
        self.listen_Thread.flag = 0
        self.buttom_listen.setEnabled(True)
        self.button_stoplisten.setEnabled(False)
        self.enableSend(True)

    # Send thread
    def sendCAN(self):
        if self.driver_name == "" or self.bitrate == 0:
            QMessageBox.critical(self, "Warning", "CAN port is not opened!", QMessageBox.Yes)
        elif len(self.receivedDataList) == 0:
            QMessageBox.information(self, "Reminder", "No data available for replay!")
        else:
            can_msg = self.creatCANMsg()
            can_msgList = [can_msg]
            parameters = self.creatParameters()
            self.send_Thread = send_Thread(self.driver_name, self.bitrate, can_msgList, parameters=parameters)
            self.send_Thread._signal.connect(self.updateTable)
            self.send_Thread.start()
            self.enableSend(False)
            self.buttom_listen.setEnabled(False)

    def creatCANMsg(self):
        type = self.CB_type.currentText()
        str_id = self.lineEdit_id.text()
        str_data = self.lineEdit_data.text()
        can_msg = parseStr2CAN(type, str_id, str_data)
        return can_msg

    def creatParameters(self):
        num = self.lineEdit_number.text()
        interval = self.lineEdit_interval.text()
        int_num = int(num)
        ms_interval = float(interval) / 1000.0
        str_times = self.lineEdit_times.text()
        str_interval2 = self.lineEdit_interval2.text()
        times = int(str_times)
        interval2 = float(str_interval2)
        parameters = [int_num, ms_interval, times, interval2]
        #print(parameters)
        return parameters

    def updateTable(self, msg):
        if len(msg) == 1:
            if msg[0] == "send_stop":
                self.enableSend(True)
                self.buttom_listen.setEnabled(True)
        else:
            if msg[1] == "send":
                # print("send")
                pass
            elif msg[1] == "receive":
                self.receivedDataList.append(msg)
                #print("receive")
            else:
                pass
                #print("Error!")
            self.insertMsg(msg)

    def clearTable(self):
        self.tableShowMsg.setRowCount(0)

    def enableSend(self, flag):
        self.button_attack.setEnabled(flag)
        self.PB_load.setEnabled(flag)

    # Table show
    def insertMsg(self, msg_noindex):
        msg = msg_noindex[:]
        row = self.tableShowMsg.rowCount()
        self.tableShowMsg.insertRow(row)
        msg.insert(0, str(row))
        for index in range(len(msg)):
            item = QTableWidgetItem(str(msg[index]))
            item.setTextAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
            self.tableShowMsg.setItem(row, index, item)


class CreateMultipleReplay(QWidget, Ui_MReplay_model):
    def __init__(self, parent=None):
        super(CreateMultipleReplay, self).__init__(parent)
        self.setupUi(self)
        self.LastData = []
        self.LoadedData = []
        self.receivedDataList = []
        self.seletedData = []
        self.PB_load.clicked.connect(self.loadData)
        self.driver_name = ""
        self.bitrate = 0

        # set Table
        self.tableShowMsg.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self.tableShowMsg.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.tableShowMsg.verticalHeader().setVisible(False)
        self.tableShowMsg.horizontalHeader().resizeSection(0, 120)
        self.tableShowMsg.horizontalHeader().resizeSection(1, 200)
        self.tableShowMsg.horizontalHeader().resizeSection(2, 100)
        self.tableShowMsg.horizontalHeader().resizeSection(3, 110)
        self.tableShowMsg.horizontalHeader().resizeSection(4, 110)
        self.tableShowMsg.horizontalHeader().resizeSection(5, 50)
        self.tableShowMsg.horizontalHeader().resizeSection(6, 400)
        # self.tableShowMsg.doubleClicked.connect(self.selectData)
        self.tableShowMsg.setSelectionMode(QAbstractItemView.MultiSelection)

        self.tableSelectedMsg.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self.tableSelectedMsg.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.tableSelectedMsg.verticalHeader().setVisible(False)
        self.tableSelectedMsg.horizontalHeader().resizeSection(0, 100)
        self.tableSelectedMsg.horizontalHeader().resizeSection(1, 200)
        # end

        # select row
        self.tableShowMsg.setContextMenuPolicy(Qt.CustomContextMenu)
        self.tableShowMsg.customContextMenuRequested.connect(self.generatemenu)
        self.PB_select.clicked.connect(self.selectIndex)
        self.button_clear.clicked.connect(self.clearTable)

        # listening
        self.buttom_listen.clicked.connect(self.listenCAN)
        self.button_stoplisten.clicked.connect(self.stoplistenCAN)
        self.button_stoplisten.setEnabled(False)
        self.button_clear.clicked.connect(self.clearTable)

        # Send
        # TODO
        # self.button_attack.setStyleSheet("color:rgb(255,0,0)")
        self.button_attack.clicked.connect(self.sendCAN)

    def transfer(self, attack_type, LoadData, driver_name, bit_rate):
        self.attack_type = attack_type
        self.LastData = LoadData
        self.label_AttackName.setText(self.attack_type)
        self.label_AttackName.adjustSize()
        self.driver_name = driver_name
        self.bitrate = bit_rate

    def loadData(self):
        length = len(self.LastData)
        if length == 0:
            QMessageBox.information(self, "Reminder", "No data has been collected yet!")
        else:
            self.LoadedData = self.LoadedData + self.LastData
            for msg in self.LastData:
                self.insertMsg(msg)
            self.receivedDataList = self.receivedDataList = self.LoadedData

    def generatemenu(self, pos):
        # print(pos)     #输出所单击鼠标右键的坐标值
        menu = QMenu()
        action_FIndex = menu.addAction("Set front index")
        action_RIndex = menu.addAction("Set rear index")
        action_select = menu.addAction("Select these messages")

        # 设置菜单显示的坐标位置为相对于单元格的位置
        screenpos = self.tableShowMsg.mapToGlobal(pos)
        #print(screenpos)

        # 设置选择菜单满足的条件
        a = menu.exec_(screenpos)
        if a == action_FIndex:
            rows = self.selectedRow()
            self.lineEdit_FIndex.setText(str(rows[0]))
        elif a == action_RIndex:
            rows = self.selectedRow()
            self.lineEdit_RIndex.setText(str(rows[-1]))
        elif a == action_select:
            rows = self.selectedRow()
            self.updateSelectedTable(rows)
            #print(self.seletedData)
        else:
            return

    def selectedRow(self):
        # all_items = self.tableShowMsg.selectedItems()
        all_items = self.tableShowMsg.selectedIndexes()
        # all_items = self.tableShowMsg.selectionModel().selection().indexes()
        selected_row = []
        for i in all_items:
            row = i.row()
            if row not in selected_row:
                selected_row.append(row)
        return selected_row

    def selectIndex(self):
        str_FrontIndex = self.lineEdit_FIndex.text()
        str_RearIndex = self.lineEdit_RIndex.text()
        if str_FrontIndex == '' or str_RearIndex == '':
            QMessageBox.critical(self, "Warning", "Please choose the index!", QMessageBox.Yes)
            return
        FrontIndex = int(str_FrontIndex)
        RearIndex = int(str_RearIndex)
        all_row = self.tableShowMsg.rowCount()
        if FrontIndex > RearIndex:
            QMessageBox.critical(self, "Warning", "Front index should be less than rear index!", QMessageBox.Yes)
        elif RearIndex > all_row:
            QMessageBox.critical(self, "Warning", "Index is out of range!", QMessageBox.Yes)
        else:
            selected_row = list(range(FrontIndex, RearIndex + 1))
            self.updateSelectedTable(selected_row)
            before_row = self.selectedRow()
            for index in before_row:
                self.tableShowMsg.selectRow(index)
            for index in selected_row:
                self.tableShowMsg.selectRow(index)
            #print(self.seletedData)

    # Listen
    def listenCAN(self):
        if self.driver_name == "" or self.bitrate == 0:
            QMessageBox.critical(self, "Warning", "CAN port is not opened!", QMessageBox.Yes)
        else:
            self.listen_Thread = listen_Thread(self.driver_name, self.bitrate)
            self.listen_Thread._signal.connect(self.updateTable)
            self.button_stoplisten.setEnabled(True)
            self.buttom_listen.setEnabled(False)
            self.listen_Thread.start()

            self.enableSend(False)

    def stoplistenCAN(self):
        self.listen_Thread.flag = 0
        self.buttom_listen.setEnabled(True)
        self.button_stoplisten.setEnabled(False)
        self.enableSend(True)

    # Send thread
    def sendCAN(self):
        if self.driver_name == "" or self.bitrate == 0:
            QMessageBox.critical(self, "Warning", "CAN port is not opened!", QMessageBox.Yes)
        elif len(self.seletedData) <= 0:
            QMessageBox.information(self, "Reminder", "No data available for replay!")
        else:
            can_msgList = self.seletedData
            parameters = self.creatParameters()
            self.send_Thread = Mulsend_Thread(self.driver_name, self.bitrate, can_msgList, parameters=parameters)
            self.send_Thread._signal.connect(self.updateTable)
            self.send_Thread.start()
            self.enableSend(False)
            self.buttom_listen.setEnabled(False)

    def creatCANMsg(self):
        type = self.CB_type.currentText()
        str_id = self.lineEdit_id.text()
        str_data = self.lineEdit_data.text()
        can_msg = parseStr2CAN(type, str_id, str_data)
        return can_msg

    def creatParameters(self):
        num = self.lineEdit_number.text()
        int_num = int(num)
        str_times = self.lineEdit_times.text()
        str_interval2 = self.lineEdit_interval2.text()
        times = int(str_times)
        interval2 = float(str_interval2)
        parameters = [int_num, times, interval2]
        #print(parameters)
        return parameters

    def updateTable(self, msg):
        if len(msg) == 1:
            if msg[0] == "send_stop":
                self.enableSend(True)
                self.buttom_listen.setEnabled(True)
        else:
            if msg[1] == "send":
                # print("send")
                pass
            elif msg[1] == "receive":
                self.receivedDataList.append(msg)
                #print("receive")
            else:
                pass
                #print("Error!")
            self.insertMsg(msg)

    def updateSelectedTable(self, rowList):
        all_column = self.tableShowMsg.columnCount()
        all_msg = []
        self.tableSelectedMsg.setRowCount(0)
        rowList.sort()
        for row in rowList:
            msg = []
            for column in range(all_column):
                tmp = self.tableShowMsg.item(row, column).text()
                msg.append(tmp)
            part_msg = [msg[0], msg[1]]
            self.insertSelectedMsg(part_msg)
            all_msg.append(msg)
        self.seletedData = all_msg

    def clearTable(self):
        self.tableShowMsg.setRowCount(0)

    def enableSend(self, flag):
        self.button_attack.setEnabled(flag)
        self.PB_load.setEnabled(flag)

    # Table show
    def insertMsg(self, msg_noindex):
        msg = msg_noindex[:]
        row = self.tableShowMsg.rowCount()
        self.tableShowMsg.insertRow(row)
        msg.insert(0, str(row))
        for index in range(len(msg)):
            item = QTableWidgetItem(str(msg[index]))
            item.setTextAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
            self.tableShowMsg.setItem(row, index, item)

    def insertSelectedMsg(self, msg):
        row = self.tableSelectedMsg.rowCount()
        self.tableSelectedMsg.insertRow(row)
        for index in range(len(msg)):
            item = QTableWidgetItem(str(msg[index]))
            item.setTextAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
            self.tableSelectedMsg.setItem(row, index, item)


class CreateRandomFuzzy(QWidget, Ui_Fuzzy_randomID):
    def __init__(self, parent=None):
        super(CreateRandomFuzzy, self).__init__(parent)
        self.setupUi(self)
        self.driver_name = ""
        self.bitrate = 0

        # set Table
        self.tableShowMsg.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self.tableShowMsg.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.tableShowMsg.verticalHeader().setVisible(False)
        self.tableShowMsg.horizontalHeader().resizeSection(0, 120)
        self.tableShowMsg.horizontalHeader().resizeSection(1, 200)
        self.tableShowMsg.horizontalHeader().resizeSection(2, 100)
        self.tableShowMsg.horizontalHeader().resizeSection(3, 110)
        self.tableShowMsg.horizontalHeader().resizeSection(4, 110)
        self.tableShowMsg.horizontalHeader().resizeSection(5, 50)
        self.tableShowMsg.horizontalHeader().resizeSection(6, 400)
        # end

        # select row
        self.button_clear.clicked.connect(self.clearTable)

        # Send
        # self.button_attack.setStyleSheet("color:rgb(255,0,0)")
        self.button_attack.clicked.connect(self.sendCAN)

    def transfer(self, attack_type, LoadData, driver_name, bit_rate):
        self.attack_type = attack_type
        self.LoadedData = LoadData
        self.label_AttackName.setText(self.attack_type)
        self.label_AttackName.adjustSize()
        self.driver_name = driver_name
        self.bitrate = bit_rate
        if self.attack_type == "Fuzzy randomly":
            self.label_range.setText("[0x700,0x7FF]")
            self.lineEdit_front.setText("700")
            self.lineEdit_rear.setText("7FF")

    # Send thread
    def sendCAN(self):
        Sfront_range = self.lineEdit_front.text()
        front_range = int(Sfront_range, 16)
        Srear_range = self.lineEdit_rear.text()
        rear_range = int(Srear_range, 16)

        if self.driver_name == "" or self.bitrate == 0:
            QMessageBox.critical(self, "Warning", "CAN port is not opened!", QMessageBox.Yes)
        elif rear_range < front_range:
            QMessageBox.information(self, "Reminder", "Index should be in right range!")
        else:
            if self.attack_type == "Fuzzy random ID":
                if rear_range > 0x700 or front_range < 0:
                    QMessageBox.information(self, "Reminder", "In-vehicle ID should be in [0x000,0x700)!")
                else:
                    parameters = self.creatParameters()
                    number = parameters[0]
                    parameters[0] = 1
                    can_msgList = []
                    for index in range(number):
                        tmp = self.creatCANMsg(front_range, rear_range)
                        can_msgList.append(tmp)
                    self.send_Thread = send_Thread(self.driver_name, self.bitrate, can_msgList, parameters=parameters)
                    self.send_Thread._signal.connect(self.updateTable)
                    self.send_Thread.start()
                    self.enableSend(False)
            else:
                if rear_range >= 0x800 or front_range < 0x700:
                    QMessageBox.information(self, "Reminder", "Diagnostic ID should be in [0x700,0x7FF]!")
                else:
                    parameters = self.creatParameters()
                    number = parameters[0]
                    parameters[0] = 1
                    can_msgList = []
                    for index in range(number):
                        tmp = self.creatCANMsg(front_range, rear_range)
                        can_msgList.append(tmp)
                    self.send_Thread = send_Thread(self.driver_name, self.bitrate, can_msgList, parameters=parameters)
                    self.send_Thread._signal.connect(self.updateTable)
                    self.send_Thread.start()
                    self.enableSend(False)

    def creatCANMsg(self, front, rear):
        is_extended = False
        can_id = fuzzID(front, rear)
        if self.attack_type == "Fuzzy random ID":
            length, data = fuzzData()
        else:
            data = fuzzDiaData()
        can_msg = can.Message(
            arbitration_id=can_id,
            data=data,
            is_extended_id=is_extended,
        )
        return can_msg

    def creatParameters(self):
        num = self.lineEdit_number.text()
        interval = self.lineEdit_interval.text()
        int_num = int(num)
        ms_interval = float(interval) / 1000.0
        str_times = self.lineEdit_times.text()
        str_interval2 = self.lineEdit_interval2.text()
        times = int(str_times)
        interval2 = float(str_interval2)
        parameters = [int_num, ms_interval, times, interval2]
        #print(parameters)
        return parameters

    def updateTable(self, msg):
        if len(msg) == 1:
            if msg[0] == "send_stop":
                self.enableSend(True)
        else:
            if msg[1] == "send":
                pass
                #print("send")
            elif msg[1] == "receive":
                pass
                #print("receive")
            else:
                pass
                #print("Error!")
            self.insertMsg(msg)

    def clearTable(self):
        self.tableShowMsg.setRowCount(0)

    def enableSend(self, flag):
        self.button_attack.setEnabled(flag)

    # Table show
    def insertMsg(self, msg_noindex):
        msg = msg_noindex[:]
        row = self.tableShowMsg.rowCount()
        self.tableShowMsg.insertRow(row)
        msg.insert(0, str(row))
        for index in range(len(msg)):
            item = QTableWidgetItem(str(msg[index]))
            item.setTextAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
            self.tableShowMsg.setItem(row, index, item)


class CreateValidFuzzy(QWidget, Ui_ValidFuzzy):
    def __init__(self, parent=None):
        super(CreateValidFuzzy, self).__init__(parent)
        self.setupUi(self)
        self.LastData = []
        self.LoadedData = []
        self.receivedDataList = []
        self.PB_load.clicked.connect(self.loadData)
        self.driver_name = ""
        self.bitrate = 0

        self.ID_list = []
        self.selectedID = []

        # set Table
        self.tableShowMsg.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self.tableShowMsg.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.tableShowMsg.verticalHeader().setVisible(False)
        self.tableShowMsg.horizontalHeader().resizeSection(0, 120)
        self.tableShowMsg.horizontalHeader().resizeSection(1, 200)
        self.tableShowMsg.horizontalHeader().resizeSection(2, 100)
        self.tableShowMsg.horizontalHeader().resizeSection(3, 110)
        self.tableShowMsg.horizontalHeader().resizeSection(4, 110)
        self.tableShowMsg.horizontalHeader().resizeSection(5, 50)
        self.tableShowMsg.horizontalHeader().resizeSection(6, 400)
        # self.tableShowMsg.doubleClicked.connect(self.selectData)

        self.tableAllID.setSelectionMode(QAbstractItemView.MultiSelection)
        self.tableAllID.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self.tableAllID.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.tableAllID.verticalHeader().setVisible(False)
        self.tableAllID.horizontalHeader().resizeSection(0, 50)
        self.tableAllID.horizontalHeader().resizeSection(1, 100)
        # end

        # select row
        self.button_clear.clicked.connect(self.clearTable)
        self.PB_validID.clicked.connect(self.UpdateIDTable)
        self.PB_selectAll.clicked.connect(self.selectAll)
        self.PB_Deselect.clicked.connect(self.Deselect)
        # listening
        self.buttom_listen.clicked.connect(self.listenCAN)
        self.button_stoplisten.clicked.connect(self.stoplistenCAN)
        self.button_stoplisten.setEnabled(False)
        self.button_clear.clicked.connect(self.clearTable)

        # Send
        # TODO
        # self.button_attack.setStyleSheet("color:rgb(255,0,0)")
        self.button_attack.clicked.connect(self.sendCAN)

    def transfer(self, attack_type, LoadData, driver_name, bit_rate):
        self.attack_type = attack_type
        self.LastData = LoadData
        self.label_AttackName.setText(self.attack_type)
        self.label_AttackName.adjustSize()
        self.driver_name = driver_name
        self.bitrate = bit_rate

    def loadData(self):
        length = len(self.LastData)
        if length == 0:
            QMessageBox.information(self, "Reminder", "No data has been collected yet!")
        else:
            for msg in self.LastData:
                self.insertMsg(msg)
            self.LoadedData = self.LoadedData + self.LastData
            self.receivedDataList = self.receivedDataList = self.LoadedData

    def selectedRow(self):
        # all_items = self.tableShowMsg.selectedItems()
        all_items = self.tableAllID.selectedIndexes()
        # all_items = self.tableShowMsg.selectionModel().selection().indexes()
        selected_row = []
        for i in all_items:
            row = i.row()
            if row not in selected_row:
                selected_row.append(row)
        return selected_row

    # Listen
    def listenCAN(self):
        if self.driver_name == "" or self.bitrate == 0:
            QMessageBox.critical(self, "Warning", "CAN port is not opened!", QMessageBox.Yes)
        else:
            self.listen_Thread = listen_Thread(self.driver_name, self.bitrate)
            self.listen_Thread._signal.connect(self.updateTable)
            self.button_stoplisten.setEnabled(True)
            self.buttom_listen.setEnabled(False)
            self.listen_Thread.start()

            self.enableSend(False)

    def stoplistenCAN(self):
        self.listen_Thread.flag = 0
        self.buttom_listen.setEnabled(True)
        self.button_stoplisten.setEnabled(False)
        self.enableSend(True)

    def sendCAN(self):
        self.GetSelectedID()
        if self.driver_name == "" or self.bitrate == 0:
            QMessageBox.critical(self, "Warning", "CAN port is not opened!", QMessageBox.Yes)
        elif len(self.selectedID) <= 0:
            QMessageBox.information(self, "Reminder", "No ID is selected!")
        else:
            parameters = self.creatParameters()
            number = parameters[0]
            parameters[0] = 1
            can_msgList = []
            for index in range(number):
                tmp = self.creatCANMsg(self.selectedID)
                can_msgList.append(tmp)
            self.send_Thread = send_Thread(self.driver_name, self.bitrate, can_msgList, parameters=parameters)
            self.send_Thread._signal.connect(self.updateTable)
            self.send_Thread.start()
            self.enableSend(False)
            self.buttom_listen.setEnabled(False)

    def GetSelectedID(self):
        rowList = self.selectedRow()
        IDList = []
        for row in rowList:
            str_id = self.tableAllID.item(row, 1).text()
            #print(str_id)
            int_id = int(str_id, 16)
            IDList.append(int_id)
        self.selectedID = IDList

    def creatCANMsg(self, idList):
        is_extended = False
        can_id = fuzzValidID(idList)
        length, data = fuzzData()
        can_msg = can.Message(
            arbitration_id=can_id,
            data=data,
            is_extended_id=is_extended,
        )
        return can_msg

    def creatParameters(self):
        num = self.lineEdit_number.text()
        interval = self.lineEdit_interval.text()
        int_num = int(num)
        ms_interval = float(interval) / 1000.0
        str_times = self.lineEdit_times.text()
        str_interval2 = self.lineEdit_interval2.text()
        times = int(str_times)
        interval2 = float(str_interval2)
        parameters = [int_num, ms_interval, times, interval2]
        #print(parameters)
        return parameters

    def updateTable(self, msg):
        if len(msg) == 1:
            if msg[0] == "send_stop":
                self.enableSend(True)
                self.buttom_listen.setEnabled(True)
        else:
            if msg[1] == "send":
                pass
                #print("send")
            elif msg[1] == "receive":
                self.receivedDataList.append(msg)
                #print("receive")
            else:
                pass
                #print("Error!")
            self.insertMsg(msg)

    def UpdateIDTable(self, idList):
        self.tableAllID.setRowCount(0)
        all_list = self.LoadedData + self.receivedDataList
        self.ID_list = findIDList(all_list)
        for id in self.ID_list:
            str_id = int2strHex(id, 4)
            msg = [str_id]
            self.insertIDMsg(msg)

    def selectAll(self):
        all_row = self.tableAllID.rowCount()
        for row in range(all_row):
            if not self.tableAllID.item(row, 0).isSelected():
                self.tableAllID.selectRow(row)

    def Deselect(self):
        selected_row = self.selectedRow()
        for row in selected_row:
            self.tableAllID.selectRow(row)

    def clearTable(self):
        self.tableShowMsg.setRowCount(0)

    def enableSend(self, flag):
        self.button_attack.setEnabled(flag)
        self.PB_load.setEnabled(flag)

    # Table show
    def insertMsg(self, msg_noindex):
        msg = msg_noindex[:]
        row = self.tableShowMsg.rowCount()
        self.tableShowMsg.insertRow(row)
        msg.insert(0, str(row))
        for index in range(len(msg)):
            item = QTableWidgetItem(str(msg[index]))
            item.setTextAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
            self.tableShowMsg.setItem(row, index, item)

    def insertIDMsg(self, msg):
        row = self.tableAllID.rowCount()
        self.tableAllID.insertRow(row)
        msg.insert(0, str(row))
        for index in range(len(msg)):
            item = QTableWidgetItem(str(msg[index]))
            item.setTextAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
            self.tableAllID.setItem(row, index, item)


class CreateDesignedFyzzy(QWidget, Ui_Fuzzy_designed):
    def __init__(self, parent=None):
        super(CreateDesignedFyzzy, self).__init__(parent)
        self.setupUi(self)
        self.LastData = []
        self.LoadedData = []
        self.receivedDataList = []
        self.driver_name = ""
        self.bitrate = 0

        # set Table
        self.tableShowMsg.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self.tableShowMsg.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.tableShowMsg.verticalHeader().setVisible(False)
        self.tableShowMsg.horizontalHeader().resizeSection(0, 120)
        self.tableShowMsg.horizontalHeader().resizeSection(1, 200)
        self.tableShowMsg.horizontalHeader().resizeSection(2, 100)
        self.tableShowMsg.horizontalHeader().resizeSection(3, 110)
        self.tableShowMsg.horizontalHeader().resizeSection(4, 110)
        self.tableShowMsg.horizontalHeader().resizeSection(5, 50)
        self.tableShowMsg.horizontalHeader().resizeSection(6, 400)
        # end

        # select row

        self.button_clear.clicked.connect(self.clearTable)

        # Send
        # self.button_attack.setStyleSheet("color:rgb(255,0,0)")
        self.button_attack.clicked.connect(self.sendCAN)

    def transfer(self, attack_type, LoadData, driver_name, bit_rate):
        self.attack_type = attack_type
        self.LastData = LoadData
        self.label_AttackName.setText(self.attack_type)
        self.label_AttackName.adjustSize()
        self.driver_name = driver_name
        self.bitrate = bit_rate

    # Send thread
    def sendCAN(self):
        if self.driver_name == "" or self.bitrate == 0:
            QMessageBox.critical(self, "Warning", "CAN port is not opened!", QMessageBox.Yes)
        else:
            FuzzBytes = self.getFuzzyBytes()
            str_data = self.lineEdit_data.text()
            length = getDataLength(str_data)
            if len(FuzzBytes) == 0:
                QMessageBox.critical(self, "Warning", "Please choose the bytes!", QMessageBox.Yes)
                return
            if length <= FuzzBytes[-1]:
                QMessageBox.critical(self, "Warning", "The length of payload is wrong!", QMessageBox.Yes)
                return
            parameters = self.creatParameters()
            number = parameters[0]
            parameters[0] = 1
            can_msgList = []
            for index in range(number):
                tmp = self.creatCANMsg(FuzzBytes)
                can_msgList.append(tmp)
            self.send_Thread = send_Thread(self.driver_name, self.bitrate, can_msgList, parameters=parameters)
            self.send_Thread._signal.connect(self.updateTable)
            self.send_Thread.start()
            self.enableSend(False)

    def getFuzzyBytes(self):
        bytes = []
        if self.CB0.isChecked():
            bytes.append(0)
        if self.CB1.isChecked():
            bytes.append(1)
        if self.CB2.isChecked():
            bytes.append(2)
        if self.CB3.isChecked():
            bytes.append(3)
        if self.CB4.isChecked():
            bytes.append(4)
        if self.CB5.isChecked():
            bytes.append(5)
        if self.CB6.isChecked():
            bytes.append(6)
        if self.CB7.isChecked():
            bytes.append(7)
        return bytes

    def creatCANMsg(self, bytes):
        type = self.CB_type.currentText()
        str_id = self.lineEdit_id.text()
        str_data = self.lineEdit_data.text()
        new_data = ChangeData(str_data, bytes)
        can_msg = parseStr2CAN(type, str_id, new_data)
        return can_msg

    def creatParameters(self):
        num = self.lineEdit_number.text()
        interval = self.lineEdit_interval.text()
        int_num = int(num)
        ms_interval = float(interval) / 1000.0
        str_times = self.lineEdit_times.text()
        str_interval2 = self.lineEdit_interval2.text()
        times = int(str_times)
        interval2 = float(str_interval2)
        parameters = [int_num, ms_interval, times, interval2]
        #print(parameters)
        return parameters

    def updateTable(self, msg):
        if len(msg) == 1:
            if msg[0] == "send_stop":
                self.enableSend(True)
        else:
            if msg[1] == "send":
                pass
                #print("send")
            elif msg[1] == "receive":
                pass
                #print("receive")
            else:
                pass
                #print("Error!")
            self.insertMsg(msg)

    def clearTable(self):
        self.tableShowMsg.setRowCount(0)

    def enableSend(self, flag):
        self.button_attack.setEnabled(flag)

    # Table show
    def insertMsg(self, msg_noindex):
        msg = msg_noindex[:]
        row = self.tableShowMsg.rowCount()
        self.tableShowMsg.insertRow(row)
        msg.insert(0, str(row))
        for index in range(len(msg)):
            item = QTableWidgetItem(str(msg[index]))
            item.setTextAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
            self.tableShowMsg.setItem(row, index, item)


class CreateControlModel(QWidget, Ui_Control):
    def __init__(self, parent=None):
        super(CreateControlModel, self).__init__(parent)
        self.setupUi(self)
        self.LastData = []
        self.LoadedData = []
        self.receivedDataList = []
        self.driver_name = ""
        self.bitrate = 0
        self.CountFrames = 0

        # set Table
        self.tableShowMsg.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self.tableShowMsg.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.tableShowMsg.verticalHeader().setVisible(False)
        self.tableShowMsg.horizontalHeader().resizeSection(0, 120)
        self.tableShowMsg.horizontalHeader().resizeSection(1, 200)
        self.tableShowMsg.horizontalHeader().resizeSection(2, 100)
        self.tableShowMsg.horizontalHeader().resizeSection(3, 110)
        self.tableShowMsg.horizontalHeader().resizeSection(4, 110)
        self.tableShowMsg.horizontalHeader().resizeSection(5, 50)
        self.tableShowMsg.horizontalHeader().resizeSection(6, 400)
        # end

        # select row

        self.button_clear.clicked.connect(self.clearTable)

        self.button_generate.clicked.connect(self.GenerateNewFrames)
        # Send
        # self.button_attack.setStyleSheet("color:rgb(255,0,0)")
        self.button_attack.clicked.connect(self.sendCAN)

    def transfer(self, attack_type, LoadData, driver_name, bit_rate):
        self.attack_type = attack_type
        self.LastData = LoadData
        self.label_AttackName.setText(self.attack_type)
        self.label_AttackName.adjustSize()
        self.driver_name = driver_name
        self.bitrate = bit_rate
        self.UpdataUi()
        self.InitFrame()
        self.clearTable()

    def UpdataUi(self):
        if self.attack_type in Control_3P:
            self.label_P3.setText("Operation:")
            self.label_P3.setGeometry(QtCore.QRect(360, 180, 41, 17))
            self.label_P3.adjustSize()
            self.LE_P3.setEnabled(True)
            self.LE_P3.setText("0")
            self.label_range3.setText("[0,1]")

            self.label_P4.setText("")
            self.label_P4.adjustSize()
            self.LE_P4.setText("")
            self.LE_P4.setEnabled(False)
            self.label_range4.setText("")
        elif self.attack_type == Control_SP[1]:
            self.label_P3.setText("")
            self.label_P3.adjustSize()
            self.LE_P3.setText("")
            self.LE_P3.setEnabled(False)
            self.label_range3.setText("")

            self.label_P4.setText("")
            self.label_P4.adjustSize()
            self.LE_P4.setText("")
            self.LE_P4.setEnabled(False)
            self.label_range4.setText("")
        else:
            self.label_P3.setText("Side:")
            self.label_P3.setGeometry(QtCore.QRect(400, 180, 41, 17))
            self.label_P3.adjustSize()
            self.LE_P3.setEnabled(True)
            self.LE_P3.setText("2")
            self.label_range3.setText("[0,2]")

            self.label_P4.setText("Operation:")
            self.label_P4.adjustSize()
            self.LE_P4.setEnabled(True)
            self.LE_P4.setText("0")
            self.label_range4.setText("[0,1]")




    def ShowFrameList(self, list):
        length = len(list)
        if length == 1:
            self.lineEdit_data2.setEnabled(False)
            first_frame = list[0]
            str_id, str_payload = CAN2Str(first_frame)
            self.lineEdit_id.setText(str_id)
            self.lineEdit_data.setText(str_payload)
        else:
            self.lineEdit_data2.setEnabled(True)
            first_frame = list[0]
            str_id, str_payload = CAN2Str(first_frame)
            self.lineEdit_id.setText(str_id)
            self.lineEdit_data.setText(str_payload)
            second_frame = list[1]
            str_id, str_payload = CAN2Str(second_frame)
            self.lineEdit_data2.setText(str_payload)

    def InitFrame(self):
        ActionDict = GenerateControlDict()
        list = ActionDict[self.attack_type]
        self.CountFrames = len(list)
        self.ShowFrameList(list)

    # Send thread
    def sendCAN(self):
        if self.driver_name == "" or self.bitrate == 0:
            QMessageBox.critical(self, "Warning", "CAN port is not opened!", QMessageBox.Yes)
        else:
            can_msgList = self.creatCANMsg()
            parameters = self.creatParameters()
            self.send_Thread = DiagnosicSend_Thread(self.driver_name, self.bitrate, can_msgList, parameters=parameters)
            self.send_Thread._signal.connect(self.updateTable)
            self.send_Thread.start()
            self.enableSend(False)

    def creatCANMsg(self):
        list = []
        type = self.CB_type.currentText()
        str_id = self.lineEdit_id.text()
        str_data = self.lineEdit_data.text()
        can_msg = parseStr2CAN(type, str_id, str_data)
        list.append(can_msg)
        if self.CountFrames >= 2:
            str_data2 = self.lineEdit_data2.text()
            can_msg2 = parseStr2CAN(type, str_id, str_data2)
            list.append(can_msg2)
        #print(list)
        return list

    def creatParameters(self):
        num = self.lineEdit_number.text()
        interval = self.lineEdit_interval.text()
        int_num = int(num)
        ms_interval = float(interval) / 1000.0
        str_times = self.lineEdit_times.text()
        str_interval2 = self.lineEdit_interval2.text()
        times = int(str_times)
        interval2 = float(str_interval2)
        parameters = [int_num, ms_interval, times, interval2]
        #print(parameters)
        return parameters

    def GenerateNewFrames(self):
        parameter1 = int(self.LE_P1.text())
        parameter2 = int(self.LE_P2.text())
        if self.LE_P3.text() == "":
            parameter3 = 0
        else:
            parameter3 = int(self.LE_P3.text())
        if self.LE_P4.text() == "":
            parameter4 = 0
        else:
            parameter4 = int(self.LE_P4.text())
        changeParameters(self.attack_type, parameter1, parameter2, parameter3, parameter4)
        ActionDict = GenerateControlDict()
        list = ActionDict[self.attack_type]
        self.ShowFrameList(list)

    def updateTable(self, msg):
        if len(msg) == 1:
            if msg[0] == "send_stop":
                self.enableSend(True)
        else:
            if msg[1] == "send":
                pass
                #print("send")
            elif msg[1] == "receive":
                pass
                #print("receive")
            else:
                pass
                #print("Error!")
            self.insertMsg(msg)

    def clearTable(self):
        self.tableShowMsg.setRowCount(0)

    def enableSend(self, flag):
        self.button_attack.setEnabled(flag)

    # Table show
    def insertMsg(self, msg_noindex):
        msg = msg_noindex[:]
        row = self.tableShowMsg.rowCount()
        self.tableShowMsg.insertRow(row)
        msg.insert(0, str(row))
        for index in range(len(msg)):
            item = QTableWidgetItem(str(msg[index]))
            item.setTextAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
            self.tableShowMsg.setItem(row, index, item)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    myWin = MainWindow()
    myWin.show()
    sys.exit(app.exec_())
