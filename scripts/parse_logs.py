import re
import os
import pandas as pd
from typing import Dict, List

WEBRTC_OUTPUT_LOGS = "/users/agot/outputs/"

def parse_webrtc_logs(log_content: str) -> pd.DataFrame:
    """
    Parse WebRTC logs and return a pandas DataFrame containing the metrics.
    Only includes entries where receiving rate is not NAN.
    """
    # Lists to store the parsed data
    data = []
    
    # Pattern to match timestamp
    timestamp_pattern = r"PeerConnection Stats Report received, timestamp: (\d+)"
    
    # Pattern to match receiving rate (non-NAN)
    rate_pattern = r"Receiving Rate: ([\d.]+) Mbps"
    
    # Patterns for other metrics
    packets_pattern = r"Packets Lost: (\d+)"
    jitter_pattern = r"Jitter: ([\d.]+)"
    fps_pattern = r"Frames per Second: (\d+)"
    freeze_rate_pattern = r"Freeze Rate: ([\d.]+)"
    freeze_duration_pattern = r"Total Freeze Duration: ([\d.]+)"
    frame_delay_pattern = r"Total Inter Frame Delay: ([\d.]+)"

    # Split logs into sections by timestamp
    sections = log_content.split("PeerConnection Stats Report received")
    
    for section in sections[1:]:  # Skip first empty section
        try:
            # Get timestamp
            timestamp_match = re.search(timestamp_pattern, "PeerConnection Stats Report received" + section)
            if not timestamp_match:
                continue
            timestamp = int(timestamp_match.group(1))
            
            # Find non-NAN receiving rate
            rate_matches = re.finditer(rate_pattern, section)
            rate_match = None
            for match in rate_matches:
                rate = float(match.group(1))
                if rate != float('nan'):
                    rate_match = match
                    break
                    
            if not rate_match:
                continue
                
            # Get associated metrics
            receiving_rate = float(rate_match.group(1))
            
            # Find packets lost and jitter in the same section as the valid receiving rate
            section_text = section[rate_match.start():]
            
            packets_match = re.search(packets_pattern, section_text)
            jitter_match = re.search(jitter_pattern, section_text)
            fps_match = re.search(fps_pattern, section_text)
            freeze_rate_match = re.search(freeze_rate_pattern, section_text)
            freeze_duration_match = re.search(freeze_duration_pattern, section_text)
            frame_delay_match = re.search(frame_delay_pattern, section_text)
            
            if not all([packets_match, jitter_match]):
                continue
                
            metrics = {
                'timestamp': timestamp,
                'receiving_rate_mbps': receiving_rate,
                'packets_lost': int(packets_match.group(1)),
                'jitter': float(jitter_match.group(1)),
                'fps': int(fps_match.group(1)) if fps_match else None,
                'freeze_rate': float(freeze_rate_match.group(1)) if freeze_rate_match else None,
                'freeze_duration': float(freeze_duration_match.group(1)) if freeze_duration_match else None,
                'frame_delay': float(frame_delay_match.group(1)) if frame_delay_match else None
            }
            
            data.append(metrics)
            
        except (ValueError, AttributeError) as e:
            print(f"Error processing section: {e}")
            continue
    
    # Create DataFrame
    df = pd.DataFrame(data)
    
    # Convert timestamp to datetime
    df['datetime'] = pd.to_datetime(df['timestamp'], unit='ms')
    
    # Sort by timestamp
    df = df.sort_values('timestamp')
    
    # Reset index
    df = df.reset_index(drop=True)

    # add file name column to the dataframe
    df['file_name'] = log_file
    
    return df

# Example usage
if __name__ == "__main__":
    # Get log file names from the output directory
  
    log_files = [f for f in os.listdir(WEBRTC_OUTPUT_LOGS) if f.endswith('.log')]

    # print(log_files)

    # for testing
    all_dfs = []
    # log_files = [log_files[0]]

    # Parse logs into DataFrame
    print("\nParsing WebRTC logs...")
    for log_file in log_files:
        print(f"Processing log file: {log_file}")
        with open(f'{WEBRTC_OUTPUT_LOGS}{log_file}', 'r') as f:
            log_content = f.read()
            df = parse_webrtc_logs(log_content)
            all_dfs.append(df)

    # Save all dataframes to a single csv file
    print("\nSaving all data to a single csv file...")
    all_data = pd.concat(all_dfs)
    all_data.to_csv(f'{WEBRTC_OUTPUT_LOGS}all_data.csv', index=False)