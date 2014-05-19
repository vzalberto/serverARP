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
#define OTRA "%02X:%02X:%02X:%02X:%02X:%02X quiere saber que onda con %d.%d.%d.%d\n"
#define IP_TEMPLATE "%u.%u.%u.%u"
#define MAC_TEMPLATE "%02X:%02X:%02X:%02X:%02X:%02X"

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
//Recepcion
printf("\nQUE PASA\n");
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
					msg.destinoIP[0], msg.destinoIP[1], msg.destinoIP[2], msg.destinoIP[3]);
					
conn = mysql_init(NULL);
				if(!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))	
				{	
					fprintf(stderr, "ERROR para conectarse a mysql", mysql_error(conn));
					return -1;
				}
				else
				{
					//Construir query
					char query[80] = "select mac from tablaARP where ip like \'";
					char ip_str[20];
					char mac_str[20];
					sprintf(ip_str, "%u.%u.%u.%u", msg.destinoIP[0], msg.destinoIP[1], msg.destinoIP[2], msg.destinoIP[3]); 
					strcat(query, ip_str);	
					strcat(query, "\'");

					sprintf(mac_str, MAC_TEMPLATE, 
						msg.origenMAC[0], msg.origenMAC[1], msg.origenMAC[2], msg.origenMAC[3],
						msg.origenMAC[4], msg.origenMAC[5]
						); 
					if(mysql_query(conn, query))
						fprintf(stderr, "\nNEL con el query\n", mysql_error(conn));			
					else
					{
						res = mysql_use_result(conn);
						if((row = mysql_fetch_row(res)) != NULL)
						{
							printf("\n");
							printf("%s\n", row[0]);
							printf("%s\n", mac_str);
							if(strcmp(row[0], mac_str) == 0) 
								printf("\nIGUALES\n");
							else
								{
									printf("\nTe toca ARP gratuito, amigo\n");
									//sendARPGRATIS
								}	
						}
						else
						{
							printf("\nNo hay registro\n");
							//insertarRegistro
							char otraQuery[80];
							strcat(otraQuery, "insert into tablaARP (mac, ip) values ('");
							strcat(otraQuery, mac_str);
							strcat(otraQuery, "\',\'");
							strcat(otraQuery, ip_str);
							strcat(otraQuery, "\')");
							if(mysql_query(conn, otraQuery))
								fprintf(stderr, "\nNEL con el otro query\n", mysql_error(conn));	
							else
								printf("\nNuevo registro\n");
							
						}
						mysql_free_result(res);
					}
				}						
					
mysql_close(conn);
				
			}
		}while(1);
	close(s);
}
