package com.ubergrund.UbarCDC;

import android.app.Service;
import android.bluetooth.*;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.*;
import android.util.Log;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.util.UUID;

/**
 * UbarCDC
 * Copyright (C) 2013 Tim Otto
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
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

    private MenuProvider menuProvider;

    private BluetoothDevice connectedDevice = null;
    private UUID sppUuid = null;
    private BluetoothSocket sppSocket = null;
    private boolean shutdown = false;
    private DataOutputStream out;
    private BluetoothA2dp a2dp = null;
    private SPPDriver sppDriver;

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate");
        super.onCreate();

        sppDriver = new SPPDriver();
        menuProvider = new MenuProvider(this);

        BluetoothAdapter.getDefaultAdapter().getProfileProxy(this, this, BluetoothProfile.A2DP);
        final IntentFilter filter = new IntentFilter();
        filter.addAction("com.android.music.metachanged");
        filter.addAction("com.android.music.playstatechanged");
        filter.addAction("com.android.music.playbackcomplete");
        registerReceiver(systemMusicReceiver, filter);
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        if (a2dp != null)
            BluetoothAdapter.getDefaultAdapter().closeProfileProxy(BluetoothProfile.A2DP, a2dp);
        unregisterReceiver(systemMusicReceiver);
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
                sppDriver.setOut(out);

                sppDriver.sendPing(0x7f);

                byte arg;
                while (true) {
                    byte cmd = in.readByte();

                    switch (cmd) {
                        case SPPDriver.MSG_PING:
                            Log.d(TAG, "Got Ping");
                            arg = in.readByte();
                            sppDriver.sendPong(arg);
                            break;

                        case SPPDriver.MSG_PONG:
                            Log.d(TAG, "Got Pong");
                            arg = in.readByte();
                            break;

                        case SPPDriver.MSG_DISCSELECT:
                            arg = in.readByte();
                            Log.d(TAG, "Got disc select ("+arg+")");

//                            PowerManager pm = (PowerManager) getSystemService(POWER_SERVICE);
//                            final PowerManager.WakeLock wl = pm.newWakeLock(PowerManager.FULL_WAKE_LOCK, TAG);
//                            wl.acquire(10000);

                            final Intent testIntent = new Intent(CDCEventReceiver.ACTION_EVENT);
                            testIntent.putExtra(CDCEventReceiver.EXTRA_BUTTON, String.valueOf(arg));
                            sendBroadcast(testIntent);

                            break;

                        case SPPDriver.MSG_GETDIRECTORY:
                            long indexId = 0;
                            indexId |= (in.readByte() << 24);
                            indexId |= (in.readByte() << 16);
                            indexId |= (in.readByte() << 8);
                            indexId |= (in.readByte());

                            int offset = 0;
                            offset |= in.readByte() << 8;
                            offset |= in.readByte();

                            int length = 0;
                            length |= in.readByte() << 8;
                            length |= in.readByte();

                            Log.d(TAG, "Received directory request, menuId ["+indexId+"], offset ["+offset+"], length ["+length+"]");
                            final Message m = txHandler.obtainMessage(MSG_SEND_DIRECTORY);
                            m.obj = indexId;
                            m.arg1 = offset;
                            m.arg2 = length;
                            txHandler.sendMessage(m);

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

    private static final int MSG_SEND_TRACKINFO = 1;
    private static final int MSG_SEND_DIRECTORY = 2;

    private StatusInfo lastInfo = null;

    private Handler txHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            try {
                switch (msg.what) {
                    case MSG_SEND_TRACKINFO:
                        sppDriver.sendTrackInfo(lastInfo);
                        break;

                    case MSG_SEND_DIRECTORY:

                        long indexId = (Long)msg.obj;

                        final MenuProvider.Menu menu = menuProvider.get(indexId);
                        if (menu == null) {
                            Log.w(TAG, "failed to obtain requested menu ["+indexId+"]");
                            return;
                        }

                        int offset = msg.arg1;
                        int maxLength = msg.arg2;
                        int length = Math.min(maxLength, menu.length() - offset);
                        if (length <= 0) {
                            Log.w(TAG, "requested menu ["+indexId+"] is empty, offset ["+offset+"], maxLength ["+maxLength+"], length ["+menu.length()+"]");
                            return;
                        }

                        sppDriver.sendDirectory(menu, offset, length);
                        break;
                }
            } catch (IOException e) {
                Log.e(TAG, "failed to send data to UbarCDC device", e);
                // TODO close? restart?
            }
        }
    };

    private BroadcastReceiver systemMusicReceiver = new BroadcastReceiver() {

        private final String TAG = "UbarCDC/MusicUpdateReceiver";

        @Override
        public final void onReceive(Context context, Intent intent) {

            if (out==null){
                Log.d(TAG, "no output stream");
                return;
            }

            String action = intent.getAction();
            Bundle bundle = intent.getExtras();

            if (action == null || bundle == null) {
                Log.e(TAG, "Got null action or null bundle");
                return;
            }

            try {
                parseIntent(bundle); // might throw
            } catch (IllegalArgumentException e) {
                Log.i(TAG, "Got a bad track, ignoring it (" + e.getMessage() + ")");
            }

        }

        protected void parseIntent(Bundle bundle) {

            if (bundle.containsKey("playstate")) {
//                boolean oldPlaystate = playing;
//                playing = bundle.getBoolean("playstate");
//                playingKnown = true;
//                Log.d(TAG, "playstate new [" + playing + "] old [" + oldPlaystate + "]");
//                if (playing != oldPlaystate) {
//                    callbackHandler.sendEmptyMessage(MSG_PLAYING_CHANGED);
//                }
            }

            final StatusInfo newInfo = new StatusInfo(
                    bundle.getString("track"),
                    bundle.getString("artist"),
                    bundle.getString("album")
            );
            if (newInfo.equals(lastInfo))return;
            if (out != null) {
                lastInfo = newInfo;
                Log.d(TAG, "newInfo ["+newInfo+"]");
                txHandler.sendEmptyMessage(MSG_SEND_TRACKINFO);
            }
        }
    };

}
