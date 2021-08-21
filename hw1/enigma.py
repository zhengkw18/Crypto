import itertools
import copy

from queue import Queue

rotor_alphabeta = ["EKMFLGDQVZNTOWYHXUSPAIBRCJ", "AJDKSIRUXBLHWTMCQGZNPYFVOE", "BDFHJLCPRTXVZNYEIWGAKMUSQO", "ESOVPZJAYQUIRHXLNFTGKDCMWB", "VZBRGITYUPSDNHLXAWMJQOFECK"]
rotor_notch_alphabeta = ["Q", "E", "V", "J", "Z"]
reflector_alphabeta = ["YRUHQSLDPXNGOKMIEBFZCWVJAT", "FVPJIAOYEDRZXWGCTKUQSBNMHL"]
rotor = []
rotor_notch = []
reflector = []
rotor_reverse = []
rotor_type_num = len(rotor_alphabeta)
reflector_type_num = len(reflector_alphabeta)
N = 26
# 若想节省测试时间，可将以下两行取消注释
# rotor_type_num = 3
# reflector_type_num = 1

results = []
current_mapping = []
guess_table = [i for i in range(N)]
swaped = [False for _ in range(N)]
rings = []

MAX_RING_LEN = 5
MAX_RING_NUM = 30

stack = [0 for _ in range(MAX_RING_LEN)]
current_p = ""
current_c = ""
current_enigmas = None
iterated = []
current_p_lst = []
current_c_lst = []
current_len = 0


def to_num(c):
    return ord(c) - ord("A")


def to_chr(i):
    return chr(ord("A") + i)


def to_lst(s):
    lst = []
    for c in s:
        lst.append(to_num(c))
    return lst if len(lst) > 1 else lst[0]


for i in range(rotor_type_num):
    rotor.append(to_lst(rotor_alphabeta[i]))
    rotor_notch.append(to_lst(rotor_notch_alphabeta[i]))

for i in range(reflector_type_num):
    reflector.append(to_lst(reflector_alphabeta[i]))


def inv(lst):
    lst_reverse = [0] * len(lst)
    for i in range(len(lst)):
        lst_reverse[lst[i]] = i
    return lst_reverse


for i in range(rotor_type_num):
    rotor_reverse.append(inv(rotor[i]))


class Rotor:
    def __init__(self, type, pos):
        self.rotor = rotor[type]
        self.rotor_notch = rotor_notch[type]
        self.rotor_reverse = rotor_reverse[type]
        self.pos = pos

    def forward(self, i):
        return (self.rotor[(i + self.pos) % N] - self.pos + N) % N

    def reverse(self, i):
        return (self.rotor_reverse[(i + self.pos) % N] - self.pos + N) % N

    def is_notch(self):
        return self.pos == self.rotor_notch

    def step(self):
        self.pos = (self.pos + 1) % N


class Reflector:
    def __init__(self, type):
        self.reflector = reflector[type]

    def forward(self, i):
        return self.reflector[i]


class PlugBoard:
    def __init__(self, swap_lst):
        self.swap_table = list(range(N))
        for (a, b) in swap_lst:
            self.swap_table[a], self.swap_table[b] = self.swap_table[b], self.swap_table[a]

    def forward(self, i):
        return self.swap_table[i]


class Setting:
    def __init__(self, rotor_types, rotor_pos, reflector_type, swap_lst):
        assert len(rotor_types) == 3
        assert len(rotor_pos) == 3
        self.rotor_types = rotor_types
        self.rotor_pos = rotor_pos
        self.reflector_type = reflector_type
        self.swap_lst = swap_lst

    def set_swap(self, swap_lst):
        self.swap_lst = swap_lst

    def swap_str(self):
        str_lst = []
        for (a, b) in self.swap_lst:
            str_lst.append("{}<->{} ".format(to_chr(a), to_chr(b)))
        return "".join(str_lst)

    def __str__(self):
        return "rotors: {}, {}, {} reflector: {} initial setting: {}, {}, {} plugboard: {}".format(
            self.rotor_types[0] + 1,
            self.rotor_types[1] + 1,
            self.rotor_types[2] + 1,
            self.reflector_type + 1,
            to_chr(self.rotor_pos[0]),
            to_chr(self.rotor_pos[1]),
            to_chr(self.rotor_pos[2]),
            self.swap_str(),
        )


class Enigma:
    def __init__(self, setting: Setting):
        self.setting = setting
        self.rotor_lst = [Rotor(setting.rotor_types[i], setting.rotor_pos[i]) for i in range(3)]
        self.reflector = Reflector(setting.reflector_type)
        self.plugboard = PlugBoard(setting.swap_lst)

    def forward(self, i):
        i = self.plugboard.forward(i)
        i = self.rotor_lst[0].forward(i)
        i = self.rotor_lst[1].forward(i)
        i = self.rotor_lst[2].forward(i)
        i = self.reflector.forward(i)
        i = self.rotor_lst[2].reverse(i)
        i = self.rotor_lst[1].reverse(i)
        i = self.rotor_lst[0].reverse(i)
        i = self.plugboard.forward(i)
        return i

    def step(self):
        # note double-stepping: ADV AEV
        step1 = self.rotor_lst[0].is_notch() or self.rotor_lst[1].is_notch()
        step2 = self.rotor_lst[1].is_notch()
        self.rotor_lst[0].step()
        if step1:
            self.rotor_lst[1].step()
        if step2:
            self.rotor_lst[2].step()

    def __str__(self):
        return self.setting.__str__()


class Enigmas:
    def __init__(self, setting: Setting, num):
        enigma = Enigma(setting)
        enigma.step()
        self.enigmas = [enigma]
        for i in range(1, num):
            enigma = copy.deepcopy(self.enigmas[i - 1])
            enigma.step()
            self.enigmas.append(enigma)

    def forward(self, pos, i):
        return self.enigmas[pos].forward(i)


def crypto(setting, s):
    enigma, re = Enigma(setting), []
    for c in s:
        enigma.step()
        re.append(to_chr(enigma.forward(to_num(c))))
    return "".join(re)


def test_double_stepping():
    setting1 = Setting([2, 1, 0], [to_num("V"), to_num("D"), to_num("A")], 0, [(to_num("A"), to_num("C"))])
    setting2 = Setting([2, 1, 0], [to_num("V"), to_num("E"), to_num("A")], 0, [(to_num("A"), to_num("C"))])
    assert crypto(setting1, "CCCCC") == "QIBMG"
    assert crypto(setting1, "QIBMG") == "CCCCC"
    assert crypto(setting2, "CCCCC") == "GIBMG"
    assert crypto(setting2, "GIBMG") == "CCCCC"
    print("Test double-stepping ok.")


def test_poland_ring():
    setting1 = Setting([2, 1, 0], [to_num("V"), to_num("D"), to_num("A")], 0, [])
    setting2 = Setting([2, 1, 0], [to_num("V"), to_num("D"), to_num("A")], 0, [(2, 3), (10, 6), (5, 7), (1, 4), (13, 14), (18, 25)])
    assert poland_ring(poland_mapping(setting1)) == poland_ring(poland_mapping(setting2))
    print("Test poland ring ok.")


def poland_mapping(setting: Setting):
    enigmas = [Enigma(setting) for _ in range(6)]
    for i in range(6):
        for j in range(i):
            enigmas[i].step()
    # a0a3 a1a4 a2a5
    mappings = [[] for _ in range(3)]
    for j in range(3):
        for i in range(N):
            mappings[j].append(enigmas[3 + j].forward(enigmas[j].forward(i)))
    return mappings


def poland_ring(mappings):
    ring_lens = [[] for _ in range(len(mappings))]
    for i in range(len(mappings)):
        mapping = mappings[i]
        s = set()
        for j in range(N):
            if j not in s:
                v, cnt = j, 0
                while True:
                    s.add(v)
                    v, cnt = mapping[v], cnt + 1
                    if v == j:
                        break
                ring_lens[i].append(cnt)
        ring_lens[i].sort()
    return ring_lens


def all_pairs(lst):
    if len(lst) < 2:
        yield []
        return
    a = lst[0]
    for i in range(1, len(lst)):
        pair = (a, lst[i])
        for rest in all_pairs(lst[1:i] + lst[i + 1 :]):
            yield [pair] + rest


def find_swaps(current, target):
    global current_mapping, rings, results
    current_mapping = current
    results = []
    rings = []
    s = set()
    for i in range(N):
        if i not in s:
            v, ring = i, []
            while True:
                s.add(v)
                ring.append(v)
                v = target[v]
                if v == i:
                    break
            rings.append(ring)
    dfs_swap(0)
    return results


def dfs_swap(depth):
    global guess_table, swaped, rings, current_mapping, results
    if depth == len(rings):
        results.append(tuple(guess_table))
        return
    ring = rings[depth]
    for i in range(N):
        current_guess = copy.deepcopy(guess_table)
        current_swaped = copy.deepcopy(swaped)
        curr = 0
        to_swap = i
        flag = False
        while curr < len(ring):
            ring_element = ring[curr]
            if not ((not swaped[ring_element] and not swaped[to_swap]) or (guess_table[ring_element] == to_swap and guess_table[to_swap] == ring[curr])):
                flag = True
                break
            swaped[ring_element], swaped[to_swap] = True, True
            guess_table[ring_element], guess_table[to_swap] = to_swap, ring_element
            curr += 1
            to_swap = current_mapping[to_swap]
        if not flag:
            dfs_swap(depth + 1)
        guess_table = current_guess
        swaped = current_swaped


def dfs_ring(depth):
    global rings, stack, current_p, current_c
    if depth > 0 and current_c[stack[depth - 1]] == current_p[stack[0]]:
        ring = []
        for i in range(depth):
            ring.append(stack[i])
        for old in rings:
            if set(old) == set(ring):
                return
        rings.append(ring)
        return
    if depth == MAX_RING_LEN or len(rings) > MAX_RING_NUM:
        return
    for i in range(len(current_p)):
        if depth == 0 or current_c[stack[depth - 1]] == current_p[i]:
            stack[depth] = i
            dfs_ring(depth + 1)


def get_pc_rings(plaintext, ciphertext):
    global current_p, current_c, rings
    current_p = plaintext
    current_c = ciphertext
    rings = []
    dfs_ring(0)
    return rings


def find_swaps_turing(enigmas):
    global current_enigmas, iterated, current_p_lst, current_c_lst, current_len, results
    current_enigmas = enigmas
    iterated = [False for _ in range(len(current_p))]
    results = []
    current_p_lst, current_c_lst = [], []
    current_len = len(current_p)
    for i in range(current_len):
        current_p_lst.append(to_num(current_p[i]))
        current_c_lst.append(to_num(current_c[i]))
    dfs_turing(0)
    return results


def dfs_turing(depth):
    global current_enigmas, iterated, guess_table, swaped, current_len
    first = -1
    for i in range(current_len):
        if not iterated[i]:
            first = i
            break
    if first == -1:
        results.append(tuple(guess_table))
        return
    for i in range(N):
        if not ((not swaped[current_p_lst[first]] and not swaped[i]) or (guess_table[current_p_lst[first]] == i and guess_table[i] == current_p_lst[first])):
            continue
        current_guess = copy.deepcopy(guess_table)
        current_swaped = copy.deepcopy(swaped)
        current_inte = copy.deepcopy(iterated)
        swaped[current_p_lst[first]], swaped[i] = True, True
        guess_table[current_p_lst[first]], guess_table[i] = i, current_p_lst[first]
        q = Queue()
        iterated[first] = True
        q.put(first)
        flag = True
        while not q.empty():
            e = q.get()
            current = current_c_lst[e]
            to_swap = current_enigmas.forward(e, guess_table[current_p_lst[e]])
            if not ((not swaped[current] and not swaped[to_swap]) or (guess_table[current] == to_swap and guess_table[to_swap] == current)):
                flag = False
                break
            swaped[current], swaped[to_swap] = True, True
            guess_table[current], guess_table[to_swap] = to_swap, current
            for j in range(current_len):
                if not iterated[j] and current_p_lst[j] == current:
                    iterated[j] = True
                    q.put(j)
        if flag:
            dfs_turing(depth + 1)
        guess_table = current_guess
        swaped = current_swaped
        iterated = current_inte


def Turing_decrypto(plaintext, ciphertext):
    print("Plaintext: ", plaintext)
    print("Ciphertext:", ciphertext)
    rings = get_pc_rings(plaintext, ciphertext)
    valid_settings = []
    for rotor_types in itertools.permutations(range(rotor_type_num), 3):
        print("Trying rotors ", rotor_types)
        for reflector_type in range(reflector_type_num):
            for i in range(N):
                for j in range(N):
                    for k in range(N):
                        setting = Setting(list(rotor_types), [i, j, k], reflector_type, [])
                        enigmas = Enigmas(setting, len(plaintext))
                        valid = True
                        for ring in rings:
                            flag = False
                            for guess in range(N):
                                g = guess
                                for pos in ring:
                                    g = enigmas.forward(pos, g)
                                if g == guess:
                                    flag = True
                                    break
                            if not flag:
                                valid = False
                                break
                        if valid:
                            valid_settings.append(setting)
    result = []
    for setting in valid_settings:
        print("Searching plugboard for enigma:", setting)
        enigmas = Enigmas(setting, len(plaintext))
        res_set = find_swaps_turing(enigmas)
        for swap_table in res_set:
            swap_list = []
            for i in range(N):
                if i != swap_table[i]:
                    swap_list.append((min(i, swap_table[i]), max(i, swap_table[i])))
            result.append(copy.deepcopy(setting))
            result[-1].set_swap(list(set(swap_list)))
    print("Found {} possible settings:".format(len(result)))
    for setting in result:
        print(setting)


def poland_decrypto(mappings):
    print("Input mappings:")
    print("A0A3: ", end="")
    for i in range(N):
        print("{}->{} ".format(to_chr(i), to_chr(mappings[0][i])), end="")
    print("")
    print("A1A4: ", end="")
    for i in range(N):
        print("{}->{} ".format(to_chr(i), to_chr(mappings[1][i])), end="")
    print("")
    print("A2A5: ", end="")
    for i in range(N):
        print("{}->{} ".format(to_chr(i), to_chr(mappings[2][i])), end="")
    print("")
    # iterate over rotor types, reflector types and rotor initial setting
    valid_settings = []
    ring = poland_ring(mappings)
    for rotor_types in itertools.permutations(range(rotor_type_num), 3):
        print("Trying rotors ", rotor_types)
        for reflector_type in range(reflector_type_num):
            for i in range(N):
                for j in range(N):
                    for k in range(N):
                        setting = Setting(list(rotor_types), [i, j, k], reflector_type, [])
                        if poland_ring(poland_mapping(setting)) == ring:
                            valid_settings.append(setting)
    # determine plugboard
    result = []
    for setting in valid_settings:
        print("Searching plugboard for enigma:", setting)
        current_mappings = poland_mapping(setting)
        swap_result = [find_swaps(current_mappings[i], mappings[i]) for i in range(3)]
        res_set = set(swap_result[0]).intersection(set(swap_result[1])).intersection(set(swap_result[2]))
        for swap_table in res_set:
            swap_list = []
            for i in range(N):
                if i != swap_table[i]:
                    swap_list.append((min(i, swap_table[i]), max(i, swap_table[i])))
            result.append(copy.deepcopy(setting))
            result[-1].set_swap(list(set(swap_list)))

    print("Found {} possible settings:".format(len(result)))
    for setting in result:
        print(setting)


if __name__ == "__main__":
    test_double_stepping()
    test_poland_ring()
    setting = Setting(
        [2, 1, 0],
        [to_num("V"), to_num("D"), to_num("A")],
        0,
        [(to_num("A"), to_num("B")), (to_num("C"), to_num("Z")), (to_num("D"), to_num("K")), (to_num("E"), to_num("F")), (to_num("M"), to_num("Q")), (to_num("X"), to_num("J"))],
    )
    poland_decrypto(poland_mapping(setting))
    plaintext = "CRYPOTOGRAPYISAVERYGOODCOURSEINTSINGHUAUNIVERSITYANDILOVEITVERYMUCH"
    Turing_decrypto(plaintext, crypto(setting, plaintext))
