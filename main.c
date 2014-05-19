#include<sys/types.h>
#include<sys/socket.h>
#include<sys/ioctl.h>
#include<net/if.h>
#include<net/ethernet.h>
#include<net/if_arp.h>
#include<netinet/in.h>
#include<string.h>
#include<stdio.h>

#include<mysql/mysql.h>

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
	
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	
	char *server = "localhost";
	char *user = "root";
	char *password = "//lsoazules:"; 
	char *database = "redes";

	conn = mysql_init(NULL);
	

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
printf("\nestamos listos\n");
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
printf("relax");

				if(!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))	
					fprintf(stderr, "ERROR para conectarse a mysql", mysql_error(conn));
				else
				{
					//Construir query
					char *query = "select * from tablaARP";

					if(mysql_query(conn, query))
						fprintf(stderr, "\nNEL con el query\n", mysql_error(conn));			
					else
					{
						res = mysql_use_result(conn);
						while(row = mysql_fetch_row(res) != NULL)
							printf("%s\n", row[0]);
						mysql_free_result(res);
					}
				}						
					
				mysql_close(conn);
				
				fprintf(stdout, OTRA,
					msg.origenMAC[0], msg.origenMAC[1], msg.origenMAC[2], msg.origenMAC[3],
					msg.origenMAC[4], msg.origenMAC[5],
					msg.destinoIP[0], msg.destinoIP[1], msg.destinoIP[2], msg.destinoIP[3]
						); 
			}
		}while(1);
printf("\nque pedo\n");
	close(s);
}
