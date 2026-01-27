def calculate_mean(file_path):
    total = 0
    count = 0

    with open(file_path, 'r') as file:
        for line in file:
            if line.strip():  # 忽略空行
                parts = line.split()
                if len(parts) == 3:
                    total += float(parts[2])
                    count += 1

    if count == 0:
        return 0

    return total / count

if __name__ == "__main__":
    file_path = "/home/wangyp/1ton/ReConstruction/preAnalysis/calibResult/PMT_Gain_List.txt"
    mean = calculate_mean(file_path)
    print(f"第三列的均值是: {455.9/mean}")