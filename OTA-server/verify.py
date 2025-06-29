import base64
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import padding

# --- Configurazione ---
PUBKEY_PATH = "public_key.pem"
FIRMWARE   = "firmware-0.0.2-prova.bin"
SIG_B64    = "AMQps+Y1GTVxCS/LKpKg7ug/jer95D4o2TG7J1eE/8LxeNYDphKU8udQjyFSKhQfhasipvHHzrQyW+eNdgTD/pUTZY/lgzp2UCRlL+J8p3XHCP0MxkiB2n/+2tnurvpkL2WmIWdIGo5ycFZFZ0dDOaqZBHxRtG3S51GcNjva45VFJABB5XbglS6qqq5Du+qGNXe/iYe44jBZ/WWpPXGMj2Mj5m3W4LBA0ZRiDXj3/DlQ427xIVe2SE8LnofaTizcMOi8JAGFcnH3PdyU5u9swpxp29EQEJqmRpqxyXhkQZXzX5ntxAcU4Y2By8NuMO1LOp3qg36SX+CD0OYJfMFkhA=="
# 1. Carica chiave pubblica
with open(PUBKEY_PATH, "rb") as f:
    pubkey = serialization.load_pem_public_key(f.read())

# 2. Decodifica la firma (se è in B64)
signature = base64.b64decode(SIG_B64)

# 3. Calcola digest SHA256 del firmware
hasher = hashes.Hash(hashes.SHA256())
with open(FIRMWARE, "rb") as f:
    for chunk in iter(lambda: f.read(4096), b""):
        hasher.update(chunk)
digest = hasher.finalize()

# 4. Verifica
try:
    pubkey.verify(
        signature,
        digest,
        padding.PKCS1v15(),
        hashes.SHA256()
    )
    print("File valido")
except Exception as e:
    print("File non valido:", e)