//
// Component Class
//
class Component {
    static Status = {
        UNKNOWN : 0,
        IDLE : 1,
        RUNNING : 2,
        PAUSED : 3,
        FINISHED : 4,
        PREPARE : 5,
        FAILED : 6
    };
    static _ws = null;
    static _status = Component.Status.UNKNOWN;

    constructor() {
    }

    static setWebSocket(socket) {
        Component._ws = socket;
    }

    updateFromJson(json) {
    }

    updateFromArrayBuffer(cmd, payload) {
    }
}