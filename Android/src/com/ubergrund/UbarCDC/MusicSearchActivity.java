package com.ubergrund.UbarCDC;

import android.app.Activity;
import android.app.ListActivity;
import android.app.LoaderManager;
import android.content.CursorLoader;
import android.content.Intent;
import android.content.Loader;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.SimpleCursorAdapter;

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
 * Date: 9/14/13
 * Time: 8:02 PM
 */
public class MusicSearchActivity extends ListActivity implements LoaderManager.LoaderCallbacks<Cursor> {

    private static final String TAG = "UbarCDC/MusicSearchActivity";

    private static final Uri ALBUM_URI = Uri.parse("content://com.google.android.music.MusicContent/album/store");
    private static final String ALBUM_PROJECTION[] = new String[]{"_id", "album_name", "album_artist", "StoreAlbumId", "album_id"};
    private static final String ALBUM_ORDER = "album_name";
    private static final int ALBUM_LOADER = 0;

    private static final Uri SEARCH_BASE_URI = Uri.parse("content://com.google.android.music.MusicContent/search/search_suggest_query/");
    private static final int SEARCH_LOADER = 1;

    private EditText queryField;
//    private ListView listView;
    private SimpleCursorAdapter albumAdapter;

    private String query = "";

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getActionBar().setDisplayHomeAsUpEnabled(true);
        setContentView(R.layout.music_search);
        queryField = (EditText) findViewById(R.id.query);

        queryField.addTextChangedListener(textWatcher);
        albumAdapter = new SimpleCursorAdapter(
                this,
                android.R.layout.simple_list_item_2,
                null,
                new String[]{"suggest_text_1", "suggest_text_2"},
                new int[]{android.R.id.text1, android.R.id.text2},
                0);
        albumAdapter.setStringConversionColumn(1);
        setListAdapter(albumAdapter);

        getLoaderManager().initLoader(SEARCH_LOADER, null, this);
    }

    @Override
    protected void onDestroy() {
        queryField.removeTextChangedListener(textWatcher);
        super.onDestroy();
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        Log.d(TAG, "Selected item id [" + id + "]");
        Cursor cursor = (Cursor) albumAdapter.getItem(position);
        final String[] columnNames = cursor.getColumnNames();
//        Log.d(TAG, "Columns: " + TextUtils.join(",", columnNames));
//                final int colId = cursor.getColumnIndexOrThrow("_id");
//                final int colName = cursor.getColumnIndexOrThrow("name");
        String[] values = new String[columnNames.length];
        for(int i=0;i<columnNames.length;i++)
            values[i] = columnNames[i]+"="+ cursor.getString(i);
        Log.d(TAG, "Values: " + TextUtils.join(",", values));

        final Uri itemUri = Uri.withAppendedPath(
                Uri.parse(cursor.getString(cursor.getColumnIndexOrThrow("suggest_intent_data"))),
                cursor.getString(cursor.getColumnIndexOrThrow("suggest_intent_data_id"))
        );
        Log.d(TAG, "Uri ["+itemUri+"]");
//        final Intent intent = new Intent(Intent.ACTION_VIEW);
        //com.google.android.music.ui.SearchActivity

//        // first arm the AccessibilityService
//        final Intent armIntent = new Intent("com.ubergrund.ubarcdc.CDC_GM_HACK");
//        armIntent.putExtra(UbarAccService.EXTRA_ACTION, UbarAccService.VALUE_PLAY);
//        sendBroadcast(armIntent);
//
        final Intent intent = new Intent();
        intent.setClassName("com.google.android.music", "com.google.android.music.ui.SearchActivity");
//        intent.setAction("android.media.action.MEDIA_PLAY_FROM_SEARCH");
        intent.setAction("android.intent.action.SEARCH_RESULT");
        intent.putExtra("suggest_text_1", cursor.getString(cursor.getColumnIndexOrThrow("suggest_text_1")));
        intent.putExtra("suggest_text_2", cursor.getString(cursor.getColumnIndexOrThrow("suggest_text_2")));
        intent.setData(itemUri);
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK | Intent.FLAG_ACTIVITY_NEW_TASK);
//        startActivity(intent);

        setResult(Activity.RESULT_OK, intent);
        finish();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                setResult(Activity.RESULT_CANCELED);
                finish();
                return true;
        }
        return super.onOptionsItemSelected(item);
    }

    private final TextWatcher textWatcher = new TextWatcher() {
        @Override
        public void beforeTextChanged(CharSequence s, int start, int count, int after) {
        }

        @Override
        public void onTextChanged(CharSequence s, int start, int before, int count) {
            query = s.toString();
            getLoaderManager().restartLoader(SEARCH_LOADER, null, MusicSearchActivity.this);
        }

        @Override
        public void afterTextChanged(Editable s) {
        }
    };

    @Override
    public Loader<Cursor> onCreateLoader(int id, Bundle args) {
        switch (id) {
            case ALBUM_LOADER:
                return new CursorLoader(this,
                        ALBUM_URI, ALBUM_PROJECTION,
                        null , null,
                        ALBUM_ORDER);

            case SEARCH_LOADER:
                return new CursorLoader(this,
                        Uri.withAppendedPath(SEARCH_BASE_URI, query),
                        null, null , null, null);
        }
        return null;
    }

    @Override
    public void onLoadFinished(Loader<Cursor> loader, Cursor data) {
        albumAdapter.changeCursor(data);
    }

    @Override
    public void onLoaderReset(Loader<Cursor> loader) {
        albumAdapter.changeCursor(null);
    }
}