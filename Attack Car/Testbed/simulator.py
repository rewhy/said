#!/usr/bin/env python
# coding=utf-8
import os
import cv2
import re
import socket
import time
from datetime import datetime
import numpy as np
import thread

np.set_printoptions(suppress=True)

# IP = '158.132.146.24'
IP = '10.13.233.73'
PORT = 9999


def onTrackbarSlide(emp):
    pass


def adbForward(a, b):
    os.system('adb forward tcp:' + a + ' tcp:' + b)


def findIndex(msg_list, time):
    if time <= msg_list[0][0]:
        return 0  # the video on first and wait the data
    else:
        for i in range(1, len(msg_list)):
            if time < msg_list[i][0]:
                return i
        return -1  # Can't find index, not mactch


def usleep(microsecond):
    second = microsecond / 1000000.0
    interval = second / 20
    global flag
    for i in range(0, 20):
        time.sleep(interval)
        if flag == False:
            return


def msleep(millionsecond):
    second = millionsecond / 1000.0
    time.sleep(second)


def showState():
    global state
    global showflag
    while (showflag):
        print "Speed: " + str(state[0]) + "\t||||\t" + "Steer: " + str(state[1])
        time.sleep(1)


def startClient(msg_list, index):
    print index
    # TODO  IP ?
    BUFFSIZE = 1024;
    ip_port = (IP, PORT)
    client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    this_index = index
    last_ts = msg_list[index][0]
    interval = 0
    global flag

    while flag and this_index < len(msg_list):
        msg = msg_list[this_index][1]
        interval = msg_list[this_index][0] - last_ts
        last_ts = msg_list[this_index][0]

        this_index = this_index + 1;

        parseMsg(msg)

        client.sendto(msg.encode('utf-8'), ip_port)
        # return
        # data,server_addr = client.recvfrom(BUFFSIZE)

        if interval < 0:
            interval = 0
        usleep(interval)
    client.close()


def parseFile(name):
    msg_list = []
    fp = open(name)

    line = fp.readline()
    while line:
        line = line.replace("\t\n", "")
        str = re.split('\[|\]|@', line)

        type = str[1]

        if (type != 'TX'):
            num = str[2]
            num = long(num)
            tmp_tuple = (num, line)
            msg_list.append(tmp_tuple)
            del tmp_tuple

        line = fp.readline()

    return msg_list


def getFirstTime(name, miltime):
    field = name.split('_')
    year = field[1][0:4]
    month = field[1][4:6]
    day = field[1][6:]
    hour = field[2][0:2]
    min = field[2][2:4]
    second = field[2][4:]
    millionsecond = miltime
    format_t = "{}-{}-{} {}:{}:{}.{}".format(year, month, day, hour, min, second, millionsecond)
    return format_t


def str2timestamp(str_time):
    dt = datetime.strptime(str_time, "%Y-%m-%d %H:%M:%S.%f")
    timestamp = long(time.mktime(dt.timetuple()) * 1000.0 + dt.microsecond / 1000.0)
    timestamp = timestamp * 1000L
    return timestamp


def signedhex2long(str):
    val = int(str, 16)
    signedval = ((val + 0x8000) & 0xFFFF) - 0x8000
    return signedval


def parseMsg(msg):
    global state
    str = re.split('\[|\]|@| ', msg)
    type = str[1]
    if (type == "RX"):
        str_data = str[-1]
        data = str_data.split('/')
        if data[1] == "62" and data[2] == "f4" and data[3] == "0d":
            state[0] = int(data[4], 16)
        elif data[1] == "62" and data[2] == "18" and data[3] == "12":
            tmp = data[4] + data[5]
            tmp = round(signedhex2long(tmp) * 0.1, 4)
            state[1] = tmp


def MouseEvent(event, x, y, flags, param):
    pass


def progress(name, video, msg_list):
    # get first time
    # TODO how to get the first timestr
    # format_time = "2019-10-19 11:45:35.796"
    format_time = getFirstTime(name, 796)
    first_ts = str2timestamp(format_time)

    Index = findIndex(msg_list, first_ts)
    print ("firstIndex: %d", Index)
    if Index < 0:
        print "the data dont't match the video"
        return 0

    # creat a video window
    cv2.namedWindow(name, 0)
    cv2.setMouseCallback(name, MouseEvent)
    cv2.resizeWindow(name, 800, 600)
    # the signal of video
    cap = cv2.VideoCapture(video)
    # the number of frames
    frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    loop_flag = 0

    pos = 0
    cv2.createTrackbar('time', name, 0, frames, onTrackbarSlide)
    global flag
    global showflag

    if Index > 0 and flag == False:
        flag = True
        showflag = True
        thread.start_new_thread(startClient, (msg_list, Index,))
        thread.start_new_thread(showState, ())

    while 1:
        pos = cv2.getTrackbarPos('time', name)
        if loop_flag == pos:
            loop_flag = loop_flag + 1
            cv2.setTrackbarPos('time', name, loop_flag)
            #
            if Index == 0 and flag == False:
                mtime = cap.get(cv2.CAP_PROP_POS_MSEC)
                ts_current = first_ts + (long)(mtime * 1000L)
                Index = findIndex(msg_list, ts_current)
                if Index > 0 and flag == False:
                    flag = True
                    thread.start_new_thread(startClient, (msg_list, Index,))
        else:
            loop_flag = pos
            cv2.setTrackbarPos('time', name, loop_flag)
            cap.set(cv2.CAP_PROP_POS_FRAMES, pos)
            ret, img = cap.read()
            cv2.imshow(name, img)
            mtime = cap.get(cv2.CAP_PROP_POS_MSEC)
            #mtime = loop_flag * 33.3333
            print mtime
            #
            flag = False
            msleep(2)
            ts_current = first_ts + (long)(mtime * 1000L)
            Index = findIndex(msg_list, ts_current)
            if Index > 0 and flag == False:
                flag = True
                thread.start_new_thread(startClient, (msg_list, Index,))

        ret, img = cap.read()
        cv2.imshow(name, img)
        # key control
        t = cv2.waitKey(33) & 0xFF
        if t == ord('q'):
            flag = False
            break
        elif t == ord(' '):
            flag = False
            Index = 0
            cv2.waitKey(0)
        elif t == ord('r'):  #
            flag = False
            Index = 0
            msleep(2)

    flag = False
    showflag = False
    cap.release()
    cv2.destroyAllWindwows()


if __name__ == '__main__':
    # video = r"/home/lyy/Documents/data/1020/video/RoadData/TimeVideo_20191019_114535.mp4"
    # video = "TimeVideo_20191019_114535.mp4"
    video = "TimeVideo_20191212_224239.mp4"
    # data = r"/home/lyy/Documents/Simulator/1571456391943068.log"
    # data = "1571456391943068.log"
    data = "1576161742657744.log"
    name = re.split('/|\.', video)[-2]
    res = parseFile(data)
    # speed,steer
    state = [0, 0]
    flag = False
    showflag = True
    progress(name, video, res)
