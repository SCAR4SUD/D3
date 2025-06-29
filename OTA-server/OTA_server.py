import os
import re
import glob
import hashlib
import base64
from dotenv import load_dotenv

from flask import Flask, jsonify, send_file, abort
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import padding

load_dotenv()

app = Flask(__name__)

PRIVATE_KEY_PATH = "private_key.pem"
PASSPHRASE    = os.getenv("OTA_KEY_PASSPHRASE").encode()
SERVER_HOST      = "0.0.0.0"
SERVER_PORT      = 5000
ECU_BASE_DIR     = "."  
FILENAME_RE      = re.compile(r"^firmware-(\d+\.\d+\.\d+)\.bin$")


with open(PRIVATE_KEY_PATH, "rb") as key_file:
    private_key = serialization.load_pem_private_key(
        key_file.read(),
        password=PASSPHRASE
    )

def parse_version(vstr):
    """Da '1.2.3' restituisce (1,2,3)."""
    return tuple(int(x) for x in vstr.split("."))

def find_latest_firmware(ecu_id):
    """Ritorna (version_str, full_path) del firmware più alto."""
    ecu_dir = os.path.join(ECU_BASE_DIR, ecu_id)
    if not os.path.isdir(ecu_dir):
        return None, None

    candidates = []
    for fname in os.listdir(ecu_dir):
        m = FILENAME_RE.match(fname)
        if m:
            ver = m.group(1)
            candidates.append((parse_version(ver), ver, os.path.join(ecu_dir, fname)))

    if not candidates:
        return None, None

    _, ver_str, path = max(candidates, key=lambda t: t[0])
    return ver_str, path

def sign_file(path):
    digest = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            digest.update(chunk)
    signature = private_key.sign(
        digest.digest(),
        padding.PKCS1v15(),
        hashes.SHA256()
    )
    return base64.b64encode(signature).decode()

@app.route("/metadata/<ecu_id>")
def metadata(ecu_id):
    version, fw_path = find_latest_firmware(ecu_id)
    if not fw_path:
        abort(404, description=f"Nessun firmware trovato per ECU '{ecu_id}'")
    signature = sign_file(fw_path)
    return jsonify({
        "ecu":       ecu_id,
        "version":   version,
        "url":       f"http://{SERVER_HOST}:{SERVER_PORT}/download/{ecu_id}",
        "signature": signature
    })

@app.route("/download/<ecu_id>")
def download(ecu_id):
    version, fw_path = find_latest_firmware(ecu_id)
    if not fw_path:
        abort(404, description=f"Nessun firmware trovato per ECU '{ecu_id}'")
    return send_file(fw_path, as_attachment=True)

@app.route("/public_key")
def public_key():
    if not os.path.isfile(PUBLIC_KEY):
        abort(404, "Public key not found")
    return send_file(PUBLIC_KEY, as_attachment=True)

if __name__ == "__main__":
    app.run(host=SERVER_HOST, port=SERVER_PORT)