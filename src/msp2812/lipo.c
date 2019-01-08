#include <msp430.h>
#include <stdio.h>

#include "lipo.h"
#include "uart.h"
#include "ref.h"

void lipo_init(void)
{
	PMAPPWD = 0x02D52;
	P2MAP2 = PM_ANALOG;
	P2MAP3 = PM_ANALOG;
	PMAPPWD = 0;
	P2SEL |= BIT3;

}

/* code mostly copied from Goodwatch' adc_getvcc(), but since we use a voltage regulator, we measure
 * the LiPo battery's voltage behind a 1.718V diode (red LED) instead of via VCC.
 */

static int16_t get_adc_measurement(uint16_t mctl)
{
  uint16_t meas;

  ADC12CTL0 = ADC12ON+ADC12SHT0_2; // Turn on and set up ADC12
  ADC12CTL1 = ADC12SHP;                     // Use sampling timer
  ADC12MCTL0 = mctl;

  // Enable conversions
  ADC12CTL0 |= ADC12ENC;                    

  //Start the conversion and fetch the result when ready.
  ADC12CTL0 |= ADC12SC;
  while ((ADC12IFG & BIT0)==0);
  meas=ADC12MEM0;

  /* The ADC is supposed to turn itself off automatically, but our
     watch will die an early death if we don't shut it down, so we do
     it manually.
     
     As an added tricky bit, we have to stop the current conversion,
     then power down the controller, or it will be stuck running.  You
     can't just zero the registers.
  */
  
  ADC12CTL0 &= ~ADC12ENC; //Disable conversion
  ADC12CTL0&=~ADC12ON; //Power off the controller.
  //Then we zero any remaining flags.
  ADC12CTL0=0;
  ADC12CTL1=0;
  ADC12MCTL0=0;

  return meas;
}

uint16_t get_lipo_voltage(void)
{

	uint16_t adc = get_adc_measurement(ADC12SREF_1 | ADC12INCH_3);
	if (adc == 0xfff)
	{
		 uint16_t adc_vcc_adc3 = get_adc_measurement(ADC12SREF_0 | ADC12INCH_3);
		 uint16_t adc_vref_vcc = 2*get_adc_measurement(ADC12SREF_1 | ADC12INCH_11);
		 adc = (uint16_t) ( ( (uint32_t)adc_vref_vcc*(uint32_t)adc_vcc_adc3 )/4096 );
	}
  /* Millivolts
   */
	return (uint16_t)(adc*0.6103515625+1430);
}

typedef enum
{
	LOW = 0, HI = 1, HI_Z = 2,
} lipo_status;

static const char *lipo_stat_sym = "LHZ";

int lipo_stat(void)
{
	P1DIR &=~BIT1;
	P1REN |= BIT1;
	P1OUT |= BIT1;
	int pu = !!(P1IN & BIT1);
	P1OUT &=~BIT1;
	int pd = !!(P1IN & BIT1);
	P1REN &=~BIT1;
	if (!pu)
		return LOW;
	if (!pd)
		return HI_Z;
	return HI;
}

void tx_num(uint16_t n)
{
	char buf[10];
	char *p;
	sprintf(buf, "%d\r\n", n);

	for (p=buf; *p; p++)
		uart_tx(*p);
}

static int vOK=1;

int lipo_voltage_ok(void)
{
	return vOK;
}

int lipo_voltage_ok_check(void)
{
	ref_on();
	__delay_cycles(1000);
	uint16_t mV = get_lipo_voltage();
	tx_num(mV);
	ref_off();
	uart_tx( lipo_stat_sym[lipo_stat()] );
	uart_tx('\r');
	uart_tx('\n');

	if (vOK)
		vOK = !!(mV > 3300);
	else
		vOK = !!( (mV > 3500) || (lipo_stat()==LOW) );

	return vOK;
}
