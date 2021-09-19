from datetime import datetime
import time

def parseBitrate(str_bitrate):
    scale = str_bitrate[-4]
    sNum = str_bitrate[0:-4]
    Num = int(sNum)
    if scale == "M":
        Num = Num * 1000000
    elif scale == "K":
        Num = Num * 1000
    else:
        print("The format of speed is wrong!")
        return None
    return Num

def findLowestID(MsgList):
    Dict = {}
    lowest_id = 999999
    for msg in MsgList:
        str_id = msg[3]
        int_id = int(str_id, 16)
        if int_id in Dict:
            Dict[int_id].append(msg)
        else:
            Dict[int_id] = [msg]
            if int_id < lowest_id:
                lowest_id = int_id
    if lowest_id < 999999:
        IDList = Dict[lowest_id]
        return IDList
    else:
        return None

def findIDList(MsgList):
    IDList = []
    for msg in MsgList:
        str_id = msg[3]
        int_id = int(str_id, 16)
        if not int_id in IDList:
            IDList.append(int_id)
    IDList.sort()
    return IDList

def parseLine(msg, direction=1, type=1):
    msg = msg.strip()
    string = msg.split(",", -1)
    length = len(string)
    #Msg with Flag
    if (string[-1].strip() == "R" or string[-1].strip() == "T") and length >= 4:
        s_time = string[0]
        time = float(s_time)
        Int_time = int(time*1000000)
        s_timestamp = str(Int_time)
        if direction == 1:
            direction = "receive"
        else:
            direction = "send"
        if type == 1:
            type = "Standard"
        else:
            type = "Extended"

        s_canID = "0x" + string[1]
        s_CanDlc = string[2]
        can_dlc = int(s_CanDlc)
        payload=""
        for i in range(can_dlc):
            tmp = "0x"+string[3+i]
            if i < can_dlc-1:
                tmp = tmp + " "
            payload = payload + tmp
        res = [s_timestamp, direction, type, s_canID, s_CanDlc, payload]
        return res
    #standard format
    elif length == 6 and (string[1] == "receive" or string[1] == "send"):
        res_list = list(string)
        return res_list
    return None

def parseFile(filename):
    DataList = []
    try:
        fp = open(filename, "r")
        index = 0
        while(index < 1000000):
            line = fp.readline()
            if line == "":
                break

            tmp = parseLine(line)
            if tmp == None:
                print("The format of content is wrong!")
                #print(line)
                continue
            else:
                DataList.append(tmp)
            index = index + 1
        fp.close()
        return DataList
    except IOError:
        print("Error %s not exist" % filename)



#l = ['1603093260206', 'receive', 'Standard', '0x070e', '8', '0x30 0x00 0x00 0x00 0x00 0x00 0x00 0x00']
##res = StringList(l)
#res2 = parseLine(res)
#print(res2)

#res = parseFile("normal.txt")
#print(len(res))
#MSG = "1479121434.850202,0350,8,05,28,84,66,6d,00,00,a2,R"
#MSG = "1479121460.443476,02b0,5,7c,00,00,07,3f,R"
#print(parseLine(MSG))
#MSG = "1603093260206,receive,Standard,0x070e,8,0x30 0x00 0x00 0x00 0x00 0x00 0x00 0x00"
#res = parseLine(MSG)

#Result Format
#['1603093260206', 'receive', 'Standard', '0x070e', '8', '0x30 0x00 0x00 0x00 0x00 0x00 0x00 0x00']
