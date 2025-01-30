import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns

WEBRTC_DF_OUTPUT = "/users/agot/outputs/all_data.csv"
WEBRTC_OUTPUT_FOLDER = "/users/agot/outputs/"

def analyze_kpi_metrics(df):
    # Create a copy to avoid modifying the original dataframe
    df_clean = df.copy()
    
    # Replace NaN values with 0
    df_clean = df_clean.fillna(0)
    
    # Define KPI metrics (excluding timestamp, datetime and filename)
    kpi_metrics = [
        'receiving_rate_mbps',
        'packets_lost',
        'jitter',
        'fps',
        'freeze_rate',
        'freeze_duration',
        'frame_delay'
    ]
    
    # Analyze each KPI metric
    analysis_results = {}
    for metric in kpi_metrics:
        metric_data = df_clean[metric]
        analysis = {
            'mean': metric_data.mean(),
            'median': metric_data.median(),
            'std': metric_data.std(),
            'min': metric_data.min(),
            'max': metric_data.max(),
            'non_zero_count': (metric_data != 0).sum(),
            'zero_count': (metric_data == 0).sum()
        }
        analysis_results[metric] = analysis
    
    # Print the analysis
    print("KPI Metrics Analysis:")
    print("=" * 50)
    for metric, stats in analysis_results.items():
        print(f"\n{metric.upper()}")
        print("-" * 30)
        for stat_name, value in stats.items():
            print(f"{stat_name:15}: {value:.3f}")
            
    return df_clean, analysis_results

def plot_kpi_distributions(df, analysis_results):
    """
    Create individual distribution plots for each KPI metric.
    
    Parameters:
    df (pandas.DataFrame): The cleaned dataframe containing KPI metrics
    analysis_results (dict): Dictionary containing statistical analysis for each metric
    """
    # Define KPI metrics
    kpi_metrics = [
        'receiving_rate_mbps',
        'packets_lost',
        'jitter',
        'fps',
        'freeze_rate',
        'freeze_duration',
        'frame_delay'
    ]

    # increase font size
    plt.rcParams.update({'font.size': 36})
    
    # Plot each metric separately
    for metric in kpi_metrics:
        # Create a new figure for each metric
        plt.figure(figsize=(20, 12))
        
        # Create distribution plot
        sns.histplot(data=df[metric], kde=True)
        plt.title(f'Distribution of {metric}')
        
        # Add mean and median lines
        plt.axvline(analysis_results[metric]['mean'], 
                   color='red', 
                   linestyle='--', 
                   label='Mean')
        plt.axvline(analysis_results[metric]['median'], 
                   color='green', 
                   linestyle='--', 
                   label='Median')
        plt.legend()

        if metric == 'fps':
            plt.xlim(0, 35)
        elif metric == 'receiving_rate_mbps':
            plt.xlim(0, 50)
        else:
            pass
        
        # Adjust layout
        plt.tight_layout()

        # Save individual plot
        plt.savefig(f"{WEBRTC_OUTPUT_FOLDER}figures/kpi_distribution_{metric}.png")
        plt.close()  # Close the figure to free memory

df = pd.read_csv(WEBRTC_DF_OUTPUT)
clean_df, results = analyze_kpi_metrics(df)
plot_kpi_distributions(clean_df, results)