import os

# Define file extensions considered as code files
CODE_EXTENSIONS = {'.py', '.js', '.cpp', '.java', '.c', '.h', '.html', '.css', '.ts', '.ino'}

def count_lines_in_file(file_path):
    """Count the number of lines in a single file."""
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as file:
            return sum(1 for _ in file)
    except Exception as e:
        print(f"Could not read file {file_path}: {e}")
        return 0

def count_lines_in_folder(folder_path):
    """Recursively count lines of code in a folder."""
    total_lines = 0
    for root, dirs, files in os.walk(folder_path):
        for file in files:
            if any(file.endswith(ext) for ext in CODE_EXTENSIONS):
                file_path = os.path.join(root, file)
                lines = count_lines_in_file(file_path)
                print(f"{file_path}: {lines} lines")
                total_lines += lines
    return total_lines

if __name__ == '__main__':
    folder = input("Enter the folder path: ")
    if os.path.isdir(folder):
        total = count_lines_in_folder(folder)
        print(f"\nTotal number of lines of code: {total}")
    else:
        print("Invalid folder path. Please provide a valid directory.")
