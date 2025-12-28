import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def main():
    print("Reading data...")
    df = pd.read_csv("freq_data.csv", names=["rank", "count", "word"], header=0)
    
    df = df.head(5000)
    
    ranks = df["rank"]
    counts = df["count"]
    
    plt.figure(figsize=(10, 6))
    
    plt.loglog(ranks, counts, marker='.', linestyle='none', markersize=2, label='Real Data')
    
    C = counts[0]
    zipf_line = [C / r for r in ranks]
    
    plt.loglog(ranks, zipf_line, 'r-', linewidth=2, label='Zipf\'s Law ($C/rank$)')
    
    plt.title("Zipf's Law Analysis (Log-Log Scale)")
    plt.xlabel("Rank (Log)")
    plt.ylabel("Frequency (Log)")
    plt.legend()
    plt.grid(True, which="both", ls="-", alpha=0.5)
    
    output_file = "zipf_graph.png"
    plt.savefig(output_file)
    print(f"Graph saved to {output_file}")
    plt.show()

if __name__ == "__main__":
    main()
