//
// Light Class
//
class Lights extends Component {
    constructor() {
        super();
        this._is_on = false;
    }

    updateFromJson(json) {
        if (Object.keys(json).includes('lights_report')) {
            var light = json['lights_report'];

            for (var i = 0; i < light.length; i++) {
                var name = 'img_' + light[i]['node'];
                var img = document.getElementById(name);
                if (img) {
                    this._is_on = (light[i]['mode'] == 'on');
                    img.setAttribute('src',  this._is_on ? 'images/ic_light_on.svg' : 'images/ic_light_off.svg');
                }

                name = 'label_' + light[i]['node'];
                var label = document.getElementById(name);
                if (label) {
                    label.innerText = this._is_on ? 'on' : 'off';
                }
            }
        }
    }

    turnOnOff(id) {
        var id_label = id.replace("btn_", "label_");
        var elt_label = document.getElementById(id_label);

        if (elt_label) {
            const new_val = (elt_label.innerText == "off") ? 100 : 0;
            const id_img = id.replace("btn_", "img_");
            var elt_img = document.getElementById(id_img);
            if (elt_img) {
                elt_img.setAttribute("src", (new_val == 0) ? "images/ic_light_off.svg" : "images/ic_light_on.svg")
            }
            elt_label.innerText = (new_val == 0) ? "off" : "on";
            console.log("onClickLight : ", elt_label.innerText);
            id_label = id.replace("btn_", "");

            const json = {
                command: 'pub',
                data: {
                    system: {
                        command: "ledctrl",
                        led_node: id_label,
                        sequence_id: 0,
                        led_mode: elt_label.innerText,
                        led_on_time: 500,
                        led_off_time: 500,
                        loop_times: 0,
                        interval_time: 0,
                    },
                }
            };
            Component._ws.sendJson(json);
        }
    }
}

//
// Speed Class
//
class Speed extends Component {
    constructor() {
        super();
        this._speed = 0;
    }

    updateFromJson(json) {
        if (Object.keys(json).includes('spd_lvl')) {
            console.log("spd_lvl : " + String(json['spd_lvl']));
            var combo = document.getElementById('spd_lvl');
            if (combo) {
                this._speed = json['spd_lvl'] - 1;
                combo.options[this._speed].selected = true;
            }
        }
    }

    changeSpeed(id) {
        const elt = document.getElementById(id);
        if (elt) {
            console.log("onChange : " + id + " " + elt.value + " " + elt.selectedIndex);
            const json = {
                command: 'pub',
                data: {
                    print: {
                        command: "print_speed",
                        sequence_id: 0,
                        param: String(elt.selectedIndex + 1),
                    },
                }
            };
            Component._ws.sendJson(json);
        }
    }
}