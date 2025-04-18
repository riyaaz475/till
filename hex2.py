import os
import random
import string

def generate_obfuscated_payload(size=8):
    if size < 2:
        raise ValueError("Size must be at least 2 to include start and end markers.")
    
    # Generate random bytes for the middle part
    middle_size = size - 2
    middle_payload = os.urandom(middle_size)
    
    # Mix bytes with printable ASCII characters for obfuscation
    mix_chars = bytes(random.choices(string.printable.encode(), k=middle_size))
    obfuscated_middle = bytes(a ^ b for a, b in zip(middle_payload, mix_chars))

    # Construct the final payload
    final_payload = b"\x3D" + obfuscated_middle + b"\x3D"

    # Convert to hex format and enclose in quotes
    hex_payload = ', '.join(f"0x{byte:02X}" for byte in final_payload)
    return f"{hex_payload}"  # Return formatted hex string

def save_payloads(filename="unique_payloads.txt", count=1000, min_size=8, max_size=32):
    with open(filename, "w") as f:
        for _ in range(count):
            size = random.randint(min_size, max_size)  # Randomize payload length
            payload = generate_obfuscated_payload(size)
            f.write(payload + "\n")

save_payloads("unique_payloads.txt", count=1000, min_size=2, max_size=64)

print("🚀 Unique obfuscated payloads saved to unique_payloads.txt")
