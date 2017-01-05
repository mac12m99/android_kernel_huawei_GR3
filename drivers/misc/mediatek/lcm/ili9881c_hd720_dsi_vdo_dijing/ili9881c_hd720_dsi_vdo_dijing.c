

#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
	#include <mach/mt_gpio.h>
#endif

#include <cust_gpio_usage.h>


// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)
#define GPIO_65132_ENP (GPIO64 | 0x80000000)
#define GPIO_65132_ENN (GPIO63 | 0x80000000)


#define REGFLAG_DELAY             							0xFEFE
#define REGFLAG_END_OF_TABLE      							0xFFFF   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE									0

#define LCM_ID_OTM1284A_BOE 0x1284

static unsigned int lcm_esd_test = FALSE;      ///only for ESD test


// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))
#define SET_GPIO_OUT(gpio_num,val)    						(lcm_util.set_gpio_out((gpio_num),(val)))


#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))


#define _LCM_DEBUG_

#ifdef BUILD_LK
#define printk printf
#endif

#ifdef _LCM_DEBUG_
#define lcm_debug(fmt, args...) printk(fmt, ##args)
#else
#define lcm_debug(fmt, args...) do { } while (0)
#endif

#ifdef _LCM_INFO_
#define lcm_info(fmt, args...) printk(fmt, ##args)
#else
#define lcm_info(fmt, args...) do { } while (0)
#endif
#define lcm_err(fmt, args...) printk(fmt, ##args)

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   			lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    
       

static struct LCM_setting_table {   
    unsigned int cmd;
    unsigned char count;
    unsigned char para_list[128];
};

static struct LCM_setting_table lcm_initialization_setting[] = {
	
	/*
	Note :

	Data ID will depends on the following rule.
	
		count of parameters > 1	=> Data ID = 0x39
		count of parameters = 1	=> Data ID = 0x15
		count of parameters = 0	=> Data ID = 0x05

	Structure Format :

	{DCS command, count of parameters, {parameter list}}
	{REGFLAG_DELAY, milliseconds of time, {}},

	...

	Setting ending by predefined flag
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
	*/

{0xff,3,{0x98,0x81,0x03}},
{0x01,1,{0x00}},
{0x02,1,{0x00}},
{0x03,1,{0x53}},
{0x04,1,{0x13}},
{0x05,1,{0x13}},
{0x06,1,{0x06}},
{0x07,1,{0x00}},
{0x08,1,{0x04}},
{0x09,1,{0x00}},
{0x0a,1,{0x00}},
{0x0b,1,{0x00}},
{0x0c,1,{0x00}},
{0x0d,1,{0x00}},
{0x0e,1,{0x00}},
{0x0f,1,{0x00}},
{0x10,1,{0x00}},
{0x11,1,{0x00}},
{0x12,1,{0x00}},
{0x13,1,{0x00}},
{0x14,1,{0x00}},
{0x15,1,{0x00}},
{0x16,1,{0x00}}, 
{0x17,1,{0x00}}, 
{0x18,1,{0x00}},
{0x19,1,{0x00}},
{0x1a,1,{0x00}},
{0x1b,1,{0x00}},
{0x1c,1,{0x00}},
{0x1d,1,{0x00}},
{0x1e,1,{0xC0}},
{0x1f,1,{0x80}},
{0x20,1,{0x04}},
{0x21,1,{0x0B}},
{0x22,1,{0x00}},
{0x23,1,{0x00}},
{0x24,1,{0x00}},
{0x25,1,{0x00}},
{0x26,1,{0x00}},
{0x27,1,{0x00}},
{0x28,1,{0x55}},
{0x29,1,{0x03}},
{0x2a,1,{0x00}},
{0x2b,1,{0x00}},
{0x2c,1,{0x00}},
{0x2d,1,{0x00}},
{0x2e,1,{0x00}},
{0x2f,1,{0x00}},
{0x30,1,{0x00}},
{0x31,1,{0x00}},
{0x32,1,{0x00}},
{0x33,1,{0x00}},
{0x34,1,{0x04}},
{0x35,1,{0x05}}, 
{0x36,1,{0x05}},
{0x37,1,{0x00}},
{0x38,1,{0x3c}},
{0x39,1,{0x00}},
{0x3a,1,{0x40}}, 
{0x3b,1,{0x40}},
{0x3c,1,{0x00}},
{0x3d,1,{0x00}},
{0x3e,1,{0x00}},
{0x3f,1,{0x00}},
{0x40,1,{0x00}},
{0x41,1,{0x00}},
{0x42,1,{0x00}},
{0x43,1,{0x00}},
{0x44,1,{0x00}},


//GIP_2
{0x50,1,{0x01}},
{0x51,1,{0x23}},
{0x52,1,{0x45}},
{0x53,1,{0x67}},
{0x54,1,{0x89}},
{0x55,1,{0xab}},
{0x56,1,{0x01}},
{0x57,1,{0x23}},
{0x58,1,{0x45}},
{0x59,1,{0x67}},
{0x5a,1,{0x89}},
{0x5b,1,{0xab}},
{0x5c,1,{0xcd}},
{0x5d,1,{0xef}},

//GIP_3
{0x5e,1,{0x01}},
{0x5f,1,{0x14}},
{0x60,1,{0x15}},
{0x61,1,{0x0C}},
{0x62,1,{0x0D}},
{0x63,1,{0x0E}},
{0x64,1,{0x0F}},
{0x65,1,{0x10}},
{0x66,1,{0x11}},
{0x67,1,{0x08}},
{0x68,1,{0x02}},
{0x69,1,{0x0A}},
{0x6a,1,{0x02}},
{0x6b,1,{0x02}},
{0x6c,1,{0x02}},
{0x6d,1,{0x02}},
{0x6e,1,{0x02}},
{0x6f,1,{0x02}},
{0x70,1,{0x02}},
{0x71,1,{0x02}},
{0x72,1,{0x06}},
{0x73,1,{0x02}},
{0x74,1,{0x02}},
{0x75,1,{0x14}},
{0x76,1,{0x15}},
{0x77,1,{0x11}},
{0x78,1,{0x10}},
{0x79,1,{0x0F}},
{0x7a,1,{0x0E}},
{0x7b,1,{0x0D}},
{0x7c,1,{0x0C}},
{0x7d,1,{0x06}},
{0x7e,1,{0x02}},
{0x7f,1,{0x0A}},
{0x80,1,{0x02}},
{0x81,1,{0x02}},
{0x82,1,{0x02}},
{0x83,1,{0x02}},
{0x84,1,{0x02}},
{0x85,1,{0x02}},
{0x86,1,{0x02}},
{0x87,1,{0x02}},
{0x88,1,{0x08}},
{0x89,1,{0x02}},
{0x8A,1,{0x02}},

//CMD_Page 4
{0xFF,3,{0x98,0x81,0x04}},
{0x00,1,{0x80}},                 //0X00 FOR 3LANE   0X80 FOR 4LANE
{0x6C,1,{0x15}},                //Set VCORE voltage =1.5V
{0x6E,1,{0x3B}},                  //di_pwr_reg=0 for power mode 2A //VGH clamp 18V
{0x6F,1,{0x57}},                // reg vcl + pumping ratio VGH=3x VGL=-2x
{0x3A,1,{0xA4}},                //POWER SAVING
{0x8D,1,{0x15}},               //VGL clamp -10V
{0x87,1,{0xBA}},               //ESD               
{0x26,1,{0x76}},            
{0xB2,1,{0xD1}},
{0x88,1,{0x0B}},
{0x17,1,{0x0C}},
{0xB5,1,{0x06}},		 
{0x31,1,{0x75}},		

//CMD_Page 1
{0xFF,3,{0x98,0x81,0x01}},
{0x22,1,{0x0A}},              //BGR, SS
{0x31,1,{0x00}},               //column inversion
{0x53,1,{0x6d}}, //0x7B            //VCOM1
{0x55,1,{0x85}},               //VCOM2
{0x50,1,{0xA7}},               // VREG1OUT=4.7V
{0x51,1,{0xA7}},               // VREG2OUT=-4.7V
{0x60,1,{0x14}},               //SDT
{0xA0,1,{0x08}},               //VP255 Gamma P
{0xA1,1,{0x26}}, //0x1F              //VP251
{0xA2,1,{0x34}}, //0x2D              //VP247
{0xA3,1,{0x17}},//0x19               //VP243
{0xA4,1,{0x17}},  //0x1D             //VP239
{0xA5,1,{0x23}},  //0x2F             //VP231
{0xA6,1,{0x1c}}, //0x21              //VP219
{0xA7,1,{0x24}}, // 0x21             //VP203
{0xA8,1,{0x9a}}, //0x92            //VP175
{0xA9,1,{0x1D}},//0x1C              //VP144
{0xAA,1,{0x27}}, //0x2A              //VP111
{0xAB,1,{0x7D}},  //0x77             //VP80
{0xAC,1,{0x19}},  //0x19             //VP52
{0xAD,1,{0x18}},               //VP36
{0xAE,1,{0x4C}},               //VP24
{0xAF,1,{0x20}}, //0x24              //VP16
{0xB0,1,{0x27}},               //VP12
{0xB1,1,{0x4D}},//0x48               //VP8
{0xB2,1,{0x5C}},               //VP4
{0xB3,1,{0x2C}},//0x23               //VP0

{0xC0,1,{0x08}},               //VN255 GAMMA N
{0xC1,1,{0x26}},               //VN251
{0xC2,1,{0x35}},//0x30                //VN247
{0xC3,1,{0x17}},//0x0D               //VN243
{0xC4,1,{0x17}},//0x11               //VN239
{0xC5,1,{0x23}},//0x24               //VN231
{0xC6,1,{0x1d}},//0x1B               //VN219
{0xC7,1,{0x24}},//0x1D               //VN203
{0xC8,1,{0x99}},//0x88               //VN175
{0xC9,1,{0x1D}},               //VN144
{0xCA,1,{0x27}},//0x27               //VN111
{0xCB,1,{0x7D}},//0x76               //VN80
{0xCC,1,{0x19}},//0x1D               //VN52
{0xCD,1,{0x18}},//0x19               //VN36
{0xCE,1,{0x4C}},//0x52               //VN24
{0xCF,1,{0x20}},//0x22               //VN16
{0xD0,1,{0x27}},//0x2C               //VN12
{0xD1,1,{0x4D}},//0x4C               //VN8
{0xD2,1,{0x5D}},               //VN4
{0xD3,1,{0x2C}},//0x23               //VN0

{0xff,3,{0x98,0x81,0x00}},
{0x35,0,{0x00}},
{0x11,0,{0x00}},
{REGFLAG_DELAY,120,{}},

{0x29,0,{0x00}},
{REGFLAG_DELAY,20,{}},

{0xff,3,{0x98,0x81,0x05}}
};


static struct LCM_setting_table lcm_sleep_out_setting[] = {
#if 0
    // Sleep Out
	{0x11, 0, {0x00}},
    {REGFLAG_DELAY, 150, {}},

    // Display ON
	{0x29, 0, {0x00}},
	{REGFLAG_DELAY, 20, {}},
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
#endif
};


static struct LCM_setting_table lcm_sleep_in_setting[] = {
#if 0
	// Display off sequence
	{0x28, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},
    // Sleep Mode On
	{0x10, 0, {0x00}},

	{REGFLAG_DELAY, 120, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
#endif
};


static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;
	lcm_debug("%s %d\n", __func__,__LINE__);
    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
//#ifdef BUILD_LK 
//	printf("push_table %x\n",cmd); 
//#else 
//	printk("push_table %x\n",cmd); 
//#endif 
		
        switch (cmd) {
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
#ifdef BUILD_LK 
	printf("MDELAY_push_table\n"); 
#else 
	printk("MDELAY_push_table\n"); 
#endif 
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
       	}
    }
	
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
	lcm_debug("%s %d\n", __func__,__LINE__);
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
	  	lcm_debug("%s %d\n", __func__,__LINE__);
		memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

		// enable tearing-free
		//params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
		//params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
		params->dsi.mode   = BURST_VDO_MODE;
#endif
	

		//params->dsi.esd_check_enable = 1;
		//params->dsi.customization_esd_check_enable = 1;
		//params->dsi.lcm_esd_check_table[0].cmd = 0x0a;
		//params->dsi.lcm_esd_check_table[0].count = 1;
		//params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9c;
		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

         params->dsi.ssc_disable = 0;  // ssc disable control (1: disable, 0: enable, default: 0)  add stone
         params->dsi.ssc_range = 5;  // ssc range control (1:min, 8:max, default: 4)  add stone 2

		params->dsi.vertical_sync_active				= 4;//4;//2;
		params->dsi.vertical_backporch					= 16;//20 stone//16;//14;
		params->dsi.vertical_frontporch					= 10;//20 stone//16;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 20;//6;//2;
		params->dsi.horizontal_backporch				= 50;//44;//44;//42;
		params->dsi.horizontal_frontporch				= 50;//90 stone//44;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
		params->dsi.HS_TRAIL = 12;
		params->dsi.PLL_CLOCK = 204;//dsi clock customization: should config clock value directly 220 stone 210
                params->dsi.cont_clock=0;//add stone

}


static unsigned int lcm_compare_id(void)
{
	int data_array[4]; 
	int array[16];
	char buffer[5] = {0xFF,0x98,0x81,0x01}; 
	char id_high=0; 
	char id_low=0; 
	int id=0; 

	mt_set_gpio_mode(GPIO_65132_ENP, GPIO_MODE_00);//sophiarui
	mt_set_gpio_dir(GPIO_65132_ENP, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_65132_ENP, GPIO_OUT_ONE);
	MDELAY(15);
	mt_set_gpio_mode(GPIO_65132_ENN, GPIO_MODE_00);//sophiarui
	mt_set_gpio_dir(GPIO_65132_ENN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_65132_ENN, GPIO_OUT_ONE);
	MDELAY(15);

	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(1);
	SET_RESET_PIN(1);
	MDELAY(10);

	dsi_set_cmdq_V2(0xFF, 5, buffer, 1);//enable to page1  

	array[0] = 0x00013700;  
	dsi_set_cmdq(array, 1, 1);  
	read_reg_v2(0x00, &data_array[0], 1);

	array[0] = 0x00013700; 
	dsi_set_cmdq(array, 1, 1); 
	read_reg_v2(0x01, &data_array[1], 1); 

	id_high = data_array[0]; 
	id_low = data_array[1]; 
	id = (id_high<<8) | id_low; 

#ifdef BUILD_LK 
	printf("%s, LK debug: ili9881c id = 0x%08x\n", __func__, id); 
#else 
	printk("%s, kernel debug: ili9881c id = 0x%08x\n", __func__, id); 
#endif 

	if(id == 0x9881)
		return 1;
	else
		return 0;
}
static void lcm_init(void)
{
    mt_set_gpio_mode(GPIO_65132_ENP, GPIO_MODE_00);//sophiarui
	mt_set_gpio_dir(GPIO_65132_ENP, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_65132_ENP, GPIO_OUT_ONE);
	MDELAY(15);
	mt_set_gpio_mode(GPIO_65132_ENN, GPIO_MODE_00);//sophiarui
	mt_set_gpio_dir(GPIO_65132_ENN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_65132_ENN, GPIO_OUT_ONE);
	MDELAY(15);

	
	lcm_debug("%s %d\n", __func__,__LINE__);
	SET_RESET_PIN(0);
	MDELAY(1);
	SET_RESET_PIN(1);
	MDELAY(10);//120   zhangqiangqiang

	push_table(lcm_initialization_setting,sizeof(lcm_initialization_setting)/sizeof(lcm_initialization_setting[0]),1);
}

static struct LCM_setting_table lcm_suspend_setting[] = 
{
	{0xff,3,{0x98,0x81,0x00}},
	{0x28,1,{0x00}},

	{REGFLAG_DELAY,10,{}},

	{0x10,1,{0x00}},

	{REGFLAG_DELAY,120,{}}
};

static void lcm_suspend(void)
{
	lcm_debug("%s %d\n", __func__,__LINE__);

	push_table(lcm_suspend_setting, sizeof(lcm_suspend_setting) / sizeof(struct LCM_setting_table), 1);
	//push_table(lcm_sleep_in_setting, sizeof(lcm_sleep_in_setting) / sizeof(struct LCM_setting_table), 1);

	SET_RESET_PIN(1);
	MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(10);	
	SET_RESET_PIN(1);
	MDELAY(80);

    mt_set_gpio_mode(GPIO_65132_ENP, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_65132_ENP, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_65132_ENP, GPIO_OUT_ZERO);
	MDELAY(10);
    mt_set_gpio_mode(GPIO_65132_ENN, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_65132_ENN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_65132_ENN, GPIO_OUT_ZERO);
	MDELAY(10);
}



static void lcm_resume(void)
{
	lcm_debug("%s %d\n", __func__,__LINE__);
/*
    mt_set_gpio_mode(GPIO_65132_ENP, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_65132_ENP, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_65132_ENP, GPIO_OUT_ONE);
	MDELAY(10);
    mt_set_gpio_mode(GPIO_65132_ENN, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_65132_ENN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_65132_ENN, GPIO_OUT_ONE);
	MDELAY(10);
*///zhangqiangqiang
	//SET_GPIO_OUT(GPIO_LCM_PWR_EN,1);  //Enable LCM Power
	lcm_init();
	//push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	lcm_debug("%s %d\n", __func__,__LINE__);
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	data_array[3]= 0x00000000;
	data_array[4]= 0x00053902;
	data_array[5]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[6]= (y1_LSB);
	data_array[7]= 0x00000000;
	data_array[8]= 0x002c3909;

	dsi_set_cmdq(&data_array, 9, 0);

}



static unsigned int lcm_esd_check(void)
{
	unsigned int ret=FALSE;
#ifndef BUILD_LK
	char  *buffer;
	int   array[4];

#if 1
	if(lcm_esd_test)
	{
		lcm_esd_test = FALSE;
		return TRUE;
	}
#endif
	array[0] = 0x00013700;
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0x0A, buffer, 1);
	printk(" esd buffer0 =%x\n", buffer[0]);

#if 1
	if(buffer[0]==0x9c)
	{
		ret=FALSE;
	}
	else
	{			 
		ret=TRUE;
	}
#endif
#endif
	return ret;

}

static unsigned int lcm_esd_recover(void)
{
	lcm_init();

#ifndef BUILD_LK
	printk("lcm_esd_recover otm1284a_boe\n");
#endif
	return TRUE;
}


LCM_DRIVER ili9881c_hd720_dsi_vdo_dijing_lcm_drv = 
{
	.name		= "ili9881c_hd720_dsi_vdo_dijing",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
	//.esd_check = lcm_esd_check,
	//.esd_recover = lcm_esd_recover,
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
    };
