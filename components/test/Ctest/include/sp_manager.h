#ifndef SP_MANAGER_H
#define SP_MANAGER_H
#include <libserialport.h>
unsigned char open_port(struct sp_port *port, enum sp_mode flags);
enum sp_return get_ports(struct sp_port **ports);

#endif // !#ifndef SP_MANAGER_H
