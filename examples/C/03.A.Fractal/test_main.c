#include <stdio.h>
#include "../YY.utest/utest.h"
#include "image.h"
#include "fractal.h"

struct ImageTestFixture {
    image_p i;
};

UTEST_F_SETUP(ImageTestFixture) {
    utest_fixture->i = create_image(300, 300);
    ASSERT_NE((void *) utest_fixture->i, NULL);
}

UTEST_F_TEARDOWN(ImageTestFixture) {
    free_image(utest_fixture->i);
}

UTEST_F(ImageTestFixture, Set_Get_Pixel1){
    set_pixel(utest_fixture->i, 10, 10, 10);
    ASSERT_EQ(10, get_pixel(utest_fixture->i, 10, 10));
}

UTEST_F(ImageTestFixture, ClearImage2){
    clear_image(utest_fixture->i);
    ASSERT_EQ(0, get_pixel(utest_fixture->i, 10, 10));
}

UTEST_MAIN();
