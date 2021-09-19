package cn.uprogrammer.sensordatacollect;

import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.FileChannel;

/**
 * Created by caesar on 7/15/15.
 */
public class Utils {

    private static String TAG = "CANAPP";



    public static void parsetoString(byte[] buf) {
        long ts = 0;
        float res = 0.0f;
        int sid = 0;
        int pid = 0;

        int[] as = new int[5];
        ByteBuffer.wrap(buf).order(ByteOrder.LITTLE_ENDIAN).asIntBuffer().get(as);
        sid = as[0];
        pid = as[1];
        res = (float) as[2] / 1000.0f;
        ts = ((long) as[3] << 32) | (long) as[4];
        //Log.d(TAG, String.format("IV-Context: 0x%08x 0x%08x %f 0x%x", sid, pid, res, ts));

        // Log.d(TAG, OBDPids.obd_cxt_str[pid]);
        // Log.d(TAG, String.format("IV-Context@%d: %s is %f", ts, OBDPids.obd_cxt_str[pid], res));
    }

    public static String getByteStringHex(byte[] data) {
        char[] DIGITS_UPPER = {'0', '1', '2', '3', '4', '5', '6', '7', '8',
                '9', 'A', 'B', 'C', 'D', 'E', 'F'};
        StringBuffer ret = new StringBuffer(64);
        for (int i = 0; i < data.length; i++) {
            byte hf = (byte) ((data[i] >> 4) & 0x0F);
            byte lf = (byte) (data[i] & 0x0F);
            ret.append(DIGITS_UPPER[hf]);
            ret.append(DIGITS_UPPER[lf]);
            if ((i + 1) < data.length)
                ret.append(",");
        }
        return ret.toString();
    }

    public static String getSingleByteStringHex(byte data) {
        char[] DIGITS_UPPER = {'0', '1', '2', '3', '4', '5', '6', '7', '8',
                '9', 'A', 'B', 'C', 'D', 'E', 'F'};
        StringBuffer ret = new StringBuffer(64);
        byte hf = (byte) ((data >> 4) & 0x0F);
        byte lf = (byte) (data & 0x0F);
        ret.append(DIGITS_UPPER[hf]);
        ret.append(DIGITS_UPPER[lf]);

        return ret.toString();
    }



    public static int min(int a, int b) {
        return (a > b) ? b : a;
    }

    public static void nioTransferCopy(File source, File target) {
        FileChannel in = null;
        FileChannel out = null;
        FileInputStream inStream = null;
        FileOutputStream outStream = null;
        try {
            inStream = new FileInputStream(source);
            outStream = new FileOutputStream(target);
            in = inStream.getChannel();
            out = outStream.getChannel();
            in.transferTo(0, in.size(), out);
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            // close(inStream);
            // close(in);
            // close(outStream);
            // close(out);
        }
    }

    public static String bytesToHexString(byte[] src) {
        StringBuilder stringBuilder = new StringBuilder();
        if (src == null || src.length <= 0) {
            return null;
        }
        for (int i = 0; i < src.length; i++) {
            int v = src[i] & 0xFF;
            String hv = Integer.toHexString(v);
            if (hv.length() < 2) {
                stringBuilder.append(0);
            }
            stringBuilder.append(hv);
        }
        return stringBuilder.toString();
    }

    public static String printHexString(byte[] b) {
        String a = "";
        for (int i = 0; i < b.length; i++) {
            String hex = Integer.toHexString(b[i] & 0xFF);
            if (hex.length() == 1) {
                hex = '0' + hex;
            }
            a = a + hex;
        }
        return a;
    }


    public static short[] bytesToShort(byte[] bytes) {
        if (bytes == null) {
            return null;
        }
        short[] shorts = new short[bytes.length / 2];
        ByteBuffer.wrap(bytes).order(ByteOrder.LITTLE_ENDIAN).asShortBuffer().get(shorts);
        return shorts;
    }

    public static byte[] shortToBytes(short[] shorts) {
        if (shorts == null) {
            return null;
        }
        byte[] bytes = new byte[shorts.length * 2];
        ByteBuffer.wrap(bytes).order(ByteOrder.LITTLE_ENDIAN).asShortBuffer().put(shorts);

        return bytes;
    }


    public static byte[] str2Bcd(String asc) {
        int len = asc.length();
        int mod = len % 2;
        if (mod != 0) {
            asc = "0" + asc;
            len = asc.length();
        }
        byte[] abt = new byte[len];
        if (len >= 2) {
            len = len / 2;
        }
        byte[] bbt = new byte[len];
        abt = asc.getBytes();
        int j, k;
        for (int p = 0; p < asc.length() / 2; p++) {
            if ((abt[2 * p] >= '0') && (abt[2 * p] <= '9')) {
                j = abt[2 * p] - '0';
            } else if ((abt[2 * p] >= 'a') && (abt[2 * p] <= 'z')) {
                j = abt[2 * p] - 'a' + 0x0a;
            } else {
                j = abt[2 * p] - 'A' + 0x0a;
            }
            if ((abt[2 * p + 1] >= '0') && (abt[2 * p + 1] <= '9')) {
                k = abt[2 * p + 1] - '0';
            } else if ((abt[2 * p + 1] >= 'a') && (abt[2 * p + 1] <= 'z')) {
                k = abt[2 * p + 1] - 'a' + 0x0a;
            } else {
                k = abt[2 * p + 1] - 'A' + 0x0a;
            }
            int a = (j << 4) + k;
            byte b = (byte) a;
            bbt[p] = b;
//            System.out.format("%02X\n", bbt[p]);
        }
        return bbt;
    }
}
