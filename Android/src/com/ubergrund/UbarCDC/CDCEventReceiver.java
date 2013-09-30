package com.ubergrund.UbarCDC;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.preference.PreferenceManager;

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
 * Date: 9/15/13
 * Time: 10:42 AM
 */
public class CDCEventReceiver extends BroadcastReceiver {

    private static final String TAG = "UbarCDC/CDCEventReceiver";

    public static final String ACTION_EVENT = "com.ubergrund.ubarcdc.CDC_EVENT";
    public static final String EXTRA_BUTTON = "button";

    public void onReceive(Context context, Intent intent) {
        if (intent == null)
            return;

        final String button = intent.getStringExtra(EXTRA_BUTTON);
        if (button == null || "".equals(button.trim()))
            return;

        final SharedPreferences p = PreferenceManager.getDefaultSharedPreferences(context);
        final String listId = p.getString("disc_" + button + "_id", null);

        if (listId != null) {
            if (listId.startsWith("content://com.google.android.music.MusicContent/")) {
                final Intent gmhs = new Intent(context, GoogleMusicHelperService.class);
                gmhs.setData(Uri.parse(listId));
                context.startService(gmhs);
            }
        }
    }
}
