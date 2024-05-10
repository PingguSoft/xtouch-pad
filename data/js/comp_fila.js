//
// Filament Class
//
class Filaments extends Component {
    constructor() {
        super();
        this._is_ams = false;
        this._is_vt_tray = false;
        this._sel_tray = -1;
    }

    updateFromJson(json) {
        if (Object.keys(json).includes('vt_tray')) {
            this._is_vt_tray = true;
            var tray = json['vt_tray'];

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
            this._is_ams = (json['ams_exist_bits'] != null);
            console.log("ams : " + this._is_ams);

            if (this._is_ams) {
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
            document.getElementById('ams_cell').style.display = this._is_ams ? '' : 'none';
        }
    }

    onClickLoadVTButton(id) {
        console.log("onClickLoadVTButton : " + id);

        if (!this._is_vt_tray || Component._status == Component.Status.RUNNING || Component._status == Component.Status.PAUSED)
            return;

        const gcode = "M620 S254\nM106 S255\nM104 S250\nM17 S\nM17 X0.5 Y0.5\nG91\nG1 Y-5 F1200\nG1 Z3\nG90\nG28 X\n" +
                      "M17 R\nG1 X70 F21000\nG1 Y245\nG1 Y265 F3000\nG4\nM106 S0\nM109 S250\nG1 X90\nG1 Y255\nG1 X120\nG1 X20 Y50 F21000\nG1 Y-3\n" +
                      "T254\nG1 X54\nG1 Y265\nG92 E0\nG1 E40 F180\nG4\nM104 S0\nG1 X70 F15000\nG1 X76\nG1 X65\nG1 X76\nG1 X65\nG1 X90 F3000\nG1 Y255\n" +
                      "G1 X100\nG1 Y265\nG1 X70 F10000\nG1 X100 F5000\nG1 X70 F10000\nG1 X100 F5000\nG1 X165 F12000\nG1 Y245\nG1 X70\nG1 Y265 F3000\nG91\n" +
                      "G1 Z-3 F1200\nG90\nM621 S254\n\n";
        Component._ws.sendGcode(gcode);
    }

    onClickUnloadVTButton(id) {
        console.log("onClickUnloadVTButton : " + id);

        if (!this._is_vt_tray || Component._status == Component.Status.RUNNING || Component._status == Component.Status.PAUSED)
            return;

        const gcode = "M620 S255\nM106 P1 S255\nM104 S250\nM17 S\nM17 X0.5 Y0.5\nG91\nG1 Y-5 F3000\nG1 Z3 F1200\nG90\nG28 X\n"+
                      "M17 R\nG1 X70 F21000\nG1 Y245\nG1 Y265 F3000\nG4\nM106 P1 S0\nM109 S250\nG1 X90 F3000\nG1 Y255 F4000\n" +
                      "G1 X100 F5000\nG1 X120 F21000\nG1 X20 Y50\nG1 Y-3\nT255\nG4\nM104 S0\nG1 X70 F3000\n\nG91\nG1 Z-3 F1200\nG90\nM621 S255\n\n";
        Component._ws.sendGcode(gcode);
    }

    onClickLoadAMSButton(id) {
        if (Component._status == Component.Status.RUNNING || Component._status == Component.Status.PAUSED)
            return;
    }

    onClickUnloadAMSButton(id) {
        if (Component._status == Component.Status.RUNNING || Component._status == Component.Status.PAUSED)
            return;
    }

    onClickAMSTrayRefresh(evt) {
        if (Component._status == Component.Status.RUNNING || Component._status == Component.Status.PAUSED)
            return;

        const ams_ref = document.getElementsByClassName("ams-tray-ref");
        for (var i = 0; i < ams_ref.length; i++) {
            if (ams_ref[i] == evt.currentTarget) {
                var idx = i;
                // refresh ams tray info for ith
                break;
            }
        }
    }

    onClickAMSTray(evt) {
        if (Component._status == Component.Status.RUNNING || Component._status == Component.Status.PAUSED)
            return;

        const ams_trays = document.getElementsByClassName("ams-tray");
        for (var ams of ams_trays) {
            ams.className = ams.className.replace(" btn-selected", "");
        }

        for (var i = 0; i < ams_trays.length; i++) {
            if (ams_trays[i] == evt.currentTarget) {
                this._sel_tray = i;
                evt.currentTarget.className += " btn-selected";
            }
        }
    }
}