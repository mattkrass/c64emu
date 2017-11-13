#!env python3.6
import sys
import pprint

if 3 > len(sys.argv):
    print('usage: %s <rom filename> <character start index> [character end index]' % sys.argv[0])
    exit(-1)

romFilename = sys.argv[1]
charStartIdx = int(sys.argv[2])
charEndIdx = charStartIdx if len(sys.argv) < 4 else int(sys.argv[3])
perLine = 4 if len(sys.argv) < 5 else int(sys.argv[4])
romData = b''

with open(romFilename, 'rb') as romFile:
    romData = romFile.read()

print('Read %d bytes from %s' % (len(romData), romFilename,))

numChars = charEndIdx - charStartIdx + 1
print('%d chars in total, %d per line.' % (numChars, perLine,))
if 1 == numChars:
    line = '0x%04X' % (charStartIdx * 8),
    for i in range(8):
        line += ' %02X' % romData[((charStartIdx * 8) + i)],
    print(line)

for charIdx in range(charStartIdx, charEndIdx+1, perLine):
    print('\ncharIdx = %d' % charIdx)
    startIdx = charIdx * 8
    for i in range(8):
        line = '%02d' % i
        for k in range(min(perLine, (charEndIdx - charIdx + 1))):
            line += ' '
            data = romData[((charIdx + k) * 8) + i]
            mask = 0x80
            for j in range(8):
                if data & mask > 0:
                    line += '\u2588'
                else:
                    line += ' '
                line += ' '
                mask >>= 1
        print(line)
