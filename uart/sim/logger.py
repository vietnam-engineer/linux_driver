from datetime import datetime

class Logger:
    def __init__(self, enabled=False):
        self.enabled = enabled

    def dump(self, message):
        if self.enabled and len(message):
            now = datetime.now().strftime("%H:%M:%S.%f")[:-3]
            print(f"[{now}] {message}")

    def dump_hex_array(self, hex_array):
        if self.enabled and len(hex_array):
            now = datetime.now().strftime("%H:%M:%S.%f")[:-3]
            print(f"[{now}] " + " ".join(f"{byte:02X}" for byte in hex_array))

