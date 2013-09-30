package com.ubergrund.UbarCDC;

import android.util.Pair;

import java.io.DataOutputStream;
import java.io.IOException;

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
 * Date: 9/26/13
 * Time: 7:15 PM
 */
public class SPPDriver {

    public static final int MSG_PING = 0x10;
    public static final int MSG_PONG = 0x11;
    public static final int MSG_TRACKINFO = 0x80;
    public static final int MSG_DIRECTORY = 0x90;
    public static final int MSG_DISCSELECT = 0x20;
    public static final int MSG_GETDIRECTORY = 0x30;

    private DataOutputStream out;

    public void sendTrackInfo(StatusInfo lastInfo) throws IOException {
        if (lastInfo==null)
            return; // TODO send 'no music playing' message

        out.writeByte(MSG_TRACKINFO);
        sendCharArrayString(lastInfo.title);
        sendCharArrayString(lastInfo.artist);
        sendCharArrayString(lastInfo.album);
        out.flush();
    }

    public void sendDirectory(MenuProvider.Menu menu, int offset, int length) throws IOException {
        final int indexId = menu.getMenuId();
        final int backIndexId = menu.getBackId();

        out.writeByte(MSG_DIRECTORY);
        out.writeInt(indexId);
        out.writeInt(backIndexId);
        out.writeInt(offset);
        out.writeInt(length);
        // TODO send index items:
        // 4-byte uint32_t indexId
        // item title as chars, 0 byte terminated
        int end = offset + length;
        for(int i=offset;i<end;i++) {
            final Pair<Long,String> pair = menu.get(i);

            sendCharArrayString(pair.second);

//            out.writeByte((int) (indexId >> 24)&0xff);
//            out.writeByte((int) (indexId >> 16)&0xff);
//            out.writeByte((int) (indexId >> 8)&0xff);
//            out.writeByte((int) (indexId)&0xff);

        }

        out.writeByte(0); // totally terminating 0 byte
        out.flush();
    }

    public void sendPing(int arg) throws IOException {
        out.writeByte(MSG_PING);
        out.writeByte(arg);
        out.flush();
    }

    public void sendPong(int arg) throws IOException {
        out.writeByte(MSG_PONG);
        out.writeByte(arg);
        out.flush();
    }

    public void setOut(DataOutputStream out) {
        this.out = out;
    }

    private void sendCharArrayString(String string) throws IOException {
        final char[] chars = string.toCharArray();
        for(char c : chars)
            out.writeByte(c);
        out.writeByte(0);
    }
}
