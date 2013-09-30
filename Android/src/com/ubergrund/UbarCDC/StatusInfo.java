package com.ubergrund.UbarCDC;

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
* Time: 7:16 PM
*/
public class StatusInfo {
    public final String title;
    public final String artist;
    public final String album;

    public StatusInfo(String title, String artist, String album) {
        this.title = nullSafeSet(title);
        this.artist = nullSafeSet(artist);
        this.album = nullSafeSet(album);
    }

    private static String nullSafeSet(String value) {
        return value!=null?value:"";
    }

    @Override
    public String toString() {
        return "{artist="+artist+", album="+album+", track="+title+"}";
    }

    @Override
    public int hashCode() {
        return title.hashCode() ^ artist.hashCode() ^ album.hashCode();
    }

    @Override
    public boolean equals(Object o) {
        return o instanceof StatusInfo && hashCode() == o.hashCode();
    }
}
