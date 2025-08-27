# phần mềm này sẽ được chạy trên máy host (Windows hoặc Linux)
# để giả lập một thiết bị có tên là vn29a80.

import sys
import threading
import time
import serial # Hãy dùng lệnh "pip install pyserial" để cài đặt pyserial

from logger import Logger
from string_utils import StringUtils

STR_ID_GET_COUNTER = "01"
STR_ID_SET_COUNTER = "02"

STR_REQ_HEADER = "$REQ"
STR_REQ_FOOTER = "@"
STR_RES_HEADER = "$RES"
STR_RES_FOOTER = "@"
STR_SEPARATOR = ":"

OFFSET_REQ_ID_START = 5
OFFSET_REQ_ID_END = 7
OFFSET_COUNTER_START= 8
OFFSET_COUNTER_END= 16

MIN_REQ_LEN = 9
MAX_U32 = 0xFFFFFFFF

class VN29A80:
    def __init__(self, port, enable_logger=False):
        self.running = False
        self.counter = 0
        self.logger = Logger(enabled=enable_logger)
        self.serial_port = serial.Serial(
            port=port,
            baudrate=38400,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_EVEN,
            stopbits=serial.STOPBITS_ONE,
        )

    def handle_req_get_counter(self) -> str:
        try:
            str_count = StringUtils.to_hex_str(self.counter)
            res = f"{STR_RES_HEADER}:{STR_ID_GET_COUNTER}:{str_count}:{STR_RES_FOOTER}"
            return res
        except (ValueError, TypeError):
            raise Exception("handle_req_get_counter")

    def handle_req_set_counter(self, hex_str: str) -> str:
        try:
            self.counter = int(hex_str, 16)
            self.logger.dump(f'changed: self.counter = {self.counter}')
            res = f"{STR_RES_HEADER}:{STR_ID_SET_COUNTER}::{STR_RES_FOOTER}"
            return res
        except (ValueError, TypeError):
            raise Exception("handle_req_set_counter")

    def communication_thread(self):
        req = ""
        while self.running:
            try:
                req = self.serial_port.read_until(b'@').decode('utf-8')
                self.logger.dump(f'received: {req}')

                while (len(req) >= MIN_REQ_LEN) and (req.startswith(STR_REQ_HEADER) == False):
                    self.logger.dump(f"Drop the 1st character({req[0]}) of {req}")
                    req = req[1:]

                if len(req) < MIN_REQ_LEN:
                    continue

                req_id = req[OFFSET_REQ_ID_START:OFFSET_REQ_ID_END]
                if req_id == STR_ID_GET_COUNTER:
                    self.logger.dump("REQ: GET_COUNTER")
                    res = self.handle_req_get_counter()
                    self.serial_port.write(res.encode('utf-8'))
                    self.logger.dump(f'sent: {res}')
                elif req_id == STR_ID_SET_COUNTER:
                    self.logger.dump("REQ: SET_COUNTER")
                    res = self.handle_req_set_counter(req[OFFSET_COUNTER_START:OFFSET_COUNTER_END])
                    self.serial_port.write(res.encode('utf-8'))
                    self.logger.dump(f'sent: {res}')
                else:
                    self.logger.dump(f"REQ: {req_id} unknown")

                req = ""
            except Exception as e:
                print(f"An unexpected error occurred: {e}")

    def increment_thread(self):
        while self.running:
            self.counter = (self.counter + 1) & MAX_U32
            self.logger.dump(f"VN29A80: {self.counter}")
            time.sleep(1)

    def run(self):
        self.running = True
        t1 = threading.Thread(target=self.increment_thread)
        t2 = threading.Thread(target=self.communication_thread)
        t1.start()
        t2.start()
        t1.join()
        t2.join()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python counter.py <COM_PORT> [dump]")
        sys.exit(1)

    port = sys.argv[1]
    enable_logger = len(sys.argv) > 2 and sys.argv[2].lower() == "dump"

    counter = VN29A80(port, enable_logger)
    counter.run()
