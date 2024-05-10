//
// Printing Class
//
class Printing extends Component {
    constructor() {
        super();
        this._printingFile = '';
        this._task_id = '';
        this._subtask_id = '';
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
        document.getElementById('axis_control').style.display = axis_control_disp;
        document.getElementById('printing_error').style.display = printing_error_disp;

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