#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <string.h>

#define String std::string

std::list <String> _png_logs = {
    "-rw-rw-rw-   1 root  root      4796 Jan 23 22:57 39585176581.png",
    "-rw-rw-rw-   1 root  root      4061 Jan 23 23:55 34251623081.png",
    "-rw-rw-rw-   1 root  root      4796 Jan 24 01:06 12718038631.png",
    "-rw-rw-rw-   1 root  root      4043 Jan 24 09:26 22181592811.png",
    "-rw-rw-rw-   1 root  root      3609 Jan 25 23:20 2613376571.png",
    "-rw-rw-rw-   1 root  root      2157 Jan 24 21:58 7143709141.png",
    "-rw-rw-rw-   1 root  root      3186 Jan 25 00:08 3553883803.png",
    "-rw-rw-rw-   1 root  root      3804 Jan 25 22:09 4380517671.png",
    "-rw-rw-rw-   1 root  root      3471 Jan 26 23:16 15642952511.png",
    "-rw-rw-rw-   1 root  root      2755 Jan 26 23:51 10363259711.png",
    "-rw-rw-rw-   1 root  root      2171 Jan 27 13:22 4678907721.png",
    "-rw-rw-rw-   1 root  root      1182 Jan 27 13:26 19799488481.png",
    "-rw-rw-rw-   1 root  root      4081 Jan 28 04:35 2064281.png",
    "-rw-rw-rw-   1 root  root      1863 Jan 28 11:41 17930814601.png",
    "-rw-rw-rw-   1 root  root      4857 Jan 28 14:13 26698210481.png",
    "-rw-rw-rw-   1 root  root      5285 Jan 31 03:09 14229151451.png",
    "-rw-rw-rw-   1 root  root      4505 Feb 03 15:51 38297837321.png",
    "-rw-rw-rw-   1 root  root      2540 Feb 05 20:19 166532401.png",
    "-rw-rw-rw-   1 root  root      2540 Feb 05 23:13 9240571371.png",
    "-rw-rw-rw-   1 root  root      4684 Feb 05 23:28 42885271611.png",
    "-rw-rw-rw-   1 root  root      4684 Feb 08 15:17 28260101861.png",
    "-rw-rw-rw-   1 root  root      4442 Feb 12 22:44 15137327011.png"
};

std::list <String> _png_logs_new = {
    "-rw-rw-rw-   1 root  root      4796 Jan 23 22:57 39585176581.png",
    "-rw-rw-rw-   1 root  root      4061 Jan 23 23:55 34251623081.png",
    "-rw-rw-rw-   1 root  root      4796 Jan 24 01:06 12718038631.png",
    "-rw-rw-rw-   1 root  root      4043 Jan 24 09:26 22181592811.png",
    "-rw-rw-rw-   1 root  root      3609 Jan 25 23:20 2613376571.png",
    "-rw-rw-rw-   1 root  root      2157 Jan 24 21:58 7143709141.png",
    "-rw-rw-rw-   1 root  root      3186 Jan 25 00:08 3553883803.png",
    "-rw-rw-rw-   1 root  root      3804 Jan 25 22:09 4380517671.png",
    "-rw-rw-rw-   1 root  root      3471 Jan 26 23:16 15642952511.png",
    "-rw-rw-rw-   1 root  root      2755 Jan 26 23:51 10363259711.png",
    "-rw-rw-rw-   1 root  root      2171 Jan 27 13:22 4678907721.png",
    "-rw-rw-rw-   1 root  root      1182 Jan 27 13:26 19799488481.png",
    "-rw-rw-rw-   1 root  root      4081 Jan 28 04:35 2064281.png",
    "-rw-rw-rw-   1 root  root      1863 Jan 28 11:41 17930814601.png",
    "-rw-rw-rw-   1 root  root      4857 Jan 28 14:13 26698210481.png",
    "-rw-rw-rw-   1 root  root      5285 Jan 31 03:09 14229151451.png",
    "-rw-rw-rw-   1 root  root      4505 Feb 03 15:51 38297837321.png",
    "-rw-rw-rw-   1 root  root      2540 Feb 05 20:19 166532401.png",
    "-rw-rw-rw-   1 root  root      2540 Feb 05 23:13 9240571371.png",
    "-rw-rw-rw-   1 root  root      4684 Feb 05 23:28 42885271611.png",
    "-rw-rw-rw-   1 root  root      4684 Feb 08 15:17 28260101861.png",
    "-rw-rw-rw-   1 root  root      4442 Feb 12 22:44 15137327011.png",
    "-rw-rw-rw-   1 root  root      4442 Feb 12 22:46 15137346711.png"
};

std::list <String> _model_logs = {
    "-rw-rw-rw-   1 root  root    326780 Jan 23 22:57 HKR -AMS Side mount spool holder.gcode.3mf",
    "-rw-rw-rw-   1 root  root    617858 Jan 23 23:55 X-Touch.gcode.3mf",
    "-rw-rw-rw-   1 root  root    595232 Jan 24 09:26 X1_Touch.gcode.3mf",
    "-rw-rw-rw-   1 root  root    413301 Jan 25 23:20 No-Catch Y-Splitter Self-Tap (long slot) (9.4mm-M3x6 mount).gcode.3mf",
    "-rw-rw-rw-   1 root  root    953086 Jan 31 03:09 1.gcode.3mf",
    "-rw-rw-rw-   1 root  root    698414 Jan 24 21:58 Bambu Trash_plate_1.gcode.3mf",
    "-rw-rw-rw-   1 root  root    301823 Jan 25 00:08 Bambu Trash_plate_3.gcode.3mf",
    "-rw-rw-rw-   1 root  root    176374 Jan 25 22:09 AMS_disconnect_tool_with_magnet_seperate_letters.gcode.3mf",
    "-rw-rw-rw-   1 root  root    710512 Jan 26 23:51 hex_bits_holder_7 x 2.gcode.3mf",
    "-rw-rw-rw-   1 root  root   1029925 Jan 27 13:22 hex_bits_holder_7x3.gcode.3mf",
    "-rw-rw-rw-   1 root  root    767304 Jan 27 13:26 swerve-gear-motorx6.gcode.3mf",
    "-rw-rw-rw-   1 root  root   1548586 Jan 28 04:35 rc_con_left_bottom.gcode.3mf",
    "-rw-rw-rw-   1 root  root    458648 Jan 28 11:41 swerve-axle-gearx3.gcode.3mf",
    "-rw-rw-rw-   1 root  root    970885 Jan 28 14:13 swerve-large-gear.gcode.3mf",
    "-rw-rw-rw-   1 root  root   2108337 Feb 03 15:51 controller-left.gcode.3mf",
    "-rw-rw-rw-   1 root  root    810058 Feb 05 20:19 제목 없음.gcode.3mf",
    "-rw-rw-rw-   1 root  root    810058 Feb 05 23:14 plate_holder.gcode.3mf",
    "-rw-rw-rw-   1 root  root    667533 Feb 05 23:28 rpi3p_case.gcode.3mf",
    "-rw-rw-rw-   1 root  root    667533 Feb 08 15:17 11111111.gcode.3mf",
    "-rw-rw-rw-   1 root  root    838822 Feb 12 22:45 galaxy watch stand.gcode.3mf",
    "drwxr-xr-x   2 root  wheel     1024 Nov 17  1993 lib"
};


class FTPListParser {
private:
    std::vector <String> _months = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Sep", "Nov", "Dec" };

public:
    class FileInfo {
    public:
        long   ts;
        long   size;
        String name;

        void set(long ts, long size, String name) {
            this->ts = ts;
            this->size = size;
            this->name = name;
        }

        FileInfo() {
            set(0, 0, "");
        }

        FileInfo(long ts, long size, String name) {
            set(ts, size, name);
        }
    };

    class FilePair {
    public:
        long   ts;
        String file_a;
        String file_b;
        long   size_a;
        long   size_b;

        void set(long ts, String a, String b) {
            this->ts = ts;
            this->file_a = a;
            this->file_b = b;
        }

        FilePair() {
            set(0, "", "");
        }

        FilePair(long ts, String a, String b) {
            set(ts, a, b);
        }
    };

    void parse(std::list <String> logs, std::list <FileInfo*> &result) {
        std::vector <char*> tokens;
        String fname;

        for (String line: logs) {
            char *token = strtok((char*)line.c_str(), " ");
            while (token != NULL) {
                tokens.push_back(token);
                token = strtok(NULL, " ");
            }
            long sz  = atoi(tokens[4]);
            int mon  = std::find(_months.begin(), _months.end(), tokens[5]) - _months.begin() + 1;
            int day  = atoi(tokens[6]);

            int hour = 0;
            int min  = 0;
            if (tokens[7][2] == ':') {
                tokens[7][2] = '\0';
                hour = atoi(tokens[7]);
                min  = atoi(&tokens[7][3]);
            }
            long ts  = min + (hour * 60) + (day * 60 * 24) + (mon * 60 * 24 * 31);
            fname = tokens[8];
            for (int i = 9; i < tokens.size(); i++)
                fname = fname + ' ' + tokens[i];
            // printf("%s %d-%d %d:%d %lu\n", fname.c_str(), mon, day, hour, min, ts);

            result.push_back(new FileInfo(ts, sz, fname));
            tokens.clear();
        }
    }

    void matches(std::list <FileInfo*> a, std::list <FileInfo*> b, std::list <FilePair*> &result) {
        FilePair r;
        bool is_found;

        for (FileInfo *info_a : a) {
            is_found = false;
            for (FileInfo *info_b : b) {
                if (std::abs(info_a->ts - info_b->ts) < 3) {
                    r.ts = info_a->ts;
                    r.file_a = info_a->name;
                    r.size_a = info_a->size;
                    r.file_b = info_b->name;
                    r.size_b = info_b->size;
                    is_found = true;
                    if (info_a->ts == info_b->ts)
                        break;
                }
            }
            if (is_found)
                result.push_back(new FilePair(r));
        }
    }

    void diff(std::list <FileInfo*> a, std::list <FileInfo*> b, std::list <FileInfo*> &result) {
        FileInfo r;
        bool is_found;

        for (FileInfo *info_b : b) {
            is_found = false;
            for (FileInfo *info_a : a) {
                if (info_a->ts == info_b->ts) {
                    is_found = true;
                    break;
                }
            }
            if (!is_found)
                result.push_back(new FileInfo(*info_b));
        }
    }
};

void tokenize(String &line, String token, std::vector<String> &tokens) {
    String tmp(line);
    char *tkn = strtok((char*)tmp.c_str(), token.c_str());

    while (tkn != NULL) {
        tokens.push_back(String(tkn));
        tkn = strtok(NULL, token.c_str());
    }
}

void test(String line) {
    String path;
    std::vector<String> tokens;

    tokenize(line, "/", tokens);
    for (int i = 0; i < tokens.size() - 1; i++) {
        path = path + "/" + tokens[i];
        printf("%s\n", path.c_str());
    }
}

int main() {
    FTPListParser  parser;

    std::list <FTPListParser::FileInfo*> info_png;
    std::list <FTPListParser::FileInfo*> info_png_new;
    std::list <FTPListParser::FileInfo*> info_diff;
    std::list <FTPListParser::FileInfo*> info_model;
    std::list <FTPListParser::FilePair*> pair;

    test("/ftps/image/1.png");

    // parser.parse(_png_logs, info_png);
    // parser.parse(_png_logs_new, info_png_new);
    // parser.diff(info_png, info_png_new, info_diff);
    // for (FTPListParser::FileInfo *i:info_diff)
    //     printf("%8ld, %s\n", i->ts, i->name.c_str());

    // parser.parse(_model_logs, info_model);
    // for (FTPListParser::FileInfo *i:info_model)
    //     printf("%8ld, %s\n", i->ts, i->name.c_str());

    // parser.matches(info_png, info_model, pair);
    // int cnt = 1;
    // for (FTPListParser::FilePair *p:pair) {
    //     printf("%3d %10ld, [%10ld] %20s, [%10ld] %s\n", cnt++, p->ts, p->size_a, p->file_a.c_str(), p->size_b, p->file_b.c_str());
    // }
}
