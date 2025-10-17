import re
import numpy as np
from scipy import stats
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors

def extract_averages(output_text):
    averages = {}
    current_category = None
    current_mode = None
    current_nthreads = None
    current_run_data = []  # Store all timing values for current run
    
    lines = output_text.strip().split('\n')
    
    # Initialize data structure for raw data
    raw_data = {}
    
    for line in lines:
        line = line.strip()
        
        # Skip empty lines
        if not line:
            continue
            
        # Check for category headers
        if line == "Sequential":
            current_category = "Sequential"
            current_mode = "avg"  # Sequential doesn't have modes
            current_nthreads = "1"  # Sequential is single-threaded
            current_run_data = []
            if current_category not in raw_data:
                raw_data[current_category] = {}
            if current_mode not in raw_data[current_category]:
                raw_data[current_category][current_mode] = {}
            if current_nthreads not in raw_data[current_category][current_mode]:
                raw_data[current_category][current_mode][current_nthreads] = []
                
        elif line == "Parallel":
            current_category = "Parallel"
            current_mode = None
            current_nthreads = None
            current_run_data = []
            
        elif line == "Parallel + Queue":
            current_category = "Parallel + Queue"
            current_mode = None
            current_nthreads = None
            current_run_data = []
            
        # Check for mode lines
        elif line.startswith("Mode:"):
            if current_category:
                current_mode = line.split(":")[1].strip()
                current_nthreads = None
                current_run_data = []
                if current_category not in raw_data:
                    raw_data[current_category] = {}
                if current_mode not in raw_data[current_category]:
                    raw_data[current_category][current_mode] = {}
                    
        # Check for nthreads lines
        elif line.startswith("nthreads:"):
            if current_category and current_mode:
                # If we have collected data from previous nthreads, process it
                if current_run_data and current_nthreads:
                    run_total = sum(current_run_data)
                    if current_nthreads not in raw_data[current_category][current_mode]:
                        raw_data[current_category][current_mode][current_nthreads] = []
                    raw_data[current_category][current_mode][current_nthreads].append(run_total)
                    current_run_data = []
                
                current_nthreads = line.split(":")[1].strip()
                if current_nthreads not in raw_data[current_category][current_mode]:
                    raw_data[current_category][current_mode][current_nthreads] = []
                    
        # Check for run lines
        elif line.startswith("run"):
            # If we have collected data from previous run, process it
            if current_run_data and current_category and current_mode and current_nthreads:
                run_total = sum(current_run_data)
                raw_data[current_category][current_mode][current_nthreads].append(run_total)
                current_run_data = []
            elif current_run_data and current_category == "Sequential":
                run_total = sum(current_run_data)
                raw_data[current_category]["avg"]["1"].append(run_total)
                current_run_data = []
                
        # Check for numeric values (timing data)
        elif re.match(r'^\d+\.\d+', line):
            # Add timing value to current run data
            try:
                current_run_data.append(float(line) / 1000)
            except ValueError:
                continue
    
    # Process any remaining run data after loop ends
    if current_run_data and current_category:
        if current_category == "Sequential":
            run_total = sum(current_run_data)
            raw_data[current_category]["avg"]["1"].append(run_total)
        elif current_mode and current_nthreads:
            run_total = sum(current_run_data)
            raw_data[current_category][current_mode][current_nthreads].append(run_total)
    
    # Calculate averages from the collected data
    for category, modes in raw_data.items():
        averages[category] = {}
        for mode, nthreads_data in modes.items():
            averages[category][mode] = {}
            for nthreads, run_times in nthreads_data.items():
                if run_times:  # Only calculate if we have data
                    avg_value = sum(run_times) / len(run_times)
                    averages[category][mode][nthreads] = avg_value
    
    return averages, raw_data

def calculate_statistics(data_dict):
    """Calculate comprehensive statistics for each category, mode and nthreads"""
    stats_results = {}
    
    for category, modes in data_dict.items():
        stats_results[category] = {}
        
        for mode, nthreads_data in modes.items():
            stats_results[category][mode] = {}
            
            for nthreads, values in nthreads_data.items():
                if len(values) < 3:  # Need at least 3 samples for meaningful stats
                    print(f"Warning: Not enough data for {category} - {mode} - nthreads:{nthreads} (n={len(values)})")
                    continue
                    
                data_array = np.array(values)
                
                # Basic statistics
                mean_val = np.mean(data_array)
                std_val = np.std(data_array, ddof=1)  # Sample standard deviation
                
                # 95% confidence interval
                n = len(data_array)
                if n >= 2:
                    sem = std_val / np.sqrt(n)  # Standard error of the mean
                    t_value = stats.t.ppf(0.975, n-1)  # 95% confidence t-value
                    ci_lower = mean_val - t_value * sem
                    ci_upper = mean_val + t_value * sem
                    ci_range = ci_upper - ci_lower
                else:
                    ci_lower, ci_upper, ci_range = np.nan, np.nan, np.nan
                
                stats_results[category][mode][nthreads] = {
                    'n': n,
                    'mean': mean_val,
                    'std': std_val,
                    'ci_lower': ci_lower,
                    'ci_upper': ci_upper,
                    'ci_range': ci_range,
                    'data': data_array
                }
    
    return stats_results

def create_plot(averages, stats_results=None):
    if not averages:
        print("No data to plot")
        return
    
    if "Sequential" in averages:
        sequential_avg = averages["Sequential"]["avg"]["1"]
    
    labels = []
    values = []
    colors = []
    errors = []  # For error bars
    
    # Оттенки синего для Parallel
    blue_shades = {
        'rowwise': '#87CEEB',    # Голубой (LightSkyBlue)
        'columnwise': '#1E90FF',  # Лазурный (DodgerBlue)
        'pilewise': '#4169E1',   # Королевский синий (RoyalBlue)
        'pixelwise': '#000080'   # Темно-синий (Navy)
    }
    
    # Оттенки зеленого для Parallel + Queue
    green_shades = {
        'rowwise': '#98FB98',    # Бледно-зеленый (PaleGreen)
        'columnwise': '#32CD32',  # Лаймовый (LimeGreen)
        'pilewise': '#228B22',   # Лесной зеленый (ForestGreen)
        'pixelwise': '#006400'   # Темно-зеленый (DarkGreen)
    }
    
    # Mode translations
    mode_translations = {
        'rowwise': 'по рядам',
        'columnwise': 'по столбцам', 
        'pilewise': 'по плиткам',
        'pixelwise': 'попиксельно',
        'avg': 'среднее'
    }
    
    def get_color_for_mode(category, mode):
        """Get color with appropriate shade for mode within category"""
        if category == "Sequential":
            return '#FF6B6B'  # Красный для Sequential
        
        if category == "Parallel":
            return blue_shades.get(mode, '#1E90FF')  # Лазурный по умолчанию
        
        if category == "Parallel + Queue":
            return green_shades.get(mode, '#32CD32')  # Лаймовый по умолчанию
        
        return 'gray'
    
    # Собираем все уникальные количества потоков
    all_nthreads = set()
    
    # Добавляем Sequential (всегда 1 поток)
    all_nthreads.add("1")
    
    # Собираем nthreads из Parallel
    if "Parallel" in averages:
        for mode, nthreads_data in averages["Parallel"].items():
            all_nthreads.update(nthreads_data.keys())
    
    # Собираем nthreads из Parallel + Queue
    if "Parallel + Queue" in averages:
        for mode, nthreads_data in averages["Parallel + Queue"].items():
            all_nthreads.update(nthreads_data.keys())
    
    # Сортируем nthreads как числа, если возможно
    def sort_nthreads(nthreads_list):
        sorted_list = []
        for nthreads in nthreads_list:
            try:
                sorted_list.append((int(nthreads), nthreads))
            except ValueError:
                sorted_list.append((float('inf'), nthreads))  # Нечисловые значения в конец
        return [nthreads for _, nthreads in sorted(sorted_list)]
    
    sorted_nthreads = sort_nthreads(list(all_nthreads))
    
    # Добавляем данные, сгруппированные по количеству потоков
    for nthreads in sorted_nthreads:
        # Добавляем Sequential только для nthreads=1
        if nthreads == "1":
            labels.append("")
            values.append(sequential_avg)
            colors.append(get_color_for_mode("Sequential", "avg"))
            
            # Add error bar if statistics available
            if stats_results and "Sequential" in stats_results and "avg" in stats_results["Sequential"] and "1" in stats_results["Sequential"]["avg"]:
                errors.append(stats_results["Sequential"]["avg"]["1"]["std"])
            else:
                errors.append(0)
        
        # Добавляем Parallel данные для текущего количества потоков
        if "Parallel" in averages:
            for mode, nthreads_data in averages["Parallel"].items():
                if nthreads in nthreads_data:
                    translated_mode = mode_translations.get(mode, mode)
                    labels.append(f"")
                    values.append(nthreads_data[nthreads])
                    colors.append(get_color_for_mode("Parallel", mode))
                    
                    # Add error bar
                    if (stats_results and "Parallel" in stats_results and 
                        mode in stats_results["Parallel"] and 
                        nthreads in stats_results["Parallel"][mode]):
                        errors.append(stats_results["Parallel"][mode][nthreads]["std"])
                    else:
                        errors.append(0)
        
        # Добавляем Parallel + Queue данные для текущего количества потоков
        if "Parallel + Queue" in averages:
            for mode, nthreads_data in averages["Parallel + Queue"].items():
                if nthreads in nthreads_data:
                    translated_mode = mode_translations.get(mode, mode)
                    labels.append(f"Паралл. + Очереди\n({translated_mode})")
                    values.append(nthreads_data[nthreads])
                    colors.append(get_color_for_mode("Parallel + Queue", mode))
                    
                    # Add error bar
                    if (stats_results and "Parallel + Queue" in stats_results and 
                        mode in stats_results["Parallel + Queue"] and 
                        nthreads in stats_results["Parallel + Queue"][mode]):
                        errors.append(stats_results["Parallel + Queue"][mode][nthreads]["std"])
                    else:
                        errors.append(0)
    
    plt.figure(figsize=(18, 10))
    bars = plt.bar(range(len(labels)), values, color=colors, alpha=0.9, 
                   yerr=errors, capsize=5, error_kw={'elinewidth': 2, 'capthick': 2})
    

    titlename = 'Среднее время исполнения для разных режимов свёртки на изображении Monet_Parasol (6001x7455 px)'
    # Russian labels for the plot
    plt.title(titlename, fontsize=14, fontweight='bold')
    plt.ylabel('Время (с)', fontsize=12)
    plt.xticks(range(len(labels)), labels, rotation=45, ha='right', fontsize=9)
    
    # Добавляем разделители между группами потоков
    current_pos = 0
    for nthreads in sorted_nthreads:
        group_size = 0
        
        # Считаем размер группы для текущего nthreads
        if nthreads == "1":
            group_size += 1  # Sequential
        
        if "Parallel" in averages:
            for mode in averages["Parallel"]:
                if nthreads in averages["Parallel"][mode]:
                    group_size += 1
        
        if "Parallel + Queue" in averages:
            for mode in averages["Parallel + Queue"]:
                if nthreads in averages["Parallel + Queue"][mode]:
                    group_size += 1
        
        # Добавляем вертикальную линию после группы
        if current_pos + group_size < len(labels) and len(sorted_nthreads) > 1:
            plt.axvline(x=current_pos + group_size - 0.5, color='gray', linestyle=':', alpha=0.5)

        # Подписываем группу
        plt.text(current_pos - 0.3, plt.ylim()[1] * 0.95, 
                f'потоки: {nthreads}', 
                ha='left', va='baseline', fontsize=10, 
                bbox=dict(boxstyle="round,pad=0.3", facecolor='white', alpha=0.8))
        current_pos += group_size
    
    # Add value labels on bars
    for i, bar in enumerate(bars):
        height = bar.get_height()
        plt.text(bar.get_x() + bar.get_width() / 2., height + errors[i] + 0.001, 
                f'{height:.2f}\n±{errors[i]:.2f}', 
                ha='center', va='bottom', fontsize=8)
    
    # Create custom legend
    from matplotlib.patches import Patch
    
    # Category legend
    red_mode_legend = [
        Patch(facecolor='#FF6B6B', label='Последовательная'),
    ]
    
    # Mode legend for blue shades (Parallel)
    blue_mode_legend = [
        Patch(facecolor='#87CEEB', label='по рядам'),
        Patch(facecolor='#1E90FF', label='по столбцам'),
        Patch(facecolor='#4169E1', label='по плиткам'),
        Patch(facecolor='#000080', label='попиксельно')
    ]
    
    # Mode legend for green shades (Parallel + Queue)
    green_mode_legend = [
        Patch(facecolor='#98FB98', label='по рядам'),
        Patch(facecolor='#32CD32', label='по столбцам'),
        Patch(facecolor='#228B22', label='по плиткам'),
        Patch(facecolor='#006400', label='попиксельно')
    ]
    
    # # Create legends
    # legend1 = plt.legend(handles=category_legend, loc='best', title='Категории')
    # plt.gca().add_artist(legend1)
    
    # legend2 = plt.legend(handles=blue_mode_legend, loc='upper center', title='Режимы (Parallel)')
    # plt.gca().add_artist(legend2)
    
    # plt.legend(handles=green_mode_legend, loc='upper left', title='Режимы (Parallel+Queue)')
    
    # Create combined legend - all in one
    all_legend_elements = red_mode_legend + blue_mode_legend
    all_legend_labels = [
        'Последовательная', 
                        'Парал. (по рядам)', 'Парал. (по столбцам)', 'Парал. (по плиткам)', 'Парал. (попиксельная)',
                        # 'по рядам (P+Q)', 'по столбцам (P+Q)', 'по плиткам (P+Q)', 'попиксельно (P+Q)'
                        ]

    plt.legend(handles=all_legend_elements, labels=all_legend_labels, 
            loc='upper right', title='Вид свёртки', bbox_to_anchor=(1.0, 1.0),
            ncol=1, fontsize=9)  # ncol=1 for single column

    plt.grid(axis='y', alpha=0.3, linestyle='--')
    plt.tight_layout()
    
    # Save the plot
    GRAPH_FILE = "benchmark_results.png"
    plt.savefig(GRAPH_FILE, dpi=300, bbox_inches='tight')
    print(f"Result graph saved in {GRAPH_FILE}")
    plt.show()

# Example usage with the new format:
OUTPUT_FILE = "bench_res.txt"

try:
    with open(OUTPUT_FILE, 'r', encoding='utf-8') as f:
        output_text = f.read()

    averages, raw_data = extract_averages(output_text)
    
    # Print extracted data for verification
    print("Extracted averages:")
    for category, modes in averages.items():
        print(f"\n{category}:")
        for mode, nthreads_data in modes.items():
            print(f"  {mode}:")
            for nthreads, avg in nthreads_data.items():
                print(f"    nthreads {nthreads}: {avg:.6f}s")
    
    # Calculate comprehensive statistics
    stats_results = calculate_statistics(raw_data)
    
    # Create enhanced plot with error bars
    create_plot(averages, stats_results)

except FileNotFoundError:
    print(f"Ошибка: Файл {OUTPUT_FILE} не найден")
except Exception as e:
    print(f"Ошибка при обработке данных: {e}")
    import traceback
    traceback.print_exc()