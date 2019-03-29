#define SUCCESS         0
#define DEVICE_NAME     "pwm"

#define PWM1_ADDR       0x4005C000
#define PWM2_ADDR       0x4005C004
#define PWM_EN          31
#define PWM_RELOADV     8
#define PWM_DUTY        0
#define PWM_CLOCK_FREQ  32000
#define PWM_CLOCK       0x400040B8
#define LCD_CONF        0x40004054

#define PWM_CLOCK_VAL   0x115
#define LCD_CONF_VAL    0;


/*
 * minor numbers
 */

 #define MIN_PWM1_FREQ      0
 #define MIN_PWM1_DUTY      1
 #define MIN_PWM1_ENABLE    2
 #define MIN_PWM2_FREQ      3
 #define MIN_PWM2_DUTY      4
 #define MIN_PWM2_ENABLE    5

static int pwm1_enabled     = 0;
static int pwm1_frequency   = 0;
static int pwm1_duty        = 0;
static int pwm2_enabled     = 0;
static int pwm2_frequency   = 0;
static int pwm2_duty        = 0;

static int result = 0;          // sscanf result
static int input_val = 0;       // value from user
static int adj_input_val = 0;   // value from user after mapping

uint32_t* PWM1_PTR;
uint32_t* PWM2_PTR;

static bool device_opened = false;
static int how_often_opened = 0;

static int major_number = 0;
static int minor_number = 0;
