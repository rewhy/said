from __future__ import print_function
from datetime import datetime
import time
import can
import random


def creatCANbus(driver_name, bit_rate):
    interface, channel = ("socketcan", driver_name)
    bus = can.Bus(interface=interface, channel=channel, bitrate=bit_rate)
    return bus


def byte2strHex(num):
    return '0x{:02x}'.format(num)

def int2strHex(num, bits):
    f = "0x{:0"+str(bits)+"x}"
    return f.format(num)

def CAN2Str(can_msg):
    can_id = can_msg.arbitration_id
    payload = can_msg.data
    tmp_payload = ['{:02x}'.format(byte) for byte in payload]
    str_id = '{:04X}'.format(can_id)
    str_payload = " ".join(tmp_payload)
    return str_id, str_payload



def parseCAN2Str(can_msg, direction=1):
    can_id = can_msg.arbitration_id
    can_data = can_msg.data
    can_dlc = can_msg.dlc
    can_isExtend = can_msg.is_extended_id
    current_milli_time = int(round(time.time() * 1000000))


    str_dlc = str(can_dlc)
    str_data = ""
    for lyy in range(can_dlc):
        byte = can_data[lyy]
        str_data = str_data + byte2strHex(byte)
        if lyy < can_dlc - 1:
            str_data = str_data + " "
    if can_isExtend:
        str_type = "Extended"
        str_id = int2strHex(can_id, 8)
    else:
        str_type = "Standard"
        str_id = int2strHex(can_id, 4)
    str_timestamp = str(current_milli_time)

    if direction == 1:
        str_direction = "receive"
    else:
        str_direction = "send"

    res = [str_timestamp, str_direction, str_type, str_id, str_dlc, str_data]
    return res


def parseStr2CAN(type, str_id, str_data):
    if type == "Standard":
        is_extended = False
    else:
        is_extended = True

    int_id = int(str_id, 16)
    strList = str_data.split(" ")
    length = len(strList)
    int_data = []
    for byte in strList:
        tmp = int(byte, 16)
        int_data.append(tmp)

    can_msg = can.Message(
        arbitration_id=int_id,
        data=int_data,
        is_extended_id=is_extended,
    )
    return can_msg

def parseStrList2CAN(msgList):
    timestamp = int(msgList[1])
    int_id = int(msgList[4], 16)
    if msgList[3] == "Standard":
        is_extended = False
    else:
        is_extended = True
    str_payload = msgList[6]
    strList = str_payload.split(" ")
    length = len(strList)
    int_data = []
    for byte in strList:
        tmp = int(byte, 16)
        int_data.append(tmp)

    can_msg = can.Message(
        arbitration_id=int_id,
        data=int_data,
        is_extended_id=is_extended,
    )
    return timestamp, can_msg


def ChangeDataFormat(str):
    new = str.replace('0x', '')
    return new

def fuzzID(a, b):
    #res = random.randint(0, 1791)
    res = random.randint(a, b)
    return res
def fuzzValidID(list):
    length = len(list)
    index = random.randint(0, length-1)
    return list[index]

def fuzzDiagID():
    #res = random.randint(1792, 2047)
    res = random.randint(1792, 2047)
    return res

def fuzzData():
    length = random.randint(0, 8)
    b = [0 for i in range(length)]
    for i in range(length):
        b[i] = random.randint(0, 255)
    return length, bytearray(b)

def fuzzDiaData():
    b = [0 for i in range(8)]
    for i in range(8):
        b[i] = random.randint(0, 255)
    return bytearray(b)

def ChangeData(str_payload, bytes):
    strList = str_payload.split(" ")
    length = len(bytes)
    for i in range(length):
        index = bytes[i]
        tmp = random.randint(0, 255)
        tmp_str = '{:02x}'.format(tmp)
        strList[index] = tmp_str
    res = " ".join(strList)
    return res

def getDataLength(str_payload):
    strList = str_payload.split(" ")
    length = len(strList)
    return length

def is_number(s):
    try:
        float(s)
        return True
    except ValueError:
        pass

    try:
        import unicodedata
        unicodedata.numeric(s)
        return True
    except (TypeError, ValueError):
        pass

    return False

def is_Int(s):
    try:
        int(s)
        return True
    except ValueError:
        pass

    try:
        import unicodedata
        unicodedata.decimal(s)
        return True
    except (TypeError, ValueError):
        pass

    return False

Control_4P = ["Beam light", "High beam", "Turn signals", "Fog light", "Rear fog light", "Brake lamp", "Lock indicator", "Reversing light", "Interior light", "Indicator lamp"]
Control_3P = ["Front-Driver window", "Front-Passenger window", "Rear_Driver window", "Rear_Passenger window"]
Control_SP = ["Windshield wiper", "Signal horn"]
#Control
# Action
beam_light = []
high_beam = []
turn_signal = []
fog_light = []
rear_fog_light = []
brake_lamp = []
lock_indicator = []
reversing_light = []
interior_light = []
Windshield_wiper = []
signal_horn = []
FD_window = []
FP_window = []
RD_window = []
RP_window = []
indicator_lamp = []

def GenerateControlDict():
    Dict = {}
    Dict["Beam light"] = beam_light
    Dict["High beam"] = high_beam
    Dict["Turn signals"] = turn_signal
    Dict["Fog light"] = fog_light
    Dict["Rear fog light"] = rear_fog_light
    Dict["Brake lamp"] = brake_lamp
    Dict["Lock indicator"] = lock_indicator
    Dict["Reversing light"] = reversing_light
    Dict["Interior light"] = interior_light
    Dict["Windshield wiper"] = Windshield_wiper
    Dict["Signal horn"] = signal_horn
    Dict["Front-Driver window"] = FD_window
    Dict["Front-Passenger window"] = FP_window
    Dict["Rear_Driver window"] = RD_window
    Dict["Rear_Passenger window"] = RP_window
    Dict["Indicator lamp"] = indicator_lamp
    return Dict

def GenerateDefaultAction():
    # 1
    beam_light.append(
        can.Message(
            arbitration_id=0x70E,
            data=[0x10, 0x08, 0x2F, 0x09, 0x4B, 0x03, 0x01, 0x02],
            is_extended_id=False,
            timestamp=0,
        )
    )
    beam_light.append(
        can.Message(
            arbitration_id=0x70E,
            data=[0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],
            is_extended_id=False,
            timestamp=0,
        ))
    # 2
    high_beam.append(
        can.Message(
            arbitration_id=0x70E,
            data=[0x10, 0x08, 0x2F, 0x09, 0x4E, 0x03, 0x01, 0x02],
            is_extended_id=False,
            timestamp=0,
        )
    )
    high_beam.append(
        can.Message(
            arbitration_id=0x70E,
            data=[0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],
            is_extended_id=False,
            timestamp=0,
        )
    )
    # 3
    turn_signal.append(
        can.Message(
            arbitration_id=0x70E,
            data=[0x10, 0x08, 0x2F, 0x09, 0x48, 0x03, 0x01, 0x02],
            is_extended_id=False,
            timestamp=0,
        )
    )
    turn_signal.append(
        can.Message(
            arbitration_id=0x70E,
            data=[0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],
            is_extended_id=False,
            timestamp=0,
        ))
    # 4
    fog_light.append(
        can.Message(
            arbitration_id=0x70E,
            data=[0x10, 0x08, 0x2F, 0x09, 0x50, 0x03, 0x01, 0x02],
            is_extended_id=False,
            timestamp=0,
        )
    )
    fog_light.append(
        can.Message(
            arbitration_id=0x70E,
            data=[0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],
            is_extended_id=False,
            timestamp=0,
        ))
    # 5
    rear_fog_light.append(
        can.Message(
            arbitration_id=0x70E,
            data=[0x10, 0x08, 0x2F, 0x09, 0x4F, 0x03, 0x01, 0x02],
            is_extended_id=False,
            timestamp=0,
        )
    )
    rear_fog_light.append(
        can.Message(
            arbitration_id=0x70E,
            data=[0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],
            is_extended_id=False,
            timestamp=0,
        ))
    # 6
    brake_lamp.append(
        can.Message(
            arbitration_id=0x70E,
            data=[0x10, 0x08, 0x2F, 0x09, 0x4D, 0x03, 0x01, 0x02],
            is_extended_id=False,
            timestamp=0,
        )
    )
    brake_lamp.append(
        can.Message(
            arbitration_id=0x70E,
            data=[0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],
            is_extended_id=False,
            timestamp=0,
        )
    )
    # 7
    lock_indicator.append(
        can.Message(
            arbitration_id=0x70E,
            data=[0x10, 0x08, 0x2F, 0x09, 0x76, 0x03, 0x01, 0x02],
            is_extended_id=False,
            timestamp=0,
        )
    )
    lock_indicator.append(
        can.Message(
            arbitration_id=0x70E,
            data=[0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],
            is_extended_id=False,
            timestamp=0,
        ))
    # 8
    reversing_light.append(
        can.Message(
            arbitration_id=0x70E,
            data=[0x10, 0x08, 0x2F, 0x09, 0x51, 0x03, 0x01, 0x02],
            is_extended_id=False,
            timestamp=0,
        )
    )
    reversing_light.append(
        can.Message(
            arbitration_id=0x70E,
            data=[0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],
            is_extended_id=False,
            timestamp=0,
        ))
    # 9
    interior_light.append(
        can.Message(
            arbitration_id=0x70E,
            data=[0x10, 0x08, 0x2F, 0x09, 0x57, 0x03, 0x01, 0x02],
            is_extended_id=False,
            timestamp=0,
        )
    )
    interior_light.append(
        can.Message(
            arbitration_id=0x70E,
            data=[0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],
            is_extended_id=False,
            timestamp=0,
        ))
    # 10
    Windshield_wiper.append(
        can.Message(
            arbitration_id=0x70E,
            data=[0x10, 0x08, 0x2F, 0x09, 0x74, 0x03, 0x01, 0x02],
            is_extended_id=False,
            timestamp=0,
        )
    )
    Windshield_wiper.append(
        can.Message(
            arbitration_id=0x70E,
            data=[0x21, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],
            is_extended_id=False,
            timestamp=0,
        ))
    # 11
    signal_horn.append(
        can.Message(
            arbitration_id=0x70E,
            data=[0x05, 0x2F, 0x09, 0x66, 0x03, 0x01, 0x00, 0x00],
            is_extended_id=False,
            timestamp=0,
        )
    )
    # 12
    FD_window.append(
        can.Message(
            arbitration_id=0x74A,
            data=[0x10, 0x08, 0x2F, 0x04, 0x10, 0x03, 0xFF, 0x00],
            is_extended_id=False,
            timestamp=0,
        )
    )
    FD_window.append(
        can.Message(
            arbitration_id=0x74A,
            data=[0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],
            is_extended_id=False,
            timestamp=0,
        ))
    # 13
    RD_window.append(
        can.Message(
            arbitration_id=0x74A,
            data=[0x10, 0x08, 0x2F, 0x04, 0x12, 0x03, 0xFF, 0x00],
            is_extended_id=False,
            timestamp=0,
        )
    )
    RD_window.append(
        can.Message(
            arbitration_id=0x74A,
            data=[0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],
            is_extended_id=False,
            timestamp=0,
        ))
    # 14
    FP_window.append(
        can.Message(
            arbitration_id=0x74B,
            data=[0x10, 0x08, 0x2F, 0x04, 0x11, 0x03, 0xFF, 0x00],
            is_extended_id=False,
            timestamp=0,
        )
    )
    FP_window.append(
        can.Message(
            arbitration_id=0x74B,
            data=[0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],
            is_extended_id=False,
            timestamp=0,
        ))
    # 15
    RP_window.append(
        can.Message(
            arbitration_id=0x74B,
            data=[0x10, 0x08, 0x2F, 0x04, 0x13, 0x03, 0xFF, 0x00],
            is_extended_id=False,
            timestamp=0,
        )
    )
    RP_window.append(
        can.Message(
            arbitration_id=0x74B,
            data=[0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],
            is_extended_id=False,
            timestamp=0,
        ))
    # 16
    indicator_lamp.append(
        can.Message(
            arbitration_id=0x714,
            data=[0x10, 0x08, 0x2F, 0x02, 0x4C, 0x03, 0xFF, 0x00],
            is_extended_id=False,
            timestamp=0,
        )
    )
    indicator_lamp.append(
        can.Message(
            arbitration_id=0x714,
            data=[0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],
            is_extended_id=False,
            timestamp=0,
        ))


def changeParameters(action_type, p1=3, p2=1, p3=2, p4=0):
    # type
    if action_type == "Beam light":
        beam_light[0].data[5] = p1
        beam_light[0].data[6] = p2
        beam_light[0].data[7] = p3
        if p4 == 1:
            beam_light[1].data[1] = 0xFF
            beam_light[1].data[2] = 0xFF
    elif action_type == "High beam":
        high_beam[0].data[5] = p1
        high_beam[0].data[6] = p2
        high_beam[0].data[7] = p3
        if p4 == 1:
            high_beam[1].data[1] = 0xFF
            high_beam[1].data[2] = 0xFF
    elif action_type == "Turn signals":
        turn_signal[0].data[5] = p1
        turn_signal[0].data[6] = p2
        turn_signal[0].data[7] = p3
        if p4 == 1:
            turn_signal[1].data[1] = 0xFF
            turn_signal[1].data[2] = 0xFF
    elif action_type == "Fog light":
        fog_light[0].data[5] = p1
        fog_light[0].data[6] = p2
        fog_light[0].data[7] = p3
        if p4 == 1:
            fog_light[1].data[1] = 0xFF
            fog_light[1].data[2] = 0xFF
    elif action_type == "Rear fog light":
        rear_fog_light[0].data[5] = p1
        rear_fog_light[0].data[6] = p2
        rear_fog_light[0].data[7] = p3
        if p4 == 1:
            rear_fog_light[1].data[1] = 0xFF
            rear_fog_light[1].data[2] = 0xFF
    elif action_type == "Brake lamp":
        brake_lamp[0].data[5] = p1
        brake_lamp[0].data[6] = p2
        brake_lamp[0].data[7] = p3
        if p4 == 1:
            brake_lamp[1].data[1] = 0xFF
            brake_lamp[1].data[2] = 0xFF
    elif action_type == "Lock indicator":
        lock_indicator[0].data[5] = p1
        lock_indicator[0].data[6] = p2
        lock_indicator[0].data[7] = p3
        if p4 == 1:
            lock_indicator[1].data[1] = 0xFF
            lock_indicator[1].data[2] = 0xFF
    elif action_type == "Reversing light":
        reversing_light[0].data[5] = p1
        reversing_light[0].data[6] = p2
        reversing_light[0].data[7] = p3
        if p4 == 1:
            reversing_light[1].data[1] = 0xFF
            reversing_light[1].data[2] = 0xFF
    elif action_type == "Interior light":
        interior_light[0].data[5] = p1
        interior_light[0].data[6] = p2
        interior_light[0].data[7] = p3
        if p4 == 1:
            interior_light[1].data[1] = 0xFF
            interior_light[1].data[2] = 0xFF
    elif action_type == "Windshield wiper":
        Windshield_wiper[0].data[5] = p1
        Windshield_wiper[0].data[6] = p2
    elif action_type == "Signal horn":
        signal_horn[0].data[4] = p1
        signal_horn[0].data[5] = p2
    elif action_type == "Front-Driver window":
        FD_window[0].data[5] = p1
        FD_window[0].data[6] = p2
        if p3 > 0:
            FD_window[0].data[7] = 0xFF
            FD_window[1].data[1] = 0xFF
            FD_window[1].data[2] = 0xFF
    elif action_type == "Rear_Driver window":
        RD_window[0].data[5] = p1
        RD_window[0].data[6] = p2
        if p3 > 0:
            RD_window[0].data[7] = 0xFF
            RD_window[1].data[1] = 0xFF
            RD_window[1].data[2] = 0xFF
    elif action_type == "Front-Passenger window":
        FP_window[0].data[5] = p1
        FP_window[0].data[6] = p2
        if p3 > 0:
            FP_window[0].data[7] = 0xFF
            FP_window[1].data[1] = 0xFF
            FP_window[1].data[2] = 0xFF
    elif action_type == "Rear_Passenger window":
        RP_window[0].data[5] = p1
        RP_window[0].data[6] = p2
        if p3 > 0:
            RP_window[0].data[7] = 0xFF
            RP_window[1].data[1] = 0xFF
            RP_window[1].data[2] = 0xFF
    elif action_type == "Indicator lamp":
        indicator_lamp[0].data[5] = p1
        indicator_lamp[0].data[6] = p2
        indicator_lamp[0].data[7] = p3
        if p4 == 1:
            indicator_lamp[1].data[1] = 0xFF
            indicator_lamp[1].data[2] = 0xFF

#t = ['2', '1603028260794', 'receive', 'Standard', '0x0576', '8', '0x02 0x01 0x0c 0x00 0x00 0x08 0x00 0x00']
#parseStrList2CAN(t)
# print(int("fff", 16))
# str_data = "00 00 00 11 11 11"
# strList = str_data.split(" ")
# print(strList)
#msg = parseStr2CAN("Standard", "0000", "00 00 00 00 00")
#print(msg)

#res = int2strHex(0xfff, 4)
#print(res)
