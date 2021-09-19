# -*- coding: utf-8 -*-
"""
Created on Thu Mar 12 15:25:43 2020

@author: Hammond
"""
import os 
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from sklearn.preprocessing import MinMaxScaler
from keras.models import Sequential
from keras.layers import LSTM, Dense, Activation,Embedding
from keras.utils import to_categorical

def loadtrain():
    #打开OBD文件中的speed和转角
    train_steerfilename='uds_09251000开发SkodaDT_steerAngle.txt'
    train_speedfilename='uds_09251000开发SkodaDT_vehicleSpeed.txt'
    with open(train_steerfilename,'r') as fo:
        subtrainMat=fo.readlines()
        subtrainMat.pop(0)
        train_datax=[]
        train_label=[]
        for i in range(len(subtrainMat)):
            train_datax.append(float(subtrainMat[i].split(',')[0]))
            train_label.append(float(subtrainMat[i].split(',')[2]))
        fo.close()
    with open(train_speedfilename,'r') as fp:
        subtrainMat=fp.readlines()
        subtrainMat.pop(0)
        train_datay=[]
        for i in range(len(subtrainMat)):
            train_datay.append(float(subtrainMat[i].split(',')[0]))
        fo.close()
    train_data=[]
    for i in range(len(train_datax)):
        train_data.append([float(train_datax[i]),float(train_datay[i])])
    sequence_length=10
    train_x=[]
    train_y=[]
    for i in range(len(train_data)-sequence_length-1):
        train_x.append(train_data[i: i + sequence_length + 1])
        train_y.append(train_label[i: i + sequence_length + 1])
    reshaped_datax=np.array(train_x).astype('float64')
    reshaped_datay=np.array(train_y).astype('float64')   
    x = reshaped_datax[:, :-1]
    y = reshaped_datay[:, -1]
    y_new=to_categorical(y)
    return x,y_new


def build_model():
    # input_dim是输入的train_x的最后一个维度，train_x的维度为(n_samples, time_steps, input_dim)
    model = Sequential()
    model.add(LSTM(input_dim=2, output_dim=20, return_sequences=True))
    print(model.layers)
    model.add(LSTM(1, return_sequences=False))
    model.add(Dense(output_dim=11))
    model.add(Activation('linear'))
    
    model.compile(loss='categorical_crossentropy', optimizer='adam', metrics=['accuracy'])
    return model
def train(train_dataarray, train_labelarray):
    model = build_model()
    model.fit(train_dataarray, train_labelarray, batch_size=8, nb_epoch=30, validation_split=0.1)
    model.save('0326.h5')

x,y=loadtrain()
build_model()
train(x,y)

            
