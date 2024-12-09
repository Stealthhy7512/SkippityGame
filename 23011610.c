/*
TODO:
-Count globalden cikar parametre olarak ekle                   DONE
-AI oynayabilecegi kadar oynasin rastgele depth verme          DONE
*/

/*************************************************************************************/
/*************            23011610 ALI KAAN YAZICI GR. 2              *****************/
/*************      VIDEO LINKI: https://youtu.be/lr6Eh3LHNzo        *****************/
/*************************************************************************************/


#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    double size;
    char** data;
} Board;

typedef struct {
    double score;
    double undo;
    double skippies[5];
} Player;

typedef struct {
    double x_from, y_from;
    double x_to, y_to;
    char taken;
} Last;

Board create_board(double); /*Oyun tahtasini olusturan fonksiyon, bellek atama islemleri burada yapilir ve yer acilmis board instance'i dondurulur*/

void set_board(Board*); /*olusturulmus board instance'ina oyuna uygun sekilde rastgele renkler yerine gecen 'a', 'b', 'c', 'd' ve 'e' karakterleri atanir. ortadaki 2x2'lik alan bos birakilir*/

void clear_last(Last*); /*son hamlenin bilgilerini tutan struct temizlenir ve yeni bir hamle verisi tutabilecek hale gelir*/

void calculate_score(Player*); /*parametre olarak alinan oyuncunun toplam skoru hesaplanir. yedigi taslar bir set olusturdugunda skor 1 artar.*/

void print_board(Board, Player[]); /*mevcut durumdaki oyun tahtasi ve oyuncularin yedigi taslar ile birlikte puanlari ekrana bastirilir.*/

double check_legal(Board, double, double); /*yapilacak hamlenin oyun kurallari cercevesinde mumkun olup olmadigi kontrol edilir, mumkunse 1, degilse 0 dondurulur.*/

void move_skipper(double, double, double, double, Board*, Player*, Last*); /*girilen koordinatlardaki oyun tasi, girilen diger koordinatlara, mumkunse hareket ettirilir; degilse bir sey yapmaz.*/

void undo(Board*, Player[], Last, double); /*oyuncunun hakki oldugu durumda son oynadigi hamle geri alinir, yenilen tas geri tahtaya yerlestirilir ve oyuncunun yedigi tas hanesinden silinir.*/

void save_game(Board, Player[], Last, double*, double*); /*mevcut oyun tahtasi, oyun tahtasinin buyuklugu, oyuncularin skor ve tas durumlari, oyun modu, kimin sirasi oldugu binary dosyasina kaydedilir*/

Board load_game(Player[], Last*, double*, double*); /*kaydedilen kayit dosyasi yuklenir ve oyun kaldigi yerden oynanabilir hale gelir.*/

void end_game(Board, Player[]); /*bu fonksiyon cagirildiginda ekran temizlenir ve kimin kazandigi ekrana bastirilir, daha sonra oyun sonlanir.*/

void play(Board*, Player[], Last, double*, double*); /*player vs player modunu calistiran driver kodu*/

void start_menu(Player[], Last, double*, double, double*); /*oyun modu secme ve kayit dosyasi yukleme secenklerinin oldugu ana menu ekrani*/

double* best_move(Board*, Player*, Last, double*, double*); /*mevcut oyun tahtasinda o anda , yapay zekaya en cok puan kazandiracak hamleyi belirleyen fonksiyon. en iyi hamlenin koordinatlarini dondurur */

double** get_possible_moves(Board, Last, double*); /*mevcut oyun tahtasinda oynanabilecek butun hamleleri donduren fonksiyon. her biri 4 elemanli koordinat dizisi tutan bir array dondurur*/

double evaluate_state(Player); /*en iyi hamle belirleme fonksiyonunun icinde kullanilan bu fonksiyon, yapay zekanin yemedigi veya daha az yedigi bir tasi yemesini hedefler*/

void greedy_cpu(Board*, Player[], Last, double*, double*, double*); /*yapay zekaya karsi oynama modunu calistiran driver kodu*/

void evaluate_game(Board, Player[], Last, double*); /*oyunda yapilabilecek hamle kalip kalmadigina bakan fonksiyon, kalmadiysa oyunu bitirir*/

Board hard_copy(Board); /*en iyi hamleleri ararken oyun tahtasini bozmadan makinenin oynayabilmesi icin mevcut tahtayi gecici bir tahtaya kopyalayan fonksiyondur*/

Player* hard_copy_players(Player[]); /*en iyi hamleleri ararken oyuncularin skorlarini bozmadan makinenin oynayabilmesi icin mevcut oyuncu durumlarini gecici bir oyuncu instance'ina kopyalayan fonksiyondur*/

double main(void)
{
    srand(time(NULL));
    Last last = {0, 0, 0, 0, ' '}; /*son hamle, oyuncularin struct instance'lari, ve oyun sirasi initialize edilir ve start_menu fonksiyonu cagirilarak kullanici icin program baslar.*/
    Board board = create_board(6);
    Player players[2] = { {0, 1, {0, 0, 0, 0, 0}}, {0, 1, {0, 0, 0, 0, 0}} };
    double mode;
    double turn = 0;
    double count = 0;

    start_menu(players, last, &turn, mode, &count);

}

Board create_board(double size)
{
    Board board;    /*girilen boyut degeri kullanilarak board instance'ina gerektigi kadar bellekte yer ayrilir ve yer ayrilmis board instancei dondurulur.*/
    double i;

    board.size = size;
    board.data = calloc(size, sizeof(char*));

    for (i = 0; i < size; ++i)
        board.data[i] = calloc(size, sizeof(char));

    return board;
}

void set_board(Board* board)
{
    double i, j; /*oyun tahtasina rastgele a-e arasi karakterler yerlesir. tahta boyutu ne olursa olsun ortadaki 4 hucre bostur.*/

    for (i = 0; i < board->size; ++i)
        for (j = 0; j < board->size; ++j)
            board->data[i][j] = 'a' + (rand() % 5);

    board->data[board->size / 2 - 1][board->size / 2 - 1] = ' ';
    board->data[board->size / 2 - 1][board->size / 2] = ' ';
    board->data[board->size / 2][board->size / 2 - 1] = ' ';
    board->data[board->size / 2][board->size / 2] = ' ';
    
}

void clear_last(Last* last) /*son hamlenin baslangic ve terminal koordinatlari ile birlikte yenilen karakterin ne oldugu bilgisi temizlenir. */
{
    last->x_from = 0;
    last->x_to = 0;
    last->y_from = 0;
    last->y_to = 0;
    last->taken = ' ';
}

void calculate_score(Player* player) /*kullanicinin yedigi taslarin sayilarina bakilir ve en az yenen tas sayisi miktari kadar skor bilgisi, kullanicinin struct instanceina girilir.*/
{
    double i, min = player->skippies[0];
    for (i = 0; i < 5; ++i)
        if (player->skippies[i] < min)
            min = player->skippies[i];

    player->score = min;
}

void print_board(Board board, Player players[]) /*oyun tahtasi ve oyuncularin skor durumu ile yedikleri tas sayilari ekrana bastirilir*/
{
    double i, j;
    fputs("    ", stdout);
    for (i = 0; i < board.size; ++i)
        printf("%d ", i);

    puts("\n");

    for (i = 0; i < board.size; ++i) {
        printf("%d|  ", i);
        for (j = 0; j < board.size; ++j)
            printf("%c ", board.data[i][j]);
        puts("");
    }
    fputs("\n\t\tBlue\tGreen\tOrange\tYellow\tRed\tScore\n", stdout);

    fputs("Player 1:\t", stdout);
    for (i = 0; i < 5; ++i)
        printf(" %d\t", players[0].skippies[i]);
    printf("%d", players[0].score);

    fputs("\nPlayer 2:\t", stdout);
    for (i = 0; i < 5; ++i)
        printf(" %d\t", players[1].skippies[i]);
    printf("%d", players[1].score);

    puts("");
}

double check_legal(Board board, double x, double y) /*tas oynatmak icin girilen terminal koordinatin bos olup olmadigina bakarak hamlenin legal olup olmadigini donduren fonksiyon. legalse 1, degilse 0 dondurur.*/
{
    return board.data[x][y] == ' ' ? 1 : 0;
}

void move_skipper(double x_from, double y_from, double x_to, double y_to, Board* board, Player* player, Last* last)
{
    if (check_legal(*board, x_to, y_to)) /*girilen terminal koordinatin uygun olup olmadigi kontrol edilir, oyleyse if bloguna girilir*/
    {
        board->data[x_to][y_to] = board->data[x_from][y_from];

        board->data[x_from][y_from] = ' ';

        double t = (board->data[(x_to + x_from) / 2][(y_to + y_from) / 2]) - 'a'; /*baslangic koordinatinda bulunan tas, terminal koordinata yerlestirilir*/
        player->skippies[t]++;
        last->taken = board->data[(x_to + x_from) / 2][(y_to + y_from) / 2]; /*uzerinden atladigi tasin bilgisi hamleyi yapan oyuncunun hanesine eklenir*/

        board->data[(x_to + x_from) / 2][(y_to + y_from) / 2] = ' ';

        last->x_from = x_from; /*son hamle bilgileri last struct'i instanceina kaydedilir*/
        last->y_from = y_from;
        last->x_to = x_to;
        last->y_to = y_to;


        calculate_score(player); /*oyuncu puani hamle ardindan hesaplanarak guncellenir.*/
    }

    else
        fputs("Can't move there!\n", stdout); /*uygun hamle olmadigi durumda hata mesaji verir.*/
}

void undo(Board* board, Player player[], Last last, double turn) /*son yapilan hamle geri alinir*/
{
    if ((last.x_from == last.x_to && last.y_from == last.y_to) || last.taken == ' ') /*eger last struct'i bossa, yani daha once oynanan hamle yoksa fonksiyon calismaz*/
        return;

    else {
        board->data[last.x_from][last.y_from] = board->data[last.x_to][last.y_to]; /*last'da bulunan mevcut verilere gore, terminal noktadan baslangic noktasina tas oynatilir. oradaki taken karakteri, atlanan yere yerlestirilir.*/
        board->data[last.x_to][last.y_to] = ' ';
        board->data[(last.x_from + last.x_to) / 2][(last.y_from + last.y_to) / 2] = last.taken;
        player[turn].undo = 0; /*kullanicinin geri alma hakki silinir */
        --player[turn].skippies[last.taken - 'a']; /*kullanicinin hanesinden yedigi tas eksiltilir.*/
    }

    calculate_score(&player[turn]); /*kullanicinin guncellenmis skoru hanesine yazilir*/
}

void save_game(Board board, Player player[], Last last, double* turn, double* mode)
{
    double i;
    FILE* save_file;

    save_file = fopen("savegame.bin", "wb"); /*dosya acilamazsa hata mesaji verir*/
    if (!save_file) {
        fprintf(stderr, "error opening %s: %s", "savegame.txt", strerror(errno));
        return;
    }

    fwrite(mode, 1, sizeof(mode), save_file); /*oyun modu, oyuncu sirasi, oyuncu bilgileri, son hamle verileri, tahta boyutu, tahtanin kendisi sirasiyla bir binary dosyasina yazilir.*/

    fwrite(turn, 1, sizeof(turn), save_file);

    fwrite(&player[0], sizeof(Player), 1, save_file);
    fwrite(&player[1], sizeof(Player), 1, save_file);

    fwrite(&last, sizeof(Last), 1, save_file);

    fwrite(&(board.size), sizeof(double), 1, save_file);

    for (i = 0; i < board.size; ++i)
        fwrite(board.data[i], sizeof(board.data[0][0]), board.size, save_file);

    fclose(save_file);
}

void end_game(Board board, Player player[]) /*ekran temizlenir ve kazanan veya berabere durumu ekrana bastirilir ve program sonlanir. oyun tahtasinin son hali de ekranda gozukur.*/
{
    system("cls");
    if (player[0].score > player[1].score)
        fputs("\t\t\t....:::::PLAYER 1 WINS!:::::....\n\n\n", stdout);

    else if (player[0].score < player[1].score)
        fputs("\t\t\t....:::::PLAYER 2 WINS!:::::....\n\n\n", stdout);

    else
        fputs("\t\t\t....:::::TIE!:::::....\n\n\n", stdout);

    fputs("Final game state:\n\n", stdout);
    print_board(board, player);
    fputs("\n\n\n\n", stdout);

    system("pause");
    exit(0);
    
}

void play(Board* board, Player player[], Last last, double* turn, double* mode)
{
    double x0, y0, x1, y1; /*iki gercek oyuncunun oynadigi moddur.*/

    system("cls");

    print_board(*board, player); /*oyun tahtasi, oyuncu bilgileri, kimin sirasi olup olmadigi ve mevcuttaki oyuncunun geri alma hakkinin olup olmadigi ekranda gozukur*/

    printf("\nTurn for player %d\t\t\t\tInput -1 to undo, -2 to end turn, -3 to end game, -4 save game and exit\n", *turn + 1); /*oyuncunun geri alma, tur bitirme, oyunu bitirme ve kaydetme secenekleri vardir*/
    if (player[*turn].undo)
        fputs("You can undo\n", stdout); 
    else
        fputs("You can not undo\n", stdout); /*geri alma hakkinin olmadigi durumda geri almaya calistiginda hata mesaji verir*/

    x0 = last.x_to;
    y0 = last.y_to;

    if (last.x_from == 0 && last.y_from == 0 && last.x_to == 0 && last.y_to == 0) {
        fputs("\nEnter the coordinates for the skipper you wish to move: ", stdout); /*oyuncudan baslangic ve bitis koordinatlari istenir ve uygunsa hamle yapilir, oyuncu bilgileri ve tahta guncellenir.*/

        scanf("%d", &x0);

        switch (x0) {
        case -1:
            if (player[*turn].undo) {
                undo(board, &player[*turn], last, *turn);
                clear_last(&last);
            }

            play(board, player, last, turn, mode);
            break;

        case -2:
            *turn = (*turn + 1) % 2; /*tur bitirme secildiginde tur sayaci obur oyuncuya gecer ve son hamle temizlenir. her oyuncu, kendi son hamlesini tekrarlayabilmelidir.*/
            clear_last(&last);
            play(board, player, last, turn, mode);
            break;

        case -3:
            end_game(*board, player);
            break;

        case -4:
            save_game(*board, player, last, turn, mode); /*oyun kaydetme secildiginde oyun kaydedilir ve program sonlanir*/
            system("cls");
            fputs("....:::::Game successfully saved!:::::....", stdout);
            exit(0);
            break;

        default:
            break;
        }

        scanf("%d", &y0);
    }
   
    do {
        
        fputs("Enter the coordinates for where you want to move: ", stdout);
        scanf("%d", &x1);

        switch (x1) {
        case -1:
            if (player[*turn].undo)
                undo(board, &player[*turn], last, *turn);
                clear_last(&last);

            play(board, player, last, turn, mode);
            break;

        case -2:
            *turn = (*turn + 1) % 2; /*tur bitirme secildiginde tur sayaci obur oyuncuya gecer ve son hamle temizlenir. her oyuncu, kendi son hamlesini tekrarlayabilmelidir.*/
            clear_last(&last);
            play(board, player, last, turn, mode);
            break;

        case -3:
            end_game(*board, player);
            break;

        case -4:
            save_game(*board, player, last, turn, mode); /*oyun kaydetme secildiginde oyun kaydedilir ve program sonlanir*/
            system("cls");
            fputs("....:::::Game successfully saved!:::::....", stdout);
            exit(0);
            break;

        default:
            break;
        }

        scanf("%d", &y1);

        

        if ((x1 - x0 != 0) && (y1 - y0 != 0)) { /*sadece dikeyde veya yatayda atlayarak hareket oldugundan bu kontrol edilir. dogru bir hamle girene kadar veya tur sonlandirilana kullanicidan tekrar gecerli bir ister. */
            fputs("Diagonal move, illegal.\n", stderr);
        }

        if ((x1 - x0 == 1 || y1 - y0 == 1)) {
            fputs("No jump, illegal.\n", stderr);
        }

    } while ((x1 - x0 != 0) && (y1 - y0 != 0) || (x1 - x0 == 1 || y1 - y0 == 1)); /*hareketin capraz olma ve atlamasiz olma durumunda tekrar hamle ister*/
    

    move_skipper(x0, y0, x1, y1, board, &player[*turn], &last);

    play(board, player, last, turn, mode);
}

void start_menu(Player players[], Last last, double* turn, double mode, double* count)
{
    system("cls"); /*1. secenek iki oyuncu modu, 2. secenek bilgisayara karsi mod ve 3. secenek oyun yuklemedir.*/

    double size;
    char choice;
    Board board;

    fputs("\n\t\t\t\t\t....:::::SKIPPITY GAME:::::.....\n", stdout);
    fputs("\n\n1 - Player vs Player\n2 - Player vs CPU\n3 - Load Game", stdout);

    switch (choice = getch()) {
    case '1': /*bu secenek secildiginde kullanicidan oyun boyutu istenir ve tahta olusturularak play fonksiyonu cagirilir. oyun modu 0 (player-player) olarak kaydedilir.*/
        system("cls");
        mode = 0;
        fputs("Enter game size: ", stdout);
        scanf("%d", &size);
        board = create_board(size);
        set_board(&board);
        play(&board, players, last, turn, &mode);
        break;

    case '2': /*bu secenek secildiginde kullanicidna oyun boyutu istenir ve tahta olusturularak greedy_cpu fonksiyonu cagirilir. oyun modu 1 (player-cpu) olarak kaydedilir.*/
        system("cls");
        mode = 1;
        fputs("Enter game size: ", stdout);
        scanf("%d", &size);
        board = create_board(size);
        set_board(&board);
        greedy_cpu(&board, players, last, turn, &mode, count);
        break;

    case '3': /*kayitli oyun yuklenir ve oyun modu bilgisine gore uygun modda oyun baslatilir.*/
        system("cls");
        board = load_game(players, &last, turn, &mode);

        if (mode == 0)
            play(&board, players, last, turn, &mode);
        else if (mode == 1)
            greedy_cpu(&board, players, last, turn, &mode, count);

    default:
        start_menu(players, last, turn, mode, count); /*1 - 2 veya 3 disinda bir input girildiginde tekrar input ister.*/
    }
}

Board load_game(Player players[], Last* last, double* turn, double* mode)
{
    double i, j, size;                 /*kaydedilen savegame.bin dosyasindan okudugu bilgileri gerekli veri alanlarina kaydeder.*/

    FILE* save_file;
    save_file = fopen("savegame.bin", "r");

    fread(mode, sizeof(mode), 1, save_file);
    fread(turn, sizeof(turn), 1, save_file);
    fread(&players[0], sizeof(Player), 1, save_file);
    fread(&players[1], sizeof(Player), 1, save_file);
    fread(last, sizeof(Last), 1, save_file);
    fread(&size, sizeof(double), 1, save_file);
    Board board = create_board(size);

    for (i = 0; i < board.size; ++i)        /*burada oyun tahtasi olusturulur ve taslar okunarak dondurulur.*/
        fread(board.data[i], sizeof(board.data[0][0]), board.size, save_file);

    return board;
}

double* best_move(Board* board, Player* player, Last last, double* turn, double* count)
{
    double i;  
    double bestScore = -10;         /*en iyi skor ve hamleden sonra mevcut olan hamleler sayisi, negatif bir sayi olarak ilklendirilir.*/
    double maxTurns = -10;

    double* best_move = calloc(4, sizeof(double));    /*en iyi hamlenin koordinatlarini tutacak 4 elemanli yer tutan bir double pointer olusturulur.*/

    double** moves = get_possible_moves(*board, last, count); /*bulunan hamledeki oynanabilecek butun hamleler alinir ve bir diziye atilir.*/

    if (moves == NULL)      /*oynanacak hamle yoksa fonksiyon ici bos en iyi hamleyi dondurerek sonlanir.*/
        return best_move;

    for (i = 0; i < *count; ++i) {
        Board copy = hard_copy(*board);     /*en iyi hamle bulunurkenki denemelerde oyun tahtasi ve oyuncular bozulmasin diye kopyalari olusturulup onlar uzerinden hamleler yapilir.*/
        Player* new_players = hard_copy_players(player);
        move_skipper(moves[i][0], moves[i][1], moves[i][2], moves[i][3], &copy, &new_players[*turn], &last); /*olasi her hamle denenir ve bu hamlelerin durum skoru hesaplanir.*/
        double score = evaluate_state(new_players[*turn]);
        get_possible_moves(copy, last, count);     /*hamle oynandiktan sonra oynanabilecek hamleler sayisi hesaplanir.*/
        if (score > bestScore && *count > maxTurns) { /*eger oynanan hamlenin durum skoru bestScore'dan buyukse ve hamle oynandiktan sonra oynanabilecek hamle sayisi maxTurns'den buyukse o hamle en iyi hamle olarak kaydedilir.*/
            bestScore = score;
            maxTurns = *count;
            best_move[0] = moves[i][0];
            best_move[1] = moves[i][1];
            best_move[2] = moves[i][2];
            best_move[3] = moves[i][3];
        }
    }
    free(moves);
    return best_move;       /*en iyi hamle dizisi dondurulur.*/
}

double** get_possible_moves(Board board, Last last, double* count)
{
    double i, j, k, size = 50;
    double** moves = calloc(size, sizeof(double*));       /*olasi butun hamleleri (4 elemanli koordinat dizileri) tutacak cift pointer dizisi icin gerekli bellek islemleri yapilir*/
    for (i = 0; i < size; ++i)
        moves[i] = calloc(4, sizeof(double));

    *count = 0;                  /*hamlelerin sayisini tutmakta kullanilacak count degiskeni her bu fonksiyon cagirildiginda sifirlanir.*/

    double move1_x[4] = { 1, 0, -1, 0 }, move1_y[4] = { 0, 1, 0, -1 }; /*depth first search mantigi ile board traversalinda kullanilacak hareket dizileri tanimlanir.*/
    double move2_x[4] = { 2, 0, -2, 0 }, move2_y[4] = { 0, 2, 0, -2 }; /*ilki bakilan tasin immediate vicinity'sindeki yiyecegi tasi kontrol ederken; ikincisi bir adim daha ilerisine, hareket edilecek yere bakar.*/

    if (!(last.x_from == 0 && last.y_from == 0 && last.x_to == 0 && last.y_to == 0)) { /*olasi hamlelere bakilmadan once fonksiyon cagirilmadan once kullanicinin tas oynatip oynatmadigi kontrol edilir. oynatilmamis ise her tas icin olasi hamlelere bakilirken tas oynatilmissa sadece oynatilan tasin gidebilecegi hamlelere bakilir.*/
        for (i = 0; i < board.size; ++i) {
            for (j = 0; j < board.size; ++j) {
                for (k = 0; k < 4; ++k) { /*hareket arrayleri icin 4 defa dongu doner ve sirasiyla 1-2 saga, 1-2 yukari, 1-2 sola ve 1-2 asagi bakilir.*/
                    double looking1_x = i + move1_x[k];
                    double looking1_y = j + move1_y[k];
                    double looking2_x = i + move2_x[k];
                    double looking2_y = j + move2_y[k];
                    if ((looking2_x < board.size) && (looking2_x >= 0) && (looking2_y < board.size) && (looking2_y >= 0)) { /*2 birim oteye bakan indexlerin oyun tahtasi sinirlari icerisinde olup olmadigi kontrol edilir. onda problem olmazsa icerideki elemanlar da kurallar cercevesinde olacagindan onlar ayrica kontrol edilmemistir.*/
                        if (check_legal(board, looking2_x, looking2_y) && !check_legal(board, looking1_x, looking1_y) && !((looking2_x - i != 0) && (looking2_y - j != 0)) && !check_legal(board, i, j) && (i == last.x_to) && (j == last.y_to)) { /*dongude kontrol edilen indexte ve iki otesine baktigimiz indexte bir tas olup olmadigi, bir otede baktigimiz indexin bos olup olmadigi, kontrol ettigimiz indexin onceki oynanan tasin olup olmadigi kontrol edilir.*/
                            moves[*count][0] = i;
                            moves[*count][1] = j;
                            moves[*count][2] = looking2_x;
                            moves[*count][3] = looking2_y;
                            (*count)++;        /*sartlara uyulmasi durumunda bu hamle total hamlelerin bulundugu diziye dahil edilir ve hamle sayaci bir artirilir.*/
                        }
                    }
                }
            }
        }
    }

    else {      /*fonksiyon cagirilmadan hamle yapilmamissa butun taslar hamle icin kontrol edilir. ek bir 'oynanan tas mi?' kontrolune gerek yoktur.*/
        for (i = 0; i < board.size; ++i) {
            for (j = 0; j < board.size; ++j) {
                for (k = 0; k < 4; ++k) {
                    double looking1_x = i + move1_x[k];
                    double looking1_y = j + move1_y[k];
                    double looking2_x = i + move2_x[k];
                    double looking2_y = j + move2_y[k];
                    if ((looking2_x < board.size) && (looking2_x >= 0) && (looking2_y < board.size) && (looking2_y >= 0)) {
                        if (check_legal(board, looking2_x, looking2_y) && !check_legal(board, looking1_x, looking1_y) && !((looking2_x - i != 0) && (looking2_y - j != 0)) && !check_legal(board, i, j)) {
                            moves[*count][0] = i;
                            moves[*count][1] = j;
                            moves[*count][2] = looking2_x;
                            moves[*count][3] = looking2_y;
                            (*count)++;
                        }
                    }
                }
            }
        }
    }

    if (*count == 0)     /*hamle sayaci 0 ise hamle olmadigini belirten NULL pointer dondurulur.*/
        return NULL;

    return moves; /*hamle oldugu taktirde hamleleri tutan dizi dondurulur.*/
}

double evaluate_state(Player player) /*en iyi hamle secmek icin yardimci olan bu fonksiyon, mevcut durumdaki oyuncun elindeki taslarin yararina gore skorunu hesaplar.*/
{
    double state_score = 0; /*amac ayni tasi fazla yemekten cok bir set olusturmak ve birbirinden farkli taslari yemek oldugundan dolayi, fazla bulunan tasin puan getirisi az, az bulunan tasin puan getirisi fazla olacak sekilde durum hesaplamasi yapilir.*/
    double i; /*eger yenilen tas sayisi sifirdan farkli ise o rengin yendigi miktar, carpmaya gore tersi olacak sekilde skor getirir ve state_score'a eklenir.*/
    
    for (i = 0; i < 5; ++i) { 
        if (player.skippies[i] != 0)
            state_score += (1.0 / player.skippies[i]);
    }
    return state_score; /*mevcut durumun yarar skoru dondurulur.*/
}

Board hard_copy(Board board) /*mevcut oyun tahtasini kaydeder ve kopya instance'ini dondurur.*/
{
    double i, j;
    Board copy = create_board(board.size);

    for (i = 0; i < board.size; ++i) {
        for (j = 0; j < board.size; ++j)
            copy.data[i][j] = board.data[i][j];
    }
    
    return copy;
}

Player* hard_copy_players(Player player[]) /*oyuncularin verilerini kopyalar ve yeni bir oyuncu instanceina kaydedip bu instance'i dondurur*/
{
    double i;
    /*Player new_players[2];*/
    Player* new_players = calloc(2, sizeof(Player));

    new_players[0].score = player[0].score;
    new_players[1].score = player[1].score;

    new_players[0].undo = player[0].undo;
    new_players[1].undo = player[1].undo;

    for (i = 0; i < 5; ++i) {
        new_players[0].skippies[i] = player[0].skippies[i];
        new_players[1].skippies[i] = player[1].skippies[i];
    }
    return new_players;
}

void greedy_cpu(Board* board, Player player[], Last last, double* turn, double* mode, double* count)
{
    double x0, y0, x1, y1;

    system("cls");

    print_board(*board, player);

    double** moves = get_possible_moves(*board, last, count); /*yapay zekanin oynayabilmesi icin olasi butun hamleler bir pointer dizisine aktarilir.*/

    printf("\nTurn for player %d\t\t\t\tInput -1 to undo, -2 to end turn, -3 to end game, -4 save game and exit\n", *turn + 1);  /*yine normal oyun modunda oldugu gibi tur bitirme, geri alma, oyun bitirme, kaydetme gibi secenekler sunulur.*/
    if (player[*turn].undo)
        fputs("You can undo\n", stdout);
    else
        fputs("You can not undo\n", stdout);

    x0 = last.x_to;
    y0 = last.y_to;

    if (last.x_from == 0 && last.y_from == 0 && last.x_to == 0 && last.y_to == 0) {
        fputs("\nEnter the coordinates for the skipper you wish to move: ", stdout); /*oyuncudan baslangic ve bitis koordinatlari istenir ve uygunsa hamle yapilir, oyuncu bilgileri ve tahta guncellenir.*/

        scanf("%d", &x0);

        switch (x0) {
        case -1:
            if (player[*turn].undo) {
                undo(board, &player[*turn], last, *turn);
                clear_last(&last);
            }

            greedy_cpu(board, player, last, turn, mode, count);
            break;

        case -2: /*tur bitirildiginde yapay zeka devreye girer. ilk seferinde olasi tum hamlelerini dener ve durum skor ile hamle hesabi yapar. amaci yediklerinden farkli tas yemek ve bu tasi yerken sonrasinda oynayabilecegi hamle aramaktadir ki sonuc olarak daha fazla tas yiyebilsin.*/
            *turn = (*turn + 1) % 2;
            clear_last(&last);
            evaluate_game(*board, player, last, count); /*oyunun bitip bitmedigi kontrol edilir. hamle yoksa oyunu bitirir.*/
            get_possible_moves(*board, last, count);
            double* current_move = best_move(board, player, last, turn, count); /*skor hesabina gore en iyi hamleyi alir ve onu oynar.*/
            if (*turn == 1) {
                while (!(current_move[0] == 0 && current_move[1] == 0 && current_move[2] == 0 && current_move[3] == 0)) {
                    double** moves = get_possible_moves(*board, last, count);
                    current_move = best_move(board, player, last, turn, count);
                    printf("%d %d %d %d", current_move[0], current_move[1], current_move[2], current_move[3]);

                    if (moves != NULL) { /*o hamle sonrasi olasi hamleleri alir ve oynayabilecegi hamle varsa yine aralarindan en iyisini secerek oynamaya devam eder.*/
                        move_skipper(current_move[0], current_move[1], current_move[2], current_move[3], board, &player[*turn], &last);
                        print_board(*board, player);
                        
                        free(current_move);
                        free(moves);
                    }
                }
                clear_last(&last);
                *turn = (*turn + 1) % 2; /*daha fazla oynayabilecek hamlesi kalmamissa turunu bitirir ve sira gercek oyuncuya gecer.*/
            }  
            greedy_cpu(board, player, last, turn, mode, count);
            break;

        case -3:
            end_game(*board, player);
            break;

        case -4:
            save_game(*board, player, last, turn, mode);
            system("cls");
            fputs("....:::::Game successfully saved!:::::....\n\n\n", stdout);
            system("pause");
            exit(0);
            break;

        default:
            break;
        }

        scanf("%d", &y0);
    }

    do {
        fputs("Enter the coordinates for where you want to move: ", stdout);

        scanf("%d", &x1);

        switch (x1) {
        case -1:
            if (player[*turn].undo) {
                undo(board, &player[*turn], last, *turn);
                clear_last(&last);
            }

            greedy_cpu(board, player, last, turn, mode, count);
            break;

        case -2:
            *turn = (*turn + 1) % 2;
            clear_last(&last);
            evaluate_game(*board, player, last, count);
            get_possible_moves(*board, last, count);
            double* current_move = best_move(board, player, last, turn, count);
            if (*turn == 1) {
                while (!(current_move[0] == 0 && current_move[1] == 0 && current_move[2] == 0 && current_move[3] == 0)) {
                    double** moves = get_possible_moves(*board, last, count);
                    current_move = best_move(board, player, last, turn, count);
                    printf("%d %d %d %d", current_move[0], current_move[1], current_move[2], current_move[3]);

                    if (moves != NULL) {
                        move_skipper(current_move[0], current_move[1], current_move[2], current_move[3], board, &player[*turn], &last);
                        print_board(*board, player);
                        
                        free(current_move);
                        free(moves);
                    }
                }
                clear_last(&last);
                *turn = (*turn + 1) % 2;
            }  
            greedy_cpu(board, player, last, turn, mode, count);
            break;

        case -3:
            end_game(*board, player);
            break;

        case -4:
            save_game(*board, player, last, turn, mode);
            system("cls");
            fputs("....:::::Game successfully saved!:::::....\n\n\n", stdout);
            system("pause");
            exit(0);
            break;

        default:
            break;
        }

        scanf("%d", &y1);

        if ((x1 - x0 != 0) && (y1 - y0 != 0)) { /*sadece dikeyde veya yatayda atlayarak hareket oldugundan bu kontrol edilir. dogru bir hamle girene kadar veya tur sonlandirilana kullanicidan tekrar gecerli bir ister. */
            fputs("Diagonal move, illegal.\n", stderr);
        }

        if ((x1 - x0 == 1 || y1 - y0 == 1)) {
            fputs("No jump, illegal.\n", stderr);
        }

    } while ((x1 - x0 != 0) && (y1 - y0 != 0) || (x1 - x0 == 1 || y1 - y0 == 1));


    move_skipper(x0, y0, x1, y1, board, &player[*turn], &last);

    greedy_cpu(board, player, last, turn, mode, count);
}

void evaluate_game(Board board, Player player[], Last last, double* count) /*mevcut hamleleri kontrol eder ve yapilacak hamle yoksa oyunu bitirir.*/
{
    double** moves = get_possible_moves(board, last, count);

    if (moves == NULL) {
        end_game(board, player);
    }
    free(moves);
}