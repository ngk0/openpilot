#!/usr/bin/env python3
import json
import math
import requests
import subprocess
import tarfile
import threading
import urllib.error
import urllib.request
import zipfile

from functools import cache
from pathlib import Path

import openpilot.system.sentry as sentry

from openpilot.frogpilot.common.frogpilot_variables import EARTH_RADIUS, KONIK_PATH

running_threads = {}

locks = {
  "backup_toggles": threading.Lock(),
  "capture_report": threading.Lock(),
  "download_all_models": threading.Lock(),
  "download_model": threading.Lock(),
  "download_theme": threading.Lock(),
  "flash_panda": threading.Lock(),
  "lock_doors": threading.Lock(),
  "send_stats": threading.Lock(),
  "update_checks": threading.Lock(),
  "update_maps": threading.Lock(),
  "update_openpilot": threading.Lock()
}

def run_thread_with_lock(name, target, args=(), report=True):
  with locks[name]:
    if not running_threads.get(name, threading.Thread()).is_alive():
      def wrapped_target(*t_args):
        try:
          target(*t_args)
        except urllib.error.HTTPError as error:
          print(f"HTTP error: {error}")
        except subprocess.CalledProcessError as error:
          print(f"CalledProcessError in thread '{name}': {error}")
        except Exception as exception:
          print(f"Error in thread '{name}': {exception}")
          if report:
            sentry.capture_exception(exception)
      thread = threading.Thread(target=wrapped_target, args=args, daemon=True)
      thread.start()
      running_threads[name] = thread

def calculate_distance_to_point(lat1, lon1, lat2, lon2):
  lat1_rad = math.radians(lat1)
  lon1_rad = math.radians(lon1)
  lat2_rad = math.radians(lat2)
  lon2_rad = math.radians(lon2)

  sin_delta_lat = math.sin((lat2_rad - lat1_rad) / 2)
  sin_delta_lon = math.sin((lon2_rad - lon1_rad) / 2)

  haversine = sin_delta_lat ** 2 + math.cos(lat1_rad) * math.cos(lat2_rad) * sin_delta_lon ** 2
  haversine = min(1, max(0, haversine))

  angular_distance = 2 * math.atan2(math.sqrt(haversine), math.sqrt(1 - haversine))
  return EARTH_RADIUS * angular_distance

def delete_file(path, print_error=True, report=True):
  path = Path(path)
  if path.is_file() or path.is_symlink():
    run_cmd(["sudo", "rm", "-f", str(path)], f"Deleted file: {path}", f"Failed to delete file: {path}", report=report)
  elif path.is_dir():
    run_cmd(["sudo", "rm", "-rf", str(path)], f"Deleted directory: {path}", f"Failed to delete directory: {path}", report=report)
  elif print_error:
    print(f"File not found: {path}")

def extract_tar(tar_file, extract_path):
  tar_file = Path(tar_file)
  extract_path = Path(extract_path)
  print(f"Extracting {tar_file} to {extract_path}")

  with tarfile.open(tar_file, "r:gz") as tar:
    tar.extractall(path=extract_path, filter="data")

  tar_file.unlink()
  print(f"Extraction completed: {tar_file} has been removed")

def extract_zip(zip_file, extract_path):
  zip_file = Path(zip_file)
  extract_path = Path(extract_path)
  print(f"Extracting {zip_file} to {extract_path}")

  extract_root = extract_path.resolve()
  with zipfile.ZipFile(zip_file, "r") as zip_ref:
    for member in zip_ref.namelist():
      if not (extract_path / member).resolve().is_relative_to(extract_root):
        raise ValueError(f"Refusing to extract path outside destination: {member}")
    zip_ref.extractall(extract_path)

  zip_file.unlink()
  print(f"Extraction completed: {zip_file} has been removed")

def is_url_pingable(url, session=requests):
  if not url:
    return False

  headers = {"User-Agent": "frogpilot-ping-test/1.0 (https://github.com/FrogAi/FrogPilot)"}
  try:
    response = session.head(url, headers=headers, timeout=10, allow_redirects=True)
    try:
      if response.status_code in (405, 501):
        response.close()
        response = session.get(url, headers=headers, timeout=10, allow_redirects=True, stream=True)

      return response.ok
    finally:
      response.close()

  except (requests.exceptions.ConnectionError, requests.exceptions.SSLError):
    return False
  except requests.exceptions.RequestException as error:
    print(f"{error.__class__.__name__} while pinging {url}: {error}")
    return False
  except Exception as exception:
    print(f"Unexpected error while pinging {url}: {exception}")
    return False

def load_json_file(path):
  path = Path(path)
  if not path.is_file():
    return {}

  try:
    with open(path) as file:
      data = json.load(file)
  except (OSError, json.JSONDecodeError):
    print(f"Failed to load JSON file: {path}")
    return {}

  if not isinstance(data, dict):
    print(f"Failed to load JSON file: {path}")
    return {}

  return data

def run_cmd(cmd, success_message, fail_message, env=None, report=True):
  try:
    result = subprocess.run(cmd, capture_output=True, check=True, env=env, text=True)
    print(success_message)
    return result.stdout.strip()
  except subprocess.CalledProcessError as exception:
    print(f"Command failed with error: {exception.stderr}")
    print(fail_message)
    if report:
      sentry.capture_exception(exception.stderr)
    return None
  except Exception as exception:
    print(f"Unexpected error occurred: {exception}")
    print(fail_message)
    if report:
      sentry.capture_exception(exception)
    return None

def update_json_file(path, data):
  with open(path, "w") as file:
    json.dump(data, file, indent=2, sort_keys=True)

@cache
def use_konik_server():
  return KONIK_PATH.is_file()
