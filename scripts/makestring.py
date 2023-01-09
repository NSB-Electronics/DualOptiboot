import sys

# Open the input file in read mode
with open(sys.argv[1], "r") as input_file:
    # Read the contents of the file into a list of lines
    lines = input_file.readlines()

# Open the input file in write mode
with open(sys.argv[1], "w") as output_file:
    # Iterate through each line in the list
    for line in lines:
        # Surround the line with double quotations
        line = f'"{line.strip()}"'
        # Write the modified line to the output file
        output_file.write(line)
        # Add a newline character after each line
        output_file.write("\n")

print("Done!")