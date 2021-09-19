package dataProcess;

import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.io.InputStreamReader;

public class transferTestdata {

    public static BufferedReader transferTestdata(double[] featuredata){

        String s = "0 ";
        int z2;
        for (int z1 = 0; z1 < featuredata.length - 2; z1 ++){
            z2 = z1 + 1;

            s += String.valueOf(z2) + ':' + String.valueOf(featuredata[z1]) + ' ';
        }

        byte[] by = s.getBytes();
        InputStream is = new ByteArrayInputStream(by);
        InputStreamReader isr = new InputStreamReader(is);
        BufferedReader reader = new BufferedReader(isr);
        return reader;
    }
}
