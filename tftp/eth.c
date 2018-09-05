//#include "../../inc/config.h"
#include "skbuff.h"
#include "eth.h"
#include "mac.h"
#include "utility.h"
#include "sys.h"

extern void TransmitPacket(char *buffer,u_short len);


int eth_init(void)
{
	InitEthernet();
}

int eth_send(struct sk_buff *skb, unsigned char *dest_addr, unsigned short proto)
{
	struct ethhdr *eth_hdr;
	unsigned char local_eth_addr[ETH_ALEN];

	eth_get_addr(local_eth_addr);

	eth_hdr = (struct ethhdr *)skb_push(skb, ETH_HLEN);

	memcpy((unsigned char *)eth_hdr->h_dest, dest_addr, ETH_ALEN);
	memcpy((unsigned char *)eth_hdr->h_source, local_eth_addr, ETH_ALEN);
	eth_hdr->h_proto = htons(proto);

	TransmitPacket((char *)skb->data, skb->len);
}

int eth_get_addr(unsigned char *addr)
{
	return board_eth_get_addr(addr);
}

void eth_skb_reserve(struct sk_buff *skb)
{
	skb_reserve(skb, ETH_HLEN);
}

