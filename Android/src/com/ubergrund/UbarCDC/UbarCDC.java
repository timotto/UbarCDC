package com.ubergrund.UbarCDC;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;

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
 * Time: 8:31 PM
 */
public class UbarCDC extends Activity {

    /**
     * content://com.google.android.music.MusicContent/album/store:
     * -album_artist,album_art,isAllLocal,artworkUrl,SongCount,album_year,
     * -hasLocal,hasRemote,HasDifferentTrackArtists,album_artist_sort,
     * -hasAny,KeepOnId,album_sort,StoreAlbumId,album_id,hasPersistNautilus,
     * -_id,_count,keeponSongCount,album_name,album_artist_id,
     * -keeponDownloadedSongCount,ArtistMetajamId
     *
     *
     * content://com.google.android.music.MusicContent/playlists/suggested
     * -isAllLocal,playlist_owner_name,hasLocal,hasRemote,hasAny,KeepOnId,
     * -playlist_art_url,playlist_share_token,_id,_count,playlist_owner_profile_photo_url,
     * -keeponSongCount,playlist_description,playlist_type,playlist_id,keeponDownloadedSongCount,playlist_name
     *
     *
     *
     */

    private static final String TAG = "UbarCDC/UbarCDC";
    private static final int buttonsIds[] = new int[]{R.id.cd1, R.id.cd2, R.id.cd3, R.id.cd4, R.id.cd5, R.id.cd6};
    private final Button buttons[] = new Button[6];
    private Button set = null;
    private boolean testMode = false;

    /**
     * Called when the activity is first created.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        ((CheckBox)findViewById(R.id.test)).setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                testMode = isChecked;
            }
        });
        for(int i=0;i<buttonsIds.length;i++)
            setupButton(i, buttonsIds[i]);

        redrawButtons();

        action();
    }

    private void setupButton(int index, int id) {
        buttons[index] = (Button)findViewById(id);
        buttons[index].setTag(index);
        buttons[index].setOnClickListener(selectDiscListener);
    }

    private final View.OnClickListener selectDiscListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            if (testMode) {
                final int disc = (Integer)v.getTag() + 1;
                final Intent testIntent = new Intent("com.ubergrund.ubarcdc.CDC_EVENT");
                testIntent.putExtra(CDCEventReceiver.EXTRA_BUTTON, String.valueOf(disc));
                sendBroadcast(testIntent);
                return;
            }
            set = (Button) v;
//            final Intent intent = new Intent();
//            intent.setClassName("com.google.android.music", "com.google.android.music.ui.CreatePlaylistShortcutActivity");
//            startActivityForResult(intent, 123);

            final Intent intent = new Intent(UbarCDC.this, MusicSearchActivity.class);
            startActivityForResult(intent, 321);
        }
    };

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (resultCode == RESULT_CANCELED)return;

        if (321 == requestCode) {
            String playlistName = data.getStringExtra("suggest_text_1");
            if (playlistName==null||playlistName.trim().equals(""))
                playlistName = data.getStringExtra("suggest_text_2");
            final String playlistId = data.getData().toString();

            final int disc = (Integer)set.getTag() + 1;
            Log.d(TAG, "assign ["+playlistName+"]/["+playlistId+"] to CD ["+ disc +"]");
            final SharedPreferences.Editor e = PreferenceManager.getDefaultSharedPreferences(this).edit();

            e.putString("disc_"+disc+"_name", playlistName);
            e.putString("disc_"+disc+"_id", playlistId);
            e.commit();
            redrawButtons();
        }

        if (123 == requestCode) {
            final String playlistName = data.getStringExtra("android.intent.extra.shortcut.NAME");

            final Intent shortcut = data.getParcelableExtra("android.intent.extra.shortcut.INTENT");
            if (shortcut==null)return;

            Log.d(TAG, "Shortcut: ["+shortcut+"]");
            for(String key : shortcut.getExtras().keySet()) {
                final Object o = shortcut.getExtras().get(key);
                Log.d(TAG, "["+key+"] = ["+ o +"]");
            }

            final String playlistId = shortcut.getStringExtra("playlist");
            if (playlistId == null)return;

            final int disc = (Integer)set.getTag() + 1;
            Log.d(TAG, "assign ["+playlistName+"]/["+playlistId+"] to CD ["+ disc +"]");
            final SharedPreferences.Editor e = PreferenceManager.getDefaultSharedPreferences(this).edit();

            e.putString("disc_"+disc+"_name", playlistName);
            e.putString("disc_"+disc+"_id", playlistId);
            e.commit();
            redrawButtons();
        }
    }

    private void redrawButtons() {
        final SharedPreferences p = PreferenceManager.getDefaultSharedPreferences(this);

        for(int i=0;i<buttons.length;i++) {
            final int disc = i+1;
            final String name = p.getString("disc_" + disc + "_name", null);

            buttons[i].setText(String.format("%d: %s", disc, name==null?"":name));
        }
    }

    private Cursor queryAlbumStore() {
        try {
            return getContentResolver().query(Uri.parse("content://com.google.android.music.MusicContent/album/store"),
                    new String[]{"album_name", "album_artist", "StoreAlbumId", "album_id"}, null, null, "album_name");
        } catch (Throwable t) {
            Log.e(TAG, "queryAlbumStore() failed: "+t.toString());
        }
        return null;
    }

    private Cursor queryAlbumArtists() {
        try {
            return getContentResolver().query(Uri.parse("content://com.google.android.music.MusicContent/album/artists"),
                    new String[]{"album_artist", "album_artist_id"}, null, null, "album_artist");
        } catch (Throwable t) {
            Log.e(TAG, "queryAlbumArtists() failed: "+t.toString());
        }
        return null;
    }
    private void action() {
//        Cursor cursor = queryAlbumStore();
        Context context = this;
        Cursor cursor = null;
        try {
            cursor = context.getContentResolver().query(
//                    Uri.parse("content://com.google.android.music.MusicContent/playlists"),
//                    Uri.parse("content://com.google.android.music.MusicContent/album"),
//                    Uri.parse("content://com.google.android.music.MusicContent/radio_stations"),
                    Uri.parse("content://com.google.android.music.MusicContent/search/search_suggest_query/Lana"),
//                    new String[]{"_id"},
                    null,
                    null,
                    null,
                    null);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "Playlist query failed early", e);
            return;
        }

        if (cursor == null) {
            Log.w(TAG, "Playlist query returned null cursor");
            return;
        }

        int rows = 0;
        try {
            if (cursor.moveToFirst()) {
                final String[] columnNames = cursor.getColumnNames();
                Log.d(TAG, "Columns: " + TextUtils.join(",", columnNames));
//                final int colId = cursor.getColumnIndexOrThrow("_id");
//                final int colName = cursor.getColumnIndexOrThrow("name");
                String[] values = new String[columnNames.length];

                while (!cursor.isAfterLast()) {
                    for(int i=0;i<columnNames.length;i++)
                        values[i] = cursor.getString(i);
//                    final long id = cursor.getLong(colId);
//                    final String name = cursor.getString(colName);
//                    entries.add(new PlaylistEntry(id, name));
                    Log.d(TAG, "Row[n]: " + TextUtils.join(",", values));
                    cursor.moveToNext();
                    rows++;
                }
            }
        } finally {
            cursor.close();
        }
        if (rows>0)
            Log.d(TAG, "received ["+rows+"] rows");
    }
}
