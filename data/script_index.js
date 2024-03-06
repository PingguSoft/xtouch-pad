
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

// When websocket is established, call the getReadings() function
function onOpen(event) {
    console.log('Connection opened');
    websocket.send("open");
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function encode (input) {
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

// Function that receives the message from the ESP32 with the readings
function onMessage(event) {
    console.log(event.data);

    if(event.data instanceof ArrayBuffer) {
        var bytes = new Uint8Array(event.data);
        var id = new Uint32Array(bytes.buffer.slice(0, 4))[0];

        console.log(id);
        if (id == 0xe0ffd8ff) {         // jpeg
            var image = document.getElementById('camera_view');
            if (image)
                image.src = 'data:image/jpeg;base64,' + encode(bytes);
        } else if (id == 0x474e5089) {  // png
            var image = document.getElementById('model_view');
            if (image)
                image.src = 'data:image/png;base64,' + encode(bytes);
        }
    } else {
        var json = JSON.parse(event.data);
        if (json) {
            json = json['print'];
            if (json) {
                var nodes = ['nozzle_temper', 'nozzle_target_temper', 'bed_temper', 'bed_target_temper'];

                for (var i = 0; i < nodes.length; i++) {
                    if (Object.keys(json).includes(nodes[i])) {
                        var label = document.getElementById(nodes[i]);
                        if (label) {
                            var val = parseInt(json[nodes[i]], 10);
                            label.innerText = val;
                            console.log(nodes[i], val);
                        }
                    }
                }

                if (Object.keys(json).includes('gcode_state')) {
                    if (json['gcode_state'] == 'IDLE') {
                        document.getElementById('printing_idle').style.display = '';
                        document.getElementById('printing_info').style.display = 'none';
                    } else {
                        document.getElementById('printing_idle').style.display = 'none';
                        document.getElementById('printing_info').style.display = '';
                    }
                }

                if (Object.keys(json).includes('ams')) {
                    json = json['ams']['ams']['0']['tray'];
                    if (json) {
                        var keys = Object.keys(json);

                        for (var i = 0; i < keys.length; i++) {
                            var idx  = String(i);
                            var btn  = 'btn_tray' + idx;
                            var tt   = 'tray' + idx + '_type';
                            var tray = json[idx];

                            console.log(tray);
                            document.getElementById(btn).style.background = '#' + tray['tray_color'];
                            document.getElementById(tt).innerText = tray['tray_type'];
                        }
                    }
                }
            }
        }
    }
}

function onClickButton(id) {
    var buttons = ['btn_nozzle_temp', 'btn_bed_temp'];
    var targets = ['nozzle_target_temper', 'bed_target_temper']


    console.log(id);
    if (id == "btn_printer") {
        var e = document.getElementById('printing_idle');
        e.style.display = (e.style.display == 'none') ? '' : 'none';
    }
    else if (id == "btn_ams") {
        var e = document.getElementById('printing_info');
        e.style.display = (e.style.display == 'none') ? '' : 'none';
    }
}

//
// 2
//