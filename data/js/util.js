//-------------------------------------------------------------------------------------------------
// CLASS
//-------------------------------------------------------------------------------------------------
class Util {
    static pad(n, width) {
        n = n + '';
        return n.length >= width ? n : new Array(width - n.length + 1).join('0') + n;
    }

    static formatTime(min) {
        var days = Math.floor(min / (60 * 24));
        min %= (60 * 24);
        var hours = Math.floor(min / 60);
        min %= 60;

        var str = "";
        if (days > 0)
            str = str + Util.pad(days, 2) + "d ";
        if (hours > 0)
            str = str + Util.pad(hours, 2) + "h ";
        str = str + Util.pad(min, 2) + "m";

        return str;
    }

    static encode(input) {
        const keyStr = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
        var output = "";
        var chr1, chr2, chr3, enc1, enc2, enc3, enc4;
        var i = 0;

        while (i < input.length) {
            chr1 = input[i++];
            chr2 = i < input.length ? input[i++] : Number.NaN; // Not sure if the index
            chr3 = i < input.length ? input[i++] : Number.NaN; // checks are needed here

            enc1 = chr1 >> 2;
            enc2 = ((chr1 & 3) << 4) | (chr2 >> 4);
            enc3 = ((chr2 & 15) << 2) | (chr3 >> 6);
            enc4 = chr3 & 63;

            if (isNaN(chr2)) {
                enc3 = enc4 = 64;
            } else if (isNaN(chr3)) {
                enc4 = 64;
            }
            output += keyStr.charAt(enc1) + keyStr.charAt(enc2) +
                keyStr.charAt(enc3) + keyStr.charAt(enc4);
        }
        return output;
    }
}

