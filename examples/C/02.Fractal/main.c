#include <stdio.h>
#include "image.h"
#include "fractal.h"

int main()
{
    image_p img = create_image(300, 300);

    fill_random(img);

    for(pixel_coord p = 2; p < 300 - 2; ++p)
        for(int offs = -2; offs < 2; ++offs)
            set_pixel(img, p, p + offs, (pixel_data) p);

    empty_fractal(img);

    save_pgm(img, "test.pgm");
    return 0;
}
