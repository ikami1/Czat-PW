#include "funkcje.c"

int main(){
   int idserwera, idklienta, moznazalogowac = 0;
   char buf[2*MAX_MESSAGE_LENGTH];
   char polecenie[MAX_MESSAGE_LENGTH];

   struct command wiadDoSerwera;
   struct message wiadOdebrana;
   wiadDoSerwera.mtype = 2;
   while(!moznazalogowac){
       while(strcmp(polecenie, "login")){
            printf("login [id_kolejki] do zalogowania\n");
            read(1, buf, sizeof(buf));
            znajdz_polecenie(buf, polecenie);
       }

       znajdz_polecenie(buf, polecenie);
       idserwera = atoi(polecenie);
       printf("Podaj nick\n");
       scanf("%s", wiadDoSerwera.username);

       key_t key = ftok("komunikacja", wiadDoSerwera.username);
       if((idklienta = msgget(key, IPC_CREAT | 0622)) == -1){
          perror("msgget klient");
          exit(1);
       }

       sprintf(wiadDoSerwera.data, "%d", idklienta);
       if(msgsnd(idserwera, &wiadDoSerwera, sizeof(wiadDoSerwera.data) + sizeof(wiadDoSerwera.username), 0) == -1){
          perror("msgsnd klient nr kolejki");
          exit(1);
       }
       if(msgrcv(idklienta, &wiadOdebrana, sizeof(wiadOdebrana) - sizeof(long), 3, 0) == -1){
            perror("msgrcv wiadomosc po logowaniu");
            exit(1);
        }
        printf("%s", wiadOdebrana.message);
        if(!strcmp(wiadOdebrana.message, "Logowanie zakonczone sukcesem\n")){
            moznazalogowac = 1;
            break;
        }
        else{
            if(msgctl(idklienta, IPC_RMID, 0)){
                perror("msgctl klient po logowaniu");
                exit(1);
            }
        }
   }

   wiadDoSerwera.mtype = 1;
   if(fork() == 0)
       while(1){                 //fork ktory wysyla na serwer
           memset(wiadDoSerwera.data, '\0', 2*MAX_MESSAGE_LENGTH);
           read(1, wiadDoSerwera.data, sizeof(wiadDoSerwera.data));
           if(msgsnd(idserwera, &wiadDoSerwera, sizeof(wiadDoSerwera.data) + sizeof(wiadDoSerwera.username), 0) == -1){
              perror("msgsnd klient do serwera");
              exit(1);
           }
           if(!strcmp(wiadDoSerwera.data,"logout\n")){
              exit(1);
          }
      }
   else
   	  while(1){                 //fork ktory odczytuje komunikacje od serwera
          if(msgrcv(idklienta, &wiadOdebrana, sizeof(wiadOdebrana) - sizeof(long), -3, 0) == -1){
            perror("msgrcv klient od serwera");
            exit(1);
          }
          if(wiadOdebrana.mtype == 3 && !strcmp(wiadOdebrana.message, "exit")){
             if(msgctl(idklienta, IPC_RMID, 0)){
                perror("msgctl klient po logowaniu");
                exit(1);
             }
             exit(1);
          }
          if(!strcmp(wiadOdebrana.from, "serwer"))
             printf("%s\n", wiadOdebrana.message);
          else{
             if(wiadOdebrana.to_symbol == '#')
                printf("%c[%dm%c%s%s", 0x1B, BLUE, wiadOdebrana.to_symbol, wiadOdebrana.from, wiadOdebrana.message);
             if(wiadOdebrana.to_symbol == '@')
                printf("%c[%dm%c%s%s", 0x1B, GREEN, wiadOdebrana.to_symbol, wiadOdebrana.from, wiadOdebrana.message);
             if(wiadOdebrana.to_symbol == '*')
                printf("%c[%dm%c%s%s", 0x1B, YELLOW, wiadOdebrana.to_symbol, wiadOdebrana.from, wiadOdebrana.message);
             Reset();
             printf("\n");
          }

       }

 return 0;
}
