package com.ubergrund.UbarCDC;

import android.app.Service;
import android.bluetooth.*;
import android.content.Intent;
import android.os.IBinder;
import android.os.ParcelUuid;
import android.util.Log;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.util.UUID;

/**
 * Created with IntelliJ IDEA.
 * User: Tim
 * Date: 8/31/13
 * Time: 8:32 PM
 *
 * This one works:
 *  adb shell am start --activity-clear-task -a com.google.android.music.PLAY -e storeId Bqtgsu6zuroqn3r3bza3ltaissy
 *
 * Now I need to browse the MusicContent provider for storeId and album_name
 */
public class UbarCDCService extends Service implements BluetoothProfile.ServiceListener {

    private static final String TAG = "UbarCDC/UbarCDCService";

    private BluetoothDevice connectedDevice = null;
    private UUID sppUuid = null;
    private BluetoothSocket sppSocket = null;
    private boolean shutdown = false;
    private DataOutputStream out;
    private BluetoothA2dp a2dp = null;

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate");
        super.onCreate();
        BluetoothAdapter.getDefaultAdapter().getProfileProxy(this, this, BluetoothProfile.A2DP);
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        if (a2dp != null)
            BluetoothAdapter.getDefaultAdapter().closeProfileProxy(BluetoothProfile.A2DP, a2dp);
        super.onDestroy();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
//        Log.d(TAG, "onStartCommand("+intent+")");
        int result = START_NOT_STICKY;
        if (intent == null)return result;

        final BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);

        if (BluetoothDevice.ACTION_ACL_CONNECTED.equals(intent.getAction())) {

            if (sppSocket != null) {
                Log.d(TAG, "already connected!");
                return START_NOT_STICKY;
            }

            final BluetoothAdapter ba = BluetoothAdapter.getDefaultAdapter();
            final ParcelUuid[] uuids = device.getUuids();

//            Log.d(TAG, "Found ["+uuids.length+"] UUIDs for device ["+device.getName()+"]");
            boolean a2dp = false, spp = false, avrcp = false;
            for(ParcelUuid puuid : uuids) {
                final UUID uuid = puuid.getUuid();
//                Log.d(TAG, "UUID ["+ uuid +"]");
                if (uuid.getMostSignificantBits() >> 32 == 0x1101) {
                    spp = true;
                    sppUuid = uuid;
                } else if (uuid.getMostSignificantBits() >> 32 == 0x110b)
                    a2dp = true;
                else if (uuid.getMostSignificantBits() >> 32 == 0x110e)
                    avrcp = true;
            }

            if (a2dp && spp && avrcp) {
                // this is a candidate!
                connectedDevice = device;

                // attempt connection asynchronous
                new ConnectThread().start();

                result = START_STICKY | START_FLAG_REDELIVERY;
            }
        } else if (BluetoothDevice.ACTION_ACL_DISCONNECTED.equals(intent.getAction())) {
            if (sppSocket != null) {
                if (connectedDevice.getAddress().equals(device.getAddress())) {
                    shutdown = true;
                    try {
                        sppSocket.close();
                    } catch (IOException ignored) {
                    }
                    stopSelf();
                }
            } else stopSelf();
        } else if ("android.bluetooth.a2dp.profile.action.CONNECTION_STATE_CHANGED".equals(intent.getAction())) {
            if (sppSocket != null) {
                final int state = intent.getIntExtra(BluetoothProfile.EXTRA_STATE, -1);
                final String da = device.getAddress();
                final String ca = connectedDevice.getAddress();

//                Log.d(TAG, "a2dp state change: "+state+"/"+da+"/"+ca);
                if (state == 0 && ca.equals(da)) {
                    shutdown = true;
                    try {
                        sppSocket.close();
                    } catch (IOException ignored) {
                    }
                    stopSelf();
                }
            } else stopSelf();
        }

        return result;
    }

    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onServiceConnected(int profile, BluetoothProfile proxy) {
        if (profile == BluetoothProfile.A2DP) {
            a2dp = (BluetoothA2dp) proxy;
        }
    }

    @Override
    public void onServiceDisconnected(int profile) {
        if (profile == BluetoothProfile.A2DP)
            a2dp = null;
    }

    private class ConnectThread extends Thread {
        @Override
        public void run() {
            try {
                sppSocket = connectedDevice.createRfcommSocketToServiceRecord(sppUuid);
                sppSocket.connect();
                Log.d(TAG, "connected!");
                final DataInputStream in = new DataInputStream(sppSocket.getInputStream());
                out = new DataOutputStream(sppSocket.getOutputStream());

                out.writeByte(MSG_PING);
                out.writeByte(0x7f);
                out.flush();

                byte arg;
                while (true) {
                    byte cmd = in.readByte();

                    switch (cmd) {
                        case MSG_PING:
                            Log.d(TAG, "Got Ping");
                            arg = in.readByte();
                            out.writeByte(MSG_PONG);
                            out.writeByte(arg);
                            out.flush();
                            break;

                        case MSG_PONG:
                            Log.d(TAG, "Got Pong");
                            arg = in.readByte();
                            break;

                        case MSG_DISCSELECT:
                            arg = in.readByte();
                            Log.d(TAG, "Got disc select ("+arg+")");

//                            PowerManager pm = (PowerManager) getSystemService(POWER_SERVICE);
//                            final PowerManager.WakeLock wl = pm.newWakeLock(PowerManager.FULL_WAKE_LOCK, TAG);
//                            wl.acquire(10000);

                            final Intent testIntent = new Intent("com.ubergrund.ubarcdc.CDC_EVENT");
                            testIntent.putExtra(CDCEventReceiver.EXTRA_BUTTON, String.valueOf(arg));
                            sendBroadcast(testIntent);

                            break;
                    }
                }
            } catch (IOException e) {
                if(!shutdown)
                    Log.e(TAG, "SPP connection error: " + e.toString());
            }
            sppUuid = null;
            connectedDevice = null;
            sppSocket = null;

            stopSelf();
        }
    }

    private static final int MSG_PING = 0x10;
    private static final int MSG_PONG = 0x11;
    private static final int MSG_DISCSELECT = 0x20;
    private static final int MSG_TRACKINFO = 0x80;
}
