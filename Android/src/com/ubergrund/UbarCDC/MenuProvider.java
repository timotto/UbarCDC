package com.ubergrund.UbarCDC;

import android.content.Context;
import android.content.res.Resources;
import android.database.Cursor;
import android.util.Pair;

/**
 * The MenuProvider class is responsible to create menus of navigation
 * and content.
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
 * Date: 9/22/13
 * Time: 3:47 PM
 */
public class MenuProvider {

    private final Menu[] textMenus;

    public MenuProvider(Context context) {
        final Resources r = context.getResources();
        textMenus = new Menu[1];
        textMenus[0] = new StringMenu(0, 0,
                new String[]{
                        r.getString(R.string.now_playing),
                        r.getString(R.string.recent),
                        r.getString(R.string.suggested),
                        r.getString(R.string.radios),
                        r.getString(R.string.playlists),
                        r.getString(R.string.artists),
                        r.getString(R.string.albums),
                        r.getString(R.string.settings),
                },
                new long[]{10, 11, 12, 13, 14, 15, 16, 17});
    }

    public Menu get(long menuId) {
        if (menuId < textMenus.length)
            return textMenus[(int) (menuId)];

        return null;
    }

    protected static class CursorMenu extends Menu {

        private final Cursor cursor;
        private final int idColumn;
        private final int textColumn;

        protected CursorMenu(Context context, int menuId, int backId,
                             Cursor cursor, int idColumn, int textColumn) {
            super(menuId, backId);
            this.cursor = cursor;
            this.idColumn = idColumn;
            this.textColumn = textColumn;
        }

        @Override
        public void close() {
            super.close();
            cursor.close();
        }

        @Override
        public int length() {
            return cursor.getCount();
        }

        @Override
        public Pair<Long,String> get(int index) {
            if(!cursor.moveToPosition(index))
                return null;

            return new Pair<Long, String>(cursor.getLong(idColumn), cursor.getString(textColumn));
        }
    }

    protected static class StringMenu extends Menu {

        private final String[] entries;
        private final long[] ids;

        protected StringMenu(int menuId, int backId, String[] entries, long[] ids) {
            super(menuId, backId);
            if (entries==null || ids == null || entries.length != ids.length)
                throw new IllegalArgumentException("entries and ids required to have the same count");

            this.entries = entries;
            this.ids = ids;
        }

        @Override
        public int length() {
            return entries.length;
        }

        @Override
        public Pair<Long,String> get(int index) {
            return new Pair<Long, String>(ids[index], entries[index]);
        }
    }

    public abstract static class Menu {

        protected final int menuId;
        protected final int backId;

        protected Menu(int menuId, int backId) {
            this.menuId = menuId;
            this.backId = backId;
        }

        public void close(){}

        public int getBackId() {
            return backId;
        }

        public int getMenuId() {
            return menuId;
        }

        abstract public int length();
        abstract public Pair<Long,String> get(int index);
    }
}
