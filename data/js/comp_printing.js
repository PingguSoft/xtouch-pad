//
// Printing Class
//
class Printing extends Component {
    constructor() {
        super();
        this._printingFile = '';
        this._task_id = '';
        this._subtask_id = '';
        this._func_status_cb = null;
    }

    getPrintingFile() {
        return this._printingFile;
    }

    setPrintingFile(file) {
        this._printingFile = file;
        return file;
    }

    setStatusCallback(func) {
        this._func_status_cb = func;
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
                if (this._func_status_cb != null) {
                    this._func_status_cb(status);
                }
                Component._status = status;
            }
        }

        if (Component._status != Component.Status.IDLE) {
            const printing_maps = [
                { tag: 'subtask_name', id: 'printing_model_name', func: null },
                { tag: 'gcode_file', id: 'printing_model_aux', func: (x) => { this._printingFile = x; return x; } },
                { tag: 'total_layer_num', id: 'printing_max_layer', func: null },
                { tag: 'layer_num', id: 'printing_cur_layer', func: null },
                { tag: 'mc_percent', id: 'printing_cur_prog_percentage', func: null },
                { tag: 'mc_percent', id: 'printing_cur_prog_bar', func: null },
                { tag: 'mc_remaining_time', id: 'printing_remaining_time', func: Util.formatTime },
                { tag: 'task_id', id: null, func: (x) => { this._task_id = x; } },
                { tag: 'subtask_id', id: null, func: (x) => { this._subtask_id = x; } }
            ];

            for (const x of printing_maps) {
                if (Object.keys(json).includes(x.tag)) {
                    console.log(x.tag + ", " + x.id + ", : " + json[x.tag]);

                    if (x.id != null) {
                        const elt = document.getElementById(x.id);
                        if (elt) {
                            if (elt.nodeName.toLowerCase() == 'label') {
                                elt.innerText = (x.func != null) ? x.func(json[x.tag]) : json[x.tag];
                            } else if (elt.nodeName.toLowerCase() == 'img') {
                                var name = (x.func != null) ? x.func(json[x.tag]) : json[x.tag];
                                elt.setAttribute('src', name);
                            } else {
                                elt.value = (x.func != null) ? x.func(json[x.tag]) : json[x.tag];
                            }
                        } else {
                            console.log("no : " + x.id);
                        }
                    } else {
                        if (x.func != null)
                            x.func(json[x.tag]);
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