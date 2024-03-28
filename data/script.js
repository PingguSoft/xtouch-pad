const Status = {
    UNKNOWN : 0,
    IDLE : 1,
    RUNNING : 2,
    PAUSED : 3,
    FINISHED : 4,
    PREPARE : 5,
    FAILED : 6
};

const Fans = ['cooling_fan_speed', 'big_fan1_speed', 'big_fan2_speed', 'heatbreak_fan_speed'];

var _selected_tray = -1;
var _selected_model_id = '';
var _selected_menu_id = '';
var _gateway = `ws://${window.location.hostname}/ws`;
var _websocket;
var _printer = {
    name : "",
    status: Status.UNKNOWN,
    is_vt_tray: false,
    is_ams: false,
    is_light_on: false,
};


window.addEventListener('load', onLoad);
window.addEventListener('resize', onResize);

//
// Web Socket
//
function onLoad(event) {
    initWebSocket();
    updateUI(Status.IDLE);
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    _websocket = new WebSocket(_gateway);
    _websocket.onopen = onOpen;
    _websocket.onclose = onClose;
    _websocket.onmessage = onMessage;
    _websocket.binaryType = "arraybuffer";
}

function onOpen(event) {
    console.log('Connection opened');

    const json = {
        command: 'open'
    };
    const str = JSON.stringify(json);
    console.log(str);
    if (_websocket.readyState == WebSocket.OPEN)
        _websocket.send(str);
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function encode(input) {
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


//-------------------------------------------------------------------------------------------------
// incoming MQTT message handlers
//-------------------------------------------------------------------------------------------------
function pad(n, width) {
    n = n + '';
    return n.length >= width ? n : new Array(width - n.length + 1).join('0') + n;
}

function updateUI(status) {
    var printing_idle_disp;
    var printing_info_disp;
    var axis_control_disp;

    switch (status) {
        case Status.IDLE:
            printing_idle_disp = 'block';
            printing_info_disp = 'none';
            axis_control_disp = 'block';
            break;

        case Status.FINISHED:
            printing_idle_disp = 'none';
            printing_info_disp = 'block';
            axis_control_disp = 'block';
            break;

        default:
            printing_idle_disp = 'none';
            printing_info_disp = 'block';
            axis_control_disp = 'none';
            break;
    }
    document.getElementById('printing_idle').style.display = printing_idle_disp;
    document.getElementById('printing_info').style.display = printing_info_disp;
    document.getElementById('axis_control').style.display  = axis_control_disp;

    switch (status) {
        case Status.RUNNING: {
                document.getElementById('btn_print_icon').setAttribute('src', 'images/print_ctrl_pause.svg');
                const disabled = document.getElementsByClassName("disabled-printing");
                for (const elt of disabled) {
                    elt.disabled = true;
                    elt.className = elt.className.replace("btn-normal", "btn-normal-disabled");
                }
            }
            break;

        case Status.PAUSED:
            document.getElementById('btn_print_icon').setAttribute('src', 'images/print_ctrl_resume.svg');
            break;

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

function updatePrintingState(json) {
    const status_maps = [
        ['IDLE', Status.IDLE],
        ['RUNNING', Status.RUNNING],
        ['PAUSE', Status.PAUSED],
        ['FINISH', Status.FINISHED],
        ['PREPARE', Status.PREPARE],
        ['FAILED', Status.FAILED]
    ];

    var status = Status.UNKNOWN;
    if (Object.keys(json).includes('gcode_state')) {
        for (const s of status_maps) {
            if (json['gcode_state'] == s[0]) {
                status = s[1];
                break;
            }
        }

        if (status == Status.UNKNOWN)
            return;

        if (_printer.status != status) {
            updateUI(status);
            _printer.status = status;
        }
        if (_printer.status != Status.IDLE) {
            const printing_maps = [
                ['subtask_name', 'printing_model_name', 1],
                ['gcode_file', 'printing_model_aux', 1],
                ['total_layer_num', 'printing_max_layer', 1],
                ['layer_num', 'printing_cur_layer', 1],
                ['mc_percent', 'printing_cur_prog_percentage', 1],
                ['mc_percent', 'printing_cur_prog_bar', 2],
                ['mc_remaining_time', 'printing_remaining_time', 3],
            ];

            for (const x of printing_maps) {
                console.log(x[0] + ", " + x[1] + ", : " + json[x[0]]);
                if (Object.keys(json).includes(x[0])) {
                    const elt = document.getElementById(x[1]);
                    if (elt) {
                        if (x[2] == 1) {
                            elt.innerText = json[x[0]];
                        } else if (x[2] == 2) {
                            elt.value = json[x[0]];
                        } else if (x[2] == 3) {
                            var min = json[x[0]];
                            var days = Math.floor(min / (60 * 24));
                            min %= (60 * 24);
                            var hours = Math.floor(min / 60);
                            min %= 60;

                            var str = "";
                            if (days > 0)
                                str = str + pad(days, 2) + "d ";
                            if (hours > 0)
                                str = str + pad(hours, 2) + "h ";
                            str = str + pad(min, 2) + "m";
                            elt.innerText = str;
                        }
                    } else {
                        console.log("no : " + x[1]);
                    }
                }
            }
        }
    }
}

function updateTemperature(json) {
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

function updateFilaments(json) {
    if (Object.keys(json).includes('vt_tray')) {
        _printer.is_vt_tray = true;
        tray = json['vt_tray'];

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
        _printer.is_ams = (json['ams_exist_bits'] != null);
        console.log("ams : " + _printer.is_ams);

        if (_printer.is_ams) {
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
        document.getElementById('ams_cell').style.display = _printer.is_ams ? 'block' : 'none';
    }
}

function updateLight(json) {
    if (Object.keys(json).includes('lights_report')) {
        light = json['lights_report'];

        for (var i = 0; i < light.length; i++) {
            var name = 'img_' + light[i]['node'];
            var img = document.getElementById(name);
            if (img) {
                _printer.is_light_on = (light[i]['mode'] == 'on');
                img.setAttribute('src',  _printer.is_light_on ? 'images/ic_light_on.svg' : 'images/ic_light_off.svg');
            }

            name = 'label_' + light[i]['node'];
            var label = document.getElementById(name);
            if (label) {
                label.innerText = _printer.is_light_on ? 'on' : 'off';
            }
        }
    }
}

function updateFans(json) {
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
        for (var i = 0; i < Fans.length; i++) {
            if (Object.keys(json).includes(Fans[i])) {
                // max : 15
                speeds[i] = Math.round(Math.floor(json[Fans[i]] / 1.5) * 25.5);
            }
        }
        console.log("fan_speed : " + speeds);
    }

    // update icon and percentage only for updated ones
    for (var i = 0; i < Fans.length; i++) {
        if (speeds[i] >= 0) {
            var speed = (speeds[i] == 0) ? 0 : Math.max(10, Math.round(speeds[i] * 100 / 255.0));
            var name = 'img_' + Fans[i];

            var img = document.getElementById(name);
            if (img) {
                img.setAttribute('src', (speed > 0) ? 'images/ic_fan_on.svg' : 'images/ic_fan_off.svg');
            }

            name = 'label_' + Fans[i];
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

function updateSpeed(json) {
    if (Object.keys(json).includes('spd_lvl')) {
        console.log("spd_lvl : " + String(json['spd_lvl']));
        var combo = document.getElementById('spd_lvl');
        if (combo) {
            var idx = json['spd_lvl'] - 1;
            combo.options[idx].selected = true;
        }
    }
}

const _update_funcs = [
    updatePrintingState,
    updateTemperature,
    updateFilaments,
    updateLight,
    updateFans,
    updateSpeed
];

function onMessage(event) {
    if (event.data instanceof ArrayBuffer) {
        const bytes = new Uint8Array(event.data);
        const cmd = bytes[0];
        const payload = bytes.subarray(1, event.data.length);
        var image;

        switch (cmd) {
            case 0x01: {        // jpeg
                image = document.getElementById('camera_view');
                if (image)
                    image.src = 'data:image/jpeg;base64,' + encode(payload);
            }
            break;

            case 0x02: {        // png
                image = document.getElementById('model_view');
                if (image)
                    image.src = 'data:image/png;base64,' + encode(data);
            }
            break;
        }
    } else {
        var json = JSON.parse(event.data);
        console.log(event.data);
        if (json) {
            if (Object.keys(json).includes('printer_name')) {
                var title = document.getElementById('printer_name');
                title.innerText = json['printer_name'];
            } else {
                json = json['print'];
                if (json) {
                    for (const f of _update_funcs) {
                        f(json);
                    }
                }
            }
        }
    }
}


//-------------------------------------------------------------------------------------------------
// outgoing MQTT messages from UI events
//-------------------------------------------------------------------------------------------------
function sendGcode(code) {
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
    const str = JSON.stringify(json);
    console.log(str);
    if (_websocket.readyState == WebSocket.OPEN)
        _websocket.send(str);
}

function ctrlLED(name, mode) {
    const json = {
        command: 'pub',
        data: {
            system: {
                command: "ledctrl",
                led_node: name,
                sequence_id: 0,
                led_mode: mode,
                led_on_time: 500,
                led_off_time: 500,
                loop_times: 0,
                interval_time: 0,
            },
        }
    };
    const str = JSON.stringify(json);
    console.log(str);
    if (_websocket.readyState == WebSocket.OPEN)
        _websocket.send(str);


}

function onChangeFan(id) {
    const elt = document.getElementById(id);
    if (elt) {
        console.log("onChange : " + id + " " + elt.value);
        var idx = 1;
        for (const fan of Fans) {
            if (id.endsWith(fan)) {
                const speed = Math.round(elt.value * 2.55);
                sendGcode("M106 P" + idx + " S" + speed + " \n");
                break;
            }
            idx++;
        }
    }
}

function onClickFan(id) {
    const id_label = id.replace("btn_", "label_");
    var   elt_label = document.getElementById(id_label);

    if (elt_label && elt_label.nodeName.toLowerCase() == 'input') {
        const new_val = (elt_label.value == 0) ? 100 : 0;
        const id_img = id.replace("btn_", "img_");
        var elt_img = document.getElementById(id_img);
        if (elt_img) {
            elt_img.setAttribute("src", (new_val == 0) ? "images/ic_fan_off.svg" : "images/ic_fan_on.svg")
        }
        elt_label.value = new_val;
        console.log("onClickFan : ", new_val);
        onChangeFan(id_label);
    }
}

function onClickLight(id) {
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
        console.log("onClickLight : ", new_val);

        id_label = id.replace("btn_", "");
        ctrlLED(id_label, elt_label.innerText);
    }
}

function onChangeTemp(id) {
    const elt = document.getElementById(id);

    if (elt) {
        console.log("onChange : " + id + " " + elt.value);

        var gcode;
        if (id == "nozzle_target_temper") {
            gcode = "M104 S" + String(elt.value) + "\n";
        } else if (id == "bed_target_temper") {
            gcode = "M140 S" + String(elt.value) + "\n";
        }
        sendGcode(gcode);
    }
}

function onChangeSpeed(id) {
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
        const str = JSON.stringify(json);
        console.log(str);
        if (_websocket.readyState == WebSocket.OPEN)
            _websocket.send(str);
    }
}

//
// AXIS control
//
function onClickAxisButton(id) {
    console.log("onClickAxisButton : " + id);

    if (_printer.status == Status.RUNNING || _printer.status == Status.PAUSED)
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
            sendGcode(gcode);
            break;
        }
    }
}

function onClickExtruderButton(id) {
    console.log("onClickExtruderButton : " + id);

    if (_printer.status == Status.RUNNING || _printer.status == Status.PAUSED)
        return;

    if (id == "btn_extr_up") {
        sendGcode("M83\nG0 E-10.0 F900\n");
    } else if (id == "btn_extr_down") {
        sendGcode("M83\nG0 E10.0 F900\n");
    }
}

function onClickHomeButton(id) {
    console.log("onClickHomeButton : " + id);
    if (_printer.status == Status.RUNNING || _printer.status == Status.PAUSED)
        return;
    sendGcode("G28\n");
}


//
// Filament / AMS control
//
function onClickLoadVTButton(id) {
    console.log("onClickLoadVTButton : " + id);

    if (!_printer.is_vt_tray || _printer.status == Status.RUNNING || _printer.status == Status.PAUSED)
        return;

    const gcode = "M620 S254\nM106 S255\nM104 S250\nM17 S\nM17 X0.5 Y0.5\nG91\nG1 Y-5 F1200\nG1 Z3\nG90\nG28 X\n" +
                  "M17 R\nG1 X70 F21000\nG1 Y245\nG1 Y265 F3000\nG4\nM106 S0\nM109 S250\nG1 X90\nG1 Y255\nG1 X120\nG1 X20 Y50 F21000\nG1 Y-3\n" +
                  "T254\nG1 X54\nG1 Y265\nG92 E0\nG1 E40 F180\nG4\nM104 S0\nG1 X70 F15000\nG1 X76\nG1 X65\nG1 X76\nG1 X65\nG1 X90 F3000\nG1 Y255\n" +
                  "G1 X100\nG1 Y265\nG1 X70 F10000\nG1 X100 F5000\nG1 X70 F10000\nG1 X100 F5000\nG1 X165 F12000\nG1 Y245\nG1 X70\nG1 Y265 F3000\nG91\n" +
                  "G1 Z-3 F1200\nG90\nM621 S254\n\n";
    sendGcode(gcode);
}

function onClickUnloadVTButton(id) {
    console.log("onClickUnloadVTButton : " + id);

    if (!_printer.is_vt_tray || _printer.status == Status.RUNNING || _printer.status == Status.PAUSED)
        return;

    const gcode = "M620 S255\nM106 P1 S255\nM104 S250\nM17 S\nM17 X0.5 Y0.5\nG91\nG1 Y-5 F3000\nG1 Z3 F1200\nG90\nG28 X\n"+
                  "M17 R\nG1 X70 F21000\nG1 Y245\nG1 Y265 F3000\nG4\nM106 P1 S0\nM109 S250\nG1 X90 F3000\nG1 Y255 F4000\n" +
                  "G1 X100 F5000\nG1 X120 F21000\nG1 X20 Y50\nG1 Y-3\nT255\nG4\nM104 S0\nG1 X70 F3000\n\nG91\nG1 Z-3 F1200\nG90\nM621 S255\n\n";
    sendGcode(gcode);
}

function onClickLoadAMSButton(id) {
    if (_printer.status == Status.RUNNING || _printer.status == Status.PAUSED)
        return;
}

function onClickUnloadAMSButton(id) {
    if (_printer.status == Status.RUNNING || _printer.status == Status.PAUSED)
        return;
}

function onClickAMSTrayRefresh(evt) {
    if (_printer.status == Status.RUNNING || _printer.status == Status.PAUSED)
        return;

    const ams_ref = document.getElementsByClassName("ams-tray-ref");
    for (i = 0; i < ams_ref.length; i++) {
        if (ams_ref[i] == evt.currentTarget) {
            var idx = i;
            // refresh ams tray info for ith
            break;
        }
    }
}

function onClickAMSTray(evt) {
    if (_printer.status == Status.RUNNING || _printer.status == Status.PAUSED)
        return;

    const ams_trays = document.getElementsByClassName("ams-tray");
    for (var ams of ams_trays) {
        ams.className = ams.className.replace(" btn-selected", "");
    }

    for (i = 0; i < ams_trays.length; i++) {
        if (ams_trays[i] == evt.currentTarget) {
            _selected_tray = i;
            evt.currentTarget.className += " btn-selected";
        }
    }
}

//
// Print / Stop
//
function doPrintAction(action) {
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
    const str = JSON.stringify(json);
    console.log(str);
    if (_websocket.readyState == WebSocket.OPEN)
        _websocket.send(str);
}

function onClickPauseResume(id) {
    switch (_printer.status) {
        case Status.RUNNING:
            doPrintAction("pause");
            break;

        case Status.PAUSED:
            doPrintAction("resume");
            break;
    }
}

function onClickStop(id) {
    doPrintAction("stop");
}

//-------------------------------------------------------------------------------------------------
// SD Card Browser
//-------------------------------------------------------------------------------------------------
var _listModels = [
    "10363259711.png", "1270915251.png", "12718038631.png",
    "15137327011.png", "15642952511.png", "17540064941.png",
    "17930814601.png", "19799488481.png", "2064281.png",
    "22181592811.png", "2528318491.png", "2613376571.png",
    "26698210481.png", "29877475381.png", "3553883803.png",
    "35901261651.png", "38297837321.png", "39585176581.png",
    "42885271611.png", "4380517671.png", "4678907721.png",
    "7143709141.png", "9240571371.png"
];

function drawViews(id) {
    const elts = document.getElementsByName("full_height_view");
    for (var v of elts) {
        const rect = v.getBoundingClientRect();
        v.style.height = String(window.innerHeight - Math.round(rect.top)) + "px";
    }

    if (id == "menu_sdcard") {
        drawModels(_listModels, _listModels);
    }
}

function onClickModel(event, id) {
    console.log("onClickModel:", id, ' ', event.x, ' ', event.y);

    if (_selected_model_id) {
        var elt = document.getElementById(_selected_model_id);
        if (elt) {
            elt.className = elt.className.replace(" model-selected", "");
        }
    }

    var elt = document.getElementById(id);
    if (elt) {
        elt.className += " model-selected";
    }
    _selected_model_id = id;

    var popup = document.getElementById("popup-menu");
    if (popup) {
        popup.style.display = "block";
        const tab = document.getElementById("tab_sdcard");
        if (tab) {
            const rect = tab.getBoundingClientRect();
            popup.style.marginLeft = String(event.x - rect.left) + "px";
            popup.style.marginTop = String(event.y - rect.top) + "px";
        }
    }
}

function onClickPopupCancel() {
    var popup = document.getElementById("popup-menu");
    if (popup) {
        popup.style.display = "none";
    }

    if (_selected_model_id) {
        var elt = document.getElementById(_selected_model_id);
        if (elt) {
            elt.className = elt.className.replace(" model-selected", "");
        }
        _selected_model_id = "";
    }
}

function onClickModelPrint() {
    if (_selected_model_id) {
        var pos = _selected_model_id.replace("model_id_", "");
        var p = parseInt(pos, 10);
        console.log("onClickModelPrint:" + p);
    }
    onClickPopupCancel();
}

function onClickModelDelete() {
    if (_selected_model_id) {
        var pos = _selected_model_id.replace("model_id_", "");
        var p = parseInt(pos, 10);
        console.log("onClickModelDelete:" + p);

        _listModels.splice(p, 1);
        drawModels(_listModels, _listModels);
    }
    onClickPopupCancel();
}

function drawModels(images, names) {
    var row = '';
    var col;

    _selected_model_id = '';
    for (var i = 0; i < images.length; i++) {
        if ((i % 3) == 0) {
            if (i != 0)
                row += '</tr>\n';
            row += '<tr>\n';
        }
        col = '<td>\n' +
              '<img src="sdcard/image/' + images[i] + '"' + 'height="128px" id="model_id_' + String(i) +
              '" onClick="onClickModel(event, this.id)" class="none">\n' +
              '<label>' + names[i] + '</label>\n' +
              '</td>\n';
        row += col;
    }

    var remain = 3 - (_listModels.length % 3);
    if (remain != 3) {
        for (var i = 0; i < remain; i++) {
            row += '<td>\n</td>\n';
        }
    }
    row += '</tr>\n';

    var table = document.getElementById("model_table");
    if (table)
        table.innerHTML = row;
}

//-------------------------------------------------------------------------------------------------
// UI Controls
//-------------------------------------------------------------------------------------------------
function openTab(id) {
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
    drawViews(id);
    _selected_menu_id = id;
}

function onResize(event) {
    drawViews(_selected_menu_id);
}
