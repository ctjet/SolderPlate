from PIL import Image


img = Image.open(r'C:\Users\Caleb Desktop\Documents\PlatformIO\Projects\SolderPlate\src\logo.jpg')
img = img.resize((128,128),Image.ANTIALIAS)
px=img.load()
print('{{',end='')
for i in range(0,128):
    for j in range(0,128):
        color = px[j,i]
        b = 0
        if(color[0]+color[1]+color[2]>255):
            b = 1

        if(j%8==0):
            if(j!=0):
                print(',', end='')
            print('0b', end='')
        print(b, end='')
    print('},')
    if(i!=127):
        print('{', end='')
print('};')
#img.show()