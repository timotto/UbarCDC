package com.ubergrund.UbarCDC;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

/**
 * Receives broadcaast intents from the Bluetooth subsystem
 *
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
 */
public class BluetoothStateReceiver extends BroadcastReceiver {

    private static final String TAG = "UbarCDC/BluetoothStateReceiver";

    public void onReceive(Context context, Intent intent) {
//        Log.d(TAG, "Intent {"+intent+"}");
        final BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);

        final Intent serviceIntent = new Intent(context, UbarCDCService.class);
        serviceIntent.setAction(intent.getAction());
        serviceIntent.putExtra(BluetoothDevice.EXTRA_DEVICE, device);

        if ("android.bluetooth.a2dp.profile.action.CONNECTION_STATE_CHANGED".equals(intent.getAction())) {
            serviceIntent.putExtra(BluetoothProfile.EXTRA_STATE, intent.getIntExtra(BluetoothProfile.EXTRA_STATE, -1));
            serviceIntent.putExtra(BluetoothProfile.EXTRA_PREVIOUS_STATE, intent.getIntExtra(BluetoothProfile.EXTRA_PREVIOUS_STATE, -1));
        }

        context.startService(serviceIntent);
    }
}
