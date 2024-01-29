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

volatile char* key[numkeys];
volatile char message_cifer[keyLen];
   
struct clients{
    char IP[200];
    int IPsize;
    int counter;
};

volatile struct clients client[200]={};

void Keys(){
    key[0] = "=))*{+][=)){%&=?=*=$!!)]=%][_{}+?+*&$}%$[*+)]#_{_}=!{$[-}#&?#]{!}![)_#[%[?#@-![[{??]!#}+_)_$&!*(+#%?%!_(+#?[}_[*)+_)(=*?[-{-=+{_";
    key[1] = "=-_]_{_{(-%-@={[*}*=(=+[+$=}&}!!_&[[*=#$+[*_%=-}@%=[*@(+![-=#$)?($![$&&#}%)({---[]-?{&=}?(_@?#-=$=(-+$-)($+*%[=@#(**+{=#+?{()?)=";
    key[2] = "[%}!=_[#}&{&#$$&#=_@]?=]}?{?-#!+*=?[@?#-_{$?%{=#(-@}?{()$!??*_![$&+=)*-}&*--={![&%#+{[{@]!(%##_&{=@)($@#@*}${+&?{$+={]*)!@?)#&%*";
    key[3] = "+)$(=+=-*]}&))@[=)*[+=**[_}[?}?*@{-!-&_@=]*{%_@__@[](#_%=__]#-=[=($$[-%($=[@*-%(*%*-}*-*%%@$?$&%&((}--?}*-[*](!@$(](+(@{!#*?#-!!";
    key[4] = "![#)&$?$(@[&**}$]+*?+}!&?!(%+){])@@!#@@}*!#[[)}}#*)$%?)?+*=&)_*$[__$#@]#&{=)-]=?(%=_]*==[$_+*{{#&(?-_@_=+_+$-#$=}!*(_%)$]!#$$@++";
    key[5] = "({+]=%+${+$*=-=}%(?[})=[!%_+%{{-)==$-)#-!++(_-=}?+[-))@{[?!(?=&#(&*[(({{_[##[([!{}!{*_!?++)?!!_()!=_]*#=)@]{_-){_[$@[_)*=!{?@-!%";
    key[6] = "__@{]_]?+-&%=#-{!@*&${-&)]_@+?&(#*%##}]](-_{%{%(#]#-?}{!*)[#*__%@!&@?(-=}=}))!%(!##]%%[}))&#-@)(!=_)$$?(&-__@?)-(==?##)}_*_{}#}=";
    key[7] = "}+-_}{]=(#$})&&}}!)!&]_)=![%_[}_@![!_&-$&@=+#{}@}[@]_*+&$&_{&??*?$*=?]*=}!*}(@&_!#?-+(+-?]}&?-$&]$+([+#*#?$_@%!@-%(&*_)}-?=?$$[}";
    key[8] = "%!@*}-&#]$%__?$_}%%-#-%[$[*?$*-?-*%&+]+*{{!#-}=#!)&&)[_*%&$?}%!*?{##&_[?#{$#{)=*%?%=+*#]@@%(-%@[%+{{?]%%?{$)![)&__*#(@=!)_)(&@}*";
    key[9] = "(}=?%=[)=)&[#!$?#*{*]-?]!+[[!-))}$!(@%!]@]{=&=$%)_#]#})@}#?@&+(!={+[=[@&)=_+%[]*_@-&-]]](*+=]&{(!!$#%}%+-?%_-&{*!&[$@)%_()(-()*}";
}
   
void fill_dummy(int start, char* data) {
    for(int i=start;i<keyLen;i++) {data[i] =  rand()%35;}
}
   
void XORCipher(char* data, int cifer, int k) {
    if(cifer==1){
        message_cifer[strlen(data)]='\0';
        fill_dummy(strlen(data)+1,(char*)message_cifer);

    }

	for (int i = 0; i < keyLen; ++i) {
        message_cifer[i] = data[i] ^ key[client[k].counter][i];
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
    char* message_init="1Obf0coLn8aacd3WQOzmKYlKhTtuGwzj3qgrPRwLs5gXIGsEdlWmp591TFXS22kFgLGuvenPOHp60R5OT7YxWQgwzt1ha5AUKogKV0Bepo3L1LhsjayFTYxkmfH6ZUm";
    char* message_test = "A temperatura está a 25ºC!";

    Keys();
       
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

        for (int k=0;k<200;k++) {
            if(client[k].IPsize>0){
                for(int i=0;i<strlen(inet_ntoa(cliaddr.sin_addr));i++){
                    if(client[k].IP[i]!=inet_ntoa(cliaddr.sin_addr)[i]){
                        break;
                    }
                    if(i==strlen(inet_ntoa(cliaddr.sin_addr))-1){

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
                }
            }
            else if(client[k].IPsize==0){
                client[k].IPsize=strlen(inet_ntoa(cliaddr.sin_addr));
                for(int i=0;i<client[k].IPsize;i++){
                        client[k].IP[i]=inet_ntoa(cliaddr.sin_addr)[i];
                }
                if (restart == false) {
                    XORCipher(buffer,0,k);
                    printf("Cliente novo (nº%d) %s:%d -> %s\n", k, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port), message_cifer);
                    XORCipher((char*)message_test,1,k);  
                }
                else {
                    client[k].counter = 0;
                    printf("Cliente novo (nº%d) %s:%d -> Deu entrada\n", k, inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port));
                }
                break;
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
