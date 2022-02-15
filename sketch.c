// Basic program skeleton for a Sketch File (.sk) Viewer
#include "displayfull.h"
#include "sketch.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Allocate memory for a drawing state and initialise it
state *newState() {

    state *s = malloc(sizeof(state));
    s->x=0;
    s->y=0;
    s->tx=0;
    s->ty=0;
    s->tool=LINE;
    s->start=0;
    s->data=0;
    s->end=false;

    return s;
  }

// Release all memory associated with the drawing state
void freeState(state *s) {

    free(s);
  }

// Extract an opcode from a byte (two most significant bits).
int getOpcode(byte b) {

    char opcode = b>>6;
    return opcode;
  }

// Extract an operand (-32..31) from the rightmost 6 bits of a byte.
int getOperand(byte b) {

    char operand = b<<2;
    operand = operand>>2;
    return operand;
  }

// Execute the next byte of the command sequence.
void obey(display *d, state *s, byte op) {

    int opcode = getOpcode(op);
    int operand = getOperand(op);

    if(opcode == TOOL) {

        if(operand == NONE) s->tool = NONE;
        else if(operand == LINE) s->tool = LINE;
        else if(operand == BLOCK) s->tool = BLOCK;
        else if(operand == COLOUR){

            unsigned int rgba = s->data;
            colour(d, rgba);
          }
        else if(operand == TARGETX) s->tx = (s->data);
        else if(operand == TARGETY) s->ty = (s->data);
        else if(operand == SHOW) show(d);
        else if(operand == PAUSE) pause(d, s->data);
        else if(operand == NEXTFRAME) s->end = true;

        s->data = 0;
      }
    else if(opcode == DX) s->tx = (s->tx + operand);
    else if(opcode == DY) {

        s->ty = (s->ty + operand);

        int width = (s->tx)-(s->x);
        int height = (s->ty)-(s->y);

        if(s->tool == LINE){

            line(d, s->x, s->y, s->tx, s->ty);
          }
        else if(s->tool == BLOCK){

            block(d, s->x, s->y, width, height);
          }

        s->x = s->tx;
        s->y = s->ty;
      }
    else if(opcode == DATA) {

        s->data=(s->data<<6) | (operand&0x3F);
      }

    //keep track of the current position in the file
    s->start++;
  }

// Draw a frame of the sketch file. For basic and intermediate sketch files
// this means drawing the full sketch whenever this function is called.
// For advanced sketch files this means drawing the current frame whenever
// this function is called.
bool processSketch(display *d, void *data, const char pressedKey) {

      if (data == NULL) return (pressedKey == 27);
      state *s = (state*) data;

      //get the starting position in the file whenever processSketch is called
      unsigned int startlocation = s->start;

      char *filename = getName(d);
      FILE *in = fopen(filename, "rb");

      //set the current position in the sketch file to starting position
      fseek(in, startlocation, SEEK_SET);

      unsigned char ch = fgetc(in);

      while (! feof(in) && ! s->end) {

          obey(d, s, ch);
          ch = fgetc(in);
        }

      //reset start field in the drawing state if EndOfFile
      if(feof(in)) s->start = 0;

      show(d);
      fclose(in);
      s->x = 0;
      s->y = 0;
      s->tx = 0;
      s->ty = 0;
      s->tool = LINE;
      s->data = 0;
      s->end = false;

      return (pressedKey == 27);
}

// View a sketch file in a 200x200 pixel window given the filename
void view(char *filename) {
  display *d = newDisplay(filename, 200, 200);
  state *s = newState();
  run(d, s, processSketch);
  freeState(s);
  freeDisplay(d);
}

// Include a main function only if we are not testing (make sketch),
// otherwise use the main function of the test.c file (make test).
#ifndef TESTING
int main(int n, char *args[n]) {
  if (n != 2) { // return usage hint if not exactly one argument
    printf("Use ./sketch file\n");
    exit(1);
  } else view(args[1]); // otherwise view sketch file in argument
  return 0;
}
#endif
