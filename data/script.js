
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
// Init web socket when the page loads
window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
    websocket.binaryType = "arraybuffer";
}

function onOpen(event) {
    console.log('Connection opened');

    var json = {
        command: 'open'
    };
    var str = JSON.stringify(json);
    console.log(str);
    websocket.send(str);
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function encode(input) {
    var keyStr = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
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


//
// update UI with MQTT
//
function pad(n, width) {
    n = n + '';
    return n.length >= width ? n : new Array(width - n.length + 1).join('0') + n;
}

function updatePrintingState(json) {
    if (Object.keys(json).includes('gcode_state')) {
        if (json['gcode_state'] == 'IDLE') {
            document.getElementById('printing_idle').style.display = '';
            document.getElementById('printing_info').style.display = 'none';
        } else {
            document.getElementById('printing_idle').style.display = 'none';
            document.getElementById('printing_info').style.display = '';

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
                    var elt = document.getElementById(x[1]);
                    if (elt) {
                        if (x[2] == 1) {
                            elt.innerText = json[x[0]];
                        } else if (x[2] == 2) {
                            elt.value = json[x[0]];
                        } else if (x[2] == 3) {
                            var min = json[x[0]];
                            var days = min / (60 * 24);
                            min %= (60 * 24);
                            var hours = min / 60;
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
    var nodes = ['nozzle_temper', 'nozzle_target_temper', 'bed_temper', 'bed_target_temper', 'chamber_temper'];
    for (var i = 0; i < nodes.length; i++) {
        if (Object.keys(json).includes(nodes[i])) {
            var labels = document.getElementsByName(nodes[i]);
            if (labels) {
                var val = Math.round(json[nodes[i]]);
                for (var j = 0; j < labels.length; j++) {
                    if (labels[j].nodeName.toLowerCase() == 'input')
                        labels[j].value = val;
                    else
                        labels[j].innerText = val;
                }
                console.log(nodes[i], val);
            }
        }
    }
}

function updateFilaments(json) {
    var is_ams = false;

    if (Object.keys(json).includes('vt_tray')) {
        tray = json['vt_tray'];

        var btns = document.getElementsByName('btn_vt_tray');
        if (btns) {
            for (var j = 0; j < btns.length; j++) {
                btns[j].style.background = '#' + tray['tray_color'];
            }
        }

        var labels = document.getElementsByName('vt_tray_type');
        if (labels) {
            for (var j = 0; j < labels.length; j++) {
                labels[j].innerText = tray['tray_type'];
            }
        }
    }

    if (Object.keys(json).includes('ams')) {
        is_ams = json['ams']['ams_exist_bits'];
        console.log("ams : " + is_ams);
        if (is_ams) {
            json = json['ams']['ams']['0']['tray'];
            if (json) {
                var keys = Object.keys(json);

                for (var i = 0; i < keys.length; i++) {
                    var idx = String(i);
                    var btn = 'btn_tray' + idx;
                    var tt = 'tray' + idx + '_type';
                    var tray = json[idx];

                    console.log(tray);
                    var btns = document.getElementsByName(btn);
                    if (btns) {
                        for (var j = 0; j < btns.length; j++) {
                            btns[j].style.background = '#' + tray['tray_color'];
                        }
                    }

                    var labels = document.getElementsByName(tt);
                    if (labels) {
                        for (var j = 0; j < labels.length; j++) {
                            labels[j].innerText = tray['tray_type'];
                        }
                    }
                }
            }
        }
        document.getElementById('ams_info').style.display = is_ams ? 'block' : 'none';
        document.getElementById('spool_info').style.display = is_ams ? 'none' : 'block';
    }
}

function updateLight(json) {
    if (Object.keys(json).includes('lights_report')) {
        light = json['lights_report'];

        for (var i = 0; i < light.length; i++) {
            var name = 'img_' + light[i]['node'];
            var img = document.getElementById(name);
            if (img) {
                if (light[i]['mode'] == 'on')
                    img.src = 'images/ic_light_on.svg'
                else
                    img.src = 'images/ic_light_off.svg'
            }

            name = 'label_' + light[i]['node'];
            var label = document.getElementById(name);
            if (label)
                label.innerText = light[i]['mode'];
                // label.checked = (light[i]['mode'] == 'on');
        }
    }
}

function updateFans(json) {
    const nodes = ['cooling_fan_speed', 'big_fan1_speed', 'big_fan2_speed', 'heatbreak_fan_speed'];
    var speeds = [-1, -1, -1, -1];

    if (Object.keys(json).includes('fan_gear')) {
        // max : 255
        var gear = json['fan_gear'];
        console.log("fan_gear : " + gear.toString(16));
        speeds[0] = (gear >> 0) & 0xFF;
        speeds[1] = (gear >> 8) & 0xFF;
        speeds[2] = (gear >> 16) & 0xFF;
        console.log("1 : " + speeds);
    } else {
        for (var i = 0; i < nodes.length; i++) {
            if (Object.keys(json).includes(nodes[i])) {
                // max : 15
                speeds[i] = Math.round(Math.floor(json[nodes[i]] / 1.5) * 25.5);
            }
        }
        console.log("2 : " + speeds);
    }

    // update icon and percentage only for updated ones
    for (var i = 0; i < nodes.length; i++) {
        if (speeds[i] >= 0) {
            var speed = (speeds[i] == 0) ? 0 : Math.max(10, Math.round(speeds[i] * 100 / 255.0));
            var name = 'img_' + nodes[i];
            var img = document.getElementById(name);
            if (img) {
                img.src = (speed > 0) ? 'images/ic_fan_on.svg' : 'images/ic_fan_off.svg';
            }

            name = 'label_' + nodes[i];
            var label = document.getElementById(name);
            if (label.nodeName.toLowerCase() == 'input')
                label.value = speed;
            else
                label.innerText = (speed > 0) ? String(speed) + '%' : 'off';
        }
    }
}

function updateSpeed(json) {
    if (Object.keys(json).includes('spd_lvl')) {
        console.log("spd_lvl : " + String(json['spd_lvl']));
        var combos = document.getElementsByName('spd_lvl');
        if (combos) {
            var idx = json['spd_lvl'] - 1;
            for (var i = 0; i < combos.length; i++) {
                combos[i].options[idx].selected = true;
            }
        }
    }
}

const func_updates = [
    updatePrintingState,
    updateTemperature,
    updateFilaments,
    updateLight,
    updateFans,
    updateSpeed
];

function onMessage(event) {
    if (event.data instanceof ArrayBuffer) {
        var bytes = new Uint8Array(event.data);
        var cmd = bytes[0];
        var payload = bytes.subarray(1, event.data.length);

        if (cmd == 0x01) {          // jpeg
            var image = document.getElementById('camera_view');
            if (image)
                image.src = 'data:image/jpeg;base64,' + encode(payload);
        } else if (cmd == 0x02) {   // png
            var image = document.getElementById('model_view');
            if (image)
                image.src = 'data:image/png;base64,' + encode(data);
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
                    for (const f of func_updates) {
                        f(json);
                    }
                }
            }
        }
    }
}


//
// send Event with MQTT
//
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
    var str = JSON.stringify(json);
    console.log(str);
    websocket.send(str);
}

function sendGcode(code) {
    var json = {
        command: 'pub',
        data: {
            print: {
                command: "gcode_line",
                sequence_id: 0,
                param: code,
            },
        }
    };
    var str = JSON.stringify(json);
    console.log(str);
    websocket.send(str);
}

function onChangeSpeed(id, name) {
    var elt = document.getElementById(id);
    if (elt) {
        console.log("onChange : " + id + " " + elt.value + " " + name);
        var labels = document.getElementsByName(name);
        if (labels) {
            for (var j = 0; j < labels.length; j++) {
                labels[j].value = elt.value;
            }
        }
    }
}

function onChangeTemp(id, name) {
    var elt = document.getElementById(id);
    if (elt) {
        console.log("onChange : " + id + " " + elt.value + " " + name);
        var labels = document.getElementsByName(name);
        if (labels) {
            for (var j = 0; j < labels.length; j++) {
                labels[j].value = elt.value;
            }
        }
    }
}

function onChangeFan(id, toggle) {
    const fan_maps = [
        ['label_cooling_fan_speed'],
        ['label_big_fan1_speed'],
        ['label_big_fan2_speed']
    ];

    var elt = document.getElementById(id);
    if (elt) {
        console.log("onChange : " + id + " " + elt.value + " button:" + toggle);
        var idx = 1;
        for (const fan of fan_maps) {
            if (id == fan[0]) {
                var speed;

                if (toggle) {
                    speed = (elt.value == 0) ? 255 : 0;
                } else {
                    speed = elt.value * 2.55;
                }
                sendGcode("M106 P" + idx + " S" + speed + " \n");
                break;
            }
            idx++;
        }
    }
}

function onClickButton(id, type) {
    var elt = document.getElementById(id);
    if (elt) {
        var label_id = id.replace("btn_", "label_");

        const elt_label = document.getElementById(label_id);
        if (elt_label) {
            var val = 0;
            if (elt_label.nodeName.toLowerCase() == 'input')
                val = elt_label.value;
            else
                val = (elt_label.innerText == "off") ? 0 : 100;

            var new_val = (val == 0) ? 100 : 0;
            console.log("onClickButton : " + id + " " + val + " => " + new_val);

            if (type == 1) {
                label_id = id.replace("btn_", "");
                ctrlLED(label_id, (new_val == 0) ? "off" : "on");
            } else if (type == 2) {
                onChangeFan(label_id, 1);
            }
        }
    }
}

function openTab(evt, tab) {
    var i, tabcontent, tablinks;

    // Get all elements with class="tabcontent" and hide them
    tabcontent = document.getElementsByClassName("tabcontent");
    for (i = 0; i < tabcontent.length; i++) {
        tabcontent[i].style.display = "none";
    }

    // Get all elements with class="tablinks" and remove the class "active"
    // tablinks = document.getElementsByClassName("tablinks");
    // for (i = 0; i < tablinks.length; i++) {
    //     tablinks[i].className = tablinks[i].className.replace(" active", "");
    // }
    // evt.currentTarget.className += " active";

    // Show the current tab, and add an "active" class to the button that opened the tab
    document.getElementById(tab).style.display = 'block';
}