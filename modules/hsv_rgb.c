#include "hsv_rgb.h"
#include "nrf_assert.h"

#define ABS(X) ( (X) < 0 ? (-(X)) : (X))

void hsv_update_component(hsv *hsv_src, hsv_idx_t component, float val)
{
    ASSERT(component >= 0 && component < HSV_COMPONENT_NUMBER);
    
    switch (component)
    {
        case HSV_H_IDX:
            hsv_src->h = val;
            break;

        case HSV_S_IDX:
            hsv_src->s = val;
            break;
        
        case HSV_V_IDX:
            hsv_src->v = val;
            break;

        case HSV_COMPONENT_NUMBER:
            ASSERT(0);    
            break;
    }   

    // float *hsv_raw = (float *) hsv_src;
    // hsv_raw[component] = val;
}

void hsv_set_values(hsv *hsv_dest, float h, float s, float v)
{
    hsv_dest->h = h;
    hsv_dest->s = s;
    hsv_dest->v = v;
}

void rgb_set_values(rgb *rgb_dest, float r, float g, float b)
{
    rgb_dest->r = r;
    rgb_dest->g = g;
    rgb_dest->b = b;
}

static void rgb_add_scalar(rgb *rgb_dest, float scalar)
{
    rgb_dest->r += scalar;
    rgb_dest->g += scalar;
    rgb_dest->b += scalar;
}

// static void rgb_div_by_scalar(rgb *rgb_dest, float scalar)
// {
//     rgb_dest->r /= scalar;
//     rgb_dest->g /= scalar;
//     rgb_dest->b /= scalar;
// }

void rgb_copy(rgb *src, rgb *dst)
{
    dst->r = src->r;
    dst->g = src->g;
    dst->b = src->b;
}

void hsv_copy(hsv *src, hsv *dst)
{
    dst->h = src->h;
    dst->s = src->s;
    dst->v = src->v;
}

void hsv_to_rgb(hsv *hsv_src, rgb *rgb_dest)
{
    float value;
    float C, X, M;
    unsigned int H;
    rgb rgb_tmp = {
        .r = 0,
        .g = 0,
        .b = 0,
    };

    value = hsv_src->v;
    H = hsv_src->h * 6;     // h * 360 / 60 = h * 6
    C = value * hsv_src->s;
    X = C * ( 1 - ABS( (H % 2) - 1 )  );
    M = value - C;

    switch (H)
    {
        case 0:
            rgb_set_values(&rgb_tmp, C, X, 0);
            break;

        case 1:
            rgb_set_values(&rgb_tmp, X, C, 0);
            break;

        case 2:
            rgb_set_values(&rgb_tmp, 0, C, X);
            break;

        case 3:
            rgb_set_values(&rgb_tmp, 0, X, C);
            break;

        case 4:
            rgb_set_values(&rgb_tmp, X, 0, C);
            break;
            
        case 5:
            rgb_set_values(&rgb_tmp, C, 0, X);
            break;
    }

    rgb_add_scalar(&rgb_tmp, M);
    rgb_copy(&rgb_tmp, rgb_dest);

}

#define MAX(A, B) (A) > (B) ? (A) : (B)
#define MIN(A, B) (A) < (B) ? (A) : (B)

void rgb_to_hsv(rgb *rgb_src, hsv *hsv_dest)
{
    float Cmax, Cmin, delta;
    float r, g, b;
    hsv hsv_tmp;
    r = rgb_src->r;
    g = rgb_src->g;
    b = rgb_src->b;

    Cmax = MAX(r, g);
    Cmax = MAX(Cmax, b);    
    Cmin = MIN(r, g);
    Cmin = MIN(Cmin, b);

    delta = Cmax - Cmin;

    if (Cmax == r)
    {
        hsv_tmp.h = (float) (((int) ((g - b) / delta)) % 6) / 6;
    } 
    else if (Cmax == g)
    {
        hsv_tmp.h = (((b - r)/delta) + 2) / 6;
    }
    else
    {
        hsv_tmp.h = (((r - g) / delta) + 4) / 6;
    }

    if(Cmax == 0)
    {
        hsv_tmp.s = 0;
    }
    else
    {
        hsv_tmp.s = delta / Cmax;
    }

    hsv_tmp.v = Cmax;

    hsv_copy(&hsv_tmp, hsv_dest);
}