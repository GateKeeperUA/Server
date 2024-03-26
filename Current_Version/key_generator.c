#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define numKeys 300
#define keyLen 128 

int main() {
    char random, last;
    FILE *file = fopen("Encryption_keys.txt", "w");
    FILE *file_stm = fopen("Encryption_keys_stm.txt", "w");
    srand((unsigned)time(NULL));

    for(int i=0;i<numKeys;i++) {
        for(int j=0;j<=keyLen+1;j++) {
            if(j==0 || j==keyLen) {
                random = '"';
                fputc(random, file_stm);
            }
            else if(j==keyLen+1 && i<numKeys-2) {
                random = ',';
                fputc(random, file_stm);
            }
            else if(i<numKeys && j<keyLen){
                do {
                    random = (char)rand()%94+33;
                }
                while(random>='0' && random<='9' || random>='a' && random<='z' || random=='"' || random==last || (last=='/' && random=='*') || (last=='*' && random=='/') || random==92);
                last = random;
                fputc(random, file);        
                fputc(random, file_stm);  
            }
        }
        if(i<numKeys-1){
            fputc('\n',file_stm);
            fputc('\n',file);
        }  
    }

    fclose(file);
    fclose(file_stm);


    return 0;
}