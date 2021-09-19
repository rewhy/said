# -*- coding: utf-8 -*-
"""
Created on Thu Mar 19 23:04:49 2020

@author: Hammond
"""

import os 
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from sklearn.preprocessing import MinMaxScaler
from keras.models import Sequential
from keras.models import load_model
from keras.layers import LSTM, Dense, Activation
from keras.utils import to_categorical
def loadtest():
    #打开OBD文件中的speed和转角
    test_steerfilename='uds_09251437开发SkodaDT_steerAngle.txt'
    test_speedfilename='uds_09251437开发SkodaDT_vehicleSpeed.txt'
    with open(test_steerfilename,'r') as fo:
        subtestMat=fo.readlines()
        subtestMat.pop(0)
        test_datax=[]
        test_label=[]
        for i in range(len(subtestMat)):
            test_datax.append(float(subtestMat[i].split(',')[0]))
            test_label.append(float(subtestMat[i].split(',')[2]))
        fo.close()
    with open(test_speedfilename,'r') as fp:
        subtestMat=fp.readlines()
        subtestMat.pop(0)
        test_datay=[]
        for i in range(len(subtestMat)):
            test_datay.append(float(subtestMat[i].split(',')[0]))
        fo.close()
    test_data=[]
    for i in range(len(test_datax)):
        test_data.append([float(test_datax[i]),float(test_datay[i])])
    sequence_length=10
    test_x=[]
    test_y=[]
    for i in range(len(test_data)-sequence_length-1):
        test_x.append(test_data[i: i + sequence_length + 1])
        test_y.append(test_label[i: i + sequence_length + 1])
    reshaped_datax=np.array(test_x).astype('float64')
    reshaped_datay=np.array(test_y).astype('float64')   
    x = reshaped_datax[:, :-1]
    y = reshaped_datay[:, -1]
    return x,y
def test(x,y):
    out=model.predict(x)
    Y_label=[]
    out1=out.tolist()
    for i in range(len(out1)):
        if out1[i].index(max(out1[i])) in range(0,10):
            Y_label.append(out1[i].index(max(out1[i]))+1)
        if out1[i].index(max(out1[i])) in range(0,10) == 10:
            Y_label.append(-1)
    plt.plot(y, 'g:')
    plt.plot(Y_label, 'r-')
    plt.show()
    return Y_label
model_name='C:/Users/73608/lunwen/DMS_HKPU/系统化整理 - 副本/0925开发_skoda/09251000/0326.h5'
model=load_model(model_name)
(x,y)=loadtest()
out=test(x,y)
