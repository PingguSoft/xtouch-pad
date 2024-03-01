
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
// Init web socket when the page loads
window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
}

function getReadings(){
    websocket.send("getReadings");
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
    websocket.binaryType = "arraybuffer";
}

// When websocket is established, call the getReadings() function
function onOpen(event) {
    console.log('Connection opened');
    getReadings();
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
            var image = document.getElementById('jpeg_image');
            image.src = 'data:image/jpeg;base64,' + encode(bytes);
        } else if (id == 0x474e5089) {  // png
            var image = document.getElementById('png_image');
            image.src = 'data:image/png;base64,' + encode(bytes);
        }
        // for (var i = 0; i < bytes.length; i++) {
        //   msg += String.fromCharCode(bytes[i]);
        // }
    } else {
        var myObj = JSON.parse(event.data);
        var keys = Object.keys(myObj);

        for (var i = 0; i < keys.length; i++) {
            var key = keys[i];
            document.getElementById(key).innerHTML = myObj[key];
        }
    }
}

function drawImage(data) {
    var imageWidth = 128, imageHeight = 128; // hardcoded width & height.
    var byteArray = new Uint8Array(data);

    var canvas = document.createElement('canvas');
    canvas.width = imageWidth;
    canvas.height = imageHeight;
    var ctx = canvas.getContext('2d');

    var imageData = ctx.getImageData(0, 0, imageWidth, imageHeight); // total size: imageWidth * imageHeight * 4; color format BGRA
    var dataLen = imageData.data.length;
    for (var i = 0; i < dataLen; i++) {
        imageData.data[i] = byteArray[i];
    }
    ctx.putImageData(imageData, 0, 0);

    // create a new element and add it to div
    var image = document.createElement('img');
    image.width = imageWidth;
    image.height = imageHeight;
    image.src = canvas.toDataURL();

    var div = document.getElementById('img');
    div.appendChild(image);
}
