#include<sys/types.h>
#include<sys/socket.h>
#include<sys/ioctl.h>
#include<net/if.h>
#include<net/ethernet.h>
#include<net/if_arp.h>
#include<netinet/in.h>
#include<string.h>
#include<stdio.h>

#define ETH "eth0"
#define RESPUESTA "Respuesta: La IP %s pertenece a %02X:%02X:%02X:%02X:%02X:%02X\n"
#define OTRA "%02X:%02X:%02X:%02X:%02X:%02X quiere saber que pedo con %02X:%02X:%02X:%02X\n"

struct msgARP{
	unsigned char destinoEthernet[6];
	unsigned char origenEthernet[6];
	unsigned short tipoEthernet;
	unsigned short tipoHardware;
	unsigned short protocolo;
	unsigned char longitudMAC;
	unsigned char longitudRed;
	unsigned short tipoARP;
	unsigned char origenMAC[6];
	unsigned char origenIP[4];
	unsigned char destinoMAC[6];
	unsigned char destinoIP[4];
};

int main(int argc, char *argv[]){
	struct msgARP msg;
	struct ifreq ifr;
	struct sockaddr sa;
	int s, optval, n;
	unsigned char ip[4];

		if((s = socket(AF_INET, SOCK_PACKET, htons(ETH_P_ARP))) < 0)
		{
			perror("ERROR al abrir socket");
			return -1;
		}

	if(setsockopt(s, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) < 0) 
		{
			perror("ERROR en la funcion setsockopt");
			return -1;
		}

//Llenado de paquete
/*

	strcpy(ifr.ifr_name, ETH);
	if(ioctl(s, SIOCGIFHWADDR, &ifr) < 0)
		{
			perror("ERROR al obtener MAC origen");
			return -1;
		}
	bcopy(&ifr.ifr_hwaddr.sa_data, &msg.origenMAC, 6);
	bcopy(&ifr.ifr_hwaddr.sa_data, &msg.origenEthernet, 6);

	if(ioctl(s, SIOCGIFADDR, &ifr) < 0)
		{
			perror("ERROR al obtener IP origen");
			return -1;
		}

	bcopy(&ifr.ifr_hwaddr.sa_data[2], &msg.origenIP, 4);
	memset(&msg.destinoEthernet, 0xff, 6);
	msg.tipoEthernet = htons(ETH_P_ARP);
	msg.tipoHardware = htons(ARPHRD_ETHER);
	msg.protocolo = htons(ETH_P_IP);
	msg.longitudMAC = 6;
	msg.longitudRed = 4;
	msg.tipoARP = htons(ARPOP_REQUEST);
	bzero(&msg.destinoMAC, 6);
	
	inet_aton(argv[1], msg.destinoIP);
	strncpy(ip, msg.destinoIP, 4);
	bzero(&sa, sizeof(sa));
	strcpy(sa.sa_data, ETH);
	
//Termina llenado del paquete

//Envio del paquete

	if(sendto(s, &msg, sizeof(msg), 0, (struct sockaddr *) &sa, sizeof(sa)) < 0)
		{
			perror("ERROR al enviar");
			return -1;
		}
	printf("\nPaquete enviado\n");

*/

//Recepcion

	do{
		bzero(&sa, sizeof(sa));
		bzero(&msg, sizeof(msg));
		n = sizeof(sa);
		if(recvfrom(s, &msg, sizeof(msg), 0, (struct sockaddr *) &sa, &n) < 0)
			{
				perror("ERROR al recibir");
				return -1;
			}
		if(ntohs(msg.tipoARP) == ARPOP_REQUEST)
			{
				fprintf(stdout, OTRA,
					msg.origenMAC[0], msg.origenMAC[1], msg.origenMAC[2], msg.origenMAC[3],
					msg.origenMAC[4], msg.origenMAC[5],
					msg.destinoIP[0], msg.destinoIP[1], msg.destinoIP[2], msg.destinoIP[3]
						); 
			}
		}while(1);

	close(s);
}
