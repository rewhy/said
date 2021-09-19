package rfPrediction;

import android.util.Log;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import weka.classifiers.Classifier;
import weka.core.Attribute;
import weka.core.DenseInstance;
import weka.core.Instances;

public  class getCurrentStatus {
    final static List<String> Straightclasses = new ArrayList<String>() {
        {

            add("Driving Event"); // cls nr 0
            add("Straight Forward Driving"); // cls nr 1

        }
    };
    final static List<String> Eventclasses = new ArrayList<String>() {
        {

            add("Left Turn"); // 2
            add("Right Turn");  // 3
            add("Left Lane Change");    //4
            add("Right Lane Change");   // 5
            add("Left U Turn"); // 6
            add("Right U Turn");    //7
            add("Braking");     //8
            add("STOP");    // 9

        }
    };

    public static int identifyDrivingEvent(final double [] featureData, Classifier multiClassClassifier){
        int [] label = new int[]{2, 3, 4, 5, 6, 7};
        if (featureData == null){
            return -1;
        }
        ArrayList<Attribute> attributeList = new ArrayList<Attribute>(2) {
            {
                for (int z1 = 0; z1 < featureData.length; z1++){
                    add(new Attribute("X" + (z1 + 1)));
                }
                Attribute attributeClass = new Attribute("@@class@@", Eventclasses);
                add(attributeClass);
            }
        };
        Instances dataUnpredicted = new Instances("TestInstances",
                attributeList, 1);
        dataUnpredicted.setClassIndex(dataUnpredicted.numAttributes() - 1);
        double result = -1 ;
        DenseInstance newInstance = new DenseInstance(featureData.length) {
            {
                for (int z1 = 0; z1 < featureData.length; z1++) {
                    setValue(z1, featureData[z1]);
                }
//                setValue(featureData.length, 1);
            }
        };
        if(multiClassClassifier==null){
//            Toast.makeText(this, "Model not loaded!", Toast.LENGTH_SHORT).show();
            return -2;
        }
        try {
            newInstance.setDataset(dataUnpredicted);
            result = multiClassClassifier.classifyInstance(newInstance);
        } catch (Exception e) {
            e.printStackTrace();
        }
        int ind = (int)result;
//        Log.d("DataSize", "RFLabel result: "+ result + ", ind: "+ind+", label:" + label[ind]);
        return ind;

    }


    public class Sample {
        public int nr;
        public int label;
        public double [] features;

        public Sample(int _nr, int _label, double[] _features) {
            this.nr = _nr;
            this.label = _label;
            this.features = _features;
        }

        @Override
        public String toString() {
            return "Nr " +
                    nr +
                    ", cls " + label +
                    ", feat: " + Arrays.toString(features);
        }
    }
}
