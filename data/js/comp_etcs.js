
//
// Others Class
//
class Etcs extends Component {
    constructor() {
        super();
        this.is_sdcard = false;
    }

    updateFromJson(json) {
        // sdcard check
        if (Object.keys(json).includes('sdcard')) {
            this.is_sdcard = json['sdcard'];
            document.getElementById('sdcard_cell').style.display = this.is_sdcard ? '' : 'none';
        }
    }
}


//
// PrinterInfo Class
//
class PrinterInfo extends Component {
    constructor() {
        super();
        this.name = null;
    }

    updateFromJson(json) {
        if (Object.keys(json).includes('printer_name')) {
            var title = document.getElementById('printer_name');
            if (json['printer_name']) {
                title.innerText = json['printer_name'];
                this.name = json['printer_name'];
            }
        }
    }
}