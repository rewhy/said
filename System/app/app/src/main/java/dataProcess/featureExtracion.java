package dataProcess;


import java.util.Arrays;

public class featureExtracion {

    public static double[] getBasicFeature(double[][] data){

        double[] featureValue = new double [18];
        double preMean = 0;
        double postMean = 0;
        for (int z1 = 0; z1 < data[0].length; z1++){

            if (z1 < Math.floor(data[0].length/2)){
                preMean += data[0][z1];
            }else{
                postMean += data[0][z1];
            }

        }
        featureValue[10] = preMean/(Math.floor(data[0].length/2));
        featureValue[11] = postMean/(data[0].length - (Math.floor(data[0].length/2)));

        // 排序
        for (int z = 0; z < data.length - 1; z++){
            Arrays.parallelSort(data[z]);
        }

        featureValue[0] = data[0][data[0].length-1] - data[0][0];   // accx range
        featureValue[1] = data[1][data[1].length-1] - data[1][0];   // accy range

        featureValue[6] = Arrays.stream(data[0]).average().orElse(0);   // mean acc x
        featureValue[7] = Arrays.stream(data[0]).average().orElse(0);   // mean acc y
        featureValue[8] = Arrays.stream(data[0]).average().orElse(0);   // mean ori x
        featureValue[9] = Arrays.stream(data[0]).average().orElse(0);   // mean ori y

        for (int z1 = 0; z1 < data.length-1; z1++){
            double variance = 0;
            for(int z2 = 0; z2 < data[z1].length; z2++ ){
                variance += (data[z1][z2] - featureValue[z1+6])*(data[z1][z2] - featureValue[z1+6]);
            }
            featureValue[z1+2] = Math.sqrt(variance/data[z1].length) ;
        }

        featureValue[12] = data[2][data[2].length -1];  // ori x max
        featureValue[13] = data[3][data[3].length -1];  // ori y max
        featureValue[14] = data[1][0];  // acc y min
        featureValue[15] = data[4][data[4].length -1] - data[4][0]; //time duration

        featureValue[16] = Math.abs(data[0][data[0].length-1]) ;   // acc x magnitude 用于判断是否进入异常状态
        featureValue[17] = Math.abs(data[1][data[1].length-1]) ; // acc y magnitude 用于判断是否进入异常状态


        return featureValue;

    }

}
