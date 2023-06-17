/*
This file is part of Chip-8 Emulator.

Chip-8 Emulator is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

Chip-8 Emulator is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
Chip-8. If not, see <https://www.gnu.org/licenses/>.
*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct CPU {
  uint8_t memory[4096];
  uint8_t display[64 * 32];
  uint16_t PC;
  uint16_t I;
  uint16_t stack[16];
  uint16_t sp;
  uint8_t delay_timer;
  uint8_t sound_timer;
  uint8_t v[16];
  uint8_t keyboard[16];
} CPU;

uint8_t fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;

void initCPU(CPU *cpu);
int loadROM(char *file_name, CPU *cpu);
uint16_t fetch(CPU *cpu);
void decode(CPU *cpu);
void cleanupSDL();
void render();

void initCPU(CPU *cpu) {
  memset(cpu->memory, 0, 4096);
  memset(cpu->display, 0, 2048);
  memset(cpu->stack, 0, 16);
  memset(cpu->v, 0, 16);
  memset(cpu->keyboard, 0, 16);
  memcpy(cpu->memory, fontset, 80 * sizeof(uint8_t));

  cpu->PC = 0x200;
  cpu->I = 0;
  cpu->sp = 0;
  cpu->delay_timer = 0;
  cpu->sound_timer = 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "info: Usage: ./chip-8 <rom_file>\n");
    return 1;
  }

  CPU cpu;
  initCPU(&cpu);

  loadROM(argv[1], &cpu);

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
    SDL_Log("Unable to initialize SDL: %s\n", SDL_GetError());
    return 1;
  }

  window = SDL_CreateWindow("Chip-8", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);

  renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  SDL_RenderSetLogicalSize(renderer, 64, 32);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                              SDL_TEXTUREACCESS_STREAMING, 64, 32);

  if (window == NULL) {
    SDL_Log("Could not create window: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Event event;
  bool running = true;

  while (running) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {

      case SDL_QUIT:
        running = false;
        break;

      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_1:
          cpu.keyboard[0] = 1;
          break;
        case SDLK_2:
          cpu.keyboard[1] = 1;
          break;
        case SDLK_3:
          cpu.keyboard[2] = 1;
          break;
        case SDLK_4:
          cpu.keyboard[3] = 1;
          break;
        case SDLK_q:
          cpu.keyboard[4] = 1;
          break;
        case SDLK_w:
          cpu.keyboard[5] = 1;
          break;
        case SDLK_e:
          cpu.keyboard[6] = 1;
          break;
        case SDLK_r:
          cpu.keyboard[7] = 1;
          break;
        case SDLK_a:
          cpu.keyboard[8] = 1;
          break;
        case SDLK_s:
          cpu.keyboard[9] = 1;
          break;
        case SDLK_d:
          cpu.keyboard[0xA] = 1;
          break;
        case SDLK_f:
          cpu.keyboard[0xB] = 1;
          break;
        case SDLK_z:
          cpu.keyboard[0xC] = 1;
          break;
        case SDLK_x:
          cpu.keyboard[0xD] = 1;
          break;
        case SDLK_c:
          cpu.keyboard[0xE] = 1;
          break;
        case SDLK_v:
          cpu.keyboard[0xF] = 1;
          break;
        }
        break;

      case SDL_KEYUP:
        switch (event.key.keysym.sym) {
        case SDLK_1:
          cpu.keyboard[0] = 0;
          break;
        case SDLK_2:
          cpu.keyboard[1] = 0;
          break;
        case SDLK_3:
          cpu.keyboard[2] = 0;
          break;
        case SDLK_4:
          cpu.keyboard[3] = 0;
          break;
        case SDLK_q:
          cpu.keyboard[4] = 0;
          break;
        case SDLK_w:
          cpu.keyboard[5] = 0;
          break;
        case SDLK_e:
          cpu.keyboard[6] = 0;
          break;
        case SDLK_r:
          cpu.keyboard[7] = 0;
          break;
        case SDLK_a:
          cpu.keyboard[8] = 0;
          break;
        case SDLK_s:
          cpu.keyboard[9] = 0;
          break;
        case SDLK_d:
          cpu.keyboard[0xA] = 0;
          break;
        case SDLK_f:
          cpu.keyboard[0xB] = 0;
          break;
        case SDLK_z:
          cpu.keyboard[0xC] = 0;
          break;
        case SDLK_x:
          cpu.keyboard[0xD] = 0;
          break;
        case SDLK_c:
          cpu.keyboard[0xE] = 0;
          break;
        case SDLK_v:
          cpu.keyboard[0xF] = 0;
          break;
        }
        break;
      }
    }
    decode(&cpu);
    render(&cpu);
  }

  cleanupSDL();
  return 0;
}

void render(CPU *cpu) {
  uint32_t pixels[2048];
  unsigned int x, y;

  memset(pixels, 0, (64 * 32) * 4);
  for (x = 0; x < 64; x++) {
    for (y = 0; y < 32; y++) {
      if (cpu->display[x + (y * 64)] == 1) {
        pixels[x + (y * 64)] = UINT32_MAX;
      }
    }
  }

  SDL_UpdateTexture(texture, NULL, pixels, 64 * sizeof(uint32_t));

  SDL_Rect position;
  position.x = 0;
  position.y = 0;
  position.w = 64;
  position.h = 32;
  SDL_RenderCopy(renderer, texture, NULL, &position);
  SDL_RenderPresent(renderer);
}

int loadROM(char *file_name, CPU *cpu) {
  FILE *file = fopen(file_name, "rb");

  if (file == NULL) {
    fprintf(stderr, "Invalid file name: %s\n", file_name);
    return 1;
  }

  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  rewind(file);

  fread(cpu->memory + 0x200, sizeof(uint16_t), size, file);
  fclose(file);
  return 0;
}

uint16_t fetch(CPU *cpu) {
  uint16_t opcode = (cpu->memory[cpu->PC] << 8) + cpu->memory[cpu->PC + 1];
  cpu->PC += 2;

  return opcode;
}

void decode(CPU *cpu) {
  uint16_t opcode = fetch(cpu);

  uint8_t X = (opcode >> 8) & 0x000F;
  uint8_t Y = (opcode >> 4) & 0x000F;
  uint8_t N = opcode & 0x000F;
  uint8_t NN = opcode & 0x00FF;
  uint16_t NNN = opcode & 0x0FFF;

  switch (opcode & 0xF000) {
  case 0x0000:
    switch (opcode & 0x000F) {
    case 0x0000:
      memset(cpu->display, 0, 2048);
      break;

    case 0x000E:
      cpu->sp--;
      cpu->PC = cpu->stack[cpu->sp];
      break;
    }
    break;

  case 0x1000:
    cpu->PC = NNN;
    break;

  case 0x2000:
    cpu->stack[cpu->sp] = cpu->PC;
    cpu->PC = NNN;
    cpu->sp++;
    break;

  case 0x3000:
    if (cpu->v[X] == NN) {
      cpu->PC += 2;
    }
    break;

  case 0x4000:
    if (cpu->v[X] != NN) {
      cpu->PC += 2;
    }
    break;

  case 0x5000:
    if (cpu->v[X] == cpu->v[Y]) {
      cpu->PC += 2;
    }
    break;

  case 0x6000:
    cpu->v[X] = NN;
    break;

  case 0x7000:
    cpu->v[X] += NN;
    break;

  case 0x8000:
    switch (opcode & 0x000F) {
    case 0x0000:
      cpu->v[X] = cpu->v[Y];
      break;

    case 0x0001:
      cpu->v[X] |= cpu->v[Y];
      break;

    case 0x0002:
      cpu->v[X] &= cpu->v[Y];
      break;

    case 0x0003:
      cpu->v[X] ^= cpu->v[Y];
      break;

    case 0x0004:;
      int sum;
      sum = cpu->v[X] + cpu->v[Y];

      if (sum > 255) {
        cpu->v[0xF] = 1;
      } else {
        cpu->v[0xF] = 0;
      }

      cpu->v[X] = sum & 0xFF;
      break;

    case 0x0005:
      if (cpu->v[X] > cpu->v[Y]) {
        cpu->v[0xF] = 1;
      } else {
        cpu->v[0xF] = 0;
      }
      cpu->v[X] -= cpu->v[Y];
      break;

    case 0x0006:
      cpu->v[X] = cpu->v[Y];
      cpu->v[0xF] = cpu->v[X] & 0xF000;
      cpu->v[X] >>= 1;
      break;

    case 0x0007:
      if (cpu->v[Y] > cpu->v[X]) {
        cpu->v[0xF] = 1;
      } else {
        cpu->v[0xF] = 0;
      }
      cpu->v[X] = cpu->v[Y] - cpu->v[X];
      break;

    case 0x000E:
      cpu->v[X] = cpu->v[Y];
      cpu->v[0xF] = cpu->v[X] & 0xF000;
      cpu->v[X] <<= 1;
      break;
    }
    break;

  case 0x9000:
    if (cpu->v[X] != cpu->v[Y]) {
      cpu->PC += 2;
    }
    break;

  case 0xA000:
    cpu->I = NNN;
    break;

  case 0xB000:
    cpu->I += NNN + cpu->v[X];
    break;

  case 0xC000:
    cpu->v[X] = (rand() % 255) + NN;
    break;

  case 0xD000:;
    uint8_t x = cpu->v[X] % 64;
    uint8_t y = cpu->v[Y] % 32;
    uint8_t sprite = 0;
    cpu->v[0xF] = 0;

    for (int i = 0; i < N; i++) {
      sprite = cpu->memory[cpu->I + i];
      for (int j = 0; j < 8; j++) {
        if ((sprite & (0x80 >> j)) != 0) {
          if (cpu->display[x + j + (y + i) * 64] == 1) {
            cpu->v[0xF] = 1;
          }
          cpu->display[x + j + (y + i) * 64] ^= 1;
        }
      }
    }
    break;

  case 0xE000:

    switch (opcode & 0x00FF) {

    case 0x009E:
      if (cpu->keyboard[cpu->v[X]]) {
        cpu->PC += 2;
      }
      break;
    case 0x00A1:
      if (cpu->keyboard[cpu->v[X]] == 0) {
        cpu->PC += 2;
      }
      break;
    }
    break;

  case 0xF000:

    switch (opcode & 0x00FF) {

    case 0x0007:
      cpu->v[X] = cpu->delay_timer;
      break;

    case 0x0015:
      cpu->delay_timer = cpu->v[X];
      break;

    case 0x0018:
      cpu->sound_timer = cpu->v[X];
      break;

    case 0x001E:
      cpu->I += cpu->v[X];

      if (cpu->I > 1000) {
        cpu->v[0xF] = 1;
      }
      break;

    case 0x000A:;
      int key_pressed = 0;

      for (int i = 0; i < 16; i++) {
        if (cpu->keyboard[i]) {
          key_pressed = 1;
          cpu->v[X] = i;
        }
      }

      if (key_pressed == 0) {
        cpu->PC -= 2;
      }
      break;

    case 0x0029:
      cpu->I = cpu->v[X] * 5;
      break;

    case 0x0033:;
      int vX = cpu->v[X];
      cpu->memory[cpu->I] = (vX - (vX % 100)) / 100;
      vX -= cpu->memory[cpu->I] * 100;
      cpu->memory[cpu->I + 1] = (vX - (vX % 10)) / 10;
      vX -= cpu->memory[cpu->I + 1] * 10;
      cpu->memory[cpu->I + 2] = vX;
      break;

    case 0x0055:
      for (int i = 0; i <= X; i++) {
        cpu->memory[cpu->I + i] = cpu->v[i];
      }
      break;

    case 0x0065:
      for (int i = 0; i <= X; i++) {
        cpu->v[i] = cpu->memory[cpu->I + i];
      }
      break;
    }
    break;
  }

  if (cpu->delay_timer > 0)
    cpu->delay_timer--;
}

void cleanupSDL() {
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
