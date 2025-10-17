import matplotlib.pyplot as plt
import re
import numpy as np
from scipy import stats
import pandas as pd

def extract_averages(output_text):
    averages = {}
    current_category = None
    current_mode = None
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
            current_run_data = []
            if current_category not in raw_data:
                raw_data[current_category] = {}
            if current_mode not in raw_data[current_category]:
                raw_data[current_category][current_mode] = []
                
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
                if current_category not in raw_data:
                    raw_data[current_category] = {}
                if current_mode not in raw_data[current_category]:
                    raw_data[current_category][current_mode] = []
                    
        # Check for run lines
        elif line.startswith("run"):
            # If we have collected data from previous run, process it
            if current_run_data and current_category and current_mode:
                # Sum all values for this run and add to category data
                run_total = sum(current_run_data)
                raw_data[current_category][current_mode].append(run_total)
                current_run_data = []
            elif current_run_data and current_category == "Sequential":
                # For sequential, mode is always "avg"
                run_total = sum(current_run_data)
                raw_data[current_category]["avg"].append(run_total)
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
            raw_data[current_category]["avg"].append(run_total)
        elif current_mode:
            run_total = sum(current_run_data)
            raw_data[current_category][current_mode].append(run_total)
    
    # Calculate averages from the collected data
    for category, modes in raw_data.items():
        averages[category] = {}
        for mode, run_times in modes.items():
            if run_times:  # Only calculate if we have data
                avg_value = sum(run_times) / len(run_times)
                averages[category][mode] = avg_value
    
    return averages, raw_data

def calculate_statistics(data_dict):
    """Calculate comprehensive statistics for each category and mode"""
    stats_results = {}
    
    for category, modes in data_dict.items():
        stats_results[category] = {}
        
        for mode, values in modes.items():
            if len(values) < 3:  # Need at least 3 samples for meaningful stats
                print(f"Warning: Not enough data for {category} - {mode} (n={len(values)})")
                continue
                
            data_array = np.array(values)
            
            # Basic statistics
            mean_val = np.mean(data_array)
            std_val = np.std(data_array, ddof=1)  # Sample standard deviation
            
            # Normality tests
            try:
                shapiro_stat, shapiro_p = stats.shapiro(data_array)
                normaltest_stat, normaltest_p = stats.normaltest(data_array)
            except Exception as e:
                print(f"Error in normality tests for {category} - {mode}: {e}")
                shapiro_stat, shapiro_p, normaltest_stat, normaltest_p = np.nan, np.nan, np.nan, np.nan
            
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
            
            stats_results[category][mode] = {
                'n': n,
                'mean': mean_val,
                'std': std_val,
                'shapiro_p': shapiro_p,
                'normaltest_p': normaltest_p,
                'ci_lower': ci_lower,
                'ci_upper': ci_upper,
                'ci_range': ci_range,
                'data': data_array
            }
    
    return stats_results

def save_statistics_to_file(stats_results, filename='statistical_analysis.txt'):
    """Save comprehensive statistical analysis to file"""
    
    # Словарь для перевода режимов
    mode_translations = {
        'rowwise': 'по рядам',
        'columnwise': 'по столбцам', 
        'pilewise': 'по плиткам',
        'pixelwise': 'попиксельно',
        'avg': 'среднее'
    }
    
    with open(filename, 'w', encoding='utf-8') as f:
        f.write("СТАТИСТИЧЕСКИЙ АНАЛИЗ РЕЗУЛЬТАТОВ ТЕСТИРОВАНИЯ\n")
        f.write("=" * 60 + "\n\n")
        
        for category, modes in stats_results.items():
            # Перевод названий категорий
            category_translation = {
                'Sequential': 'Последовательная',
                'Parallel': 'Параллельная', 
                'Parallel + Queue': 'Параллельная + Очереди'
            }.get(category, category)
            
            f.write(f"КАТЕГОРИЯ: {category_translation}\n")
            f.write("-" * 40 + "\n")
            
            for mode, stats_data in modes.items():
                translated_mode = mode_translations.get(mode, mode)
                f.write(f"\nРежим: {translated_mode}\n")
                f.write(f"  Количество измерений (n): {stats_data['n']}\n")
                f.write(f"  Среднее значение: {stats_data['mean']:.6f} сек\n")
                f.write(f"  Стандартное отклонение: {stats_data['std']:.6f} сек\n")
                f.write(f"  95% доверительный интервал: [{stats_data['ci_lower']:.6f}, {stats_data['ci_upper']:.6f}] сек\n")
                f.write(f"  Ширина доверительного интервала: {stats_data['ci_range']:.6f} сек\n")
                
                # Normality test interpretation
                f.write(f"  Тест Шапиро-Уилка (нормальность): p-value = {stats_data['shapiro_p']:.4f}")
                if stats_data['shapiro_p'] > 0.05:
                    f.write(" ✓ Нормальное распределение (p > 0.05)\n")
                else:
                    f.write(" ✗ Отклонение от нормальности (p ≤ 0.05)\n")
                
                f.write(f"  Тест нормальности Д'Агостино: p-value = {stats_data['normaltest_p']:.4f}")
                if stats_data['normaltest_p'] > 0.05:
                    f.write(" ✓ Нормальное распределение (p > 0.05)\n")
                else:
                    f.write(" ✗ Отклонение от нормальности (p ≤ 0.05)\n")
                
                # Individual data points
                f.write(f"  Отдельные измерения: {[f'{x:.6f}' for x in stats_data['data']]}\n")
            
            f.write("\n" + "=" * 60 + "\n\n")
        
        # Summary table
        f.write("СВОДНАЯ ТАБЛИЦА РЕЗУЛЬТАТОВ\n")
        f.write("-" * 80 + "\n")
        f.write(f"{'Категория':<25} {'Режим':<15} {'Среднее':<12} {'Std':<12} {'CI 95%':<20} {'Нормальность'}\n")
        f.write("-" * 80 + "\n")
        
        for category, modes in stats_results.items():
            category_translation = {
                'Sequential': 'Последовательная',
                'Parallel': 'Параллельная', 
                'Parallel + Queue': 'Паралл. + Очереди'
            }.get(category, category)
            
            for mode, stats_data in modes.items():
                translated_mode = mode_translations.get(mode, mode)
                ci_str = f"[{stats_data['ci_lower']:.4f}-{stats_data['ci_upper']:.4f}]"
                normality = "✓" if stats_data['shapiro_p'] > 0.05 or stats_data['normaltest_p'] > 0.05 else "✗"
                f.write(f"{category_translation:<25} {translated_mode:<15} {stats_data['mean']:.4f} {stats_data['std']:.4f} {ci_str:<20} {normality}\n")
    
    print(f"Статистический анализ сохранен в файл: {filename}")

def create_plot(averages, stats_results=None):
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
    errors = []  # For error bars
    
    # Словарь для перевода режимов
    mode_translations = {
        'rowwise': 'по рядам',
        'columnwise': 'по столбцам', 
        'pilewise': 'по плиткам',
        'pixelwise': 'попиксельно',
        'avg': 'среднее'
    }
    
    # Русские подписи для категорий
    labels.append("Послед.")
    values.append(sequential_avg)
    colors.append('red')
    
    # Add error bar if statistics available
    if stats_results and "Sequential" in stats_results and "avg" in stats_results["Sequential"]:
        errors.append(stats_results["Sequential"]["avg"]["std"])
    else:
        errors.append(0)
    
    # Add Parallel data if available
    if "Parallel" in averages:
        for mode, value in averages["Parallel"].items():
            translated_mode = mode_translations.get(mode, mode)
            labels.append(f"Паралл.\n({translated_mode})")
            values.append(value)
            colors.append('blue')
            
            # Add error bar
            if stats_results and "Parallel" in stats_results and mode in stats_results["Parallel"]:
                errors.append(stats_results["Parallel"][mode]["std"])
            else:
                errors.append(0)
    
    # Add Parallel + Queue data if available
    if "Parallel + Queue" in averages:
        for mode, value in averages["Parallel + Queue"].items():
            translated_mode = mode_translations.get(mode, mode)
            labels.append(f"Паралл. + Очереди\n({translated_mode})")
            values.append(value)
            colors.append('green')
            
            # Add error bar
            if stats_results and "Parallel + Queue" in stats_results and mode in stats_results["Parallel + Queue"]:
                errors.append(stats_results["Parallel + Queue"][mode]["std"])
            else:
                errors.append(0)
    
    plt.figure(figsize=(14, 8))
    bars = plt.bar(range(len(labels)), values, color=colors, alpha=0.7, 
                   yerr=errors, capsize=5, error_kw={'elinewidth': 2, 'capthick': 2})
    
    # Русские подписи для графика
    plt.title('Среднее время исполнения для разных реализаций свёртки', fontsize=14, fontweight='bold')
    plt.ylabel('Время (секунды)', fontsize=12)
    plt.xticks(range(len(labels)), labels, rotation=45, ha='right', fontsize=10)
    
    # Add value labels on bars
    for i, bar in enumerate(bars):
        height = bar.get_height()
        plt.text(bar.get_x() + bar.get_width() / 2., height + errors[i] + 0.001, 
                f'{height:.4f}с\n±{errors[i]:.4f}', 
                ha='center', va='bottom', fontsize=8)
    
    plt.grid(axis='y', alpha=0.3, linestyle='--')
    plt.tight_layout()
    plt.savefig('benchmark_results_with_stats.png', dpi=300, bbox_inches='tight')
    plt.show()

# Example usage with the new format:
OUTPUT_FILE = "bench_res.txt"

try:
    with open(OUTPUT_FILE, 'r', encoding='utf-8') as f:
        output_text = f.read()

    averages, raw_data = extract_averages(output_text)
    print("Извлеченные средние значения:")
    print(averages)
    
    # Calculate comprehensive statistics
    stats_results = calculate_statistics(raw_data)
    
    # Save statistics to file
    save_statistics_to_file(stats_results)
    
    # Create enhanced plot with error bars
    create_plot(averages, stats_results)
    
    # Print quick summary to console
    print("\nКРАТКАЯ СВОДКА:")
    print("-" * 50)
    
    # Словари для перевода
    category_translations = {
        'Sequential': 'Последовательная',
        'Parallel': 'Параллельная', 
        'Parallel + Queue': 'Параллельная + Очереди'
    }
    mode_translations = {
        'rowwise': 'по рядам',
        'columnwise': 'по столбцам', 
        'pilewise': 'по плиткам',
        'pixelwise': 'попиксельно',
        'avg': 'среднее'
    }

    for category, modes in stats_results.items():
        for mode, stats_data in modes.items():
            translated_category = category_translations.get(category, category)
            translated_mode = mode_translations.get(mode, mode)
            normality = "НОРМАЛЬНО" if stats_data['shapiro_p'] > 0.05 and stats_data['normaltest_p'] > 0.05 else "НЕНОРМАЛЬНО"
            print(f"{translated_category} - {translated_mode}: {stats_data['mean']:.4f} ± {stats_data['std']:.4f} сек ({normality})")

except FileNotFoundError:
    print(f"Ошибка: Файл {OUTPUT_FILE} не найден")
except Exception as e:
    print(f"Ошибка при обработке данных: {e}")