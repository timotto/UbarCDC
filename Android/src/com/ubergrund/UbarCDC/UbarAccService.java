package com.ubergrund.UbarCDC;

import android.accessibilityservice.AccessibilityService;
import android.accessibilityservice.AccessibilityServiceInfo;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.util.Log;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

/**
 * Uses the Android Accessibility Service to "remote-control" the
 * Google Music App, as there seems no way to open an album and
 * also start playing.
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
 * Date: 9/15/13
 * Time: 1:53 PM
 */
public class UbarAccService extends AccessibilityService {

    private static final String TAG = "UbarCDC/UbarAccService";
    /**
     * ACTION!!
     */
    public static final String EXTRA_GEYGUARD_ENABLE = "keyguard_enable";
    public static final String EXTRA_ACTION = "action";
    public static final String VALUE_PLAY = "play";
    public static final String VALUE_RADIO = "radio";
    public static final String VALUE_SHUFFLE = "shuffle";

    private int inspectorState = 0;
    private AccessibilityNodeInfo playView = null;
    private AccessibilityNodeInfo radioView = null;
    private AccessibilityNodeInfo shuffleView = null;
    private AccessibilityNodeInfo menuView = null;

    private boolean armed = false;
    private String action = null;
    private boolean reEnableKeyguard = false;

    private void onButtonsFound() {
        Log.d(TAG, "onButtonsFound()");
        armed=false;
        if (VALUE_PLAY.equals(action))
            playView.performAction(AccessibilityNodeInfo.ACTION_CLICK);
        else if (VALUE_RADIO.equals(action))
            radioView.performAction(AccessibilityNodeInfo.ACTION_CLICK);
        else if (VALUE_SHUFFLE.equals(action))
            shuffleView.performAction(AccessibilityNodeInfo.ACTION_CLICK);

        performGlobalAction(AccessibilityService.GLOBAL_ACTION_BACK);

        final Intent intent = new Intent(UbarAccService.this, GoogleMusicHelperService.class);
        intent.setAction(GoogleMusicHelperService.ACTION_FINISH);
        if (reEnableKeyguard) {
            Log.d(TAG, "going to re-enable keyguard");
            intent.putExtra(GoogleMusicHelperService.EXTRA_LOCK, true);
        }
        startService(intent);

        playView.recycle();
        radioView.recycle();
        shuffleView.recycle();
        playView=null;
        radioView=null;
        shuffleView=null;
        Log.d(TAG, "done with onButtonsFound()");
    }

    private void onMenuFound() {
        Log.d(TAG, "onMenuFound()");
        menuView.performAction(AccessibilityNodeInfo.ACTION_CLICK);
    }

    @Override
    public void onCreate() {
        super.onCreate();
        registerReceiver(actionReceiver, new IntentFilter("com.ubergrund.ubarcdc.CDC_GM_HACK"));
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        unregisterReceiver(actionReceiver);
    }

    private final BroadcastReceiver actionReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if(intent==null)return;
            action = intent.getStringExtra(EXTRA_ACTION);
            reEnableKeyguard = intent.getBooleanExtra(EXTRA_GEYGUARD_ENABLE, false);
            armed = (action!=null);
            Log.d(TAG, armed?"Armed!"+(reEnableKeyguard?" + re-enable":""):"Disarmed!");
        }
    };

    @Override
    public void onAccessibilityEvent(AccessibilityEvent event) {
        if(!armed)return;
        if (event==null)return;
        try {
//            Log.d(TAG, "event ["+event+"]");
            AccessibilityNodeInfo source = event.getSource();
            AccessibilityNodeInfo parent = source.getParent();
            AccessibilityNodeInfo root = null;

//            Log.d(TAG, "source [" + source + "]");
            while (parent != null) {
    //            Log.d(TAG, "parent [" + parent + "]");
                source.recycle();
                source = parent;
                parent = source.getParent();
            }

            root = source;
//            Log.d(TAG, "root [" + root + "]");
            resetInspector();
            inspectNode(root);
        } catch (NullPointerException ignored) {
//            Log.e(TAG, "Error", e);
        }
    }

    private void resetInspector() {
        inspectorState=0;
    }

    private static final String RES_ID_ALBUM_ART = "com.google.android.music:id/album_art";
    private static final String RES_ID_PIN_BUTTON = "com.google.android.music:id/pin_button";
    private static final String RES_ID_OVERFLOW = "com.google.android.music:id/overflow";
    private static final String RES_ID_TEXT1 = "android:id/text1";
    private static final String RES_ID_POPUPLIST = "com.google.android.music:id/popup_list";
    private static final String RES_ID_PLAY_RADIO = "com.google.android.music:id/play_radio";
    private static final String RES_ID_SHUFFLE = "com.google.android.music:id/shuffle";

    private boolean omgwt(AccessibilityNodeInfo info) {
        final String resName = info.getViewIdResourceName();
        switch (inspectorState) {
            case 0:
                if (RES_ID_ALBUM_ART.equals(resName)) {
                    if (playView!=null)playView.recycle();
                    playView = info;
                    inspectorState=1;
                    return true;
                }
                if (RES_ID_POPUPLIST.equals(resName)) {
                    inspectorState = 3;
                    return false;
                }
                return false;

            case 1:
                if (RES_ID_PIN_BUTTON.equals(resName)) {
                    inspectorState = 2;
                    return false;
                }

                if (RES_ID_PLAY_RADIO.equals(resName)) {
                    if (radioView!=null)radioView.recycle();
                    radioView = info;
                    inspectorState = 5;
                    return true;
                }
                break;

            case 2:
                if (RES_ID_OVERFLOW.equals(resName)) {
                    if (menuView!=null)menuView.recycle();
                    menuView = info;
                    if (VALUE_PLAY.equals(action))
                        onButtonsFound();
                    else
                         onMenuFound();
                    return true;
                }
                break;

            case 3:
                if (RES_ID_TEXT1.equals(resName)) {
                    // TODO confirm menu text vs i18n values/strings
                    if (radioView!=null)radioView.recycle();
                    radioView = info;
                    inspectorState = 4;
                    return true;
                }
                break;

            case 4:
                if (RES_ID_TEXT1.equals(resName)) {
                    // TODO confirm menu text vs i18n values/strings
                    if (shuffleView!=null)shuffleView.recycle();
                    shuffleView = info;
                    onButtonsFound();
                    return true;
                }
                break;

            case 5:
                if (RES_ID_SHUFFLE.equals(resName)) {
                    if (shuffleView!=null)shuffleView.recycle();
                    shuffleView = info;
                    onButtonsFound();
                    return true;
                }
                break;

        }
        inspectorState=0;
        return omgwt(info);
    }

    private void inspectNode(AccessibilityNodeInfo info) {
        if (info==null)return;
        boolean keep = false;
        if (info.isClickable() && info.isEnabled()) {
//            Log.d(TAG, "Candidate ["+info.getViewIdResourceName()+"] ["+info+"]");
            keep = omgwt(info);
        }
        final int n = info.getChildCount();
        for(int i=0;i<n;i++) {
            final AccessibilityNodeInfo child = info.getChild(i);
            if (child != null) {
                inspectNode(child);
            }
        }
        if(!keep)
            info.recycle();
    }

    @Override
    public void onInterrupt() {
    }

    @Override
    protected void onServiceConnected() {
        super.onServiceConnected();

        final AccessibilityServiceInfo serviceInfo = getServiceInfo();
        serviceInfo.flags |= AccessibilityServiceInfo.FLAG_REPORT_VIEW_IDS;
        setServiceInfo(serviceInfo);

    }
}
