
//
// AXISControl Class
//
class AXISControl extends Component {
    constructor() {
        super();
    }

    onClickAxisButton(id) {
        console.log("onClickAxisButton : " + id);

        if (Component._status == Component.Status.RUNNING || Component._status == Component.Status.PAUSED)
            return;

        const axis_maps = [
            ['btn_head_y_p10', 'Y', 10],
            ['btn_head_y_m10', 'Y', -10],
            ['btn_head_y_p1', 'Y', 1],
            ['btn_head_y_m1', 'Y', -1],
            ['btn_head_x_p10', 'X', 10],
            ['btn_head_x_m10', 'X', -10],
            ['btn_head_x_p1', 'X', 1],
            ['btn_head_x_m1', 'X', -1],

            ['btn_bed_p10', 'Z', -10],
            ['btn_bed_p1', 'Z', -1],
            ['btn_bed_m10', 'Z', 10],
            ['btn_bed_m1', 'Z', 1],
        ];

        const speed_xy = 3000;
        const speed_z  = 1500;

        for (const axis of axis_maps) {
            if (id == axis[0]) {
                const speed = (axis[1] == 'Z') ? speed_z : speed_xy;
                const gcode = "M211 S\nM211 X1 Y1 Z1\nM1002 push_ref_mode\nG91\nG1 " + axis[1] + axis[2] + " F" + speed + "\nM1002 pop_ref_mode\nM211 R\n";
                Component._ws.sendGcode(gcode);
                break;
            }
        }
    }

    onClickExtruderButton(id) {
        console.log("onClickExtruderButton : " + id);

        if (Component._status == Component.Status.RUNNING || Component._status == Component.Status.PAUSED)
            return;

        if (id == "btn_extr_up") {
            Component._ws.sendGcode("M83\nG0 E-10.0 F900\n");
        } else if (id == "btn_extr_down") {
            Component._ws.sendGcode("M83\nG0 E10.0 F900\n");
        }
    }

    onClickHomeButton(id) {
        console.log("onClickHomeButton : " + id);
        if (Component._status == Component.Status.RUNNING || Component._status == Component.Status.PAUSED)
            return;
        Component._ws.sendGcode("G28\n");
    }
}