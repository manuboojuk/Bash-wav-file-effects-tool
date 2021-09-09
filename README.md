# Bash-wav-file-effects-tool
A bash tool written in C to apply audio effects to .wav files. Made as a project to learn about bash scripting, C programming (fileIO, string manipulations, pointers, memory management, etc), piping, endianness, WAV file formats and audio file formats in general (mono, stereo audio files). 

This works for both single channel and dual channel (stereo) wav files.

The program takes a hexdump of a wav file from stdin and outputs a hexdump with the effects applied so you would have to convert the hexdump back to a binary file to get the actual audio file with the effects applied. Check the 'Effects description and usage' section for more specific instructions on how to run the program. 

There are also two six-second, dual channel wav files included in this repo to play around with.

## Effects description and usage
There are three effects:
- Fade-in:      audio fades in from the beginning                                                                 (use '-fin' to apply fade-in)
- Fade-out:     audio fades out from the end                                                                      (use '-fout' to apply the fade-out)
- Pan:          audio pans from left audio channel to right audio channel, only works for dual channel audio      (use '-pan' to apply the pan)

All effects are applied for a specified duration.

Note this program only takes the input file in the form of a hexdump (using `xxd -g1`) from stdin.

Usage: `xxd -g1 [INPUT_WAV_FILE] | ./a.out -[COMMAND] [EFFECT_TIME_IN_MILLISECONDS]`

Examples: 
- `xxd -g1 rain.wav | ./a.out -fout 1000`
- `xxd -g1 water.wav | ./a.out -fin 550`
- `xxd -g1 water.wav | ./a.out -pan 3000`

## Walkthrough to actually create files with effects applied
- First compile effects.c using gcc: `gcc effects.c`
- Then use xxd with the -g1 flag to make a hexdump and pipe it into the program then pipe the output into xxd with the -r flag to revert the hexdump back to a binary and finally pipe the output into a new wav file: `xxd -g1 rain.wav | ./a.out -fin 2000 | xxd -r > rainShortFade.wav`
- `rainShortFade.wav` should be the new file created with the specified audio effect applied for the specified time 

