import matplotlib.pyplot as plt

def extract_averages(output_text):
    averages = {}
    current_category = None
    current_mode = None
    
    lines = output_text.strip().split('\n')
    
    for line in lines:
        line = line.strip()
        
        if line == "Sequential":
            current_category = "Sequential"
            current_mode = None
        elif line == "Parallel":
            current_category = "Parallel"
        elif line == "Parallel + Queue":
            current_category = "Parallel + Queue"
        elif line.startswith("Mode:"):
            current_mode = line.split(":")[1].strip()
        elif line.startswith("avg:"):
            avg_value = float(line.split(":")[1].strip().replace("s", ""))
            
            if current_category not in averages:
                averages[current_category] = {}
            
            if current_mode:
                averages[current_category][current_mode] = avg_value
            else:
                averages[current_category]["avg"] = avg_value
    
    return averages

def create_plot(averages):
    sequential_avg = averages["Sequential"]["avg"]
    
    labels = []
    values = []
    colors = []
    
    labels.append("Sequential")
    values.append(sequential_avg)
    colors.append('red')
    
    for mode, value in averages["Parallel"].items():
        labels.append(f"Parallel\n({mode})")
        values.append(value)
        colors.append('blue')
    
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

output_text = """
small image
Sequential
0.2220 0.2108 0.2113 0.2127 0.2148 0.2153 0.2131 0.2128 0.2205 0.2148 0.2144 0.2140 0.2182 0.2178 0.2133 0.2162 0.2135 0.2177 0.2138 0.2153 0.2138 0.2139 0.2150 0.2140 0.2142 0.2146 0.2155 0.2140 0.2153 0.2150 0.2143 0.2148 0.2146 0.2149 0.2147 0.2138 0.2164 0.2141 0.2143 0.2144 0.2145 0.2141 0.2152 0.2161 0.2148 0.2175 0.2143 0.2193 0.2146 0.2138 0.2149 0.2139 0.2143 0.2139 0.2172 0.2149 0.2151 0.2151 0.2143 0.2139 0.2145 0.2144 0.2143 0.2161 0.2190 0.2174 0.2181 0.2175 0.2137 0.2136 0.2138 0.2139 0.2152 0.2183 0.2162 0.2150 0.2151 0.2141 0.2145 0.2153 
avg: 0.8603 s


Parallel
Mode: pilewise
0.0689 0.0625 0.0677 0.1260 0.0682 0.0689 0.0680 0.0721 0.0692 0.0683 0.0690 0.0650 0.0641 0.0712 0.0657 0.0690 0.0844 0.0674 0.0666 0.0643 0.0642 0.0598 0.0636 0.1136 0.0639 0.0640 0.0641 0.0643 0.0643 0.0658 0.0640 0.0589 0.0588 0.0634 0.0644 0.0593 0.0639 0.0670 0.0630 0.0642 0.0637 0.0590 0.0634 0.1141 0.0639 0.0653 0.0634 0.0644 0.0634 0.0639 0.0649 0.0625 0.0587 0.0651 0.1210 0.0626 0.0662 0.0664 0.0664 0.0667 0.0668 0.0624 0.0663 0.1210 0.0663 0.0663 0.0692 0.0669 0.0669 0.0664 0.0670 0.0620 0.0624 0.0668 0.0670 0.0619 0.0658 0.0668 0.0925 0.1065 
avg: 0.2782 s
Mode: pixelwise
0.1110 0.0976 0.1051 0.1895 0.1051 0.1053 0.1046 0.0737 0.0648 0.0687 0.0660 0.0624 0.0612 0.0656 0.0622 0.0658 0.0671 0.0664 0.0662 0.0671 0.0659 0.0619 0.0668 0.1202 0.0680 0.0687 0.0704 0.0706 0.0715 0.0714 0.0716 0.0667 0.0675 0.0703 0.0715 0.0701 0.0717 0.0709 0.0724 0.0705 0.0715 0.0663 0.0715 0.1314 0.0677 0.0661 0.0664 0.0669 0.0665 0.0663 0.0664 0.0622 0.0627 0.0675 0.1268 0.0655 0.0661 0.0665 0.0669 0.0668 0.0638 0.0593 0.0663 0.1170 0.0641 0.0665 0.0677 0.0656 0.0645 0.0640 0.0640 0.0612 0.0596 0.0641 0.0648 0.0650 0.0645 0.0658 0.0640 0.0649 
avg: 0.2954 s
Mode: rowwise
0.0697 0.0605 0.0641 0.1172 0.0639 0.0642 0.0662 0.0699 0.0697 0.0702 0.0698 0.0659 0.0655 0.0697 0.0700 0.1311 0.0698 0.0699 0.0698 0.0708 0.0697 0.0658 0.0694 0.1297 0.0698 0.0701 0.0724 0.0682 0.0682 0.0682 0.0681 0.0643 0.0640 0.0706 0.0706 0.1330 0.0715 0.0720 0.0725 0.0718 0.0730 0.0680 0.0730 0.1331 0.0715 0.0719 0.0723 0.0715 0.0730 0.0717 0.0726 0.0672 0.0681 0.0717 0.1330 0.1338 0.0728 0.0715 0.0729 0.0722 0.0730 0.0678 0.0726 0.1341 0.0716 0.0722 0.0746 0.0881 0.0712 0.0715 0.0732 0.0673 0.0682 0.0712 0.0725 0.1335 0.0713 0.0720 0.0724 0.0714 
avg: 0.3081 s


Parallel + Queue
Mode: pilewise
0.1903 0.1910 0.1926 0.1770 0.1884 0.1860 0.1836 0.2230 0.1748 0.1970 0.1903 0.1847 0.1955 0.1815 0.1887 0.1805 0.1959 0.1969 0.2016 0.1869 
avg: 0.1903 s
Mode: pixelwise
0.1884 0.1790 0.1808 0.2103 0.1787 0.1786 0.1752 0.1806 0.1938 0.1772 0.1783 0.1798 0.1742 0.1881 0.1706 0.1842 0.1892 0.1778 0.1811 0.1685 
avg: 0.1817 s
Mode: rowwise
0.1932 0.1999 0.1852 0.1909 0.1887 0.1819 0.2077 0.1899 0.1949 0.1888 0.2121 0.2020 0.1837 0.1911 0.1908 0.1777 0.1943 0.1914 0.2004 0.1906 
avg: 0.1928 s
"""

averages = extract_averages(output_text)
create_plot(averages)