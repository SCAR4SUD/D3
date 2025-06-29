# OTA Server README

This document describes how to set up and run the OTA server that manages multiple firmware files for ECUs, dynamically selecting the latest version based on the ECU ID.

## System Prerequisites

Before proceeding, make sure your system has:

- **Python 3.7+**
- **pip** (Python package manager)
- **OpenSSL CLI** (for RSA key generation)
- **virtualenv** (optional, for creating isolated environments)

On Debian/Ubuntu, you can install the system dependencies with:

```bash
sudo apt update && sudo apt install -y \
    python3 python3-pip python3-venv \
    openssl
```

## Project Structure

```text
ota-server/
├── .env                  # Environment variables file (passphrase)
├── server.py             # Flask script for metadata and download endpoints
├── private_key.pem       # RSA private key (AES-256 encrypted)
├── public_key.pem        # RSA public key (PEM format)
├── requirements.txt      # Python dependencies
└── <ECU_ID>/             # Firmware directories for each ECU
    ├── firmware-0.0.1.bin
    └── firmware-0.0.2.bin
```

Each `<ECU_ID>/` folder (e.g. `ECU001/`, `ECU002/`) should contain one or more files:

```
firmware-<version>.bin  # Where <version> follows the X.Y.Z format
```

## 1. `.env` Configuration File

In the project root, create a `.env` file with the following content:

```dotenv
# Passphrase for decrypting private_key.pem
OTA_KEY_PASSPHRASE=LaMiaPassphraseSicura
```

## 2. Generating the RSA Key Pair

Run the following commands to generate the encrypted private key and the corresponding public key:

```bash
# 1. Generate the RSA private key encrypted with AES-256, using the passphrase from .env
o
openssl genpkey \
  -algorithm RSA \
  -aes256 \
  -pass pass:LaMiaPassphraseSicura \
  -out private_key.pem \
  -pkeyopt rsa_keygen_bits:2048

# 2. Extract the public key (unencrypted)
openssl rsa \
  -pubout \
  -in private_key.pem \
  -passin pass:LaMiaPassphraseSicura \
  -out public_key.pem
```

> **Important**: Always protect `private_key.pem` (for example, with `chmod 600`).

## 3. Installing Python Dependencies

Create or update `requirements.txt` with the following packages:

```text
Flask>=1.1.0
cryptography>=3.3.1
python-dotenv>=0.15.0
```

Then, in a virtual environment (optional but recommended):

```bash
# Create and activate a virtual environment (optional)
python3 -m venv venv
source venv/bin/activate

# Upgrade pip and install dependencies
pip install --upgrade pip
pip install -r requirements.txt
```

## 4. Server Configuration

1. Verify that the `.env` file contains the `OTA_KEY_PASSPHRASE` variable.
2. In `server.py`, ensure you load environment variables at the top of the file:

```python
import os
from dotenv import load_dotenv

# Load variables from .env file
load_dotenv()
```

3. Check that the constants in `server.py` match your setup:
   - `PASSPHRASE = os.getenv("OTA_KEY_PASSPHRASE").encode()`
   - `PRIVATE_KEY_PATH = "private_key.pem"`
   - `ECU_BASE_DIR = "."` (or the path to your ECU folders)

## 5. Starting the Server

```bash
python server.py
```

The server will be available at `http://0.0.0.0:5000/`.

## 6. Testing the Endpoints

- **Metadata endpoint**:

  ```bash
  curl http://localhost:5000/metadata/ECU001
  ```

- **Download endpoint**:

  ```bash
  curl -O http://localhost:5000/download/ECU001
  ```

If everything is set up correctly, you will receive JSON containing `version`, `url`, and `signature`, and you can download the latest firmware file.

---

*End of README*

