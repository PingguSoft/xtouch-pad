//
// Camera Class
//
class Camera extends Component {
    constructor() {
        super();
        this._is_ipcam = false;
        this._is_ipcam_record = false;
        this._is_ipcam_timelapse = false;
        this._is_ipcam_show = false;
        this._is_enabled = true;
    }

    updateFromJson(json) {
        if (Object.keys(json).includes('ipcam')) {
            const ipcam = json['ipcam'];

            if (Object.keys(ipcam).includes('ipcam_dev')) {
                this._is_ipcam = (ipcam['ipcam_dev'] == '1');
            }
            if (Object.keys(ipcam).includes('ipcam_record')) {
                this._is_ipcam_record = (ipcam['ipcam_record'] == 'enable');
            }
            if (Object.keys(ipcam).includes('timelapse')) {
                this._is_ipcam_timelapse = (ipcam['timelapse'] == 'enable');
            }
            console.log("ipcam : " + String(ipcam));
            document.getElementById('cam_monitoring').style.display = this._is_ipcam ? 'block' : 'none';
        }
    }

    updateFromArrayBuffer(cmd, payload) {
        var image;
        switch (cmd) {
            case 0x01: {        // jpeg
                image = document.getElementById('camera_view');
                if (image && this._is_ipcam_show) {
                    image.src = 'data:image/jpeg;base64,' + Util.encode(payload);
                }
            }
            break;

            case 0x02: {        // png
                image = document.getElementById('model_view');
                if (image)
                    image.src = 'data:image/png;base64,' + Util.encode(data);
            }
            break;
        }
    }

    turnOn(on) {
        if (!this._is_ipcam)
            return;

        this._is_ipcam_show = on;
        var image = document.getElementById('camera_view');
        if (image) {
            if (!this._is_ipcam_show)
                image.src = 'images/m_camera.svg';
        }
        const json = {
            command: 'camera_view',
            data: this._is_ipcam_show
        };
        Component._ws.sendJson(json);
    }

    enable(en) {
        this._is_enabled = en;
        var elt = document.getElementById('cam_monitoring');
        elt.className = (en) ? elt.className.replace("cam-view-disabled", "cam-view") :
                            elt.className.replace("cam-view", "cam-view-disabled");
    }

    isEnabled() {
        return this._is_enabled;
    }

    isOn() {
        return this._is_ipcam && this._is_ipcam_show;
    }

    turnOnOff() {
        this.turnOn(!this._is_ipcam_show)
    }
}