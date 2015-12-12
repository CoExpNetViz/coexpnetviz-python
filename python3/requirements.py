"""
Perform some post-processing on a requirements.txt outputted by pip freeze
"""

project_name = 'deep-blue-genome'

with open('requirements.txt', 'r') as f:
    lines = []
    for line in f.readlines():
        if line.startswith(project_name):
            continue
        elif line.startswith('scipy') or line.startswith('numpy'):
            # Reordering due to unspecified dependencies
            # scikit-learn forgets to specify scipy as dependency
            # numexpr forgets to specify numpy
            lines.insert(0, line) 
        else: 
            lines.append(line)

with open('requirements.txt', 'w') as f:
    f.writelines(lines)
