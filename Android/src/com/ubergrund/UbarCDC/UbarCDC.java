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

public class UbarCDC extends Activity {

    private static final String TAG = "UbarCDC/UbarCDC";
    private static final int buttonsIds[] = new int[]{R.id.cd1, R.id.cd2, R.id.cd3, R.id.cd4, R.id.cd5, R.id.cd6};
    private final Button buttons[] = new Button[6];
    private Button set = null;

    /**
     * Called when the activity is first created.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
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
            set = (Button) v;
            final Intent intent = new Intent();
            intent.setClassName("com.google.android.music", "com.google.android.music.ui.CreatePlaylistShortcutActivity");
            startActivityForResult(intent, 123);
        }
    };

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (resultCode == RESULT_CANCELED)return;

        if (123 == requestCode) {
            final String playlistName = data.getStringExtra("android.intent.extra.shortcut.NAME");

            final Intent shortcut = data.getParcelableExtra("android.intent.extra.shortcut.INTENT");
            if (shortcut==null)return;

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

    private void action() {
        Context context = this;
        Cursor cursor = null;
        try {
            cursor = context.getContentResolver().query(
                    Uri.parse("content://com.google.android.music.MusicContent/playlists"),
//                    Uri.parse("content://com.google.android.music.MusicContent/album"),
//                    Uri.parse("content://com.google.android.music.MusicContent/radio_stations"),
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
                }
            }
        } finally {
            cursor.close();
        }

    }
}
