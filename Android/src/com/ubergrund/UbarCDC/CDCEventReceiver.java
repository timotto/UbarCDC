package com.ubergrund.UbarCDC;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.preference.PreferenceManager;

/**
 * Created with IntelliJ IDEA.
 * User: Tim
 * Date: 9/15/13
 * Time: 10:42 AM
 */
public class CDCEventReceiver extends BroadcastReceiver {

    private static final String TAG = "UbarCDC/CDCEventReceiver";

    public static final String EXTRA_BUTTON = "button";

    public void onReceive(Context context, Intent intent) {
        if (intent == null || ! "com.ubergrund.ubarcdc.CDC_EVENT".equals(intent.getAction()))
            return;

        final String button = intent.getStringExtra(EXTRA_BUTTON);
        if (button == null || "".equals(button.trim()))
            return;

        final SharedPreferences p = PreferenceManager.getDefaultSharedPreferences(context);
        final String listId = p.getString("disc_" + button + "_id", null);

        if (listId != null) {
            if (listId.startsWith("content://com.google.android.music.MusicContent/"))
                handleGMIntent(context, Uri.parse(listId));
        }

    }

    private void handleGMIntent(Context context, Uri itemUri) {
        // first arm the AccessibilityService
        final Intent armIntent = new Intent("com.ubergrund.ubarcdc.CDC_GM_HACK");
        armIntent.putExtra(UbarAccService.EXTRA_ACTION, UbarAccService.VALUE_PLAY);
        context.sendBroadcast(armIntent);

        final Intent intent = new Intent();
        intent.setClassName("com.google.android.music", "com.google.android.music.ui.SearchActivity");
//        intent.setAction("android.media.action.MEDIA_PLAY_FROM_SEARCH");
        intent.setAction("android.intent.action.SEARCH_RESULT");
        intent.setData(itemUri);
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK | Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent);
    }
}
