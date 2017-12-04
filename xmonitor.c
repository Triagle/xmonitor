#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/extensions/Xrandr.h>
#include <X11/Xlib.h>

static int BUFFER_SIZE = 255;

/*
  Count the number of outputs available to the system.
 */
int count_outputs(Display *disp, Window win) {
  int out_count;
  XRRScreenResources *screen;
  screen = XRRGetScreenResources(disp, win);
  out_count = screen->noutput;
  XRRFreeScreenResources(screen);
  return out_count;
}

/*
  Initialize output vector to give all elements a value of 1 if the output is connected and 0 otherwise.
 */
void initialize_configuration(Display *disp, Window win, int* outputs) {
  XRRScreenResources *screen;
  screen = XRRGetScreenResources(disp, win);
  XRROutputInfo *info;
  for (int i = 0; i < screen->noutput; i++) {
    info = XRRGetOutputInfo(disp, screen, screen->outputs[i]);
    if (info->connection == RR_Connected) {
      outputs[i] = 1;
    } else {
      outputs[i] = 0;
    }
    XRRFreeOutputInfo(info);
  }
  XRRFreeScreenResources(screen);
}

/*
  Dynamically grow string buffer if needed to accommodate more string storage.
 */
char* grow_string(char *str, int *buf_size, int end) {
  if (end < *buf_size) {
    return str;
  } else {
    int new_buf_size = *buf_size;
    while (new_buf_size < *buf_size + end) {
      new_buf_size *= 2;
    }
    char *new_str = calloc(new_buf_size, sizeof(char));
    memset(new_str, '\0', new_buf_size);
    memcpy(new_str, str, *buf_size);
    *buf_size = new_buf_size;
    free(str);
    return new_str;
  }
}

/*
  Respond to a display configuration update by printing any newly connected/disconnected devices.
 */
void print_display_configuration(Display *disp, Window win, int *outputs) {
  XRRScreenResources *screen;
  screen = XRRGetScreenResources(disp, win);
  XRROutputInfo *info;
  char *configuration = calloc(BUFFER_SIZE, sizeof(char));
  memset(configuration, '\0', BUFFER_SIZE);
  int buf_size = BUFFER_SIZE;
  int end = 0;
  for (int i = 0; i < screen->noutput; i++) {
    info = XRRGetOutputInfo(disp, screen, screen->outputs[i]);
    if (info->connection == RR_Connected) {
      // Just a connected device
      end += strlen(info->name) + 1;
      configuration = grow_string(configuration, &buf_size, end);
      strncat(configuration, info->name, strlen(info->name));
      configuration[end - 1] = ':';
      if (outputs[i] == 0) {
        // Newly connected device
        printf("A:%s\n", info->name);
      }
      outputs[i] = 1;
    } else if (info->connection == RR_Disconnected && outputs[i] == 1) {
      // Disconnected device
      outputs[i] = 0;
      printf("R:%s\n", info->name);
    }
    XRRFreeOutputInfo(info);
  }
  if (end > 0) {
    configuration[end - 1] = '\0';
    printf("C:%s\n", configuration);
  }
  free(configuration);
  fflush(stdout);
  XRRFreeScreenResources(screen);
}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  Display *disp;
  Window w;
  int screen;
  int rr_event_base, rr_error_base;
  disp = XOpenDisplay(NULL);
  screen = DefaultScreen(disp);
  w = XRootWindow(disp, screen);
  XRRQueryExtension(disp, &rr_event_base, &rr_error_base);
  XRRSelectInput(disp, w, RRScreenChangeNotifyMask);
  int output_count = count_outputs(disp, w);
  int *outputs = calloc(output_count, sizeof(int));
  initialize_configuration(disp, w, outputs);
  while (1) {
    XEvent event;
    XNextEvent(disp, &event);
    if (event.type == rr_event_base + RRScreenChangeNotify) {
      print_display_configuration(disp, w, outputs);
    }
  }
  free(outputs);
  XCloseDisplay(disp);
  return 0;
}
