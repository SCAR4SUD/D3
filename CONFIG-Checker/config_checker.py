#!/usr/bin/env python3
import os
import sqlite3
import yaml
import schedule
import time
from datetime import datetime
import logging
import smtplib
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
import json

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S"
)

CONFIG_FILES = {
    'prod': 'configuration-prod.yml',
    'pre-prod': 'configuration-pre-prod.yml',
    'develop': 'configuration-develop.yml',
}

DB_PATH = 'configurations.db'
TABLE_NAME = 'configurations'
CONFIG_PATH = 'config.json'

def flatten_dict(d, parent_key='', sep='.'):
    items = {}
    for k, v in d.items():
        new_key = f"{parent_key}{sep}{k}" if parent_key else k
        if isinstance(v, dict):
            items.update(flatten_dict(v, new_key, sep=sep))
        else:
            items[new_key] = v
    return items

def init_db():
    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()
    c.execute(f'''
        CREATE TABLE IF NOT EXISTS {TABLE_NAME} (
            env TEXT,
            key   TEXT,
            value   TEXT,
            PRIMARY KEY (env, key)
        )
    ''')
    conn.commit()
    conn.close()

def load_db_configs():
    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()
    c.execute(f"SELECT env, key, value FROM {TABLE_NAME}")
    rows = c.fetchall()
    conn.close()
    return {(amb, k): v for amb, k, v in rows}

def populate_db_initial():
    db = load_db_configs()
    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()
    for env, path in CONFIG_FILES.items():
        if not os.path.exists(path):
            continue
        with open(path, 'r') as f:
            data = yaml.safe_load(f) or {}
        flat = flatten_dict(data)
        for key, val in flat.items():
            if (env, key) not in db:
                c.execute(
                    f"INSERT INTO {TABLE_NAME}(env, key, value) VALUES (?, ?, ?)",
                    (env, key, str(val))
                )
    conn.commit()
    conn.close()


def send_via_email_differences(differences):
    if not differences:
        logging.info("No difference found.")
        return

    str_differences = "\n".join(differences)
    send_email_alert(str_differences)


def load_config(path):
    with open(path, 'r') as f:
        cfg = json.load(f)
    if 'email' not in cfg:
        raise ValueError("'email' missed in config")
    return cfg

def send_email_alert(msg_diff):
    smtp_user = os.getenv('SMTP_USER')
    smtp_pass = os.getenv('SMTP_PASS')
    cfg_email  = load_config(CONFIG_PATH)['email']
    server = smtplib.SMTP(cfg_email['smtp_server'], cfg_email['smtp_port'])
    if cfg_email.get('use_tls', True):
        server.starttls()
    server.login(smtp_user, smtp_pass)
    msg = MIMEMultipart()
    msg['From'] = cfg_email['from']
    msg['To'] = ', '.join(cfg_email['to'])
    msg['Subject'] = f"[ALERT] configurations are changed"

    msg.attach(MIMEText(msg_diff, 'plain'))
    server.sendmail(cfg_email['from'], cfg_email['to'], msg.as_string())
    server.quit()
    logging.info(f"Email alert send to: {cfg_email['to']}")


def check_configs():
    now = datetime.now().isoformat(sep=' ', timespec='seconds')
    db = load_db_configs()
    for env, path in CONFIG_FILES.items():
        if not os.path.exists(path):
            logging.info(f"{now}: File not found: {path}")
            continue
        try:
            with open(path, 'r') as f:
                data = yaml.safe_load(f) or {}
        except Exception as e:
            logging.error(f"{now}: Errore parsing {path}: {e}")
            continue
        differences = []
        flat = flatten_dict(data)
        for key, val in flat.items():
            str_val = str(val)
            db_val = db.get((env, key))
            if db_val is None:
                logging.info(f"{now}: [NEW] {env} | key '{key}' of configuration not exists in DB (file='{str_val}')")
            elif db_val != str_val:
                msg = (
                    f"{now}: [DIFF] {env} | key '{key}': "
                    f"file='{str_val}' vs db='{db_val}'"
                )
                logging.info(msg)
                differences.append(msg)

        send_via_email_differences(differences)

if __name__ == '__main__':
    init_db()
    populate_db_initial()
    check_configs()
    schedule.every().hour.do(check_configs)
    while True:
        schedule.run_pending()
        time.sleep(1)