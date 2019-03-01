#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <random>
#include "time.h" 
#include <chrono>
#include <thread>
#include "stdint.h"
#include "SDL2/SDL.h" 

using namespace std; // standard namespace for C++ is used here

uint8_t memory[4096]; // 4KB of RAM
uint8_t V[16]; //REGISTERS V1 to V16
int pc; //PC - Program Counter 
uint16_t top; // STACK TOP
uint16_t stack[16];
uint8_t delay_timer; //Delay Timer
uint8_t sound_timer; //Sound Timer

uint16_t opcode; //Current opcode under execution
uint16_t I; //Index register

uint8_t pix[64*32]; // Display Pixels
uint8_t key[16]; //KEYS

bool dflag = false; // Draw flag for ditoplay

//The fontset for the emulator copied from other source, link mentioned in documentation references
unsigned char chip8_fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, 
    0x20, 0x60, 0x20, 0x20, 0x70, 
    0xF0, 0x10, 0xF0, 0x80, 0xF0, 
    0xF0, 0x10, 0xF0, 0x10, 0xF0, 
    0x90, 0x90, 0xF0, 0x10, 0x10, 
    0xF0, 0x80, 0xF0, 0x10, 0xF0, 
    0xF0, 0x80, 0xF0, 0x90, 0xF0, 
    0xF0, 0x10, 0x20, 0x40, 0x40, 
    0xF0, 0x90, 0xF0, 0x90, 0xF0, 
    0xF0, 0x90, 0xF0, 0x10, 0xF0, 
    0xF0, 0x90, 0xF0, 0x90, 0x90, 
    0xE0, 0x90, 0xE0, 0x90, 0xE0, 
    0xF0, 0x80, 0x80, 0x80, 0xF0, 
    0xE0, 0x90, 0x90, 0x90, 0xE0,
    0xF0, 0x80, 0xF0, 0x80, 0xF0, 
    0xF0, 0x80, 0xF0, 0x80, 0x80  
};

 
uint8_t keymap[16] = {
    SDLK_x,
    SDLK_q,
    SDLK_w,
    SDLK_e,
    SDLK_a,
    SDLK_s,
    SDLK_d,
    SDLK_z,
    SDLK_c,
    SDLK_4,
    SDLK_r,
    SDLK_f,
    SDLK_v,
    SDLK_1,
    SDLK_2,
    SDLK_3,

};


void execute(){
	opcode = memory[pc] << 8 | memory[pc + 1];   //8 bit information is combined into 1 16 bit instruction and PC is incremented by 2 after execution of each 
                                                    //Instruction
    cout <<"OPCODE: "<<std::hex<<opcode<<endl;
    switch(opcode & 0xF000){

        case 0x0000:

            switch (opcode & 0x000F) {
                //Clear screen
                case 0x0000:
                    for (int i = 0; i < 2048; ++i) {
                        pix[i] = 0;
                    }
                    dflag = true;
                    pc+=2;
                    break;

                //Return from subroutine
                case 0x000E:
                    --top;
                    pc = stack[top];
                    pc += 2;
                    break;

                default:
                    cout<<"\nUnknown op code: : "<<opcode<<endl;
                    exit(3);
            }
            break;

        // Jumps to address XYZ
        case 0x1000:
            pc = opcode & 0x0FFF;
            break;

        // Calls subroutine at XYZ
        case 0x2000:
            stack[top] = pc;
            ++top;
            pc = opcode & 0x0FFF;
            break;

        // Skips the next instruction if REG X equals N.
        case 0x3000:
            if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
                pc += 4;
            else
                pc += 2;
            break;

        //Skips the next instruction if REG X does not equal N.
        case 0x4000:
            if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
                pc += 4;
            else
                pc += 2;
            break;

        // Skips the next instruction if REG X equals REG Y.
        case 0x5000:
            if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
                pc += 4;
            else
                pc += 2;
            break;

        // Sets REG X to N.
        case 0x6000:
            V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            pc += 2;
            break;

        // Adds N to REG X.
        case 0x7000:
            V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            pc += 2;
            break;

        //Instruction 8---
        case 0x8000:
            switch (opcode & 0x000F) {

                //Sets REG X to the value of REG Y.
                case 0x0000:
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // Sets REG X to REG X OR REG Y.
                case 0x0001:
                    V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // Sets REG X to REG X AND REG Y.
                case 0x0002:
                    V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // Sets REG X to REG X EX-OR REG Y.
                case 0x0003:
                    V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // Adds REG Y to REG X. VF is set to 1 when there's a carry, and to 0 when there isn't any carry .
                case 0x0004:
                    V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
                    if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
                        V[0xF] = 1; //carry
                    else
                        V[0xF] = 0;
                    pc += 2;
                    break;

                // REG Y is subtracted from REG X. REG F is set to 0 when there's a borrow, and 1 when there isn't a borrow.
                case 0x0005:
                    if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
                        V[0xF] = 0; // there is a borrow
                    else
                        V[0xF] = 1;
                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                //  Shifts REG X right by one. REG F is set to the value of the LSB of REG X before the shift.
                case 0x0006:
                    V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
                    V[(opcode & 0x0F00) >> 8] >>= 1;
                    pc += 2;
                    break;

                //Sets REG X to REG Y minus REG X and borrow flag
                case 0x0007:
                    if(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])	
                        V[0xF] = 0; 
                    else
                        V[0xF] = 1;
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                //Shifts REG X left by one
                case 0x000E:
                    V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
                    V[(opcode & 0x0F00) >> 8] <<= 1;
                    pc += 2;
                    break;

                default:
                    cout<<"\nUnknown op code: "<<opcode<<endl;
                    exit(3);
            }
            break;

        //Skips the next instruction if REG X doesn't equal REG Y.
        case 0x9000:
            if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
                pc += 4;
            else
                pc += 2;
            break;

        // Sets I to the address ---.
        case 0xA000:
            I = opcode & 0x0FFF;
            pc += 2;
            break;

        //Jumps to the address plus V0.
        case 0xB000:
            pc = (opcode & 0x0FFF) + V[0];
            break;

        // Set regiseter to random value...
        case 0xC000:
            V[(opcode & 0x0F00) >> 8] = (rand() % (0xFF + 1)) & (opcode & 0x00FF);
            pc += 2;
            break;

        //Display render instruction copied from other source program, link mentioned in documentation
        case 0xD000:
        {
            unsigned short x = V[(opcode & 0x0F00) >> 8];
            unsigned short y = V[(opcode & 0x00F0) >> 4];
            unsigned short height = opcode & 0x000F;
            unsigned short pixel;

            V[0xF] = 0;
            for (int yline = 0; yline < height; yline++)
            {
                pixel = memory[I + yline];
                for(int xline = 0; xline < 8; xline++)
                {
                    if((pixel & (0x80 >> xline)) != 0)
                    {
                        if(pix[(x + xline + ((y + yline) * 64))] == 1)
                        {
                            V[0xF] = 1;
                        }
                        pix[x + xline + ((y + yline) * 64)] ^= 1;
                    }
                }
            }

            dflag = true;
            pc += 2;
        }
            break;

        // Instruction E---
        case 0xE000:

            switch (opcode & 0x00FF) {
                // Skips the next instruction if the key stored in REG X is pressed.
                case 0x009E:
                    if (key[V[(opcode & 0x0F00) >> 8]] != 0)
                        pc +=  4;
                    else
                        pc += 2;
                    break;

                // Skips the next instruction if the key stored
                // in REG X isn't pressed.
                case 0x00A1:
                    if (key[V[(opcode & 0x0F00) >> 8]] == 0)
                        pc +=  4;
                    else
                        pc += 2;
                    break;

                default:
                   cout<< "\nUnknown op code: "<<std::hex << opcode<<endl;
                    exit(0);
            }
            break;


        case 0xF000:
            switch(opcode & 0x00FF)
            {
                // ets REG X to the value of the delay timer
                case 0x0007:
                    V[(opcode & 0x0F00) >> 8] = delay_timer;
                    pc += 2;
                    break;

                // A key press is awaited, and then stored in REG X
                case 0x000A:
                {
                    bool key_pressed = false;

                    for(int i = 0; i < 16; ++i)
                    {
                        if(key[i] != 0)
                        {
                            V[(opcode & 0x0F00) >> 8] = i;
                            key_pressed = true;
                        }
                    }

                    // If no key is pressed, return and try again.
                    if(!key_pressed)
                        return;

                    pc += 2;
                }
                    break;

                // Sets the delay timer to REG X
                case 0x0015:
                    delay_timer = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                // Sets the sound timer to REG X
                case 0x0018:
                    sound_timer = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                //Adds REG X to I
                case 0x001E:
            
                    if(I + V[(opcode & 0x0F00) >> 8] > 0xFFF)
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;
                    I += V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                //  Sets I to the location of the sprite for the character in REG X.
                case 0x0029:
                    I = V[(opcode & 0x0F00) >> 8] * 0x5;
                    pc += 2;
                    break;

                // Store the BCD representation of REG X at the addresses I, I+1 & I+2
                case 0x0033:
                    memory[I]     = V[(opcode & 0x0F00) >> 8] / 100;
                    memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
                    memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
                    pc += 2;
                    break;

                // Stores REG 0 to REG X in memory starting at address I
                case 0x0055:
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                        memory[I + i] = V[i];
                    I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;

                case 0x0065:
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                        V[i] = memory[I + i];
                    I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;

                default:
                    cout<<"Unknown opcode [0xF000]: "<< opcode<<endl;
            }
            break;

        default:
            cout<<"\nUnimplemented op code: " << opcode<<endl;
            exit(3);
    }




    if (delay_timer > 0)
        --delay_timer;

    if (sound_timer > 0){}
        if(sound_timer == 1){
        --sound_timer;
		}
	}

void sys_init() {
    pc  = 0x200;    
    opcode = 0;       
    I  = 0;          
    top = 0;       
    int i;
    for (i = 0; i < 2048; ++i) {
        pix[i] = 0;
    }

  
    for (i = 0; i < 16; ++i) {
        stack[i]    = 0;
        key[i]      = 0;
        V[i]        = 0;
    }

    for (i = 0; i < 4096; ++i) {
        memory[i] = 0;
    }

    for (i = 0; i < 80; ++i) {
        memory[i] = chip8_fontset[i];
    }


    delay_timer = 0;
    sound_timer = 0;

    srand (time(NULL));
}


int main(int argc, char* argv[])

{
    //SDL renders are copied from the SDL2 documentation tutorials with slight changes in values to meet the requirements
    //Link is mentioned in the documentation
    int width = 1024;                 
    int height = 512;                   
    uint32_t temp[2048]; //temp is a temporary arrray that is used to map the pixels to screen

    SDL_Window* window = NULL;


	 sys_init();

    cout<<"Loading ROM: %s\n"<< argv[1]<<endl;

    FILE* rom = fopen(argv[1], "rb");
    if (rom == NULL) {
        cerr << "Failed to open ROM" << endl;
        return false;
    }

    fseek(rom, 0, SEEK_END);
    long rom_size = ftell(rom);
    rewind(rom);


    char* rom_buffer = (char*) malloc(sizeof(char) * rom_size);
    if (rom_buffer == NULL) {
        cerr << "Failed to allocate memory for ROM!!" << endl;
        return false;
    }
    int result = fread(rom_buffer, sizeof(char), (int)rom_size, rom);
    if (result != rom_size) {
        cerr << "Failed to read ROM!!" << endl;
        return false;
    }

    if ((4096-512) > rom_size){
        for (int i = 0; i < rom_size; ++i) {
            memory[i + 512] = (uint8_t)rom_buffer[i]; 
        }
    }
    else {
        cerr << "ROM is too large!!!"<< endl;
        return false;
    }

    fclose(rom);
	free(rom_buffer);

     if ( SDL_Init(SDL_INIT_EVERYTHING) < 0 ) {
        cout<<"SDL could not initialize! SDL_Error: %s\n"<< SDL_GetError()<<endl;
        exit(1);
    }

    window = SDL_CreateWindow(
            "CHIP 8 Emulator by KEVAL, NELSON, VINCENT",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            width, height, SDL_WINDOW_SHOWN
    );


    if (window == NULL){
        cout<<"Window could not be created! SDL_Error: "<<SDL_GetError()<<endl;
        exit(2);
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderSetLogicalSize(renderer, width, height);
    SDL_Texture* sdlTexture = SDL_CreateTexture(renderer,
                SDL_PIXELFORMAT_ARGB8888,
                SDL_TEXTUREACCESS_STREAMING,
    64, 32);

while (true) {
        execute();


        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) exit(0);

            // Process keydown events
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE)
                    exit(0);

                for (int i = 0; i < 16; ++i) {
                    if (e.key.keysym.sym == keymap[i]) {
                        key[i] = 1;
                    }
                }
            }
            // Process keyup events
            if (e.type == SDL_KEYUP) {
                for (int i = 0; i < 16; ++i) {
                    if (e.key.keysym.sym == keymap[i]) {
                        key[i] = 0;
                    }
                }
            }
        }

        if (dflag) {
            dflag = false;

            // Store pixels in temp
            for (int i = 0; i < 2048; ++i) {
                uint8_t pixel = pix[i];
                temp[i] = (0x00FFFFFF * pixel) | 0xFF000000;  
            }
            // Update SDL texture
            SDL_UpdateTexture(sdlTexture, NULL, temp, 64 * sizeof(Uint32));
            // Clear screen and render
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }
        std::this_thread::sleep_for(std::chrono::microseconds(1200));

}

	return 0;   
}
