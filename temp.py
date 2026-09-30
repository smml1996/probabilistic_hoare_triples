# from os import path
#
# folder = path.join("parsed_results", "basic_zero_plus_discr", "raw_algorithms")
#
# values = {
#     9: 1,
#     8: 0,
#     12: 0,
#     13: 1
# }
#
# for algorithm_index in range(0, 284):
#     file_path = path.join(folder, "A_" + str(algorithm_index) + ".txt")
#     f = open(file_path)
#     text = f.read()
#     f.close()
#     for (old, new) in values.items():
#         text = text.replace(f'c = {old}', f'c = {new}')
#
#
#     f = open(file_path, "w")
#     f.write(text)
#     f.close()
#
# for algorithm_index in range(0, 284):
#     file_path = path.join(folder, "R_" + str(algorithm_index) + ".txt")
#     f = open(file_path)
#     text = f.read()
#     f.close()
#     for (old, new) in values.items():
#         text = text.replace(f'"classical_state": "{old}"', f'"classical_state": "{new}"')
#
#     f = open(file_path, "w")
#     f.write(text)
#     f.close()

# ---------

from os import path

folder = path.join("parsed_results", "bell_state_reach", "raw_algorithms")

values = {
    1: 4,
}

for algorithm_index in range(0, 19):
    file_path = path.join(folder, "A_" + str(algorithm_index) + ".txt")
    f = open(file_path)
    text = f.read()
    f.close()
    for (old, new) in values.items():
        text = text.replace(f'c = {old}', f'c = {new}')


    f = open(file_path, "w")
    f.write(text)
    f.close()

for algorithm_index in range(0, 19):
    file_path = path.join(folder, "R_" + str(algorithm_index) + ".txt")
    f = open(file_path)
    text = f.read()
    f.close()
    for (old, new) in values.items():
        text = text.replace(f'"classical_state": "{old}"', f'"classical_state": "{new}"')

    f = open(file_path, "w")
    f.write(text)
    f.close()




