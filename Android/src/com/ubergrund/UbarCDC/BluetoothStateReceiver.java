package com.ubergrund.UbarCDC;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

/**
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
