#include <iostream>
using namespace std;
uint8_t memory[4096]; // 4KB of RAM
uint8_t V[16]; //REGISTERS V1 to V16
uint8_t pc; // PROGRAM COUNTER 
uint16_t sp; // STACK TOP

uint8_t delay_timer; //Delay Timer
uint8_t sound_timer; //Sound Timer

uint16_t opcode; //Current opcode under execution
uint16_t I; //Index register

uint8_t gfx[64][32]; //DISPLAY
uint8_t key[16]; //KEYS
bool dlag; // Draw flag for display

int main(int argc, char* argv[])
{
	cout << "Chip 8\n";
	return 0;
}
