package cn.uprogrammer.sensordatacollect.helper;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;

import java.io.IOException;
import java.lang.reflect.Method;
import java.util.UUID;

public class BluetoothManager {

    private final static String TAG = BluetoothManager.class.getSimpleName();

    public static BluetoothSocket socket = null;

    public static void connectTo(final BluetoothDevice device, final UUID uuid) throws IOException {
        socket = null;
        BluetoothSocket fallbackSocket = null;

        try {
            socket = device.createRfcommSocketToServiceRecord(uuid);
            socket.connect();
        } catch (Exception e1) {
            android.util.Log.e(TAG, "Error when connecting socket!");

            try {
                Class<?> clazz = socket.getRemoteDevice().getClass();
                Class<?>[] paramTypes = new Class<?>[] { Integer.TYPE };
                Method m = clazz.getMethod("createRfcommSocket", paramTypes);

                Object[] params = new Object[] { Integer.valueOf(1) };
                fallbackSocket = (BluetoothSocket) m.invoke(socket.getRemoteDevice(), params);
                fallbackSocket.connect();

                socket = fallbackSocket;
            } catch (Exception e2) {
                android.util.Log.e(TAG, "Error when fallback socket connection!", e2);
                socket = null;
                throw new IOException(e1.getMessage() + "\n" + e2.getMessage());
            }
        }
    }
}
