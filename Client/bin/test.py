f = open("C.exe", "rb")
b = f.read()

tar = "http://127.0.0.1:5000/backend/"
space = 50 - len(tar)
last = tar + " " * space
i = 0

newbarry = bytearray(b)
p = newbarry.find(("b" * 50).encode())
print(p)
print(len(newbarry))
for i2 in last:
    print(p + i)
    newbarry[p + i] = ord(i2)
    i += 1
f2 = open("output.exe", "wb")
f2.write(newbarry)
f2.close()
f.close()
