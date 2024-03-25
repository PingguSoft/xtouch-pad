.platformio\penv\Lib\site-packages\platformio\util.py 수정 for arm64
arm64 package가 대부분 없으므로 amd64용으로 받기위함


def get_systype():
    system = platform.system().lower()
    arch = platform.machine().lower()
    if system == "windows":
        if not arch:  # issue #4353
            arch = "x86_" + platform.architecture()[0]
        if "x86" in arch:
            arch = "amd64" if "64" in arch else "x86"
        if "arm64" in arch:
            arch = "amd64"

    if arch == "aarch64" and platform.architecture()[0] == "32bit":
        arch = "armv7l"
    return "%s_%s" % (system, arch) if arch else system