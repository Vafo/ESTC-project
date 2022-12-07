#include <stdio.h>

#define PWM0    0
#define PWM1    1
#define PWM2    2
#define PWM3    3

int arr[10];

#define CONCAT_2_(A, B) A ## B
#define CONCAT_2(A, B) CONCAT_2_(A, B)

#define ARR_SIZE(ARR)   ( ( sizeof((ARR)) )/ ( sizeof((ARR)[0]) ) )

#define DEF_FUNC(IDX)    \
void CONCAT_2(pwm_jalap, IDX)  (void)    \
{               \
    printf("%d", arr[IDX]);    \
}

#define FUNC_NAME(IDX)  CONCAT_2(pwm_jalap, IDX)

typedef void (*func_ptr)(void);

DEF_FUNC(PWM0)
DEF_FUNC(PWM1)
DEF_FUNC(PWM2)

func_ptr funcies[] = {
    FUNC_NAME(PWM0),
    FUNC_NAME(PWM1),
    FUNC_NAME(PWM2),
};


int main()
{
    for (size_t i = 0; i < ARR_SIZE(arr); i++)
    {
        arr[i] = i;
    }
    

    for(int i = 0; i < ARR_SIZE(funcies); i++)
        funcies[i]();
}