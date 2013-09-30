package com.ubergrund.UbarCDC;

import android.app.Activity;
import android.app.KeyguardManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.IBinder;
import android.os.PowerManager;
import android.util.Log;

/**
 * Starts playlists and albums in the Google Music App
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
 * Date: 9/23/13
 * Time: 4:55 PM
 */
public class GoogleMusicHelperService extends Service {

    private static final String TAG = "UbarCDC/GoogleMusicHelperService";

    public static final String ACTION_FINISH = "finish";
    public static final String EXTRA_LOCK = "lock";

    private KeyguardManager.KeyguardLock kgmLock = null;
    private PowerManager.WakeLock wakeLock;
    private KeyguardManager keyguardManager;

    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "onCreate()");
        // turn on phone
        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
        wakeLock = pm.newWakeLock(
                PowerManager.ACQUIRE_CAUSES_WAKEUP | PowerManager.SCREEN_BRIGHT_WAKE_LOCK,
                "startMusicActivity");

        keyguardManager = (KeyguardManager)getSystemService(Activity.KEYGUARD_SERVICE);
        kgmLock = keyguardManager.newKeyguardLock("UbarCDC");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy()");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (ACTION_FINISH.equals(intent.getAction())) {
            if (intent.getBooleanExtra(EXTRA_LOCK, false)) {
                if (kgmLock == null) {
                    Log.w(TAG, "unable to re-enable keyguard, kgmLock is null");
                } else {
                    kgmLock.reenableKeyguard();
                }
            }
            Log.d(TAG, "releasing wakeLock");
            wakeLock.release();
            stopSelf();
        }else handleGMIntent(intent.getData());

        return START_NOT_STICKY;
    }

    private void handleGMIntent(Uri itemUri) {

        wakeLock.acquire();

        // arm the AccessibilityService
        Intent armIntent = new Intent("com.ubergrund.ubarcdc.CDC_GM_HACK");
        armIntent.putExtra(UbarAccService.EXTRA_ACTION, UbarAccService.VALUE_PLAY);

        // intent to open Google Music with the selected media, unfortunately does not play just show
        Intent actionIntent = new Intent();
        actionIntent.setClassName("com.google.android.music", "com.google.android.music.ui.SearchActivity");
        actionIntent.setAction("android.intent.action.SEARCH_RESULT");
        actionIntent.setData(itemUri);
        actionIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK | Intent.FLAG_ACTIVITY_NEW_TASK);

        // need to unlock the phone, so Google Music Activity is displayed for the
        // AccessibilityService to look for the buttons and actually press them
        if (keyguardManager.isKeyguardLocked()) {
            Log.d(TAG, "disabling keyguard");
            // enable the keyguard again
            armIntent.putExtra(UbarAccService.EXTRA_GEYGUARD_ENABLE, true);
            kgmLock.disableKeyguard();
        }

        sendBroadcast(armIntent);
        startActivity(actionIntent);
    }

}
