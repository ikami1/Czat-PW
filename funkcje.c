#include "specyfikacja.h"

void Reset(){
  printf("%c[%dm ",0x1B,0); //przywrocenie naturalnych kolorow konsoli
}

void znajdz_polecenie(char linia[], char polecenie[]){
    int i=0, j=0, n=0;

    while(linia[i] == ' ' || linia[i] == '\n')
        i+=1;

    while(linia[i] != ' ' && linia[i] != '\n' && linia[i] != '\0' && linia[i] != '\t'){
        polecenie[j] = linia[i];
        i+=1;
        j += 1;
    }

    polecenie[j] = '\0';                //znak '\0' na koniec

    while(linia[i-1] != '\0'){          //i-1 zeby skopiowa³o '\0'
        linia[n] = linia[i];
        n+=1;
        i+=1;
    }
}

int znajdzIndeksPokoju(char *tablica[MAX_USERS+1][MAX_GROUPS], char nazwa[]){
    int i=0,n;
    for(i=0; i<MAX_GROUPS; i++)
        if(!strcmp(tablica[0][i],nazwa)){
            n = i;
            break;
        }
    return n;
}

int znajdzIndeks(char *tablica[2][MAX_USERS], char nickname[]){
    int i=0,n;
    for(i=0; i<MAX_USERS; i++)
        if(!strcmp(tablica[0][i],nickname)){
            n = i;
            break;
        }
    return n;
}


char* splitToWho(char polecenie[]){
    int i=1;
    while(polecenie[i-1] != '\0'){
        polecenie[i-1] = polecenie[i];
        i+=1;
    }
    return polecenie;
}

int czyZajety(char *tablica[2][MAX_USERS], char nickname[]){
    int i=0;
    for(i=0; i<MAX_USERS; i++)
        if(!strcmp(tablica[0][i],nickname)){
            return 1;
        }
    if(!strcmp(nickname,"serwer"))              //pod ta nazwa jest serwer
        return 1;
    return 0;
}

int istniejePokoj(char *tablica[MAX_USERS][MAX_GROUPS], char nazwa[]){
    int i;
    for(i=0; i<MAX_GROUPS; i++)
        if(!strcmp(tablica[0][i], nazwa))
            return 1;

    return 0;
}
