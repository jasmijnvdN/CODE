#ifndef F_CPU
#define F_CPU 16000000
#endif
#include <avr/io.h>
#include <util/delay.h>

#define SDI_BIT		PH5
#define DDR_SDI		DDRH
#define PORT_SDI	PORTH

#define SFTCLK_BIT	PH4
#define DDR_SFTCLK	DDRH
#define PORT_SFTCLK	PORTH

#define LCHCLK_BIT	PG5
#define DDR_LCHCLK	DDRG
#define PORT_LCHCLK	PORTG



#define z_as_1 PL5 //pin 44 //groen wit
#define z_as_2 PL4 //pin 45 // oranje grijs
#define schakelaar_Z_as_onderkant PG0//pin 41
#define schakelaar_Z_as_bovenkant PC2//pin 35
#define schakelaar_startpunt_Y_as PD7

#define y_as_1 PC7// pin 30
#define y_as_2 PC5// pin 32

#define schakelaar_Y_as PJ0// 15
#define schakelaar_X_as PH1// 16

#define magneet PC4//pin  33
static unsigned int segmentcodes[] =
{
    ~0xFC, ~0x60, ~0xDA, ~0xF2,
    ~0x66, ~0xB6, ~0xBE, ~0xE0,
    ~0xFE, ~0xF6, ~0xEE, ~0x3E,
    ~0x9C, ~0x7A, ~0x9E, ~0x8E, ~0xBC, 0X91, 0x9F
};

void init (void)
{
    DDR_SDI    |= (1 << SDI_BIT);
    DDR_SFTCLK |= (1 << SFTCLK_BIT);
    DDR_LCHCLK |= (1 << LCHCLK_BIT);
    PORT_SFTCLK &= ~(1 << SFTCLK_BIT);
    PORT_LCHCLK &= ~(1 << LCHCLK_BIT);
}
void send_data(char data)
{
    for (unsigned i = 0; i < 8; i++)
    {
        int bit = data & 1;
        data >>= 1;
        if (bit)
        {
            PORT_SDI |= (1 << SDI_BIT);
        }
        else
        {
            PORT_SDI &= ~(1 << SDI_BIT);
        }
        PORT_SFTCLK |= (1 << SFTCLK_BIT);
        PORT_SFTCLK &= ~(1 << SFTCLK_BIT);
    }
}
void send_enable(int display_nummer)
{
    send_data(0x10 << display_nummer);
}
void display(char data, int display_nummer)
{
    send_data(data);
    send_enable(display_nummer);
    PORT_LCHCLK |= (1 << LCHCLK_BIT);
    PORT_LCHCLK &= ~(1 << LCHCLK_BIT);
}
void display_getal(unsigned int getal)
{
    for (int i = 0; i < 4; i++)
    {
        display(segmentcodes[getal%16], i);
        getal /= 16;
        _delay_ms(1);   // 1 kHz
    }
}
int knop1_ingedrukt (void)
{
    return ((PINF &(1<<PF1)) == 0);
}
int knop2_ingedrukt (void)
{
    return ((PINF &(1<<PF2)) == 0);
}
int knop3_ingedrukt (void)
{
    return ((PINF &(1<<PF3)) == 0);
}

void startknop (void)
{
    DDRB |= (1<<PB7);
    PORTB |=(1<<7);
    //  PORTB &= ~(1<<7);

}
int main(void)
{
    //  startknop();
    //  int state=0;
    //  int knop =0;
    DDRC |= (1<<schakelaar_Z_as_bovenkant);
    init();
    int kwak = 0;
    int pakop = 0;
    int home_y = 0;
    int jeff= 0;
    int teller_y= 0;
    int go_y= 0;
    int go_down = 1;
    int nogeenkeer= 0;
    int nummer = 1;
    int letter = 10;
    int knop1_is_al_ingedrukt = 0;
    int knop2_is_al_ingedrukt = 0;
    int knop3_is_al_ingedrukt = 0;

    //OUTPUT MAGNEET
    DDRC |= (1<<PC4);
    PORTB |= (1<<PC4);

    DDRB |=(1<<7);
    PORTB |=(1<<7);
    DDRC |=(1<<y_as_1);
    DDRC |=(1<<y_as_2);
    DDRL |= (1<<z_as_1);//OUTPUT z-as vooruit pin 44
    DDRL |= (1<<z_as_2);//z-as achteruit pin 45
    // PORTB &= ~(1<<7);//LED UIT

    //  int counter= 10;
    while (1)
    {
        if (knop1_ingedrukt()) //display nummer
        {
            if (!knop1_is_al_ingedrukt)
            {
                _delay_ms(20);
                knop1_is_al_ingedrukt = 1;
                nummer++;
                if (nummer >= 8)
                {
                    nummer = 1;
                }
            }
        }
        else
        {
            if (knop1_is_al_ingedrukt)
            {
                _delay_ms(20);
                knop1_is_al_ingedrukt = 0;
            }
        }
        display(segmentcodes[nummer], 3);

        if (knop3_ingedrukt()) //display letter
        {
            if (!knop3_is_al_ingedrukt)
            {
                _delay_ms(20);
                knop3_is_al_ingedrukt = 1;
                letter++;
                if (letter >= 15)
                {
                    letter = 10;
                }
            }
        }
        else
        {
            if (knop3_is_al_ingedrukt)
            {
                _delay_ms(20);
                knop3_is_al_ingedrukt = 0;
            }
        }
        display(segmentcodes[letter], 0);


        if (knop2_ingedrukt()) //Start knop
        {
            if (!knop2_is_al_ingedrukt)
            {
                _delay_ms(20);
                knop2_is_al_ingedrukt = 1;
                //PORTB &= ~(1<<7);//z-AS AAN tot schakelaar
                jeff= 1;
            }
        }
///////////////////////////////////////////////////////////////////////////////////////////////////////z as oppakken blokje
        if(jeff ==1)
        {


            if(!((PING & (1<<schakelaar_Z_as_onderkant))==0) &&go_down==1)
            {

                //PORTA &= ~(1<<z_as_2); // naar beneden
                // PORTA |=(1<<z_as_1);
                PORTL |= (1<<z_as_1); //OMLAAG
                PORTL &= ~(1<<z_as_2);
            }

            if(((PING & (1<<schakelaar_Z_as_onderkant))==0) &&go_down==1)
            {
                PORTL &= ~(1<<z_as_1); //UIT
                PORTL &= ~(1<<z_as_2);

                // PORTA |= (1<<z_as_2); // uit
                //PORTA |=(1<<z_as_1);
                go_down= 0;

            }

            if(!((PINC & (1<<schakelaar_Z_as_bovenkant))==0) &&go_down==0)// NOT PRESSED
            {

                PORTL &= ~(1<<z_as_1); //NAAR BOVEN
                PORTL |= (1<<z_as_2);
                ////THIS ONE PORTB &= ~(1<<4);

            }

            if(((PINC & (1<<schakelaar_Z_as_bovenkant))==0) &&go_down==0)
            {
                //PORTB &= ~(1<<4);
                PORTL &= ~(1<<z_as_1); //UIT
                PORTL &= ~(1<<z_as_2);                 //STOND AAN
                go_y = 1;
                go_down = 2;
                PORTB &= ~(1<<7);

            }



//////////////////////////////////////////////////////////////////////////////////////z as klaar, opgepakt
//////////////////////////////////////////////////////////////////////////////////////////y as heen

            if(go_y ==1 )
            {

                if(!((PINJ & (1<<schakelaar_Y_as))==0)) // when pressed
                {
                    ++teller_y;
                    _delay_ms(500);
                    PORTB &= ~(1<<7);

                }
                else
                {

                    PORTB |=(1<<7);
                }
                if(teller_y >= nummer)
                {
                    PORTB &= ~(1<<6);
                    PORTC |= (1<<y_as_2); //Y-as uit
                    PORTC |=(1<<y_as_1);
                    go_y= 0;
                    kwak= 1;
                    pakop =1;
                }
                if(teller_y < nummer)
                {
                    PORTC &= ~(1<<y_as_2); //Y-as vooruit
                    PORTC |=(1<<y_as_1);
                    //  PORTB |= ~(1<<7);
                }
            }
            /////////////////////////////////////////////////////////////////////////////////// blok neerkwakken
            if(pakop ==1)
            {
                if(!((PING & (1<<schakelaar_Z_as_onderkant))==0) &&kwak==1)
                {

                    //PORTA &= ~(1<<z_as_2); // naar beneden
                    // PORTA |=(1<<z_as_1);
                    PORTL |= (1<<z_as_1); //OMLAAG
                    PORTL &= ~(1<<z_as_2);
                }

                if(((PING & (1<<schakelaar_Z_as_onderkant))==0) &&kwak==1)
                {
                    PORTL &= ~(1<<z_as_1); //UIT
                    PORTL &= ~(1<<z_as_2);

                    // PORTA |= (1<<z_as_2); // uit
                    //PORTA |=(1<<z_as_1);
                    kwak= 0;

                     PORTC ^= (1<<magneet);//magneet uit met delay

                }

                if(!((PINC & (1<<schakelaar_Z_as_bovenkant))==0) &&kwak==0)
                {

                    PORTL &= ~(1<<z_as_1); //NAAR BOVEN
                    PORTL |= (1<<z_as_2);
                    ////THIS ONE PORTB &= ~(1<<4);

                }

                if(((PINC & (1<<schakelaar_Z_as_bovenkant))==0) &&kwak==0)
                {
                    //PORTB &= ~(1<<4);
                    PORTL &= ~(1<<z_as_1); //UIT
                    PORTL &= ~(1<<z_as_2);
                    pakop=0;
                    kwak= 2;
                    home_y = 1;


                    //magneet aan (kan met delay)
                     PORTC &= ~(1<<magneet);
                    //terug naar home

                }
            }
            //////////////////////////////////////////// terug naar home y-as
            if(home_y == 1)
            {

                if(!(PIND & (1<<schakelaar_startpunt_Y_as))==0)
                {
                    PORTC &= ~(1<<y_as_1); //Y-as naar home
                    PORTC |=(1<<y_as_2);

                }
                if((PIND & (1<<schakelaar_startpunt_Y_as))==0)
                {
                    PORTC |= (1<<y_as_1); //stop motor y
                    PORTC |=(1<<y_as_2);
                    home_y= 0;
                    nogeenkeer =1;



                }
            }
            if(nogeenkeer==1)
            {
                    kwak = 0;
                     pakop = 0;
                     home_y = 0;
                     jeff= 0;
                     teller_y= 0;
                     go_y= 0;
                     go_down = 1;
                     nogeenkeer =0;
            }
        }

    }
}
