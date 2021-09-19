package cn.uprogrammer.sensordatacollect;

public class KalmanFilter {

    /**
     * Kalman Filter
     */

    private double predict;
    private double current;
    private double estimate;
    private double pdelt;
    private double mdelt;
    private double Gauss;
    private double kalmanGain;
    private final static double Q = 0.00001;
    private final static double R = 0.1;

    public void initial() {
        pdelt = 4;    //系统测量误差
        mdelt = 3;
    }

    public double KalmanFilter(double oldValue, double value) {
        //(1)第一个估计值
        predict = oldValue;
        current = value;
        //(2)高斯噪声方差
        Gauss = Math.sqrt(pdelt * pdelt + mdelt * mdelt) + Q;
        //(3)估计方差
        kalmanGain = Math.sqrt((Gauss * Gauss) / (Gauss * Gauss + pdelt * pdelt)) + R;
        //(4)估计值
        estimate =  (kalmanGain * (current - predict) + predict);
        //(5)新的估计方差
        mdelt = Math.sqrt((1 - kalmanGain) * Gauss * Gauss);

        return estimate;
    }
}