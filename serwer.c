#include "funkcje.c"

int main(){
   int idserwer,i,j,liczbaUserow=0, liczbaPokoi=0, indeksUser;
   char zajeteid[7] = {'s', 'e', 'r', 'w', 'e', 'r', '\0'};
   char polecenie[MAX_MESSAGE_LENGTH];
   char nazwa[MAX_MESSAGE_LENGTH];
   char nazwaPokoju[MAX_NAME_LENGTH];

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

    char *tablicaPokoi[MAX_USERS+1][MAX_GROUPS];
    for(j=0; j<MAX_USERS+1; j++)
        for(i=0; i<MAX_GROUPS; i++)
            tablicaPokoi[j][i] = (char*)malloc(sizeof(char)*MAX_NAME_LENGTH);
    for(j=0; j<MAX_USERS+1; j++)
        for(i=0; i<MAX_GROUPS; i++)
            strcpy(tablicaPokoi[j][i], "");

    int ileUzytkownikow[MAX_GROUPS];
    for(i=0; i<MAX_GROUPS; i++)
        ileUzytkownikow[i] = 0;

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
                if(msgsnd(atoi(wiadOdebrana.data), &wiadWyslana, sizeof(wiadWyslana) - sizeof(long), 0) == -1){
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
                if(msgsnd(atoi(wiadOdebrana.data), &wiadWyslana, sizeof(wiadWyslana) - sizeof(long), 0) == -1){
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
            if(msgsnd(atoi(wiadOdebrana.data), &wiadWyslana, sizeof(wiadWyslana) - sizeof(long), 0) == -1){
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
                if(msgsnd(atoi(tablicaKolejek[1][znajdzIndeks(tablicaKolejek, wiadOdebrana.username)]), &wiadWyslana, sizeof(wiadWyslana) - sizeof(long), 0) == -1){
                    perror("msgsnd logout");
                    exit(1);
                }
                strcpy(tablicaKolejek[0][znajdzIndeks(tablicaKolejek, wiadOdebrana.username)],tablicaKolejek[0][--liczbaUserow]);
                strcpy(tablicaKolejek[1][znajdzIndeks(tablicaKolejek, wiadOdebrana.username)],tablicaKolejek[1][liczbaUserow]);
                strcpy(tablicaKolejek[0][liczbaUserow], "12345678901234567890-1234567890");
            }
            if(!strcmp(polecenie, "users")){            //wyswietla wszytskich uzytkownikow
                wiadWyslana.mtype = 1;
                strcpy(wiadWyslana.to, wiadOdebrana.username);
                strcpy(wiadWyslana.from, zajeteid);
                int i=0;
                printf("Przesylam wszytskich uzytkownikow\n");
                for( i=0; i<liczbaUserow; i++){
                    strcpy(wiadWyslana.message, "");
                    strcpy(wiadWyslana.message, tablicaKolejek[0][i]);
                    if(msgsnd(atoi(tablicaKolejek[1][znajdzIndeks(tablicaKolejek, wiadOdebrana.username)]), &wiadWyslana, sizeof(wiadWyslana) - sizeof(long), 0) == -1){
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
                if(!czyZajety(tablicaKolejek, wiadWyslana.to)){
                    strcpy(wiadWyslana.from, zajeteid);
                    strcpy(wiadWyslana.to, wiadOdebrana.username);
                    strcpy(wiadWyslana.message, "Nie ma takiego uzytkownika");
                }
                if(msgsnd(atoi(tablicaKolejek[1][znajdzIndeks(tablicaKolejek, wiadWyslana.to)]), &wiadWyslana, sizeof(wiadWyslana) - sizeof(long), 0) == -1){
                        perror("msgsnd miedzy userami");
                        exit(1);
                }
            }

            if(polecenie[0] == '*'){            //wiadomosc do wszystkich
                wiadWyslana.mtype = 1;
                wiadWyslana.to_symbol = polecenie[0];
                strcpy(wiadWyslana.from, wiadOdebrana.username);
                strcpy(wiadWyslana.to, "");
                strcpy(wiadWyslana.message, wiadOdebrana.data);
                for(i=0; i<liczbaUserow; i++)
                    if(msgsnd(atoi(tablicaKolejek[1][i]), &wiadWyslana, sizeof(wiadWyslana) - sizeof(long), 0) == -1){
                            perror("msgsnd miedzy userami");
                            exit(1);
                    }
            }
            if(!strcmp(polecenie, "join")){
                znajdz_polecenie(wiadOdebrana.data, nazwa);
                if(!istniejePokoj(tablicaPokoi, nazwa)){
                    if(liczbaPokoi == MAX_GROUPS){
                        wiadWyslana.mtype = 1;
                        strcpy(wiadWyslana.from, zajeteid);
                        strcpy(wiadWyslana.message, "Nie mozna utworzyc pokoju. Osiagnieto maksymalna liczbe pokoi");
                        if(msgsnd(atoi(tablicaKolejek[1][znajdzIndeks(tablicaKolejek, wiadOdebrana.username)]), &wiadWyslana, sizeof(wiadWyslana) - sizeof(long), 0) == -1){
                            perror("msgsnd join nowy pokoj");
                            exit(1);
                        }
                    }
                    else{
                        strcpy(tablicaPokoi[0][liczbaPokoi], nazwa);
                        strcpy(tablicaPokoi[1][liczbaPokoi], wiadOdebrana.username);
                        ileUzytkownikow[liczbaPokoi]+=1;
                        liczbaPokoi+=1;

                        wiadWyslana.mtype = 1;
                        strcpy(wiadWyslana.from, zajeteid);
                        strcpy(wiadWyslana.message, "");
                        sprintf(wiadWyslana.message, "Utworzono pokoj o nazwie %s", nazwa);
                        if(msgsnd(atoi(tablicaKolejek[1][znajdzIndeks(tablicaKolejek, wiadOdebrana.username)]), &wiadWyslana, sizeof(wiadWyslana) - sizeof(long), 0) == -1){
                            perror("msgsnd join nowy pokoj");
                            exit(1);
                        }
                    }
                }
                else{
                    int numerPokoju = znajdzIndeksPokoju(tablicaPokoi, nazwa);
                    if(znajdzIndeksUsera(tablicaPokoi, nazwa, numerPokoju) == -1){
                        wiadWyslana.mtype = 1;
                        strcpy(wiadWyslana.from, zajeteid);
                        strcpy(wiadWyslana.message, "");
                        sprintf(wiadWyslana.message, "Juz nalezysz do pokoju o nazwie %s", nazwa);
                        if(msgsnd(atoi(tablicaKolejek[1][znajdzIndeks(tablicaKolejek, wiadOdebrana.username)]), &wiadWyslana, sizeof(wiadWyslana) - sizeof(long), 0) == -1){
                            perror("msgsnd join drugi raz");
                            exit(1);
                        }
                    }
                    else{
                        strcpy(tablicaPokoi[ileUzytkownikow[numerPokoju]+1][numerPokoju], wiadOdebrana.username);
                        ileUzytkownikow[numerPokoju] += 1;

                        wiadWyslana.mtype = 1;
                        strcpy(wiadWyslana.from, zajeteid);
                        strcpy(wiadWyslana.message, "");
                        sprintf(wiadWyslana.message, "Dolaczono do pokoju o nazwie %s", nazwa);
                        if(msgsnd(atoi(tablicaKolejek[1][znajdzIndeks(tablicaKolejek, wiadOdebrana.username)]), &wiadWyslana, sizeof(wiadWyslana) - sizeof(long), 0) == -1){
                            perror("msgsnd join pokoj");
                            exit(1);
                        }
                    }
                }
            }
            if(!strcmp(polecenie, "rooms")){
                wiadWyslana.mtype = 1;
                strcpy(wiadWyslana.to, wiadOdebrana.username);
                strcpy(wiadWyslana.from, zajeteid);
                int i=0;
                printf("Przesylam wszytskie pokoje\n");
                for( i=0; i<liczbaPokoi; i++){
                    strcpy(wiadWyslana.message, tablicaPokoi[0][i]);
                    if(msgsnd(atoi(tablicaKolejek[1][znajdzIndeks(tablicaKolejek, wiadOdebrana.username)]), &wiadWyslana, sizeof(wiadWyslana) - sizeof(long), 0) == -1){
                        perror("msgsnd rooms");
                        exit(1);
                    }
                }
            }

            if(!strcmp(polecenie,"leave")){
                znajdz_polecenie(wiadOdebrana.data,nazwa);
                if(!istniejePokoj(tablicaPokoi, nazwa)){
                    wiadWyslana.mtype=1;
                    strcpy(wiadWyslana.to, wiadOdebrana.username);
                    strcpy(wiadWyslana.from, zajeteid);
                    strcpy(wiadWyslana.message, "");
                    sprintf(wiadWyslana.message, "Nie mozna opuscic pokoju. Nie ma pokoju o nazwie %s", nazwa);
                    if(msgsnd(atoi(tablicaKolejek[1][znajdzIndeks(tablicaKolejek, wiadOdebrana.username)]), &wiadWyslana, sizeof(wiadWyslana) - sizeof(long), 0) == -1){
                        perror("msgsnd leave nie mozna");
                        exit(1);
                    }
                }
                else{
                    int numerPokoju = znajdzIndeksPokoju(tablicaPokoi, nazwa);
                    if(ileUzytkownikow[numerPokoju] == 1){
                        strcpy(tablicaPokoi[0][numerPokoju],tablicaPokoi[0][liczbaPokoi-1]);
                        for(i=1; i<ileUzytkownikow[numerPokoju]+1; i++)
                            strcpy(tablicaPokoi[i][numerPokoju],tablicaPokoi[i][liczbaPokoi-1]);
                        ileUzytkownikow[numerPokoju] = ileUzytkownikow[liczbaPokoi-1];
                        liczbaPokoi -=1;

                        wiadWyslana.mtype=1;
                        strcpy(wiadWyslana.to, wiadOdebrana.username);
                        strcpy(wiadWyslana.from, zajeteid);
                        strcpy(wiadWyslana.message, "");
                        sprintf(wiadWyslana.message, "Zlikwidowales i wyszedles z pokoju o nazwie %s", nazwa);
                        if(msgsnd(atoi(tablicaKolejek[1][znajdzIndeks(tablicaKolejek, wiadOdebrana.username)]), &wiadWyslana, sizeof(wiadWyslana) - sizeof(long), 0) == -1){
                            perror("msgsnd leave and destroy");
                            exit(1);
                        }
                    }
                    else{
                        indeksUser = znajdzIndeksUsera(tablicaPokoi, wiadOdebrana.username, numerPokoju);
                        strcpy(tablicaPokoi[indeksUser][numerPokoju], tablicaPokoi[ileUzytkownikow[numerPokoju]][numerPokoju]);
                        strcpy(tablicaPokoi[ileUzytkownikow[numerPokoju]][numerPokoju], "");
                        ileUzytkownikow[numerPokoju]-=1;
                        wiadWyslana.mtype=1;
                        strcpy(wiadWyslana.to, wiadOdebrana.username);
                        strcpy(wiadWyslana.from, zajeteid);
                        strcpy(wiadWyslana.message, "");
                        sprintf(wiadWyslana.message, "Wyszedles z pokoju o nazwie %s", nazwa);
                        if(msgsnd(atoi(tablicaKolejek[1][znajdzIndeks(tablicaKolejek, wiadOdebrana.username)]), &wiadWyslana, sizeof(wiadWyslana) - sizeof(long), 0) == -1){
                            perror("msgsnd leave");
                            exit(1);
                        }
                    }
                }
            }

            if(polecenie[0] == '#'){            //wiadomosc do pokoju
                wiadWyslana.to_symbol = polecenie[0];
                strcpy(nazwaPokoju, splitToWho(polecenie));
                j = znajdzIndeksPokoju(tablicaPokoi, nazwaPokoju);

                if(j == -1){
                    wiadWyslana.mtype = 1;
                    strcpy(wiadWyslana.from, zajeteid);
                    strcpy(wiadWyslana.to, wiadOdebrana.username);
                    strcpy(wiadWyslana.message, "Nie mozna pisac do nieistniejacego pokoju");
                    if(msgsnd(atoi(tablicaKolejek[1][znajdzIndeks(tablicaKolejek, wiadOdebrana.username)]), &wiadWyslana, sizeof(wiadWyslana) - sizeof(long), 0) == -1){
                        perror("msgsnd do nieistniejacej grupy");
                        exit(1);
                    }
                }
                else{
                    wiadWyslana.mtype = 1;
                    strcpy(wiadWyslana.from, "");
                    sprintf(wiadWyslana.from,"%s.%s",nazwaPokoju,wiadOdebrana.username);
                    strcpy(wiadWyslana.message, wiadOdebrana.data);
                    for(i=1; i<ileUzytkownikow[j]+1; i++){
                        strcpy(wiadWyslana.to, tablicaPokoi[i][j]);
                        if(msgsnd(atoi(tablicaKolejek[1][znajdzIndeks(tablicaKolejek, tablicaPokoi[i][j])]), &wiadWyslana, sizeof(wiadWyslana) - sizeof(long), 0) == -1){
                                perror("msgsnd do grupy");
                                exit(1);
                        }
                    }
                }
            }

            if(!strcmp(polecenie, "help")){
                wiadWyslana.mtype = 1;
                strcpy(wiadWyslana.to, wiadOdebrana.username);
                strcpy(wiadWyslana.from, zajeteid);
                strcpy(wiadWyslana.message, "KOMENDA                    OPIS\nlogin [id_kolejki]          zalogowanie\n\njoin [nazwa_pokoju]         dolacz do pokoju\nleave [nazwa_pokoju]        wyjdz z pokoju\nrooms                       wyswietl pokoje");
                if(msgsnd(atoi(tablicaKolejek[1][znajdzIndeks(tablicaKolejek, wiadOdebrana.username)]), &wiadWyslana, sizeof(wiadWyslana) - sizeof(long), 0) == -1){
                        perror("msgsnd help");
                        exit(1);
                }
                strcpy(wiadWyslana.message,"users                       wyswietl uzytkownikow\nhelp                        wyswietl dostepne komendy\n\n@[nick] [tresc]             wiadomosc prywatna\n#[nazwa_pokoju] [tresc]     wiadomosc do pokoju\n* [tresc]                   wiadomoœæ do wszystkich");
                if(msgsnd(atoi(tablicaKolejek[1][znajdzIndeks(tablicaKolejek, wiadOdebrana.username)]), &wiadWyslana, sizeof(wiadWyslana) - sizeof(long), 0) == -1){
                        perror("msgsnd help2");
                        exit(1);
                }
            }
        }
    }

   return 0;
}
