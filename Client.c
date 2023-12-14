// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
   
#define PORT     5005
#define keyLen   128
#define numkeys  10

volatile int counter;
volatile char* key[numkeys];
volatile char message_cifer[128];


void Keys(){
    counter = 0;
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
    for(int i=start;i<keyLen;i++) {data[i] = rand()%35;}
}
   
void XORCipher(char* data, int cifer) {
    if(cifer==1){
        message_cifer[strlen(data)]='\0';
        fill_dummy(strlen(data)+1,(char*)message_cifer);
    }

	for (int i = 0; i < keyLen; ++i) {
        message_cifer[i] = data[i] ^ key[counter][i];        
        if(message_cifer[i]=='\0' && cifer==0) {break;}
	}

    if(counter==numkeys-1){counter = 0;}
    else {counter++;}    
}


// Driver code
int main() {
    int sockfd;
    char buffer[keyLen];
    char message[keyLen];
    struct sockaddr_in servaddr;
    int len;

    Keys();

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
   
    memset(&servaddr, 0, sizeof(servaddr));
       
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("192.168.0.240");

    char* message_init="1Obf0coLn8aacd3WQOzmKYlKhTtuGwzj3qgrPRwLs5gXIGsEdlWmp591TFXS22kFgLGuvenPOHp60R5OT7YxWQgwzt1ha5AUKogKV0Bepo3L1LhsjayFTYxkmfH6ZUm";
    sendto(sockfd, message_init, keyLen, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    recvfrom(sockfd, buffer, keyLen, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
    if(strcmp(message_init,buffer)==0) { printf("Conexão estabelecida\n");}
    
    while(1){
        memset(message, 0, sizeof(message));
        memset(buffer, 0, sizeof(buffer));
        printf("\nMessage to send: ");
        fgets(message, keyLen, stdin);

        for(int i=0;i<strlen(message);i++){
            if(message[i]=='\n') {message[i]='\0';}
        }

        XORCipher(message,1);
        
        sendto(sockfd, (char*)message_cifer, keyLen, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
        
        recvfrom(sockfd, buffer, keyLen, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);

        XORCipher(buffer,0);

        printf("Server: %s\n", message_cifer);
    }

    close(sockfd);
    return 0;
}
