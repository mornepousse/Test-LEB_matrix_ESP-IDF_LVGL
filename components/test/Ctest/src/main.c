#include "main.h"
// #include "sp_manager.h"
#include <libserialport.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  struct sp_port *port;
  char *port_name = "/dev/ttyUSB0";
  sp_get_port_by_name(port_name, &port);
  sp_set_baudrate(port, 1500000);
  sp_set_bits(port, 8);
  sp_set_parity(port, SP_PARITY_NONE);
  sp_set_stopbits(port, 1);
  sp_set_flowcontrol(port, SP_FLOWCONTROL_NONE);
  printf("trying to open port %s\n", port_name);
  if (sp_open(port, 3)) {
    printf("connected");
  }
  printf("Hello, World!\n");
  return 0;
}
