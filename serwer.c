#include "funkcje.c"

int main(){
   int idserwer,i,j,liczbaUserow=0;
   char zajeteid[7] = {'s', 'e', 'r', 'w', 'e', 'r', '\0'};
   char polecenie[MAX_MESSAGE_LENGTH];

   key_t key = ftok("komunikacja", zajeteid);

   if((idserwer = msgget(key, IPC_CREAT | 0622)) == -1){
        perror("msgget serwer");
    	exit(1);
    }

    printf("ID serwera: %d\n", idserwer);

    char *tablicaKolejek[2][MAX_USERS];
    for(j=0; j<2; j++)
        for(i=0; i<MAX_USERS; i++)
            tablicaKolejek[j][i] = (char*)malloc(sizeof(char)*MAX_NAME_LENGTH);
    for(j=0; j<2; j++)
        for(i=0; i<MAX_USERS; i++)
            strcpy(tablicaKolejek[j][i], "12345678901234567890-1234567890");

    struct command wiadOdebrana;
    struct message wiadWyslana;

    while(1){
        if(msgrcv(idserwer, &wiadOdebrana, sizeof(wiadOdebrana.data) + sizeof(wiadOdebrana.username), 0, 0) == -1){
            perror("msgrcv");
            exit(1);
        }
        if(wiadOdebrana.mtype == 2){
            if(liczbaUserow == MAX_USERS){
                wiadWyslana.mtype = 3;
                strcpy(wiadWyslana.message, "Przekroczono liczbe uzytkownikow. Sprobuj ponownie pozniej\n");
                strcpy(wiadWyslana.from, zajeteid);
                strcpy(wiadWyslana.to, wiadOdebrana.username);
                if(msgsnd(atoi(wiadOdebrana.data), &wiadWyslana, sizeof(wiadWyslana), 0) == -1){
                        perror("msgsnd za duzo userow");
                        exit(1);
                }
                continue;
            }

            if(czyZajety(tablicaKolejek, wiadOdebrana.username)){
                wiadWyslana.mtype = 3;
                strcpy(wiadWyslana.message, "Nick zajety\n");
                strcpy(wiadWyslana.from, zajeteid);
                strcpy(wiadWyslana.to, wiadOdebrana.username);
                if(msgsnd(atoi(wiadOdebrana.data), &wiadWyslana, sizeof(wiadWyslana), 0) == -1){
                        perror("msgsnd zajety nick");
                        exit(1);
                }
                continue;
            }

            printf("Zalogowano %s o numerze kolejki %s\n", wiadOdebrana.username, wiadOdebrana.data);
            strcpy(tablicaKolejek[0][liczbaUserow], wiadOdebrana.username);
            strcpy(tablicaKolejek[1][liczbaUserow++], wiadOdebrana.data);
            wiadWyslana.mtype = 3;
            strcpy(wiadWyslana.message, "Logowanie zakonczone sukcesem\n");
            strcpy(wiadWyslana.from, zajeteid);
            strcpy(wiadWyslana.to, wiadOdebrana.username);
            if(msgsnd(atoi(wiadOdebrana.data), &wiadWyslana, sizeof(wiadWyslana), 0) == -1){
                    perror("msgsnd login succesfull");
                    exit(1);
                }
        }

        if(wiadOdebrana.mtype == 1){
            znajdz_polecenie(wiadOdebrana.data, polecenie);
            if(!strcmp(polecenie,"logout")){
                wiadWyslana.mtype = 3;
                strcpy(wiadWyslana.from, zajeteid);
                strcpy(wiadWyslana.message, "exit");
                strcpy(wiadWyslana.to, wiadOdebrana.username);
                if(msgsnd(atoi(tablicaKolejek[1][znajdzIndeks(tablicaKolejek, wiadOdebrana.username)]), &wiadWyslana, sizeof(wiadWyslana), 0) == -1){
                    perror("msgsnd logout");
                    exit(1);
                }
                strcpy(tablicaKolejek[0][znajdzIndeks(tablicaKolejek, wiadOdebrana.username)],tablicaKolejek[0][--liczbaUserow]);
                strcpy(tablicaKolejek[1][znajdzIndeks(tablicaKolejek, wiadOdebrana.username)],tablicaKolejek[1][liczbaUserow]);
                strcpy(tablicaKolejek[0][liczbaUserow], "12345678901234567890-1234567890");
            }
            if(!strcmp(polecenie, "users")){
                wiadWyslana.mtype = 1;
                strcpy(wiadWyslana.to, wiadOdebrana.username);
                strcpy(wiadWyslana.from, zajeteid);
                int i=0;
                printf("Przesylam wszytskich użytkownikow\n");
                for( i=0; i<liczbaUserow; i++){
                    strcpy(wiadWyslana.message, tablicaKolejek[0][i]);
                    if(msgsnd(atoi(tablicaKolejek[1][znajdzIndeks(tablicaKolejek, wiadOdebrana.username)]), &wiadWyslana, sizeof(wiadWyslana), 0) == -1){
                        perror("msgsnd users");
                        exit(1);
                    }
                }
            }
            if(polecenie[0] == '@'){            //komunikacja miedzy uzytkownikami
                wiadWyslana.mtype = 1;
                wiadWyslana.to_symbol = polecenie[0];
                strcpy(wiadWyslana.from, wiadOdebrana.username);
                strcpy(wiadWyslana.to, splitToWho(polecenie));
                strcpy(wiadWyslana.message, wiadOdebrana.data);
                if(czyZajety(tablicaKolejek, wiadWyslana.to)){
                    strcpy(wiadWyslana.from, zajeteid);
                    strcpy(wiadWyslana.to, wiadOdebrana.username);
                    strcpy(wiadWyslana.message, "Nie ma takiego uzytkownika\n");
                }
                if(msgsnd(atoi(tablicaKolejek[1][znajdzIndeks(tablicaKolejek, wiadWyslana.to)]), &wiadWyslana, sizeof(wiadWyslana), 0) == -1){
                        perror("msgsnd miedzy userami");
                        exit(1);
                }
            }

            if(polecenie[0] == '*'){            //komunikacja miedzy uzytkownikami
                wiadWyslana.mtype = 1;
                wiadWyslana.to_symbol = polecenie[0];
                strcpy(wiadWyslana.from, wiadOdebrana.username);
                strcpy(wiadWyslana.to, "");
                strcpy(wiadWyslana.message, wiadOdebrana.data);
                for(i=0; i<liczbaUserow; i++)
                    if(msgsnd(atoi(tablicaKolejek[1][i]), &wiadWyslana, sizeof(wiadWyslana), 0) == -1){
                            perror("msgsnd miedzy userami");
                            exit(1);
                    }
            }

            if(!strcmp(polecenie, "help")){
                wiadWyslana.mtype = 1;
                strcpy(wiadWyslana.to, wiadOdebrana.username);
                strcpy(wiadWyslana.from, zajeteid);
                strcpy(wiadWyslana.message, "KOMENDA                    OPIS\nlogin [id_kolejki]          zalogowanie\n\njoin [nazwa_pokoju]         do³¹cz do pokoju\nleave [nazwa_pokoju]        wyjdŸ z pokoju\nrooms                       wyœwietl pokoje");
                if(msgsnd(atoi(tablicaKolejek[1][znajdzIndeks(tablicaKolejek, wiadOdebrana.username)]), &wiadWyslana, sizeof(wiadWyslana), 0) == -1){
                        perror("msgsnd help");
                        exit(1);
                }
                strcpy(wiadWyslana.message,"users                       wyœwietl u¿ytkowników\nhelp                        wyœwietl dostêpne komendy\n\n@[nick] [treœæ]             wiadomoœæ prywatna\n#[nazwa_pokoju] [treœæ]     wiadomoœæ do pokoju\n* [treœæ]                   wiadomoœæ do wszystkich");
                if(msgsnd(atoi(tablicaKolejek[1][znajdzIndeks(tablicaKolejek, wiadOdebrana.username)]), &wiadWyslana, sizeof(wiadWyslana), 0) == -1){
                        perror("msgsnd help2");
                        exit(1);
                }
            }
        }
    }


    /*if (semctl(id, IPC_RMID, 0) == -1){
    	perror("semctl - usuwanie");
    	exit(1);
    }*/

   return 0;
}
