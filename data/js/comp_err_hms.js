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
        this._q_err = [];

        var json = JSON.parse(JSON.stringify(hms_err_tbl));
        var err = json['data']['device_error']['en'];

        for (const x of err) {
            this._err_tbl.codes.push(x.ecode);
            this._err_tbl.msgs.push(x.intro);
        }
    }

    showError() {
        if (this._q_err.length == 0)
            return;

        const code = this._q_err[0];
        const idx = this._err_tbl.codes.indexOf(code);

        console.log(this._err_tbl);
        console.log("print error : " + code + ", len:" + code.length);
        console.log("print error : " + this._err_tbl.codes[0] + ", len:" + this._err_tbl.codes[0]);
        console.log("idx:" + idx);

        if (idx >= 0) {
            var label = document.getElementById('label_error');
            label.innerText = "Device Error !";

            label = document.getElementById('label_error_desc');
            label.innerText = this._err_tbl.msgs[idx];

            var btn_retry = document.getElementById('btn_retry');
            btn_retry.style.display = String(this._err_tbl.msgs[idx]).includes('"Retry"') ? block : none;
            var btn_done = document.getElementById('btn_done');
            btn_done.style.display = String(this._err_tbl.msgs[idx]).includes('"Done"') ? block : none;
        }
    }

    send(response) {
        const json = {
            command: 'pub',
            data: {
                print: {
                    command: "ams_control",
                    sequence_id: 0,
                    param: response,
                },
            }
        };
        Component._ws.sendJson(json);
    }

    clear(code) {
        const json = {
            command: 'pub',
            data: {
                print: {
                    command: "clean_print_error",
                    sequence_id: 0,
                    subtask_id: "",
                    print_error: code,
                },
            }
        };
        Component._ws.sendJson(json);
    }

    onClickRetry() {
        this.send("resume");
        this._q_err.shift();
        this.showError();
    }

    onClickDone() {
        this.send("done");
        const code = parseInt(this._q_err.shift(), 16);
        this.clear(code);
        this.showError();
    }

    updateFromJson(json) {
        if (Object.keys(json).includes('print_error')) {
            const error = json['print_error'];
            const code = Util.pad(error.toString(16), 8).toUpperCase();
            this._q_err.push(code);
            this.showError();
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
        this._q_hms = [];

        var json = JSON.parse(JSON.stringify(hms_err_tbl));
        var hms = json['data']['device_hms']['en'];
        for (const x of hms) {
            this._hms_tbl.codes.push(x.ecode);
            this._hms_tbl.msgs.push(x.intro);
        }
    }

    showHMS() {
        if (this._q_hms.length == 0)
            return;

        const hms_code = this._q_hms[0];
        const idx = _hms_tbl.codes.indexOf(hms_code);
        if (idx >= 0) {
            var label = document.getElementById('label_error');
            label.innerText = "HMS !";

            label = document.getElementById('label_error_desc');
            label.innerText = _hms_tbl.msgs[idx];

            var btn_retry = document.getElementById('btn_retry');
            btn_retry.style.display = String(this._hms_tbl.msgs[idx]).includes('"Retry"') ? block : none;
            var btn_done = document.getElementById('btn_done');
            btn_done.style.display = String(this._hms_tbl.msgs[idx]).includes('"Done"') ? block : none;
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
                const part_id = (attr >> 8) & 0xff;
                const part_no = (attr >> 0) & 0xff;

                const alert_level = (code >> 16) & 0xffff;
                const error_code = (code >> 0) & 0xffff;

                const hms_code = Util.pad(module_id.toString(16), 2) + Util.pad(module_no.toString(16), 2) + Util.pad(part_id.toString(16), 2) +
                    Util.pad(alert_level.toString(16), 6) + Util.pad(error_code.toString(16), 4);
                console.log("print HMS : " + hms_code);
                this._q_hms.push(hms_code);
                this.showHMS();
            }
        }
    }
}