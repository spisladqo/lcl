import matplotlib.pyplot as plt
import re


def extract_averages(output_text):
    averages = {}
    current_category = None
    current_mode = None
    current_run_data = []  # Store all timing values for current run
    
    lines = output_text.strip().split('\n')
    
    # Initialize data structure
    category_data = {}
    
    for line in lines:
        line = line.strip()
        
        # Skip empty lines
        if not line:
            continue
            
        # Check for category headers
        if line == "Sequential":
            current_category = "Sequential"
            current_mode = "avg"  # Sequential doesn't have modes
            current_run_data = []
            if current_category not in category_data:
                category_data[current_category] = {}
            if current_mode not in category_data[current_category]:
                category_data[current_category][current_mode] = []
                
        elif line == "Parallel":
            current_category = "Parallel"
            current_mode = None
            current_run_data = []
            
        elif line == "Parallel + Queue":
            current_category = "Parallel + Queue"
            current_mode = None
            current_run_data = []
            
        # Check for mode lines
        elif line.startswith("Mode:"):
            if current_category:
                current_mode = line.split(":")[1].strip()
                current_run_data = []
                if current_category not in category_data:
                    category_data[current_category] = {}
                if current_mode not in category_data[current_category]:
                    category_data[current_category][current_mode] = []
                    
        # Check for run lines
        elif line.startswith("run"):
            # If we have collected data from previous run, process it
            if current_run_data and current_category and current_mode:
                # Sum all values for this run and add to category data
                run_total = sum(current_run_data)
                category_data[current_category][current_mode].append(run_total)
                current_run_data = []
            elif current_run_data and current_category == "Sequential":
                # For sequential, mode is always "avg"
                run_total = sum(current_run_data)
                category_data[current_category]["avg"].append(run_total)
                current_run_data = []
                
        # Check for numeric values (timing data)
        elif re.match(r'^\d+\.\d+', line):
            # Add timing value to current run data
            try:
                current_run_data.append(float(line))
            except ValueError:
                continue
    
    # Process any remaining run data after loop ends
    if current_run_data and current_category:
        if current_category == "Sequential":
            run_total = sum(current_run_data)
            category_data[current_category]["avg"].append(run_total)
        elif current_mode:
            run_total = sum(current_run_data)
            category_data[current_category][current_mode].append(run_total)
    
    # Calculate averages from the collected data
    for category, modes in category_data.items():
        averages[category] = {}
        for mode, run_times in modes.items():
            if run_times:  # Only calculate if we have data
                avg_value = sum(run_times) / len(run_times)
                averages[category][mode] = avg_value
    
    return averages

def create_plot(averages):
    if not averages:
        print("No data to plot")
        return
    
    # Check if we have the required categories
    if "Sequential" not in averages:
        print("Error: Sequential data missing")
        return
        
    sequential_avg = averages["Sequential"]["avg"]
    
    labels = []
    values = []
    colors = []
    
    labels.append("Sequential")
    values.append(sequential_avg)
    colors.append('red')
    
    # Add Parallel data if available
    if "Parallel" in averages:
        for mode, value in averages["Parallel"].items():
            labels.append(f"Parallel\n({mode})")
            values.append(value)
            colors.append('blue')
    
    # Add Parallel + Queue data if available
    if "Parallel + Queue" in averages:
        for mode, value in averages["Parallel + Queue"].items():
            labels.append(f"Parallel+Queue\n({mode})")
            values.append(value)
            colors.append('green')
    
    plt.figure(figsize=(12, 6))
    bars = plt.bar(range(len(labels)), values, color=colors, alpha=0.7)
    
    plt.title('Average Execution Times')
    plt.ylabel('Time (seconds)')
    plt.xticks(range(len(labels)), labels, rotation=45, ha='right')
    
    for i, bar in enumerate(bars):
        height = bar.get_height()
        plt.text(bar.get_x() + bar.get_width() / 2., height + 0.001, 
                f'{height:.4f}s', ha='center', va='bottom', fontsize=9)
    
    plt.tight_layout()
    plt.savefig('benchmark_results.png', dpi=300, bbox_inches='tight')
    plt.show()


# Example usage with the new format:

OUTPUT_FILE = "bench_res.txt"
with open(OUTPUT_FILE, 'r') as f:
    output_text = f.read()

averages = extract_averages(output_text)
print(averages)
create_plot(averages)