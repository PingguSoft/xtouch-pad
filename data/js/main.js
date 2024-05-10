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
    const printer_comp = [_printing, _temperature, _filaments, _lights, _fans, _speed, _errors, _hms, _camera, _others ];
    const ui_comp = [_printerinfo, _camera, _sdbrowser, _controller];

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
}

function onClickRetry(id) {
}
