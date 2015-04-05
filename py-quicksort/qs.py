import random

def Print(str, level = 0):
    global debug_level

    if(debug_level >= level):
        print(str)

def PrintNumAry(ary, level = 2):
    global debug_level

    if(debug_level >= level):
        print(', '.join(map(str, ary)))

def PrintRound(ary, s, e, p, level = 3):
    global debug_level

    if(debug_level >= level):
        print("")
        print("\tstart: ary[{0}]: {1}".format(s, ary[s]))
        print("\tend: ary[{0}]: {1}".format(e, ary[e]))
        print("\tpivot: ary[{0}]: {1}".format(p, ary[p]))

def GenNumAry(auto = False):
    ary = []

    if(auto == True):
        size = random.randint(1, 50)
    else:
        size = input(">>> Size[10]: ").strip()
        if(size == ""):
            size = 10
        else:
            size = int(size)

    Print("Generate number sequence with size {0} ...".format(size), 1)
    while(size > 0):
        ary.append(random.randint(-50, 50))
        size = size - 1
    PrintNumAry(ary, 1)
    return ary

def Verify(ary):
    global pass_time, fail_time

    cursor = len(ary) - 2
    while(cursor >= 0):
        if(ary[cursor] > ary[cursor+1]):
            fail_time = fail_time + 1
            Print("Verify: [Fail]", 0)
            PrintNumAry(ary, 0)
            return
        cursor = cursor - 1

    pass_time = pass_time + 1
    Print("Verify: [Pass]", 1)

def Swap(ary, i, j):
    Print("swap [{0}] <-> [{1}]".format(i, j), 3)
    if(i != j):
        ary[i] = ary[i] ^ ary[j]
        ary[j] = ary[i] ^ ary[j]
        ary[i] = ary[i] ^ ary[j]
        PrintNumAry(ary, 3)

def QuickSort(ary, start, end, pivot):
    PrintRound(ary, start, end, pivot, 3)

    # move Pivot to the last position
    Swap(ary, pivot, end)
    pivot = end

    # Start compare with Pivot from left to right
    index = start
    while(index < pivot):
        if(ary[index] > ary[end]):
            pivot = pivot - 1
            Swap(ary, index, pivot)
        else:
            index = index + 1

    # Move Pivot to correct position (ary[pivot])
    Swap(ary, pivot, end)

    # Next run
    # left-hand-side
    if(start < pivot - 1):
        QuickSort(ary, start, pivot - 1, (start + pivot) // 2)
    # right-hand-side
    if(pivot + 1 < end):
        QuickSort(ary, pivot + 1, end, (pivot + end) // 2)

# Main part
print("==========================================QuickSort==========================================")
debug_level = 0
auto = True
test_time = 500
fail_time = 0
pass_time = 0

while(test_time > 0):
    test_time = test_time - 1

    # 1. Prepare target number sequence array
    ary = GenNumAry(auto)
    start = 0
    end = len(ary) - 1
    pivot = (start + end) // 2

    # 2. QuickSort
    if(start < end):
        QuickSort(ary, start, end, pivot)
    Print("\nResult:", 1)
    PrintNumAry(ary, 1)

    # 3. Verifiation
    Verify(ary)

if(fail_time == 0):
    Print("Verify: [All Pass]", 0)
Print("Total pass {0}, fail {1}".format(pass_time, fail_time), 0)
print("=====================================QuickSort Run Done======================================")
