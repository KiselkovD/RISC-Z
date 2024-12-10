#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/**
 * @brief One pixel data in-place 
 */
typedef uint8_t pixel_data;

/**
 * @brief Coordinate of a pixel and dimension of an image
 */
typedef unsigned int pixel_coord;

struct image;

/**
 * @brief Image for application
 */
typedef struct image image_t, *image_p;

/**
 * @brief Create image of given width and height
 * @param width,height Dimensions of an image
 * @returns an image
 */
image_p create_image(pixel_coord width, pixel_coord height);

void fill_random(image_p picture);

void clear_image(image_p picture);

void free_image(image_p picture);

int save_pgm(image_p picture, const char *filename);

/**
 * @brief Set the pixel of an image to given color
 *
 * @param picture Image
 * @param x,y Pixel coordinates
 * @param color Color value
 */
void set_pixel(image_p picture, pixel_coord x, pixel_coord y, pixel_data color);

/**
 * @brief Get the pixel of an image
 *
 * @param picture Image
 * @param x,y Pixel coordinates
 * @returns pixel Color value
 */
pixel_data get_pixel(image_p picture, pixel_coord x, pixel_coord y);

#endif // _IMAGE_H_
