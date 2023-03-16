#include <stdint.h>

extern "C" void plt_sound_init(uint32_t bufferlen);
extern "C" void plt_sound_restart(void);
extern "C" void plt_sound_pause(void);
extern "C" int32_t plt_sound_buffer_free(void);
extern "C" void plt_sound_buffer_play(uint32_t samplerate, const char *buffer, uint32_t length);

void plt_sound_init(uint32_t bufferlen)
{}

void plt_sound_restart(void)
{}

void plt_sound_pause(void)
{}

int32_t plt_sound_buffer_free(void)
{
	return 0;
}

void plt_sound_buffer_play(uint32_t samplerate, const char *buffer, uint32_t length)
{}
