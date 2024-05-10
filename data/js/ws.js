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