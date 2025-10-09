#include "sp_manager.h"
#include <libserialport.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char open_port(struct sp_port *port, enum sp_mode flags) {
  if (sp_open(port, flags) != SP_OK) {
    return 0;
  }
  return 1;
}

unsigned char close_port(struct sp_port *port) {
  if (sp_close(port) != SP_OK) {
    return 0;
  }
  return 1;
}

char *get_port_name(struct sp_port *port) {
  char *name = sp_get_port_name(port);
  if (name == NULL) {
    return NULL;
  }
  return name;
}

char *get_port_description(struct sp_port *port) {
  char *description = sp_get_port_description(port);
  if (description == NULL) {
    return NULL;
  }
  return description;
}

enum sp_return get_ports(struct sp_port **ports) {
  int count = sp_list_ports(&ports);
  if (count < 0) {
    return SP_ERR_FAIL;
  }
  return SP_OK;
}

unsigned char writeData(const unsigned char *data, size_t size) {}
