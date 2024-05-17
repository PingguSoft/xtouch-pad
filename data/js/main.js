//
// Controller Class
//
class Controller extends Component {
    constructor() {
        super();
        this._sel_menu_id = '';
        this._is_err_process = false;
        this._file = '';
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

    onStatusChanged(status) {
        var printing_idle_disp;
        var printing_info_disp;
        var axis_control_disp;
        var printing_error_disp;

        switch (status) {
            case Component.Status.IDLE:
                printing_idle_disp = 'block';
                printing_info_disp = 'none';
                axis_control_disp = 'block';
                break;

            case Component.Status.FAILED:
            case Component.Status.FINISHED:
                printing_idle_disp = 'none';
                printing_info_disp = 'block';
                axis_control_disp = 'block';
                break;

            // case Component.Status.FAILED:
            //     printing_idle_disp = 'none';
            //     printing_info_disp = 'none';
            //     printing_error_disp = 'block';
            //     axis_control_disp = 'none';
            //     // const err_json = {
            //     //     print_error: 83902522,
            //     // };
            //     // updateError(err_json);
            //     break;

            default:
                printing_idle_disp = 'none';
                printing_info_disp = 'block';
                axis_control_disp = 'none';
                break;
        }
        document.getElementById('printing_idle').style.display = printing_idle_disp;
        document.getElementById('printing_info').style.display = printing_info_disp;
        document.getElementById('axis_control').style.display = axis_control_disp;
        // document.getElementById('printing_error').style.display = printing_error_disp;

        switch (status) {
            case Component.Status.RUNNING: {
                document.getElementById('printing_model_png').setAttribute('src', 'images/ic_comment_model.svg');
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

    processError() {
        if (_errors.isError()) {
            if (!this._is_err_process) {
                document.getElementById('printing_error').style.display = 'block';
                document.getElementById('printing_idle').style.display = 'none';
                document.getElementById('printing_info').style.display = 'none';
                _errors.showError();
                this._is_err_process = true;
            }
        } else {
            if (this._is_err_process) {
                document.getElementById('printing_error').style.display = 'none';
                this.onStatusChanged(Component._status);
                this._is_err_process = false;
            }
        }
    }

    updateFromJson(json) {
        if (!_sdbrowser.isReloading() && !_camera.isEnabled())
            _camera.enable(true);

        // update printing model PNG
        const curFile = _printing.getPrintingFile();
        if (curFile.length > 0 && curFile != this._file) {
            const name = _sdbrowser.getModelPNG(curFile);
            if (name != null) {
                document.getElementById('printing_model_png').setAttribute('src', name);
                this._file = curFile;
            }
        }
        this.processError();
    }

    onClickDone() {
        _errors.onClickDone();
        this.processError();
    }

    onClickRetry() {
        _errors.onClickRetry();
        this.processError();
    }

    onClickOkay() {
        _errors.onClickOkay();
        this.processError();
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
var _etcs = new Etcs();
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
    const websocket = new WS(`ws://${window.location.hostname}:${window.location.port}/ws`);
    const ui_comp = [_printerinfo, _camera, _sdbrowser];
    const printer_comp = [_printing, _temperature, _filaments, _lights, _fans, _speed, _errors, _hms, _camera, _etcs, _controller ];

    _printing.setStatusCallback(_controller.onStatusChanged);
    websocket.setComponents(printer_comp, ui_comp);
    Component.setWebSocket(websocket);
}

function openTab(id) {
    _controller.openTab(id);
}

function onResize(event) {
    _controller.onResize(event);
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

//
// Error
//
function onClickDone(id) {
    _controller.onClickDone();
}

function onClickRetry(id) {
    _controller.onClickRetry();
}

function onClickOkay(id) {
    console.log("okay !!!");
    _controller.onClickOkay();
}
