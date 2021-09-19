package cn.uprogrammer.sensordatacollect.dataRingbuffer;

import java.util.Arrays;

public class RingBuffer<T> {
    private final static int DEFAULT_SIZE  = 1024;//max: size - 1
    private Object[] buffer;
    private int head = 0;
    private int tail = 0;
    private int bufferSize;

    public RingBuffer(){
        this.bufferSize = DEFAULT_SIZE;
        this.buffer = new Object[bufferSize];
    }

    public RingBuffer(int initSize){
        this.bufferSize = initSize+1;
        this.buffer = new Object[bufferSize];
    }

    private Boolean isEmpty() {
        return head == tail;
    }

    //Full is (buffSize-1)
    private Boolean isFull() {
        return (tail + 1) % bufferSize == head;
    }

    public void clear(){
        Arrays.fill(buffer,null);
        this.head = 0;
        this.tail = 0;
    }

    public int getSize(){

        return (this.tail+bufferSize-this.head)%bufferSize;
    }

    public void add(final T element){
        buffer[tail] = element;
        if(isFull()){
            tail++;
            buffer[head] = null;
            head = (head + 1)%bufferSize;
        }
        else {
            tail++;
        }
        tail=tail%bufferSize;
    }

    public Object get(){
        if(isEmpty()){
            return null;
        }
        Object result = buffer[head];
        head = (head + 1) % bufferSize;
        return result;
    }

    public Object[] getQueue(){
        if(isEmpty()){
            return new Object[0];
        }
        int len = getSize();
        Object[] result = new Object[len];

        if(tail>=head){
            for(int i=0;i<len;i++){
                result[i] = buffer[head+i];
            }
        }else {
            for(int i=0;i<bufferSize-head;i++){
                result[i] = buffer[head+i];
            }
            for(int j=0;j<tail;j++){
                result[bufferSize-head+j]=buffer[j];
            }
        }
        return result;

    }


}
