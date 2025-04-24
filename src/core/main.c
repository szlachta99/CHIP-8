#include "CHIP8.h"
#include <string.h>
#define __USE_MISC
#include <math.h>
#undef __USE_MISC
#include <stdlib.h>
#include <stdio.h>

#include <SDL3/SDL.h>

#define CPU_FREQ 500       // CPU frequency in Hz
#define BEEP_FREQUENCY 350 // Frequency of beep sound in Hz
#define BEEP_AMPLITUDE 128 // Amplitude of beep sound

typedef struct {
    int sample_rate;
    int frequency;
    int phase;
    bool is_beeping;
} BeepData;

typedef struct {
    CHIP8 chip8;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_AudioDeviceID audio_device;
    SDL_AudioStream* audio_stream;
    SDL_FRect pixel_buffer[64 * 32];
    BeepData beep_data;
    bool running;
    int pixel_size;
    SDL_Color color;
    int screen_width;
    int screen_height;
} App;


void init(App* app, const char* rom);
void draw(App* app);
void update_kbd(CHIP8* chip8);
void cleanup(App* app);

int main(int argc, char* argv[]){
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <ROM file>\n", argv[0]);
        return 1;
    }
    App app = {0};

    init(&app, argv[1]);

    LoadROM(&app.chip8, argv[1]);
    SDL_Time start, end;
    while(app.running) {
        start = SDL_GetTicks();
        Instruction instruction = FetchInstruction(&app.chip8);
        ExecuteInstruction(&app.chip8, instruction);
        // Update timers and other logic here
        if(app.chip8.screen_changed) {
            app.chip8.screen_changed = 0;
            draw(&app);
        }
        if(app.chip8.delay_timer > 0) {
            app.chip8.delay_timer--;
        }

        if(app.chip8.sound_timer > 0) {
            if (!app.beep_data.is_beeping) {
                app.beep_data.is_beeping = true;
            }
            app.chip8.sound_timer--;
        } else {
            if (app.beep_data.is_beeping) {
                app.beep_data.is_beeping = false;
            }
        }
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                app.running = false;
            }
        }
        update_kbd(&app.chip8);
        end = SDL_GetTicks();
        int delay = (1.0 / CPU_FREQ) * 1000 - (end - start);
        if (delay > 0) {
            SDL_Delay(delay);
        }
    }
    cleanup(&app);
    return 0;
}

static void fill_callback(void *userdata, SDL_AudioStream *stream, int approx_request, int _) {
    BeepData *beep = (BeepData *)userdata;

    uint8_t *buffer = (uint8_t *)malloc(approx_request);
    if (!buffer) return;
    if(!beep->is_beeping) {
        memset(buffer, 128, approx_request);
        uint8_t tone = sin((2 * M_PI * beep->phase) / (beep->sample_rate / beep->frequency)) * BEEP_AMPLITUDE + 128;
        int i = 0;
        while(abs(tone - 128) > 10) {
            beep->phase = (beep->phase + 1) % (beep->sample_rate / beep->frequency);
            tone = sin((2 * M_PI * beep->phase) / (beep->sample_rate / beep->frequency)) * BEEP_AMPLITUDE + 128;
            buffer[i] = tone;
            i++;
        }
        SDL_PutAudioStreamData(stream, buffer, approx_request);
        free(buffer);
        return;
    }
    for (int i = 0; i < approx_request; i++) {
        beep->phase = (beep->phase + 1) % (beep->sample_rate / beep->frequency);
        buffer[i] = sin((2 * M_PI * beep->phase) / (beep->sample_rate / beep->frequency)) * BEEP_AMPLITUDE + 128;
    }
    
    SDL_PutAudioStreamData(stream, buffer, approx_request);
    
    free(buffer);
}

void init(App* app, const char* rom)
{
    app->pixel_size = 10; // Size of each pixel in the window
    app->running = true;
    app->screen_width = 64 * app->pixel_size;
    app->screen_height = 32 * app->pixel_size;
    app->color.r = 255;
    app->color.g = 255;
    app->color.b = 255;
    app->color.a = 255;
    InitializeCHIP8(&app->chip8);


    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }


    // Set up the audio
    SDL_AudioSpec audio_spec = {
        .freq = 44100,
        .format = SDL_AUDIO_U8,
        .channels = 1,
    };

    app->audio_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audio_spec);
    if (!app->audio_device) {
        fprintf(stderr, "Failed to open audio device: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }
    
    // Initialize beep data
    app->beep_data.sample_rate = 44100;
    app->beep_data.frequency = BEEP_FREQUENCY;
    app->beep_data.phase = 0;
    app->beep_data.is_beeping = false;
    
    app->audio_stream = SDL_CreateAudioStream(&audio_spec, &audio_spec);
    if(!app->audio_stream) {
        fprintf(stderr, "Failed to create audio stream: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    if(!SDL_BindAudioStream(app->audio_device, app->audio_stream)) {
        fprintf(stderr, "Failed to bind audio stream: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }


    SDL_SetAudioStreamGetCallback(app->audio_stream, fill_callback, &app->beep_data);

    // Set up the video
    int offset = strlen(rom) - 1;
    for(; rom[offset] != '/' && offset != 0; offset--);
    char window_name[256];
    snprintf(window_name, sizeof(window_name), "CHIP-8 Emulator - %s", rom + offset + 1);
    
    SDL_CreateWindowAndRenderer(window_name, app->screen_width, app->screen_height, 0, &app->window, &app->renderer);
    if (!app->window || !app->renderer) {
        fprintf(stderr, "Could not create window or renderer: %s\n", SDL_GetError());
        SDL_CloseAudioDevice(app->audio_device);
        SDL_Quit();
        exit(1);
    }
}

void draw(App* app)
{
    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
    SDL_RenderClear(app->renderer);
    SDL_SetRenderDrawColor(app->renderer, app->color.r, app->color.g, app->color.b, app->color.a);
    int count = 0;
    for (int row = 0; row < 32; row++) {
        for (int col = 0; col < 64; col++) {
            uint8_t pixel = (app->chip8.screen[row] >> (63 - col)) & 1;
            if (pixel) {
                app->pixel_buffer[count++] = (SDL_FRect){.x = col * app->pixel_size, row * app->pixel_size, app->pixel_size, app->pixel_size};
            }
        }
    }
    if (count > 0) {
        SDL_RenderFillRects(app->renderer, app->pixel_buffer, count);
    }
    SDL_RenderPresent(app->renderer);
}

void update_kbd(CHIP8* chip8)
{
    memcpy(chip8->prev_keypad, chip8->keypad, sizeof(chip8->keypad)); // save last frame

    memset(chip8->keypad, 0, sizeof(chip8->keypad));

    const bool* keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_1]) chip8->keypad[0x0] = 1;
    if (keys[SDL_SCANCODE_2]) chip8->keypad[0x1] = 1;
    if (keys[SDL_SCANCODE_3]) chip8->keypad[0x2] = 1;
    if (keys[SDL_SCANCODE_4]) chip8->keypad[0x3] = 1;
    if (keys[SDL_SCANCODE_Q]) chip8->keypad[0x4] = 1;
    if (keys[SDL_SCANCODE_W]) chip8->keypad[0x5] = 1;
    if (keys[SDL_SCANCODE_E]) chip8->keypad[0x6] = 1;
    if (keys[SDL_SCANCODE_R]) chip8->keypad[0x7] = 1;
    if (keys[SDL_SCANCODE_A]) chip8->keypad[0x8] = 1;
    if (keys[SDL_SCANCODE_S]) chip8->keypad[0x9] = 1;
    if (keys[SDL_SCANCODE_D]) chip8->keypad[0xA] = 1;
    if (keys[SDL_SCANCODE_F]) chip8->keypad[0xB] = 1;
    if (keys[SDL_SCANCODE_Z]) chip8->keypad[0xC] = 1;
    if (keys[SDL_SCANCODE_X]) chip8->keypad[0xD] = 1;
    if (keys[SDL_SCANCODE_C]) chip8->keypad[0xE] = 1;
    if (keys[SDL_SCANCODE_V]) chip8->keypad[0xF] = 1;
}

void cleanup(App* app)
{
    SDL_CloseAudioDevice(app->audio_device);
    SDL_DestroyRenderer(app->renderer);
    SDL_DestroyWindow(app->window);
    SDL_Quit();
}