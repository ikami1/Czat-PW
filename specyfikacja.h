#ifndef IPCCHAT_IPCCHAT_H
#define IPCCHAT_IPCCHAT_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/sem.h>

// maksymalna liczba u¿ytkowników
#define MAX_USERS 32

// maksymalna liczba grup (u¿ytkownik mo¿e byæ na raz pod³¹czony do wszystkich grup)
#define MAX_GROUPS 32

// Mo¿na (nawet bêdzie ³atwiej) trzymaæ u¿ytkowników i grupy na liœcie zamiast w tablicy.
// Przy wyborze tablicy trzeba stosowaæ siê do powy¿szych dwóch sta³ych.

// maksymalna d³ugoœæ nazwy u¿ytkownika/kana³u
#define MAX_NAME_LENGTH 32

// maksymalna d³ugoœæ wiadomoœci
#define MAX_MESSAGE_LENGTH 2048


/*
# Wymiana da#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/sem.h>nych
Serwer tworzy kolejkê i wypisuje jej ID, ¿eby klient wiedzia³, gdzie siê pod³¹czyæ.
Klient te¿ tworzy swoj¹ kolejkê i zaraz po otwarciu kolejki serwera wysy³a tam komendê login ze swoim queue_id.
Po wylogowaniu klient nie wy³¹cza siê sam z siebie, ale czeka na wiadomoœæ po¿egnaln¹ (typu 3).
Komunikacja klient -> serwer przez kolejkê serwera, struktura command
Komunikacja serwer -> klient przez kolejkê klienta, struktura message
# Lista komend
KOMENDA                 MTYPE   OPIS                        UWAGI
login [id_kolejki]      2       zalogowanie                 to powinna byæ pierwsza komenda wysy³ana po pod³¹czeniu klienta
logout                  1       wylogowanie                 trzeba odpowiedzieæ wiadomoœci¹ o mtype = 3, ¿eby wy³¹czyæ klienta
join [nazwa_pokoju]     1       do³¹cz do pokoju
leave [nazwa_pokoju]    1       wyjdŸ z pokoju
rooms                   1       wyœwietl pokoje             serwer powinien odpowiedzieæ wiadomoœci¹ (server jako nadawca)
users                   1       wyœwietl u¿ytkowników       jw.
help                    1       wyœwietl dostêpne komendy   jw.
@[nick] [treœæ]         1       wiadomoœæ prywatna          loginy nie zawieraj¹ bia³ych znaków, po @ nie ma spacji!
#[nazwa_pokoju] [treœæ] 1       wiadomoœæ do pokoju         jw.
* [treœæ]               1       wiadomoœæ do wszystkich
*/


// struktura komend wysy³anych przez klienta na serwer
// (wys³anie wiadomoœci te¿ rozumiemy jako komendê)
struct command {
    long mtype; // zale¿nie od typu komendy, opisane wy¿ej

    char data[2*MAX_MESSAGE_LENGTH]; // treœæ komendy
    char username[MAX_NAME_LENGTH]; // nazwa u¿ytkownika wysy³aj¹cego komendê
};

// struktura wiadomoœci wysy³anych od serwera do klienta
struct message {
    long mtype;
    /* 1    dla wiadomoœci od u¿ytkowników
       2    dla wiadomoœci od serwera
                (np. odpowiedzi na rooms, users, help; ewentualna wiadomoœæ powitalna)
       3    wiadomoœæ specjalna od serwera, po której otrzymaniu klient powinien siê zakoñczyæ
                (np. komunikat o odrzuceniu po³¹czenia z powodu przekroczonej liczby u¿ytkowników,
                zajêtego nicku, wiadomoœæ po¿egnalna po wylogowaniu albo gdy serwer dosta³ sygna³ zamykaj¹cy)
    */

    char from[MAX_NAME_LENGTH]; // nazwa u¿ytkownika, który wys³a³ wiadomoœæ
    char to_symbol; // @ - wiadomoœæ prywatna, # - wiadomoœæ na kana³, * - wiadomoœæ do wszystkich
    char to[MAX_NAME_LENGTH];  // nazwa u¿ytkownika/kana³u albo puste, jeœli to_symbol = *
    char message[MAX_MESSAGE_LENGTH]; // treœæ wiadomoœci
};

void znajdz_polecenie(char linia[], char polecenie[]);
int znajdzIndeks(char tablica[][], char nickname);
char* splitToWho(char polecenie[]);
int czyZajety(char *tablica[2][MAX_USERS], char nickname[]);

#endif //IPCCHAT_IPCCHAT_H
