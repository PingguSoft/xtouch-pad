//
// Temperature Class
//
class Temperature extends Component {
    constructor() {
        super();
    }

    updateFromJson(json) {
        const nodes = ['nozzle_temper', 'nozzle_target_temper', 'bed_temper', 'bed_target_temper', 'chamber_temper'];
        for (var i = 0; i < nodes.length; i++) {
            if (Object.keys(json).includes(nodes[i])) {
                const label = document.getElementById(nodes[i]);
                if (label) {
                    const val = Math.round(json[nodes[i]]);
                    if (label.nodeName.toLowerCase() == 'input')
                        label.value = val;
                    else
                        label.innerText = val;

                    console.log(nodes[i], val);
                }
            }
        }
    }

    changeTemp(id) {
        const elt = document.getElementById(id);

        if (elt) {
            console.log("onChange : " + id + " " + elt.value);

            var gcode;
            if (id == "nozzle_target_temper") {
                gcode = "M104 S" + String(elt.value) + "\n";
            } else if (id == "bed_target_temper") {
                gcode = "M140 S" + String(elt.value) + "\n";
            }
            Component._ws.sendGcode(gcode);
        }
    }
}