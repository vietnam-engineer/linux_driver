class StringUtils:
    @staticmethod
    def from_hex_str(hex_str: str) -> bytes:
        """
        Converts a hexadecimal string (e.g., "A0B1C2D3") to binary format (e.g., b'\xa0\xb1\xc2\xd3').
        """
        try:
            return bytes.fromhex(hex_str)
        except ValueError:
            raise ValueError(f"Invalid hex string: {hex_str}")

    @staticmethod
    def to_hex_str(bin_data) -> str:
        """
        Converts binary data (e.g., b'\xa0\xb1\xc2\xd3') or a number (e.g, 0xa0b1c2d3)
        to a hexadecimal string in uppercase (e.g., "A0B1C2D3").
        """
        if isinstance(bin_data, (bytes, bytearray)):
            return bin_data.hex().upper()
        elif isinstance(bin_data, int):
            return f"{bin_data:08X}"
        else:
            raise TypeError("Input must be bytes, bytearray, or int")
