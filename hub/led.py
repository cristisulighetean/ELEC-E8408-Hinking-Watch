import db

hdb = db.HubDatabase()
while True:

    current_element = hdb.get_sessions()[-1] # this retreives the last element
    
    # TODO ... convert current_element to drawable image

    for scroll in range(text_width - width):
        for x in range(width):
            for y in range(height):
                pixel = image.getpixel((x + scroll, y))
                r, g, b = [int(n) for n in pixel]
                unicornhathd.set_pixel(width - 1 - x, y, r, g, b)

