''' This script is used to separate the data dumped over BLE into CSV files
for training ML to identify the difference in impact patterns'''

import os
import tkinter
import tkinter.filedialog

output_dir = './testing/training_data/'
working_dir = os.getcwd()
def get_file(root: tkinter.Tk, prompt: str):
    global working_dir
    file = tkinter.filedialog.askopenfile(parent=root, initialdir=working_dir, title=prompt)
    if file:
        return file
    else:
        quit()

def remove_newlines(file_path):
    """Opens a file, removes all newline characters, and writes back the modified content."""
    with open(file_path, 'r', encoding='utf-8') as file:
        content = file.read().replace('\n', '')

    with open(file_path, 'w', encoding='utf-8') as file:
        file.write(content)

def parse_file(input_file, output_file_name):
    lines = input_file.readlines()
    impacts = []
    
    start_parsing = False
    parsed_data = []
    
    for line in lines:
        if start_parsing:
            for char in line:
                if char == '[':
                    pass
                elif char == '\n':
                    pass
                elif char == ']': # ending case
                        start_parsing = False
                        impacts.append(parsed_data)
                        parsed_data = []
                else:
                    parsed_data.append(char)

        elif "Impact Array:" in line:
            start_parsing = True
                    
    with open((output_dir+output_file_name), 'a', encoding='utf-8') as outfile:
        for impact in impacts:
            impact_string = ''.join(impact)
            impact_string += '\n'
            outfile.write(impact_string)

if __name__ == "__main__":

    root = tkinter.Tk()
    root.withdraw()

    prompt = "Select BLE dump file"
    print(prompt)
    in_file = get_file(root, prompt)
    print(in_file)

    out_file_name = input("Name output file: ")
    out_file_name += ".csv"
    print("Creating output file:", out_file_name)

    remove_newlines(in_file) # remove newlines to account for BLE chunks
    parse_file(in_file, out_file_name)