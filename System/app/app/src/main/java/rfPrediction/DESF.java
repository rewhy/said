package rfPrediction;

public class DESF {
    public DESF(double[][] sensorData, double v) {
    }

    public static double[][] DESF_filter(double[][] data, double alph){
        // 数据为d行 n列，按特征（行）过滤
        int dataDim = data.length;
        int dataSize = data[0].length;
        if (dataDim == 7){
            dataDim  = 6;
        }

        for (int z1 = 0; z1 < dataDim; z1++){
            double xi_1 = data[z1][0];
            for (int z2 = 1; z2 < dataSize; z2++){
                if (data[z1][z2] >= xi_1){
                    data[z1][z2] = (1 - alph) * xi_1 + alph * data[z1][z2];
                }
                else {
                    data[z1][z2] = alph * xi_1 + (1 - alph) * data[z1][z2];
                }
                xi_1 = data[z1][z2];

            }
        }
        return data;
    }
}
