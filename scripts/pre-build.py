import json
import subprocess
import os
import hashlib

Import("env")

env = DefaultEnvironment()


# Make sure 'vscode init' is not the current command
def is_pio_build():
    from SCons.Script import DefaultEnvironment
    env = DefaultEnvironment()
    if "IsCleanTarget" in dir(env) and env.IsCleanTarget():
        return False
    return not env.IsIntegrationDump()


if is_pio_build() == True:
    print(f"XTOUCH PREBUILD")
    result = subprocess.run(['node', 'scripts/download-errors.js'],
                            text=True, check=True, capture_output=True)
    print(result.stdout)

    # if os.path.exists('.pio/build/esp32dev/firmware.elf'):
    #     os.remove('.pio/build/esp32dev/firmware.elf')
    # if os.path.exists('.pio/build/esp32dev/firmware.bin'):
    #     os.remove('.pio/build/esp32dev/firmware.bin')
