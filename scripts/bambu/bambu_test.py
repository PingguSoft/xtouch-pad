import signal
import asyncio
import sys
import getopt
import logging

from pybambu import BambuClient
from ftpsclient import IoTFTPSClient

logging.basicConfig(level=logging.DEBUG)

number = 0


def new_update(device):
    print(device.__dict__)


async def new_main(argv):
    opts, args = getopt.getopt(argv, "hd:i:a:s:", ["devicetype=", "host_ip=", "access_code=", "serial="])
    for opt, arg in opts:
        logging.debug(opt)
        if opt == '-h':
            print('websocket.py -d <device_type> -i <host_ip> -a <access_code> -s <serial>')
            sys.exit()
        elif opt in ("-d", "--devicetype"):
            device_type = arg
        elif opt in ("-i", "--host_ip"):
            host = arg
        elif opt in ("-a", "--access_code"):
            access_code = arg
        elif opt in ("-s", "--serial"):
            serial = arg

    bambu = BambuClient(device_type=device_type,
                        serial=serial,
                        host=host,
                        username="bblp",
                        access_code=access_code,
                        local_mqtt=True,
                        region=None,
                        email=None,
                        auth_token=None
                        )
    success = await bambu.try_connection()
    if success:
        print("Starting MQTT")

        ftp = IoTFTPSClient(f"{host}", 990, "bblp", f"{access_code}", ssl_implicit=True)
        filelist = ftp.list_files("", ".3mf")
        print(filelist)

        def event_handler(event):
            global number

            device_instance = bambu.get_device()
            print('event:', device_instance, event)
            if event == 'event_printer_chamber_image_update':
                data = device_instance.chamber_image.get_jpeg()
                if data is not None:
                    filename = f'cam_{number:03d}.jpg'
                    print(f'saving {filename}..')
                    with open(filename, 'wb') as f:
                        f.write(data)
                    number += 1

        bambu.connect(callback=event_handler)
        await asyncio.sleep(10)
        bambu.disconnect()

    else:
        print("Connection failed")


if __name__ == "__main__":
    co = new_main(sys.argv[1:])
    try:
        asyncio.run(co)
    except KeyboardInterrupt:
        print('111111111111')
        co.close()
