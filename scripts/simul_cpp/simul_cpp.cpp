#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <string.h>

std::list <std::string> _png_logs = {
    " -rw-rw-rw-   1 root  root      4796 Jan 23 22:57 39585176581.png",
    " -rw-rw-rw-   1 root  root      4061 Jan 23 23:55 34251623081.png",
    " -rw-rw-rw-   1 root  root      4796 Jan 24 01:06 12718038631.png",
    " -rw-rw-rw-   1 root  root      4043 Jan 24 09:26 22181592811.png",
    " -rw-rw-rw-   1 root  root      3609 Jan 25 23:20 2613376571.png",
    " -rw-rw-rw-   1 root  root      2157 Jan 24 21:58 7143709141.png",
    " -rw-rw-rw-   1 root  root      3186 Jan 25 00:08 3553883803.png",
    " -rw-rw-rw-   1 root  root      3804 Jan 25 22:09 4380517671.png",
    " -rw-rw-rw-   1 root  root      3471 Jan 26 23:16 15642952511.png",
    " -rw-rw-rw-   1 root  root      2755 Jan 26 23:51 10363259711.png",
    " -rw-rw-rw-   1 root  root      2171 Jan 27 13:22 4678907721.png",
    " -rw-rw-rw-   1 root  root      1182 Jan 27 13:26 19799488481.png",
    " -rw-rw-rw-   1 root  root      4081 Jan 28 04:35 2064281.png",
    " -rw-rw-rw-   1 root  root      1863 Jan 28 11:41 17930814601.png",
    " -rw-rw-rw-   1 root  root      4857 Jan 28 14:13 26698210481.png",
    " -rw-rw-rw-   1 root  root      5285 Jan 31 03:09 14229151451.png",
    " -rw-rw-rw-   1 root  root      4505 Feb 03 15:51 38297837321.png",
    " -rw-rw-rw-   1 root  root      2540 Feb 05 20:19 166532401.png",
    " -rw-rw-rw-   1 root  root      2540 Feb 05 23:13 9240571371.png",
    " -rw-rw-rw-   1 root  root      4684 Feb 05 23:28 42885271611.png",
    " -rw-rw-rw-   1 root  root      4684 Feb 08 15:17 28260101861.png",
    " -rw-rw-rw-   1 root  root      4442 Feb 12 22:44 15137327011.png"
};

std::list <std::string> _model_logs = {
    " -rw-rw-rw-   1 root  root    326780 Jan 23 22:57 HKR -AMS Side mount spool holder.gcode.3mf",
    " -rw-rw-rw-   1 root  root    617858 Jan 23 23:55 X-Touch.gcode.3mf",
    " -rw-rw-rw-   1 root  root    595232 Jan 24 09:26 X1_Touch.gcode.3mf",
    " -rw-rw-rw-   1 root  root    413301 Jan 25 23:20 No-Catch Y-Splitter Self-Tap (long slot) (9.4mm-M3x6 mount).gcode.3mf",
    " -rw-rw-rw-   1 root  root    953086 Jan 31 03:09 1.gcode.3mf",
    " -rw-rw-rw-   1 root  root    698414 Jan 24 21:58 Bambu Trash_plate_1.gcode.3mf",
    " -rw-rw-rw-   1 root  root    301823 Jan 25 00:08 Bambu Trash_plate_3.gcode.3mf",
    " -rw-rw-rw-   1 root  root    176374 Jan 25 22:09 AMS_disconnect_tool_with_magnet_seperate_letters.gcode.3mf",
    " -rw-rw-rw-   1 root  root    710512 Jan 26 23:51 hex_bits_holder_7 x 2.gcode.3mf",
    " -rw-rw-rw-   1 root  root   1029925 Jan 27 13:22 hex_bits_holder_7x3.gcode.3mf",
    " -rw-rw-rw-   1 root  root    767304 Jan 27 13:26 swerve-gear-motorx6.gcode.3mf",
    " -rw-rw-rw-   1 root  root   1548586 Jan 28 04:35 rc_con_left_bottom.gcode.3mf",
    " -rw-rw-rw-   1 root  root    458648 Jan 28 11:41 swerve-axle-gearx3.gcode.3mf",
    " -rw-rw-rw-   1 root  root    970885 Jan 28 14:13 swerve-large-gear.gcode.3mf",
    " -rw-rw-rw-   1 root  root   2108337 Feb 03 15:51 controller-left.gcode.3mf",
    " -rw-rw-rw-   1 root  root    810058 Feb 05 20:19 제목 없음.gcode.3mf",
    " -rw-rw-rw-   1 root  root    810058 Feb 05 23:14 plate_holder.gcode.3mf",
    " -rw-rw-rw-   1 root  root    667533 Feb 05 23:28 rpi3p_case.gcode.3mf",
    " -rw-rw-rw-   1 root  root    667533 Feb 08 15:17 11111111.gcode.3mf",
    " -rw-rw-rw-   1 root  root    838822 Feb 12 22:45 galaxy watch stand.gcode.3mf",
};

class file_info_t {
public:
    long   ts;
    std::string filename;

    file_info_t(long ts, std::string filename) {
        this->ts = ts;
        this->filename = filename;
    }
};

class file_pair_t {
public:
    long   ts;
    std::string a;
    std::string b;

    file_pair_t() {
        this->ts = 0;
        this->a = "";
        this->b = "";
    }

    file_pair_t(long ts, std::string a, std::string b) {
        this->ts = ts;
        this->a = a;
        this->b = b;
    }
};

void parse(std::list <std::string> logs, std::list <file_info_t*> &result) {
    std::vector <std::string> tokens;
    std::vector <std::string> months = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Sep", "Nov", "Dec" };

    for (std::string line: logs) {
        tokens.clear();
        char *token = strtok((char*)line.c_str(), " :");
        while (token != NULL) {
            tokens.push_back(std::string(token));
            token = strtok(NULL, " :");
        }

        int mon  = std::find(months.begin(), months.end(), tokens[5]) - months.begin() + 1;
        int day  = std::stoi(tokens[6]);
        int hour = std::stoi(tokens[7]);
        int min  = std::stoi(tokens[8]);
        long ts  = min + (hour * 60) + (day * 60 * 24) + (mon * 60 * 24 * 31);
        // printf("%s %d-%d %d:%d %lu\n", tokens[9].c_str(), mon, day, hour, min, ts);

        std::string fname = tokens[9];
        for (int i = 10; i < tokens.size(); i++)
            fname = fname + ' ' + tokens[i];

        result.push_back(new file_info_t(ts, fname));
    }
}

void match_files(std::list <file_info_t*> a, std::list <file_info_t*> b, std::list <file_pair_t*> &result) {
    file_pair_t r;
    bool is_found;

    for (file_info_t *info_a : a) {
        is_found = false;
        for (file_info_t *info_b : b) {
            if (std::abs(info_a->ts - info_b->ts) < 3) {
                r.a = info_a->filename;
                r.b = info_b->filename;
                r.ts = info_a->ts;
                is_found = true;
                if (info_a->ts == info_b->ts)
                    break;
            }
        }
        if (is_found)
            result.push_back(new file_pair_t(r));
    }
}

int main() {
    std::list <file_info_t*> info_png;
    std::list <file_info_t*> info_model;
    std::list <file_pair_t*> pair;

    parse(_png_logs, info_png);
    // for (file_info_t *i:info_png)
    //     printf("%8ld, %s\n", i->ts, i->filename.c_str());

    parse(_model_logs, info_model);
    // for (file_info_t *i:info_model)
    //     printf("%8ld, %s\n", i->ts, i->filename.c_str());

    match_files(info_png, info_model, pair);
    int cnt = 1;
    for (file_pair_t *p:pair) {
        printf("%3d %10ld, %20s, %s\n", cnt++, p->ts, p->a.c_str(), p->b.c_str());
    }
}
