//
// SDBrowser Class
//
class SDBrowser extends Component {
    constructor() {
        super();
        this._sel_model_id = '';
        this._is_sd_reloading = false;
        this._sdcard_model_list = [
            // {
            //     "ts": 1955357392,
            //     "size": 766263,
            //     "3mf": "Cute Schnauzer keychain.gcode.3mf",
            //     "png": "17540064941.png"
            // },
            // {
            //     "ts": 1955353346,
            //     "size": 1196949,
            //     "3mf": "hello-kitty-6-Dom.gcode.3mf",
            //     "png": "29877475381.png"
            // },
            // {
            //     "ts": 1955353337,
            //     "size": 1573271,
            //     "3mf": "Pikachu.gcode.3mf",
            //     "png": "1270915251.png"
            // },
        ];
        this._reload =
        {
            "ts": 0,
            "size": 0,
            "3mf": "Reload",
            "png": "reload.svg"
        };

        // add reload in the sdcard_model_list
        if (this._sdcard_model_list.length == 0 ||
            (this._sdcard_model_list.length != 0 && this._sdcard_model_list.at(this._sdcard_model_list.length - 1) != this._reload)) {
                this._sdcard_model_list.push(this._reload);
        }
    }

    updateFromJson(json) {
        if (Object.keys(json).includes('sdcard_list')) {
            console.log(json);
            json = json['sdcard_list'];
            this._sdcard_model_list = json;
            this._sdcard_model_list.push(this._reload);
            this._is_sd_reloading = false;
            this._drawModels(this._sdcard_model_list);
        }
    }

    updateTabView() {
        this._drawModels(this._sdcard_model_list);
    }

    reloadSD() {
        const json = {
            command: 'sdcard_list',
        };
        Component._ws.sendJson(json);
        this._is_sd_reloading = true;
        this.onClickPopupCancel();
        this._drawModels(this._sdcard_model_list);
    }

    cancelReload() {
        const json = {
            command: 'sdcard_list_cancel',
        };
        Component._ws.sendJson(json);
        this._is_sd_reloading = false;
        this.updateTabView();
    }

    onClickModel(event, id) {
        console.log("onClickModel:", id, ' ', event.x, ' ', event.y);

        if (this._sel_model_id) {
            var elt = document.getElementById(this._sel_model_id);
            if (elt) {
                elt.className = elt.className.replace(" model-selected", "");
            }
        }

        var elt = document.getElementById(id);
        if (elt) {
            elt.className += " model-selected";
        }
        this._sel_model_id = id;

        var popup = document.getElementById("popup-menu");
        if (popup) {
            popup.style.display = "block";
            const tab = document.getElementById("tab_sdcard");
            if (tab) {
                const rect = tab.getBoundingClientRect();
                var x = Math.min(event.x - rect.left, rect.right - 150);
                var y = Math.min(event.y - rect.top, rect.bottom - 150);
                popup.style.marginLeft = String(x) + "px";
                popup.style.marginTop = String(y) + "px";
            }
        }
    }

    onClickPopupCancel() {
        var popup = document.getElementById("popup-menu");
        if (popup) {
            popup.style.display = "none";
        }

        if (this._sel_model_id) {
            var elt = document.getElementById(this._sel_model_id);
            if (elt) {
                elt.className = elt.className.replace(" model-selected", "");
            }
            this._sel_model_id = "";
        }
    }

    onClickModelPrint() {
        if (this._sel_model_id) {
            var pos = this._sel_model_id.replace("model_id_", "");
            var p = parseInt(pos, 10);
            console.log("onClickModelPrint:" + this._sdcard_model_list[p]["3mf"]);

            // const json = {
            //     command: 'print',
            //     data: {
            //         "3mf" : this._sdcard_model_list[p]["3mf"],
            //         "png" : this._sdcard_model_list[p]["png"]
            //     }
            // };
            // Component._ws.sendJson(json);

            const json = {
                command: 'pub',
                data: {
                    print: {
                        command: "project_file",
                        sequence_id: 0,
                        param: "Metadata/plate_1.gcode",
                        project_id: "0",
                        profile_id: "0",
                        subtask_id: "0",
                        task_id: "0",
                        subtask_name: this._sdcard_model_list[p]["3mf"].replace(".gcode.3mf", ""),
                        url: "ftp://" + this._sdcard_model_list[p]["3mf"],
                        bed_type: "auto",
                        timelapse: true, //self._settings.get_boolean(["timelapse"]),
                        bed_leveling: true, //self._settings.get_boolean(["bed_leveling"]),
                        flow_cali: true, //self._settings.get_boolean(["flow_cali"]),
                        vibration_cali: true, //self._settings.get_boolean(["vibration_cali"]),
                        layer_inspect: true, //self._settings.get_boolean(["layer_inspect"]),
                        use_ams: true, //self._settings.get_boolean(["use_ams"])
                    },
                }
            };
            Component._ws.sendJson(json);
        }
        this.onClickPopupCancel();
    }

    onClickModelDelete() {
        if (this._sel_model_id) {
            var pos = this._sel_model_id.replace("model_id_", "");
            var p = parseInt(pos, 10);
            console.log("onClickModelDelete:" + p);

            this._sdcard_model_list.splice(p, 1);
            this._drawModels(this._sdcard_model_list);

            const json = {
                command: 'delete',
                data: {
                    "3mf" : this._sdcard_model_list[p]["3mf"],
                    "png" : this._sdcard_model_list[p]["png"]
                }
            };
            Component._ws.sendJson(json);
        }
        this.onClickPopupCancel();
    }

    isReloading() {
        return this._is_sd_reloading;
    }

    getModelPNG(name) {
        var png = null; // = "images/ic_comment_model.svg";

        for (const item of this._sdcard_model_list) {
            if (item['3mf'] == name) {
                png = 'sd/image/' + item['png'];
                break;
            }
        }
        return png;
    }

    _drawModels(models) {
        var row = '';
        var col;

        if (this._is_sd_reloading) {
            row = '<img src="images/reload.gif" style="vertical-align: center;" onClick="onClickReloadIcon()">';
        } else {
            this._sel_model_id = '';
            for (var i = 0; i < models.length; i++) {
                if ((i % 3) == 0) {
                    if (i != 0)
                        row += '</tr>\n';
                    row += '<tr>\n';
                }
                const name = models[i]["3mf"].replace(".gcode.3mf", "");
                if (models[i]["ts"] == 0 && models[i]["size"] == 0 && models[i]["3mf"] == "Reload") {
                    col = '<td>\n' +
                        '<img src="images/' + models[i]["png"] + '"' + 'height="128px" width="128px" id="model_id_' + String(i) +
                        '" onClick="onClickReloadSD()" class="none">\n' +
                        '<label>' + name + '</label>\n' +
                        '</td>\n';
                } else {
                    col = '<td>\n' +
                        '<img src="sd/image/' + models[i]["png"] + '"' + 'height="128px" width="128px" id="model_id_' + String(i) +
                        '" onClick="onClickModel(event, this.id)" class="none">\n' +
                        '<label>' + name + '</label>\n' +
                        '</td>\n';
                }
                row += col;
            }

            var remain = 3 - (models.length % 3);
            if (remain != 3) {
                for (var i = 0; i < remain; i++) {
                    row += '<td>\n</td>\n';
                }
            }
            row += '</tr>\n';
        }

        var table = document.getElementById("model_table");
        if (table)
            table.innerHTML = row;
    }
}