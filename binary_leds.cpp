#include <iostream>
#include <csignal>

#include <chrono>
#include <thread>

#include <wiringPi.h>

using namespace std;

#define BUTTON 22

#define LED_1 23
#define LED_2 24
#define LED_3 25

#define NUM_LEDS 3 // number of leds
#define MAX_DEC 7 // 3 leds, so can only display up to 111(bin) == 7(dec)

#define LEDS (int[]) {LED_1, LED_2, LED_3}

string decimalToBinary(int dec) {
	string bin = "";
	int mask = 1 << (NUM_LEDS - 1); // for 3 leds, mask = 100(bin)
	while (mask > 0) {
		bin.push_back((dec & mask) > 0 ? '1' : '0'); // add the correct digit at the end
		mask >>= 1; // lshift mask to check the next binary digit
	}
	return bin;
}

void signalHandler(int sig) {
	// restore leds
	for (int i = 0; i < NUM_LEDS; i++) {
		digitalWrite(LEDS[i], LOW);
		pinMode(LEDS[i], INPUT);
	}
	
	// restore button (pin wPi 22 has V=1 by default)
	pullUpDnControl(BUTTON, PUD_UP);
	
	cout << "Program closed" << endl;
	exit(sig);
}

int main() {
	if (wiringPiSetup() == -1) { // use wPi numbering
		cout << "Wiring Pi setup failed" << endl;
		return 0;
	}
	cout << "Program start" << endl;
	
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGSEGV, signalHandler);
	
	for (int i = 0; i < NUM_LEDS; i++) {
		pinMode(LEDS[i], OUTPUT);
		digitalWrite(LEDS[i], LOW);
	}
	pullUpDnControl(BUTTON, PUD_UP);

	int num = 0; // number to display on leds
	bool wasButtonDown = false; // button was pressed down
	
	while (true) {
		// sleep so program isn't using CPU at 100%
		this_thread::sleep_for(chrono::milliseconds(100));
		
		// button just got pressed down if digitalRead == 0 (LOW)
		bool isButtonDown = !digitalRead(BUTTON);
		
		// button was not previously down (was up) & just got pressed down
		// all other states: do nothing
		if (!wasButtonDown && isButtonDown) {
			num = (num + 1) % (MAX_DEC + 1); // increment num, use mod to keep in range [0,7]
			
			int i = NUM_LEDS - 1; // start at the last led
			// keep reading dec until it's only zeroes left, then nothing left to read
			for (int dec = num; dec > 0; dec >>= 1) { // get next significant bit at the end of each loop
				digitalWrite(LEDS[i--], dec & 1); // display least significant bit
			}
			while (i >= 0) { // write remaining leds to 0
				digitalWrite(LEDS[i--], LOW);
			}
			
			cout << num << ", " << decimalToBinary(num) << endl;
		}
		
		wasButtonDown = isButtonDown;
	}
}
