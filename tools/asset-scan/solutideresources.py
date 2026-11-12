from pathlib import Path
import shutil
import hashlib

# Global configurations
production_mod_dir = Path("./legacy/solitude-production/Solitude/")
solitude_dir = Path("./legacy/solitude-vita-backup/Solitude/")
quakespasm_dir = Path("./quakespasm/")
assets_dir = Path("./assets/maps/")
qcc_dir = Path("./qcc-src/")
not_found_files = []
not_found_file = Path("not_found_files.txt")
filetype_blacklist = []
filetype_blacklist_file = "filetype_blacklist.txt"
include_filetype_in_search = False
enable_file_copying = True

def calculate_sha(file_path):
    # Calculate SHA hash of the file
    sha_hash = hashlib.sha256()
    with open(file_path, 'rb') as file:
        while True:
            data = file.read(65536)  # Read in 64KB chunks
            if not data:
                break
            sha_hash.update(data)
    return sha_hash.hexdigest()

def search_files(directory, curr_filename, filetype_blacklist):
    found = False
    for filepath in directory.glob('**/*'):
        file_extension = filepath.suffix.lower()
        if filepath.is_file() and file_extension not in [item.lower() for item in filetype_blacklist]:
            try:
                with open(filepath, 'r', encoding='utf-8') as file:
                    content = file.read()
                    if curr_filename in content:
                        # print(f"Found '{curr_filename}' in '{filepath.relative_to(directory)}'")
                        found = True
            except Exception as e:
                print(f"Error reading file '{filepath}': {e}")
                if file_extension not in [item.lower() for item in filetype_blacklist]:
                    filetype_blacklist.append(file_extension)
    return found

# Load filetype blacklist
if Path(filetype_blacklist_file).is_file():
    with open(filetype_blacklist_file, 'r') as f:
        filetype_blacklist = [line.strip() for line in f]

print("starting...")
for filename in solitude_dir.glob('**/*'):
    if filename.is_file():
        if include_filetype_in_search:
            curr_filename = filename.name
        else:
            curr_filename = filename.stem
        # search engine directory
        if (
            #     not search_files(quakespasm_dir, curr_filename, filetype_blacklist)
            # # search qcc quake game logic directory
            # and not search_files(qcc_dir, curr_filename, filetype_blacklist)
            # # search final directory, in case files like maps reference other files like textures
            # and not search_files(production_mod_dir, curr_filename, filetype_blacklist)
            # ## search in maps etc assets_dir
            # and
                not search_files(assets_dir, curr_filename, filetype_blacklist)):

            not_found_files.append(filename.absolute())
            with open(not_found_file, 'a', encoding='utf-8') as not_found:
                not_found.write(str(filename.absolute()) + '\n')
        elif enable_file_copying:
            # Copy the file to production_mod_dir only if the SHA hashes do not match
            relative_path = filename.relative_to(solitude_dir)
            source_path = filename
            destination_path = production_mod_dir / relative_path
            destination_path.parent.mkdir(parents=True, exist_ok=True)

            if not destination_path.is_file() or calculate_sha(source_path) != calculate_sha(destination_path):
                print(f"Copying file '{curr_filename}' to {production_mod_dir}")
                shutil.copy(source_path, destination_path)

# Save filetype blacklist
with open(filetype_blacklist_file, 'w') as f:
    for file_extension in filetype_blacklist:
        f.write(file_extension + '\n')

print("Files where search string wasn't found:")
for file_path in not_found_files:
    print(file_path)

print("DONE.\n")
