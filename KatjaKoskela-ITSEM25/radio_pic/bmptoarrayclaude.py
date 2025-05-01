from PIL import Image

# Open PNG and convert to RGB
img = Image.open('patrik.jpg').convert('RGB')

# Convert to RGB565
with open('patrik.bin', 'wb') as f:
    for y in range(img.height):
        for x in range(img.width):
            r, g, b = img.getpixel((x, y))
            r = (r >> 3) & 0x1F
            g = (g >> 2) & 0x3F
            b = (b >> 3) & 0x1F
            rgb565 = (r << 11) | (g << 5) | b
            f.write(rgb565.to_bytes(2, 'little'))
