import os
import shutil 

from tempfile import mkstemp
from shutil import move, copymode
from os import fdopen, remove

license_text = "// Copyright (C) NOMAD Group <nomad-group.net>."
new_license_text = "// Copyright (C) Force67 <github.com/Force67>."

def replace(file_path, pattern, subst):
    #Create temp file
    fh, abs_path = mkstemp()
    with fdopen(fh,'w') as new_file:
        with open(file_path) as old_file:
            for line in old_file:
                new_file.write(line.replace(pattern, subst))
    #Copy the file permissions from the old file to the new file
    copymode(file_path, abs_path)
    #Remove original file
    remove(file_path)
    #Move new file
    move(abs_path, file_path)

dir = "C:\\Users\\vince\\Documents\\Development\\Force\\RETK\\code"
for root, dirs, files in os.walk(dir):
    for file in files:

        if file.find(".fbs") != -1:
            replace(root + "\\" + file, license_text, new_license_text)
