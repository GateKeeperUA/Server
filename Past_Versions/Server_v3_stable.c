// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
   
#define PORT    5005
#define keyLen  128
#define numkeys 10
#define MaxClients 200

volatile char* key[numkeys];
volatile char message_cifer[keyLen];
   
struct clients{
    char IP[15];
    int IPsize;
    int counter;
};

volatile struct clients client[MaxClients]={};

void Initialize(){
    key[0] = "GgG44lIj@ic+*oKh)rdasqRFQi#_v9(3XP?8VYEm-V-RlV8FumCs!_S(a5pCOnGOb5z0MpT*)j%_q*UKZe!{(eh12J(Z()nT0tyS8Pxm5=*TTJGCo!TC)9)MzwwWU?L";
    key[1] = ")sNti9f@F3r3]}A3IrF[gu!ewCDml=mwKyw38ZpqC{Je}r*8wwHdKd97!bZxMoN1AcBU#&e$qIx&SWU*vl%61A*N0u0r%6MHF!67ti0nhVxq_Ay%j{{H=p!Gi_IXJ5b";
    key[2] = "68]VsE[qKpJA?lq%?d=SfG=50ZG=_mOkiy3]7AtGu+(dX6xuXRLzfrG}_1SX7hBYh}=f!YRc$paIv1jujn[tV3cP-bleN}*(wp2G5N4meH-S+etL)kv1*4nS!I2[CZ)";
    key[3] = "3DdOqyJ@(foC=neBNMfQ%G0n}Br*)cbCQoH)1MvnAGu+huvGufj?@vPpl0TnnkNTltsfaLv*iy36L&EYLIu3*X*6QJ]qi{!J{0UhhaFVnTi}Z*D2Cfz+WR3%P6Bm!sm";
    key[4] = "@z}iJZ}=2k#yYNNii%qitHbiBGaeg5ifU[DpeUuRuoJwX#DSh*BFt)S#=ejS$6*)htt0&KL5+-Y0p3=+=SKC%wmhOG&Vid}@+fNboRKe3!hw1i_niLBxLlzB3EYsCOZ";
    key[5] = "CWrqCk#H9FsQAYi@hNGS=kab**7D{W7OtSroFA}13ar*k0O?P]UVm$dP6CHPYZEE2hvt+_D9jG+E=]vS$1DoDJAQ-tgmzOWRhjuQ_A4Z=iuWXkP{r1#6%(*t_}*P_jW";
    key[6] = "NJ8)Fart=-oESNnX?D0vA?AwOC3f=M=i$2v)*7I7xm+-]w56ydtei]b-[YG$KpNTqOVoRF?el[#OcNJ@]Qp]&9$Ve_(*7-gODS&QOoX$OdEKIXPY_@h3ot{DrBY1Fz[";
    key[7] = "u4VNE([p=v]RQKGI3=3rRm(!&5r&PTCCXN0jcCP7=N-[5DM&#Egs6-8gGGLwd@)W*Vu}mP0r}Pk@_sXsVMBl8G{(3X?1@XEjkbmlS5[FBN524DhG${fCaNJ(){xmMh)";
    key[8] = "=8j3GxpW3AWfz@SBRBAfd}GWHm@414c)87Vz}KHzJjtO7EwH*x4lb$XdGf=p[xAdJ4s+LBCWEpaw(i!3WVT9Yt96zR3x}0gqb)G9OUJd*(&N41-+9vbcXQtsg7E3)-l";
    key[9] = "$+n*fdf8%Ex&n#csB3r}]]RP{XorkwiQak&uFcOqvzCIHV@Bj6@5ua$xI]t!efmZlYjzAxi?pXy#Ll?077qjD9F(d_ESu4mQuF}6vw)S9ie(H4Ed%[C}q3dk-tB)+]$";

    for(int i=0;i<MaxClients;i++) {client[i].IPsize = 0;}
}
   
void fill_dummy(int start, char* data) {
    for(int i=start;i<keyLen-1;i++) {data[i] = 97+rand()%20;}
    data[keyLen-1]='\0';
}
   
void XORCipher(char* data, int cifer, int k) {
    if(cifer==1){
        for(int i=0;i<strlen(data);i++) {message_cifer[i]=data[i];}
        message_cifer[strlen(data)]='\0';
        fill_dummy(strlen(data)+1,(char*)message_cifer);
    }
    else {
        for(int i=0;i<keyLen;i++) {message_cifer[i]=data[i];}
    }

	for (int i=0;i<keyLen;++i) {
        message_cifer[i] = message_cifer[i] ^ key[client[k].counter][i];
        if(message_cifer[i]=='\0' && cifer==0) {break;}
	}

    if(client[k].counter==numkeys-1){client[k].counter = 0;}
    else {client[k].counter++;}
}

// Driver code
int main() {
    int sockfd;
    char buffer[keyLen];
    struct sockaddr_in servaddr, cliaddr;
    int len;    
    bool restart;
    char* message_init="_%&hqt6G+WuXa4oq*uISC?V20k{gpRgcE&#G_0A62rua7vEoc*2+JrZuHaW*ZSr!=LT=yVK)ef-)w5p[gjyI{emT4nk=C*%QKQ#[Tuk}HQ0){ISk#JYrxUJ8UO-m&xq";
    char* message_test = "A temperatura está a 25ºC!";

    Initialize();
       
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
       
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
       
    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = inet_addr("192.168.0.240");
    servaddr.sin_port = htons(PORT);
   
    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
            sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
       
    len = sizeof(cliaddr);  //len is value/result


    while(1){
        memset(buffer, 0, sizeof(buffer));
        recvfrom(sockfd, buffer, keyLen, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);
        if(strcmp(buffer,message_init)==0) {restart = true;}
        else {restart = false;}   
        
        for (int k=0;k<MaxClients;k++) {
            if(client[k].IPsize>0 && strcmp((const char*)client[k].IP,inet_ntoa(cliaddr.sin_addr))==0){
                if (restart == false) {
                    XORCipher(buffer,0,k);
                    printf("Cliente existente (nº%d) %s:%d -> %s\n", k, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port), message_cifer);
                    XORCipher((char*)message_test,1,k);  
                      
                }
                else {
                    client[k].counter = 0;
                    printf("Cliente existente (nº%d) %s:%d -> Deu entrada\n", k, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port));
                }
                goto exit;
            }
            else if(client[k].IPsize==0){
                client[k].IPsize=strlen(inet_ntoa(cliaddr.sin_addr));
                for(int i=0;i<client[k].IPsize;i++){
                        client[k].IP[i]=inet_ntoa(cliaddr.sin_addr)[i];
                }
                client[k].counter = 0;
                printf("Cliente novo (nº%d) %s:%d -> Deu entrada\n", k, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port));
                goto exit;
            }
        }
        exit:
        if(restart == false) {
            sendto(sockfd, (char*)message_cifer, keyLen, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
        }
        else {
            sendto(sockfd, (char*)message_init, keyLen, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);   
        }
    }
    return 0;
}
