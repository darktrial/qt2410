#ifndef __MAC_H
#define __MAC_H

int board_eth_init(void);
int board_eth_lnk_stat(void);
int board_eth_send(unsigned char *data, unsigned int len);
int board_eth_rcv(unsigned char *data, unsigned int *len);
int board_eth_get_addr(unsigned char *addr);

#endif /* __MAC_H */
