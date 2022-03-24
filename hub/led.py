import db
import colorsys
import time
import unicornhathd
import sys

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError:
    sys.exit('This LED display requires the pillow module\nInstall with: sudo pip install pillow\n')

hdb = db.HubDatabase()

while True:
    
    current_elements = hdb.get_sessions()
    
    if len(current_elements) > 0:
        current_element = current_elements[-1]

        lines = [f"Steps: {current_element.steps}",
                        f"Distance: {current_element.km}",
                        f"Kcal: {current_element.kcal}"]

        colour = (255, 0, 0)
        FONT = ('/usr/share/fonts/truetype/freefont/FreeSansBold.ttf', 12)
        
        unicornhathd.rotation(90)
        unicornhathd.brightness(0.6)

        width, height = unicornhathd.get_shape()

        text_x = width
        text_y = 2

        font_file, font_size = FONT
        font = ImageFont.truetype(font_file, font_size)
        text_width, text_height = width, 0

        try:
            for line in lines:
                w, h = font.getsize(line)
                text_width += w + width
                text_height = max(text_height, h)

            text_width += width + text_x + 1

            image = Image.new('RGB', (text_width, max(16, text_height)), (0, 0, 0))
            draw = ImageDraw.Draw(image)

            offset_left = 0

            for line in lines:
                draw.text((text_x + offset_left, text_y), line, colour, font=font)

                offset_left += font.getsize(line)[0] + width

            for scroll in range(text_width - width):
                for x in range(width):
                    for y in range(height):
                        pixel = image.getpixel((x + scroll, y))
                        r, g, b = [int(n) for n in pixel]
                        unicornhathd.set_pixel(width - 1 - x, y, r, g, b)

                unicornhathd.show()
                time.sleep(0.05)

        except KeyboardInterrupt:
            unicornhathd.off()
    

