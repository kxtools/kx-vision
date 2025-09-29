"""
Guild Wars 2 API Data Fetcher
Part of the kx-vision project: https://github.com/kxtools/kx-vision

Fetches item and stat data from the GW2 API and caches it locally.
Requires: requests, tqdm
"""

import requests
import time
import os
import json
import logging
from tqdm import tqdm

# Setup simple logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

# Configuration
ENDPOINTS_TO_FETCH = [
    # {'name': 'ITEM', 'path': '/items', 'fetch_type': 'paginated'},
    {'name': 'STAT', 'path': '/itemstats', 'fetch_type': 'single'},
]
API_BASE_URL = "https://api.guildwars2.com/v2"
PAGE_SIZE = 200
CACHE_DIR = "api_cache"
REQUEST_TIMEOUT = 30
API_RETRY_DELAY = 5
API_RETRY_ATTEMPTS = 3

def validate_response_data(data, endpoint_name):
    """Basic validation of API response data."""
    if not isinstance(data, list):
        raise ValueError(f"Expected list for {endpoint_name}, got {type(data).__name__}")
    
    if not data:
        logger.warning(f"No data received for endpoint {endpoint_name}")
        return
    
    # Check first item has required fields
    required_fields = {'ITEM': ['id', 'name'], 'STAT': ['id', 'name']}
    first_item = data[0]
    
    if not isinstance(first_item, dict):
        raise ValueError(f"Invalid data structure in {endpoint_name}")
    
    for field in required_fields.get(endpoint_name, []):
        if field not in first_item:
            raise ValueError(f"Missing required field '{field}' in {endpoint_name} data")

def fetch_paginated_endpoint(endpoint):
    """Fetch paginated endpoint with improved error handling."""
    try:
        logger.info(f"[{endpoint['name']}] Fetching all IDs...")
        ids_url = f"{API_BASE_URL}{endpoint['path']}"
        ids_response = requests.get(ids_url, timeout=REQUEST_TIMEOUT)
        ids_response.raise_for_status()
        all_ids = ids_response.json()
        logger.info(f"Found {len(all_ids)} total entries for {endpoint['name']}")
        
        if not all_ids:
            logger.warning(f"No IDs found for {endpoint['name']}")
            return []
            
    except requests.exceptions.RequestException as e:
        logger.error(f"Failed to fetch ID list for {endpoint['name']}: {e}")
        return None
    except (json.JSONDecodeError, ValueError) as e:
        logger.error(f"Invalid JSON response for {endpoint['name']}: {e}")
        return None

    all_data = []
    failed_chunks = 0
    
    with tqdm(total=len(all_ids), unit=" entries", desc=f"Fetching {endpoint['name']:<5}") as pbar:
        for i in range(0, len(all_ids), PAGE_SIZE):
            chunk = all_ids[i:i + PAGE_SIZE]
            chunk_str = ",".join(map(str, chunk))
            
            for attempt in range(API_RETRY_ATTEMPTS):
                try:
                    data_url = f"{API_BASE_URL}{endpoint['path']}?ids={chunk_str}"
                    response = requests.get(data_url, timeout=REQUEST_TIMEOUT)
                    response.raise_for_status()
                    chunk_data = response.json()
                    
                    if not isinstance(chunk_data, list):
                        raise ValueError("Expected list response")
                    
                    all_data.extend(chunk_data)
                    pbar.update(len(chunk))
                    time.sleep(0.05)  # Rate limiting
                    break
                    
                except (requests.exceptions.RequestException, json.JSONDecodeError, ValueError) as e:
                    if attempt < API_RETRY_ATTEMPTS - 1:
                        logger.warning(f"Chunk {i//PAGE_SIZE + 1} failed (attempt {attempt + 1}): {e}")
                        time.sleep(API_RETRY_DELAY)
                    else:
                        logger.error(f"Chunk {i//PAGE_SIZE + 1} failed permanently: {e}")
                        failed_chunks += 1
                        pbar.update(len(chunk))  # Skip this chunk
                        break
    
    if failed_chunks > 0:
        logger.warning(f"Failed to fetch {failed_chunks} chunks for {endpoint['name']}")
    
    return all_data if all_data else None

def fetch_single_endpoint(endpoint):
    """Fetch single endpoint with improved error handling."""
    url = f"{API_BASE_URL}{endpoint['path']}?ids=all"
    
    for attempt in range(API_RETRY_ATTEMPTS):
        try:
            logger.info(f"[{endpoint['name']}] Fetching all entries...")
            response = requests.get(url, timeout=REQUEST_TIMEOUT)
            response.raise_for_status()
            data = response.json()
            
            if not isinstance(data, list):
                raise ValueError("Expected list response")
            
            logger.info(f"Found {len(data)} total entries for {endpoint['name']}")
            return data
            
        except (requests.exceptions.RequestException, json.JSONDecodeError, ValueError) as e:
            if attempt < API_RETRY_ATTEMPTS - 1:
                logger.warning(f"Attempt {attempt + 1} failed for {endpoint['name']}: {e}")
                time.sleep(API_RETRY_DELAY)
            else:
                logger.error(f"Failed to fetch {endpoint['name']} after {API_RETRY_ATTEMPTS} attempts: {e}")
                return None

def save_data_safely(data, output_path):
    """Save data with atomic write to prevent corruption."""
    temp_path = f"{output_path}.tmp"
    try:
        with open(temp_path, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
        
        # Atomic move
        if os.path.exists(output_path):
            os.replace(temp_path, output_path)
        else:
            os.rename(temp_path, output_path)
            
        logger.info(f"Successfully saved {len(data)} entries to {output_path}")
        return True
        
    except (IOError, OSError) as e:
        logger.error(f"Failed to save data to {output_path}: {e}")
        if os.path.exists(temp_path):
            try:
                os.remove(temp_path)
            except OSError:
                pass
        return False

if __name__ == "__main__":
    logger.info("Starting API data fetcher...")
    
    try:
        os.makedirs(CACHE_DIR, exist_ok=True)
    except OSError as e:
        logger.error(f"Failed to create cache directory {CACHE_DIR}: {e}")
        exit(1)
    
    success_count = 0
    total_endpoints = len(ENDPOINTS_TO_FETCH)
    
    for endpoint in ENDPOINTS_TO_FETCH:
        logger.info("-" * 50)
        data = None
        
        try:
            if endpoint['fetch_type'] == 'paginated':
                data = fetch_paginated_endpoint(endpoint)
            elif endpoint['fetch_type'] == 'single':
                data = fetch_single_endpoint(endpoint)
            else:
                logger.error(f"Unknown fetch_type: {endpoint['fetch_type']}")
                continue
            
            if data is not None:
                validate_response_data(data, endpoint['name'])
                output_path = os.path.join(CACHE_DIR, f"{endpoint['name']}_data.json")
                
                if save_data_safely(data, output_path):
                    success_count += 1
                    
        except Exception as e:
            logger.error(f"Unexpected error processing {endpoint['name']}: {e}")
            continue
    
    if success_count == total_endpoints:
        logger.info("All API data successfully fetched and cached!")
    elif success_count > 0:
        logger.warning(f"Partial success: {success_count}/{total_endpoints} endpoints completed")
    else:
        logger.error("No endpoints were successfully processed")
        exit(1)