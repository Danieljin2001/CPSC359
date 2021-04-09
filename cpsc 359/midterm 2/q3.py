from smbus import SMBus
bus= SMBus()
bus.open(1)

#part a
data = bus.read_i2c_block_data(0x53, 0x32, 6)

#part b
iax = data[0] << 8 + data[1]
iax = ((iax + 32768) & 65535) - 32768
iay = data[2] << 8 + data[3]
iay = ((iay + 32768) & 65535) - 32768
iaz = data[4] << 8 + data[5]
iaz = ((iaz + 32768) & 65535) - 32768

#part c
iax = fax
iay = fay
iaz = faz
if fax >= 32768:
    fax -= 65536
fax = fax * 0.004
if fay >= 32768:
    fay -= 65536
fay = fay * 0.004
if faz >= 32768:
    faz -= 65536
faz = faz * 0.004


bus.close()
