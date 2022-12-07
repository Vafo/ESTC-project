#ifndef HSV_RGB_H
#define HSV_RGB_H

typedef enum 
{
    HSV_H_IDX,
    HSV_S_IDX,
    HSV_V_IDX,
    
    HSV_COMPONENT_NUMBER
} hsv_idx_t;

enum 
{
    RGB_R_IDX,
    RGB_G_IDX,
    RGB_B_IDX,

    RGB_COMPONENT_NUMBER
} rgb_idx;

typedef struct 
{
    float h;
    float s;
    float v;
} hsv;

typedef struct 
{
    float r;
    float g;
    float b;
}   rgb;

// HSV TO RGB
// https://en.wikipedia.org/wiki/HSL_and_HSV
// RGB TO HSV
// https://math.stackexchange.com/questions/556341/rgb-to-hsv-color-conversion-algorithm

void hsv_update_component(hsv *hsv_src, hsv_idx_t component, float val);
void hsv_to_rgb(hsv *hsv_src, rgb *rgb_dest);
void rgb_to_hsv(rgb *rgb_src, hsv *hsv_dest);

void hsv_set_values(hsv *hsv_dest, float h, float s, float v);
void rgb_set_values(rgb *rgb_dest, float r, float g, float b);

void rgb_copy(rgb *src, rgb *dst);
void hsv_copy(hsv *src, hsv *dst);

#endif