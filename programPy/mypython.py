'''
1. When executed, create 3 files in the same directory as your script, each
named differently (the name of the files is up to you), which remain there
after your script finishes executing. Each of these 3 files must contain
exactly 10 random characters from the lowercase alphabet, with no spaces
("hoehdgwkdq", for example). The final (eleventh) character of each file MUST
be a newline character. Additional runs of the script should not append to the
files. Thus, running `wc -c myfile` against your files in the following manner
must return 11

2. When executed, output sent to stdout should first print out the contents of
the 3 files it is creating in exactly this format:

        $ python mypython.py
        vwdzwuxwvh
        nibqaackrp
        pxeugdqnwc
        8
        10
        80

3. After the file contents of all three files have been printed, print out two
random integers each on a separate line (whose range is from 1 to 42,
inclusive).

4. Finally, on the last (sixth) line, print out the product of the two numbers.

'''
from os import getpid
from random import randint, seed


STR_CNT             =   3
CHAR_CNT            =   10
NUM_CNT             =   2
DEEP_THOUGHT        =   42
ASCII_LC_BEGIN      =   97
ASCII_LC_END        =   122


def prod(numbers):
    result = 1
    for n in numbers:
        result *= n
    return result


def doNumerics(count):
    adamsNumbers = []
    fish = 0
    for i in range(count):
        fish = randint(1, DEEP_THOUGHT)
        adamsNumbers.append(fish)
        print(fish)
    p = prod(adamsNumbers)
    print(p)


def getRandStr(chars):
    zaphod = ''
    vogon = 'A'
    for c in range(chars):
        vogon = chr(randint(ASCII_LC_BEGIN, ASCII_LC_END))
        zaphod += vogon
    return zaphod


def writeStrToFile(rstr, pid):
    filename = rstr + '_' + str(pid)
    with open(filename, "w", CHAR_CNT+1, encoding="ascii") as f:
        if f.write(rstr+'\n') != CHAR_CNT+1:
            print("Write error.")


def doCharStrs(count, pid):
    randstr = 'notrand'
    for n in range(count):
        randstr = getRandStr(CHAR_CNT)
        print(randstr)
        writeStrToFile(randstr, pid)


def main():
    seed()
    doCharStrs(STR_CNT, getpid())
    doNumerics(NUM_CNT)


if __name__ == "__main__":
    main()
