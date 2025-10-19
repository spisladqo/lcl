import matplotlib.pyplot as plt
import re
import numpy as np
from scipy import stats

def parse_data(filename):
    """Parse the data file and extract sum of times for each run"""
    run_sums = []
    
    with open(filename, 'r') as file:
        content = file.read()
    
    # Split content into runs using "run X" as delimiter
    runs = re.split(r'run \d+', content)
    
    # Skip the first empty split if it exists
    if runs[0].strip() == '':
        runs = runs[1:]
    
    for run_data in runs:
        # Extract all floating point numbers from the run
        numbers = re.findall(r'\d+\.\d+', run_data)
        if numbers:
            # Convert to float and calculate sum
            times = [float(num) for num in numbers]
            run_sum = sum(times)
            run_sums.append(run_sum)
    
    return run_sums

# Main execution
if __name__ == "__main__":
    # Replace 'your_data_file.txt' with the actual filename
    filename = 'gist.txt'
    
    try:
        # Parse the data
        run_sums = parse_data(filename)

        plt.hist(run_sums)
        plt.show()
        
        # print(f"Analysis Results:")
        # print(f"Total runs: {len(run_sums)}")
        # print(f"Minimum sum: {min(run_sums):.6f}")
        # print(f"Maximum sum: {max(run_sums):.6f}")
        # print(f"Average sum: {sum(run_sums)/len(run_sums):.6f}")

        # # Create single smooth distribution plot
        # create_single_distribution_plot(run_sums)
        
    except FileNotFoundError:
        print(f"Error: File '{filename}' not found.")
    except Exception as e:
        print(f"Error processing file: {e}")