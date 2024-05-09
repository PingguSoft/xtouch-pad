//-------------------------------------------------------------------------------------------------
// CLASS
//-------------------------------------------------------------------------------------------------
class WS {
    static _ws = null;
    static _comp_printer = [];
    static _comp_ui = [];

    constructor(gateway) {
        this._gateway = gateway;
        this._init();
    }

    _init() {
        WS._ws = new WebSocket(this._gateway);
        WS._ws.onopen = WS._onOpen;
        WS._ws.onclose = WS._onClose;
        WS._ws.onmessage = WS._onMessage;
        WS._ws.binaryType = "arraybuffer";
    }

    setComponents(printer, ui) {
        WS._comp_printer = printer;
        WS._comp_ui = ui;
    }

    static _onOpen(event) {
        console.log('Connection opened');

        const json = {
            command: 'open'
        };
        const str = JSON.stringify(json);
        console.log(str);
        if (WS._ws.readyState == WebSocket.OPEN)
            WS._ws.send(str);
    }

    static _onClose(event) {
        console.log('Connection closed');
        setTimeout(this._init, 2000);
    }

    static _onMessage(event) {
        if (event.data instanceof ArrayBuffer) {
            const bytes = new Uint8Array(event.data);
            const cmd = bytes[0];
            const payload = bytes.subarray(1, event.data.length);

            for (const comp of WS._comp_ui) {
                comp.updateFromArrayBuffer(cmd, payload);
            }
        } else {
            var json = JSON.parse(event.data);
            console.log(event.data);
            if (json) {
                if (Object.keys(json).includes('webui')) {
                    json = json['webui'];
                    if (json) {
                        for (const comp of WS._comp_ui) {
                            comp.updateFromJson(json);
                        }
                    }
                } else if (Object.keys(json).includes('print')) {
                    json = json['print'];
                    if (json) {
                        for (const comp of WS._comp_printer) {
                            comp.updateFromJson(json);
                        }
                    }
                }
            }
        }
    }

    send(str) {
        console.log(str);
        if (WS._ws.readyState == WebSocket.OPEN)
            WS._ws.send(str);
    }

    sendJson(json) {
        const str = JSON.stringify(json);
        this.send(str);
    }

    sendGcode(code) {
        const json = {
            command: 'pub',
            data: {
                print: {
                    command: "gcode_line",
                    sequence_id: 0,
                    param: code,
                },
            }
        };
        this.sendJson(json);
    }
}

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


//
// Component Class
//
class Component {
    static Status = {
        UNKNOWN : 0,
        IDLE : 1,
        RUNNING : 2,
        PAUSED : 3,
        FINISHED : 4,
        PREPARE : 5,
        FAILED : 6
    };
    static _ws = null;
    static _status = Component.Status.UNKNOWN;

    constructor() {
    }

    static setWebSocket(socket) {
        Component._ws = socket;
    }

    updateFromJson(json) {
    }

    updateFromArrayBuffer(cmd, payload) {
    }
}

//
// Printing Class
//
class Printing extends Component {
    constructor() {
        super();
        this._printingFile = '';
    }

    getPrintingFile() {
        return this._printingFile;
    }

    setPrintingFile(file) {
        this._printingFile = file;
        return file;
    }

    _updateUI(status) {
        var printing_idle_disp;
        var printing_info_disp;
        var axis_control_disp;
        var printing_error_disp;

        switch (status) {
            case Component.Status.IDLE:
                printing_idle_disp = 'block';
                printing_info_disp = 'none';
                printing_error_disp = 'none';
                axis_control_disp = 'block';
                break;

            case Component.Status.FINISHED:
                printing_idle_disp = 'none';
                printing_info_disp = 'block';
                printing_error_disp = 'none';
                axis_control_disp = 'block';
                break;

            case Component.Status.FAILED:
                printing_idle_disp = 'none';
                printing_info_disp = 'none';
                printing_error_disp = 'block';
                axis_control_disp = 'none';
                // const err_json = {
                //     print_error: 83902522,
                // };
                // updateError(err_json);
                break;

            default:
                printing_idle_disp = 'none';
                printing_info_disp = 'block';
                printing_error_disp = 'none';
                axis_control_disp = 'none';
                break;
        }
        document.getElementById('printing_idle').style.display = printing_idle_disp;
        document.getElementById('printing_info').style.display = printing_info_disp;
        document.getElementById('axis_control').style.display  = axis_control_disp;
        document.getElementById('printing_error').style.display  = printing_error_disp;

        switch (status) {
            case Component.Status.RUNNING: {
                    document.getElementById('btn_print_icon').setAttribute('src', 'images/print_ctrl_pause.svg');
                    const disabled_printing = document.getElementsByClassName("disabled-printing");
                    for (const elt of disabled_printing) {
                        elt.disabled = true;
                        elt.className = elt.className.replace("btn-normal", "btn-normal-disabled");
                    }

                    const disabled_finished = document.getElementsByClassName("disabled-finished");
                    for (const elt of disabled_finished) {
                        elt.style.display = 'block';
                    }
                }
                break;

            case Component.Status.PAUSED:
                document.getElementById('btn_print_icon').setAttribute('src', 'images/print_ctrl_resume.svg');
                break;

            case Component.Status.FINISHED:
                const disabled = document.getElementsByClassName("disabled-finished");
                for (const elt of disabled) {
                    elt.style.display = 'none';
                }
                // no break

            default: {
                const disabled = document.getElementsByClassName("disabled-printing");
                for (const elt of disabled) {
                    elt.disabled = false;
                    elt.className = elt.className.replace("btn-normal-disabled", "btn-normal");
                }
            }
            break;
        }
    }

    updateFromJson(json) {
        const status_maps = [
            ['IDLE', Component.Status.IDLE],
            ['RUNNING', Component.Status.RUNNING],
            ['PAUSE', Component.Status.PAUSED],
            ['FINISH', Component.Status.FINISHED],
            ['PREPARE', Component.Status.PREPARE],
            ['FAILED', Component.Status.FAILED]
        ];
        var status = Component.Status.UNKNOWN;

        if (Object.keys(json).includes('gcode_state')) {
            for (const s of status_maps) {
                if (json['gcode_state'] == s[0]) {
                    status = s[1];
                    break;
                }
            }
            if (Component._status != status) {
                this._updateUI(status);
                Component._status = status;
            }
        }

        if (Component._status != Component.Status.IDLE) {
            const printing_maps = [
                ['subtask_name', 'printing_model_name', null],
                ['gcode_file', 'printing_model_aux', this.setPrintingFile],
                ['total_layer_num', 'printing_max_layer', null],
                ['layer_num', 'printing_cur_layer', null],
                ['mc_percent', 'printing_cur_prog_percentage', null],
                ['mc_percent', 'printing_cur_prog_bar', null],
                ['mc_remaining_time', 'printing_remaining_time', Util.formatTime],
            ];

            for (const x of printing_maps) {
                if (Object.keys(json).includes(x[0])) {
                    console.log(x[0] + ", " + x[1] + ", : " + json[x[0]]);
                    const elt = document.getElementById(x[1]);
                    if (elt) {
                        if (elt.nodeName.toLowerCase() == 'label') {
                            elt.innerText = (x[2] != null) ? x[2](json[x[0]]) : json[x[0]];
                        } else if (elt.nodeName.toLowerCase() == 'img') {
                            var name = (x[2] != null) ? x[2](json[x[0]]) : json[x[0]];
                            elt.setAttribute('src', name);
                        } else {
                            elt.value = (x[2] != null) ? x[2](json[x[0]]) : json[x[0]];
                        }
                    } else {
                        console.log("no : " + x[1]);
                    }
                }
            }
        }
    }

    doPrintAction(action) {
        const json = {
            command: 'pub',
            data: {
                print: {
                    command: action,
                    sequence_id: 0,
                    param: "",
                },
            }
        };
        Component._ws.sendJson(json);
    }

    onClickPauseResume(id) {
        switch (Component._status) {
            case Component.Status.RUNNING:
                this.doPrintAction("pause");
                break;

            case Component.Status.PAUSED:
                this.doPrintAction("resume");
                break;
        }
    }

    onClickStop(id) {
        this.doPrintAction("stop");
    }
}


//
// Temperature Class
//
class Temperature extends Component {
    constructor() {
        super();
    }

    updateFromJson(json) {
        const nodes = ['nozzle_temper', 'nozzle_target_temper', 'bed_temper', 'bed_target_temper', 'chamber_temper'];
        for (var i = 0; i < nodes.length; i++) {
            if (Object.keys(json).includes(nodes[i])) {
                const label = document.getElementById(nodes[i]);
                if (label) {
                    const val = Math.round(json[nodes[i]]);
                    if (label.nodeName.toLowerCase() == 'input')
                        label.value = val;
                    else
                        label.innerText = val;

                    console.log(nodes[i], val);
                }
            }
        }
    }

    changeTemp(id) {
        const elt = document.getElementById(id);

        if (elt) {
            console.log("onChange : " + id + " " + elt.value);

            var gcode;
            if (id == "nozzle_target_temper") {
                gcode = "M104 S" + String(elt.value) + "\n";
            } else if (id == "bed_target_temper") {
                gcode = "M140 S" + String(elt.value) + "\n";
            }
            Component._ws.sendGcode(gcode);
        }
    }
}


//
// Filament Class
//
class Filaments extends Component {
    constructor() {
        super();
        this._is_ams = false;
        this._is_vt_tray = false;
        this._sel_tray = -1;
    }

    updateFromJson(json) {
        if (Object.keys(json).includes('vt_tray')) {
            this._is_vt_tray = true;
            var tray = json['vt_tray'];

            var btn = document.getElementById('btn_vt_tray');
            if (btn) {
                btn.style.background = '#' + tray['tray_color'];
            }

            var label = document.getElementById('vt_tray_type');
            if (label) {
                label.innerText = tray['tray_type'];
            }
        }

        if (Object.keys(json).includes('ams')) {
            json = json['ams'];
            this._is_ams = (json['ams_exist_bits'] != null);
            console.log("ams : " + this._is_ams);

            if (this._is_ams) {
                json = json['ams']['0']['tray'];

                if (json) {
                    const keys = Object.keys(json);

                    for (var i = 0; i < keys.length; i++) {
                        const idx = String(i);
                        const btn = 'btn_tray' + idx;
                        const tt = 'tray' + idx + '_type';
                        const tray = json[idx];

                        console.log(tray);
                        var elt = document.getElementById(btn);
                        if (elt) {
                            elt.style.background = '#' + tray['tray_color'];
                        }

                        var label = document.getElementById(tt);
                        if (label) {
                            label.innerText = tray['tray_type'];
                        }
                    }
                }
            }
            document.getElementById('ams_cell').style.display = this._is_ams ? '' : 'none';
        }
    }

    onClickLoadVTButton(id) {
        console.log("onClickLoadVTButton : " + id);

        if (!this._is_vt_tray || Component._status == Component.Status.RUNNING || Component._status == Component.Status.PAUSED)
            return;

        const gcode = "M620 S254\nM106 S255\nM104 S250\nM17 S\nM17 X0.5 Y0.5\nG91\nG1 Y-5 F1200\nG1 Z3\nG90\nG28 X\n" +
                      "M17 R\nG1 X70 F21000\nG1 Y245\nG1 Y265 F3000\nG4\nM106 S0\nM109 S250\nG1 X90\nG1 Y255\nG1 X120\nG1 X20 Y50 F21000\nG1 Y-3\n" +
                      "T254\nG1 X54\nG1 Y265\nG92 E0\nG1 E40 F180\nG4\nM104 S0\nG1 X70 F15000\nG1 X76\nG1 X65\nG1 X76\nG1 X65\nG1 X90 F3000\nG1 Y255\n" +
                      "G1 X100\nG1 Y265\nG1 X70 F10000\nG1 X100 F5000\nG1 X70 F10000\nG1 X100 F5000\nG1 X165 F12000\nG1 Y245\nG1 X70\nG1 Y265 F3000\nG91\n" +
                      "G1 Z-3 F1200\nG90\nM621 S254\n\n";
        Component._ws.sendGcode(gcode);
    }

    onClickUnloadVTButton(id) {
        console.log("onClickUnloadVTButton : " + id);

        if (!this._is_vt_tray || Component._status == Component.Status.RUNNING || Component._status == Component.Status.PAUSED)
            return;

        const gcode = "M620 S255\nM106 P1 S255\nM104 S250\nM17 S\nM17 X0.5 Y0.5\nG91\nG1 Y-5 F3000\nG1 Z3 F1200\nG90\nG28 X\n"+
                      "M17 R\nG1 X70 F21000\nG1 Y245\nG1 Y265 F3000\nG4\nM106 P1 S0\nM109 S250\nG1 X90 F3000\nG1 Y255 F4000\n" +
                      "G1 X100 F5000\nG1 X120 F21000\nG1 X20 Y50\nG1 Y-3\nT255\nG4\nM104 S0\nG1 X70 F3000\n\nG91\nG1 Z-3 F1200\nG90\nM621 S255\n\n";
        Component._ws.sendGcode(gcode);
    }

    onClickLoadAMSButton(id) {
        if (Component._status == Component.Status.RUNNING || Component._status == Component.Status.PAUSED)
            return;
    }

    onClickUnloadAMSButton(id) {
        if (Component._status == Component.Status.RUNNING || Component._status == Component.Status.PAUSED)
            return;
    }

    onClickAMSTrayRefresh(evt) {
        if (Component._status == Component.Status.RUNNING || Component._status == Component.Status.PAUSED)
            return;

        const ams_ref = document.getElementsByClassName("ams-tray-ref");
        for (var i = 0; i < ams_ref.length; i++) {
            if (ams_ref[i] == evt.currentTarget) {
                var idx = i;
                // refresh ams tray info for ith
                break;
            }
        }
    }

    onClickAMSTray(evt) {
        if (Component._status == Component.Status.RUNNING || Component._status == Component.Status.PAUSED)
            return;

        const ams_trays = document.getElementsByClassName("ams-tray");
        for (var ams of ams_trays) {
            ams.className = ams.className.replace(" btn-selected", "");
        }

        for (var i = 0; i < ams_trays.length; i++) {
            if (ams_trays[i] == evt.currentTarget) {
                this._sel_tray = i;
                evt.currentTarget.className += " btn-selected";
            }
        }
    }
}


//
// Light Class
//
class Lights extends Component {
    constructor() {
        super();
        this._is_on = false;
    }

    updateFromJson(json) {
        if (Object.keys(json).includes('lights_report')) {
            var light = json['lights_report'];

            for (var i = 0; i < light.length; i++) {
                var name = 'img_' + light[i]['node'];
                var img = document.getElementById(name);
                if (img) {
                    this._is_on = (light[i]['mode'] == 'on');
                    img.setAttribute('src',  this._is_on ? 'images/ic_light_on.svg' : 'images/ic_light_off.svg');
                }

                name = 'label_' + light[i]['node'];
                var label = document.getElementById(name);
                if (label) {
                    label.innerText = this._is_on ? 'on' : 'off';
                }
            }
        }
    }

    turnOnOff(id) {
        var id_label = id.replace("btn_", "label_");
        var elt_label = document.getElementById(id_label);

        if (elt_label) {
            const new_val = (elt_label.innerText == "off") ? 100 : 0;
            const id_img = id.replace("btn_", "img_");
            var elt_img = document.getElementById(id_img);
            if (elt_img) {
                elt_img.setAttribute("src", (new_val == 0) ? "images/ic_light_off.svg" : "images/ic_light_on.svg")
            }
            elt_label.innerText = (new_val == 0) ? "off" : "on";
            console.log("onClickLight : ", elt_label.innerText);
            id_label = id.replace("btn_", "");

            const json = {
                command: 'pub',
                data: {
                    system: {
                        command: "ledctrl",
                        led_node: id_label,
                        sequence_id: 0,
                        led_mode: elt_label.innerText,
                        led_on_time: 500,
                        led_off_time: 500,
                        loop_times: 0,
                        interval_time: 0,
                    },
                }
            };
            Component._ws.sendJson(json);
        }
    }
}


//
// Fans Class
//
class Fans extends Component {
    constructor() {
        super();
        this._table_fans = [
            'cooling_fan_speed',
            'big_fan1_speed',
            'big_fan2_speed',
            'heatbreak_fan_speed'
        ];
    }

    updateFromJson(json) {
        var speeds = [-1, -1, -1, -1];

        if (Object.keys(json).includes('fan_gear')) {
            // max : 255
            const gear = json['fan_gear'];
            speeds[0] = (gear >> 0) & 0xFF;
            speeds[1] = (gear >> 8) & 0xFF;
            speeds[2] = (gear >> 16) & 0xFF;
            speeds[3] = (gear >> 24) & 0xFF;
            console.log("fan_gear : " + gear.toString(16) + " " + speeds);
        } else {
            for (var i = 0; i < this._table_fans.length; i++) {
                if (Object.keys(json).includes(this._table_fans[i])) {
                    // max : 15
                    speeds[i] = Math.round(Math.floor(json[this._table_fans[i]] / 1.5) * 25.5);
                }
            }
            console.log("fan_speed : " + speeds);
        }

        // update icon and percentage only for updated ones
        for (var i = 0; i < this._table_fans.length; i++) {
            if (speeds[i] >= 0) {
                var speed = (speeds[i] == 0) ? 0 : Math.max(10, Math.round(speeds[i] * 100 / 255.0));
                var name = 'img_' + this._table_fans[i];

                var img = document.getElementById(name);
                if (img) {
                    img.setAttribute('src', (speed > 0) ? 'images/ic_fan_on.svg' : 'images/ic_fan_off.svg');
                }

                name = 'label_' + this._table_fans[i];
                var label = document.getElementById(name);
                if (label) {
                    if (label.nodeName.toLowerCase() == 'input')
                        label.value = speed;
                    else
                        label.innerText = (speed > 0) ? String(speed) + '%' : 'off';
                }
            }
        }
    }

    changeSpeed(id) {
        const elt = document.getElementById(id);
        if (elt) {
            console.log("onChange : " + id + " " + elt.value);
            var idx = 1;
            for (const fan of this._table_fans) {
                if (id.endsWith(fan)) {
                    const id_img = id.replace("label_", "img_");
                    var elt_img = document.getElementById(id_img);
                    if (elt_img) {
                        elt_img.setAttribute("src", (elt.value == 0) ? "images/ic_fan_off.svg" : "images/ic_fan_on.svg")
                    }

                    const speed = Math.round(elt.value * 2.55);
                    Component._ws.sendGcode("M106 P" + idx + " S" + speed + " \n");
                    break;
                }
                idx++;
            }
        }
    }

    turnOnOff(id) {
        const id_label = id.replace("btn_", "label_");
        var   elt_label = document.getElementById(id_label);

        if (elt_label && elt_label.nodeName.toLowerCase() == 'input') {
            const new_val = (elt_label.value == 0) ? 100 : 0;
            elt_label.value = new_val;
            console.log("onClickFan : ", new_val);
            this.changeSpeed(id_label);
        }
    }
}


//
// Speed Class
//
class Speed extends Component {
    constructor() {
        super();
        this._speed = 0;
    }

    updateFromJson(json) {
        if (Object.keys(json).includes('spd_lvl')) {
            console.log("spd_lvl : " + String(json['spd_lvl']));
            var combo = document.getElementById('spd_lvl');
            if (combo) {
                this._speed = json['spd_lvl'] - 1;
                combo.options[this._speed].selected = true;
            }
        }
    }

    changeSpeed(id) {
        const elt = document.getElementById(id);
        if (elt) {
            console.log("onChange : " + id + " " + elt.value + " " + elt.selectedIndex);
            const json = {
                command: 'pub',
                data: {
                    print: {
                        command: "print_speed",
                        sequence_id: 0,
                        param: String(elt.selectedIndex + 1),
                    },
                }
            };
            Component._ws.sendJson(json);
        }
    }
}


//
// Errors Class
//
class Errors extends Component {
    constructor() {
        super();
        this._err_tbl = {
            codes: [],
            msgs: []
        };

        var json = JSON.parse(JSON.stringify(hms_err_json));
        var err = json['data']['device_error']['en'];

        for (const x of err) {
            this._err_tbl.codes.push(x.ecode);
            this._err_tbl.msgs.push(x.intro);
        }
    }

    updateFromJson(json) {
        if (Object.keys(json).includes('print_error')) {
            const error = json['print_error'];
            const str_code = Util.pad(error.toString(16), 8).toUpperCase();
            const idx = this._err_tbl.codes.indexOf(str_code);
            console.log(this._err_tbl);
            console.log("print error : " + str_code + ", len:" + str_code.length);
            console.log("print error : " + this._err_tbl.codes[0] + ", len:" + this._err_tbl.codes[0]);
            console.log("idx:" + idx);

            if (idx >= 0) {
                var label = document.getElementById('label_error');
                label.innerText = "Device Error !";

                label = document.getElementById('label_error_desc');
                label.innerText = this._err_tbl.msgs[idx];
            }
        }
    }
}

//
// HMS Class
//
class HMS extends Component {
    constructor() {
        super();
        this._hms_tbl = {
            codes: [],
            msgs: []
        };

        var json = JSON.parse(JSON.stringify(hms_err_json));
        var hms = json['data']['device_hms']['en'];
        for (const x of hms) {
            this._hms_tbl.codes.push(x.ecode);
            this._hms_tbl.msgs.push(x.intro);
        }
    }

    updateFromJson(json) {
        if (Object.keys(json).includes('hms')) {
            const hms = json['hms'];

            for (const v of hms) {
                const attr = v['attr'];
                const code = v['code'];

                const module_id = (attr >> 24) & 0xff;
                const module_no = (attr >> 16) & 0xff;
                const part_id   = (attr >>  8) & 0xff;
                const part_no   = (attr >>  0) & 0xff;

                const alert_level = (code >> 16) & 0xffff;
                const error_code  = (code >>  0) & 0xffff;

                const str_code = Util.pad(module_id.toString(16), 2) + Util.pad(module_no.toString(16), 2) + Util.pad(part_id.toString(16), 2) +
                                 Util.pad(alert_level.toString(16), 6) + Util.pad(error_code.toString(16), 4);
                console.log("print HMS : " + str_code);

                var idx = _hms_tbl.codes.indexOf(str_code);
                if (idx >= 0) {
                    var label = document.getElementById('label_error');
                    label.innerText = "HMS !";

                    label = document.getElementById('label_error_desc');
                    label.innerText = _hms_tbl.msgs[idx];
                }
            }
        }
    }
}


//
// Camera Class
//
class Camera extends Component {
    constructor() {
        super();
        this._is_ipcam = false;
        this._is_ipcam_record = false;
        this._is_ipcam_timelapse = false;
        this._is_ipcam_show = false;
        this._is_enabled = true;
    }

    updateFromJson(json) {
        if (Object.keys(json).includes('ipcam')) {
            const ipcam = json['ipcam'];

            if (Object.keys(ipcam).includes('ipcam_dev')) {
                this._is_ipcam = (ipcam['ipcam_dev'] == '1');
            }
            if (Object.keys(ipcam).includes('ipcam_record')) {
                this._is_ipcam_record = (ipcam['ipcam_record'] == 'enable');
            }
            if (Object.keys(ipcam).includes('timelapse')) {
                this._is_ipcam_timelapse = (ipcam['timelapse'] == 'enable');
            }
            console.log("ipcam : " + String(ipcam));
            document.getElementById('cam_monitoring').style.display = this._is_ipcam ? 'block' : 'none';
        }
    }

    updateFromArrayBuffer(cmd, payload) {
        var image;
        switch (cmd) {
            case 0x01: {        // jpeg
                image = document.getElementById('camera_view');
                if (image && this._is_ipcam_show) {
                    image.src = 'data:image/jpeg;base64,' + Util.encode(payload);
                }
            }
            break;

            case 0x02: {        // png
                image = document.getElementById('model_view');
                if (image)
                    image.src = 'data:image/png;base64,' + Util.encode(data);
            }
            break;
        }
    }

    turnOn(on) {
        if (!this._is_ipcam)
            return;

        this._is_ipcam_show = on;
        var image = document.getElementById('camera_view');
        if (image) {
            if (!this._is_ipcam_show)
                image.src = 'images/m_camera.svg';
        }
        const json = {
            command: 'camera_view',
            data: this._is_ipcam_show
        };
        Component._ws.sendJson(json);
    }

    enable(en) {
        this._is_enabled = en;
        var elt = document.getElementById('cam_monitoring');
        elt.className = (en) ? elt.className.replace("cam-view-disabled", "cam-view") :
                            elt.className.replace("cam-view", "cam-view-disabled");
    }

    isEnabled() {
        return this._is_enabled;
    }

    isOn() {
        return this._is_ipcam && this._is_ipcam_show;
    }

    turnOnOff() {
        this.turnOn(!this._is_ipcam_show)
    }
}


//
// Others Class
//
class Others extends Component {
    constructor() {
        super();
        this.is_sdcard = false;
    }

    updateFromJson(json) {
        // sdcard check
        if (Object.keys(json).includes('sdcard')) {
            this.is_sdcard = json['sdcard'];
            document.getElementById('sdcard_cell').style.display = this.is_sdcard ? '' : 'none';
        }
    }
}


//
// PrinterInfo Class
//
class PrinterInfo extends Component {
    constructor() {
        super();
        this.name = null;
    }

    updateFromJson(json) {
        if (Object.keys(json).includes('printer_name')) {
            var title = document.getElementById('printer_name');
            if (json['printer_name']) {
                title.innerText = json['printer_name'];
                this.name = json['printer_name'];
            }
        }
    }
}


//
// SDBrowser Class
//
class SDBrowser extends Component {
    constructor() {
        super();
        this._sel_model_id = '';
        this._is_sd_reloading = false;
        this._sdcard_model_list = [
            {
                "ts": 1955357392,
                "size": 766263,
                "3mf": "Cute Schnauzer keychain.gcode.3mf",
                "png": "17540064941.png"
            },
            {
                "ts": 1955353346,
                "size": 1196949,
                "3mf": "hello-kitty-6-Dom.gcode.3mf",
                "png": "29877475381.png"
            },
            {
                "ts": 1955353337,
                "size": 1573271,
                "3mf": "Pikachu.gcode.3mf",
                "png": "1270915251.png"
            },
        ];
        this._reload =
        {
            "ts": 0,
            "size": 0,
            "3mf": "Reload",
            "png": "reload.svg"
        };

        // add reload in the sdcard_model_list
        if (this._sdcard_model_list.length == 0 ||
            (this._sdcard_model_list.length != 0 && this._sdcard_model_list.at(this._sdcard_model_list.length - 1) != this._reload)) {
                this._sdcard_model_list.push(this._reload);
        }
    }

    updateFromJson(json) {
        if (Object.keys(json).includes('sdcard_list')) {
            console.log(json);
            json = json['sdcard_list'];
            this._sdcard_model_list = json;
            this._sdcard_model_list.push(this._reload);
            this._is_sd_reloading = false;
            this._drawModels(this._sdcard_model_list);
        }
    }

    updateTabView() {
        this._drawModels(this._sdcard_model_list);
    }

    reloadSD() {
        const json = {
            command: 'sdcard_list',
        };
        Component._ws.sendJson(json);
        this._is_sd_reloading = true;
        this.onClickPopupCancel();
        this._drawModels(this._sdcard_model_list);
    }

    cancelReload() {
        const json = {
            command: 'sdcard_list_cancel',
        };
        Component._ws.sendJson(json);
        this._is_sd_reloading = false;
        this.updateTabView();
    }

    onClickModel(event, id) {
        console.log("onClickModel:", id, ' ', event.x, ' ', event.y);

        if (this._sel_model_id) {
            var elt = document.getElementById(this._sel_model_id);
            if (elt) {
                elt.className = elt.className.replace(" model-selected", "");
            }
        }

        var elt = document.getElementById(id);
        if (elt) {
            elt.className += " model-selected";
        }
        this._sel_model_id = id;

        var popup = document.getElementById("popup-menu");
        if (popup) {
            popup.style.display = "block";
            const tab = document.getElementById("tab_sdcard");
            if (tab) {
                const rect = tab.getBoundingClientRect();
                var x = Math.min(event.x - rect.left, rect.right - 150);
                var y = Math.min(event.y - rect.top, rect.bottom - 150);
                popup.style.marginLeft = String(x) + "px";
                popup.style.marginTop = String(y) + "px";
            }
        }
    }

    onClickPopupCancel() {
        var popup = document.getElementById("popup-menu");
        if (popup) {
            popup.style.display = "none";
        }

        if (this._sel_model_id) {
            var elt = document.getElementById(this._sel_model_id);
            if (elt) {
                elt.className = elt.className.replace(" model-selected", "");
            }
            this._sel_model_id = "";
        }
    }

    onClickModelPrint() {
        if (this._sel_model_id) {
            var pos = this._sel_model_id.replace("model_id_", "");
            var p = parseInt(pos, 10);
            console.log("onClickModelPrint:" + this._sdcard_model_list[p]["3mf"]);

            const json = {
                command: 'print',
                data: {
                    "3mf" : this._sdcard_model_list[p]["3mf"],
                    "png" : this._sdcard_model_list[p]["png"]
                }
            };
            Component._ws.sendJson(json);
        }
        this.onClickPopupCancel();
    }

    onClickModelDelete() {
        if (this._sel_model_id) {
            var pos = this._sel_model_id.replace("model_id_", "");
            var p = parseInt(pos, 10);
            console.log("onClickModelDelete:" + p);

            this._sdcard_model_list.splice(p, 1);
            this._drawModels(this._sdcard_model_list);

            const json = {
                command: 'delete',
                data: {
                    "3mf" : this._sdcard_model_list[p]["3mf"],
                    "png" : this._sdcard_model_list[p]["png"]
                }
            };
            Component._ws.sendJson(json);
        }
        this.onClickPopupCancel();
    }

    isReloading() {
        return this._is_sd_reloading;
    }

    getModelPNG(name) {
        var png = "images/ic_comment_model.svg";

        for (const item of this._sdcard_model_list) {
            if (item['3mf'] == name) {
                png = item['png'];
                break;
            }
        }
        return png;
    }

    _drawModels(models) {
        var row = '';
        var col;

        if (this._is_sd_reloading) {
            row = '<img src="images/reload.gif" style="vertical-align: center;" onClick="onClickReloadIcon()">';
        } else {
            this._sel_model_id = '';
            for (var i = 0; i < models.length; i++) {
                if ((i % 3) == 0) {
                    if (i != 0)
                        row += '</tr>\n';
                    row += '<tr>\n';
                }
                const name = models[i]["3mf"].replace(".gcode.3mf", "");
                if (models[i]["ts"] == 0 && models[i]["size"] == 0 && models[i]["3mf"] == "Reload") {
                    col = '<td>\n' +
                        '<img src="images/' + models[i]["png"] + '"' + 'height="128px" width="128px" id="model_id_' + String(i) +
                        '" onClick="onClickReloadSD()" class="none">\n' +
                        '<label>' + name + '</label>\n' +
                        '</td>\n';
                } else {
                    col = '<td>\n' +
                        '<img src="sd/image/' + models[i]["png"] + '"' + 'height="128px" width="128px" id="model_id_' + String(i) +
                        '" onClick="onClickModel(event, this.id)" class="none">\n' +
                        '<label>' + name + '</label>\n' +
                        '</td>\n';
                }
                row += col;
            }

            var remain = 3 - (models.length % 3);
            if (remain != 3) {
                for (var i = 0; i < remain; i++) {
                    row += '<td>\n</td>\n';
                }
            }
            row += '</tr>\n';
        }

        var table = document.getElementById("model_table");
        if (table)
            table.innerHTML = row;
    }
}

//
// AXISControl Class
//
class AXISControl extends Component {
    constructor() {
        super();
    }

    onClickAxisButton(id) {
        console.log("onClickAxisButton : " + id);

        if (Component._status == Component.Status.RUNNING || Component._status == Component.Status.PAUSED)
            return;

        const axis_maps = [
            ['btn_head_y_p10', 'Y', 10],
            ['btn_head_y_m10', 'Y', -10],
            ['btn_head_y_p1', 'Y', 1],
            ['btn_head_y_m1', 'Y', -1],
            ['btn_head_x_p10', 'X', 10],
            ['btn_head_x_m10', 'X', -10],
            ['btn_head_x_p1', 'X', 1],
            ['btn_head_x_m1', 'X', -1],

            ['btn_bed_p10', 'Z', -10],
            ['btn_bed_p1', 'Z', -1],
            ['btn_bed_m10', 'Z', 10],
            ['btn_bed_m1', 'Z', 1],
        ];

        const speed_xy = 3000;
        const speed_z  = 1500;

        for (const axis of axis_maps) {
            if (id == axis[0]) {
                const speed = (axis[1] == 'Z') ? speed_z : speed_xy;
                const gcode = "M211 S\nM211 X1 Y1 Z1\nM1002 push_ref_mode\nG91\nG1 " + axis[1] + axis[2] + " F" + speed + "\nM1002 pop_ref_mode\nM211 R\n";
                Component._ws.sendGcode(gcode);
                break;
            }
        }
    }

    onClickExtruderButton(id) {
        console.log("onClickExtruderButton : " + id);

        if (Component._status == Component.Status.RUNNING || Component._status == Component.Status.PAUSED)
            return;

        if (id == "btn_extr_up") {
            Component._ws.sendGcode("M83\nG0 E-10.0 F900\n");
        } else if (id == "btn_extr_down") {
            Component._ws.sendGcode("M83\nG0 E10.0 F900\n");
        }
    }

    onClickHomeButton(id) {
        console.log("onClickHomeButton : " + id);
        if (Component._status == Component.Status.RUNNING || Component._status == Component.Status.PAUSED)
            return;
        Component._ws.sendGcode("G28\n");
    }
}

//
// Controller Class
//
class Controller extends Component {
    constructor() {
        super();
        this._sel_menu_id = '';
    }

    updateTabView(id) {
        const elts = document.getElementsByName("full_height_view");
        for (var v of elts) {
            const rect = v.getBoundingClientRect();
            v.style.height = String(window.innerHeight - (Math.round(rect.top) + 2)) + "px";
        }

        if (id == "menu_sdcard") {
            _sdbrowser.updateTabView();
        }
    }

    openTab(id) {
        // Get all elements with class="tabcontent" and hide them
        const tabs = document.getElementsByClassName("tabcontent");
        for (var t of tabs) {
            t.style.display = "none";
        }
        const tab_id = id.replace("menu_", "tab_");
        var elt = document.getElementById(tab_id);
        if (elt) {
            elt.style.display = 'block';
        }

        // mark selected menu
        const links = document.getElementsByClassName("tablink");
        for (var l of links) {
            l.className = l.className.replace(" btn-selected", "");
        }
        elt = document.getElementById(id);
        if (elt) {
            elt.className += " btn-selected";
        }
        this.updateTabView(id);
        this._sel_menu_id = id;
    }

    onResize(event) {
        this.updateTabView(this._sel_menu_id);
    }

    onClickCamera() {
        if (!_sdbrowser.isReloading())
            _camera.turnOnOff();
    }

    onClickReloadSD() {
        if (_camera.isOn()) {
            _camera.turnOn(false);
        }
        _camera.enable(false);
        _sdbrowser.reloadSD();
    }

    onClickReloadIcon() {
        _sdbrowser.cancelReload();
        _camera.enable(true);
    }

    updateFromJson(json) {
        if (!_sdbrowser.isReloading() && !_camera.isEnabled())
            _camera.enable(true);

        // update printing model PNG
        if (Object.keys(json).includes('gcode_file')) {
            const elt = document.getElementById('printing_model_png');
            if (elt) {
                const name = _sdbrowser.getModelPNG(_printing.getPrintingFile());
                elt.setAttribute('src', name);
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
// VARIABLES
//-------------------------------------------------------------------------------------------------
// printer components
var _printing = new Printing();
var _temperature = new Temperature();
var _filaments = new Filaments();
var _lights = new Lights();
var _fans = new Fans();
var _speed = new Speed();
var _errors = new Errors();
var _hms = new HMS();
var _camera = new Camera();
var _others = new Others();
var _axiscontrol = new AXISControl();

// ui components
var _printerinfo = new PrinterInfo();
var _sdbrowser = new SDBrowser();
var _controller = new Controller();


//-------------------------------------------------------------------------------------------------
// CODES
//-------------------------------------------------------------------------------------------------
window.addEventListener('load', onLoad);
window.addEventListener('resize', onResize);

function onLoad(event) {
    const websocket = new WS(`ws://${window.location.hostname}/ws`);
    const print_comp = [_printing, _temperature, _filaments, _lights, _fans, _speed, _errors, _hms, _camera, _others ];
    const ui_comp = [_printerinfo, _camera, _sdbrowser, _controller];

    websocket.setComponents(print_comp, ui_comp);
    Component.setWebSocket(websocket);
}

function openTab(id) {
    _controller.openTab(id);
}

function onResize(event) {
    _controller.onResize(event);
}

function onClickConfirm(id) {
}


//-------------------------------------------------------------------------------------------------
// UI events
//-------------------------------------------------------------------------------------------------

//
// Fan, Temperature, Speed
//
function onChangeFanSpeed(id) {
    _fans.changeSpeed(id);
}

function onChangeTemp(id) {
    _temperature.changeTemp(id);
}

function onChangeSpeed(id) {
    _speed.changeSpeed(id);
}

function onClickFanOnOff(id) {
    _fans.turnOnOff(id);
}

function onClickLightOnOff(id) {
    _lights.turnOnOff(id);
}

//
// AXIS control
//
function onClickAxisButton(id) {
    _axiscontrol.onClickAxisButton(id);
}

function onClickExtruderButton(id) {
    _axiscontrol.onClickExtruderButton(id);
}

function onClickHomeButton(id) {
    _axiscontrol.onClickHomeButton(id);
}


//
// Filament / AMS control
//
function onClickLoadVTButton(id) {
    _filaments.onClickLoadVTButton(id);
}

function onClickUnloadVTButton(id) {
    _filaments.onClickUnloadVTButton(id);
}

function onClickLoadAMSButton(id) {
    _filaments.onClickLoadAMSButton(id);
}

function onClickUnloadAMSButton(id) {
    _filaments.onClickUnloadAMSButton(id);
}

function onClickAMSTrayRefresh(evt) {
    _filaments.onClickAMSTrayRefresh(evt);
}

function onClickAMSTray(evt) {
    _filaments.onClickAMSTray(evt);
}

//
// Print / Stop
//
function onClickPauseResume(id) {
    _printing.onClickPauseResume(id);
}

function onClickStop(id) {
    _printing.onClickStop(id);
}

function onClickCamera() {
    _controller.onClickCamera();
}

//
// SD Card Browser
//
function onClickReloadSD() {
    _controller.onClickReloadSD();
}

function onClickModel(event, id) {
    _sdbrowser.onClickModel(event, id);
}

function onClickPopupCancel() {
    _sdbrowser.onClickPopupCancel();
}

function onClickModelPrint() {
    _sdbrowser.onClickModelPrint();
}

function onClickModelDelete() {
    _sdbrowser.onClickModelDelete();
}

function onClickReloadIcon() {
    _controller.onClickReloadIcon();
}