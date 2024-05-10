//
// Fans Class
//
class Fans extends Component {
    constructor() {
        super();
        this._table_fans = [
            'cooling_fan_speed',
            'big_fan1_speed',
            'big_fan2_speed',
            'heatbreak_fan_speed'
        ];
    }

    updateFromJson(json) {
        var speeds = [-1, -1, -1, -1];

        if (Object.keys(json).includes('fan_gear')) {
            // max : 255
            const gear = json['fan_gear'];
            speeds[0] = (gear >> 0) & 0xFF;
            speeds[1] = (gear >> 8) & 0xFF;
            speeds[2] = (gear >> 16) & 0xFF;
            speeds[3] = (gear >> 24) & 0xFF;
            console.log("fan_gear : " + gear.toString(16) + " " + speeds);
        } else {
            for (var i = 0; i < this._table_fans.length; i++) {
                if (Object.keys(json).includes(this._table_fans[i])) {
                    // max : 15
                    speeds[i] = Math.round(Math.floor(json[this._table_fans[i]] / 1.5) * 25.5);
                }
            }
            console.log("fan_speed : " + speeds);
        }

        // update icon and percentage only for updated ones
        for (var i = 0; i < this._table_fans.length; i++) {
            if (speeds[i] >= 0) {
                var speed = (speeds[i] == 0) ? 0 : Math.max(10, Math.round(speeds[i] * 100 / 255.0));
                var name = 'img_' + this._table_fans[i];

                var img = document.getElementById(name);
                if (img) {
                    img.setAttribute('src', (speed > 0) ? 'images/ic_fan_on.svg' : 'images/ic_fan_off.svg');
                }

                name = 'label_' + this._table_fans[i];
                var label = document.getElementById(name);
                if (label) {
                    if (label.nodeName.toLowerCase() == 'input')
                        label.value = speed;
                    else
                        label.innerText = (speed > 0) ? String(speed) + '%' : 'off';
                }
            }
        }
    }

    changeSpeed(id) {
        const elt = document.getElementById(id);
        if (elt) {
            console.log("onChange : " + id + " " + elt.value);
            var idx = 1;
            for (const fan of this._table_fans) {
                if (id.endsWith(fan)) {
                    const id_img = id.replace("label_", "img_");
                    var elt_img = document.getElementById(id_img);
                    if (elt_img) {
                        elt_img.setAttribute("src", (elt.value == 0) ? "images/ic_fan_off.svg" : "images/ic_fan_on.svg")
                    }

                    const speed = Math.round(elt.value * 2.55);
                    Component._ws.sendGcode("M106 P" + idx + " S" + speed + " \n");
                    break;
                }
                idx++;
            }
        }
    }

    turnOnOff(id) {
        const id_label = id.replace("btn_", "label_");
        var   elt_label = document.getElementById(id_label);

        if (elt_label && elt_label.nodeName.toLowerCase() == 'input') {
            const new_val = (elt_label.value == 0) ? 100 : 0;
            elt_label.value = new_val;
            console.log("onClickFan : ", new_val);
            this.changeSpeed(id_label);
        }
    }
}