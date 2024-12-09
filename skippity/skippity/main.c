/*
TODO:
    - Count globalden cikar parametre olarak ekle
    - AI oynayabilecegi kadar oynasin rastgele depth verme
*/


#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

int count = 0;

typedef struct {
    int size;
    char** data;
} Board;

typedef struct {
    int score;
    int undo;
    int skippies[5];
} Player;

typedef struct {
    int x_from, y_from;
    int x_to, y_to;
    char taken;
} Last;

Board create_board(const int); /*Oyun tahtasini olusturan fonksiyon, bellek atama islemleri burada yapilir ve yer acilmis board instance'i dondurulur*/

void set_board(Board*); /*olusturulmus board instance'ina oyuna uygun sekilde rastgele renkler yerine gecen 'a', 'b', 'c', 'd' ve 'e' karakterleri atanir. ortadaki 2x2'lik alan bos birakilir*/

void clear_last(Last*); /*son hamlenin bilgilerini tutan struct temizlenir ve yeni bir hamle verisi tutabilecek hale gelir*/

void calculate_score(Player*); /*parametre olarak alinan oyuncunun toplam skoru hesaplanir. yedigi taslar bir set olusturdugunda skor 1 artar.*/

void print_board(const Board, const Player[]); /*mevcut durumdaki oyun tahtasi ve oyuncularin yedigi taslar ile birlikte puanlari ekrana bastirilir.*/

int check_legal(const Board, const int, const int); /*yapilacak hamlenin oyun kurallari cercevesinde mumkun olup olmadigi kontrol edilir, mumkunse 1, degilse 0 dondurulur.*/

void move_skipper(int, int, int, int, Board*, Player*, Last*); /*girilen koordinatlardaki oyun tasi, girilen diger koordinatlara, mumkunse hareket ettirilir; degilse bir sey yapmaz.*/

void undo(Board*, Player[], Last, const int); /*oyuncunun hakki oldugu durumda son oynadigi hamle geri alinir, yenilen tas geri tahtaya yerlestirilir ve oyuncunun yedigi tas hanesinden silinir.*/

void save_game(const Board, const Player[], const Last, const int*, const int*); /*mevcut oyun tahtasi, oyun tahtasinin buyuklugu, oyuncularin skor ve tas durumlari, oyun modu, kimin sirasi oldugu binary dosyasina kaydedilir*/

Board load_game(Player[], Last*, int*, int*); /*kaydedilen kayit dosyasi yuklenir ve oyun kaldigi yerden oynanabilir hale gelir.*/

void end_game(const Board, Player); /*bu fonksiyon cagirildiginda ekran temizlenir ve kimin kazandigi ekrana bastirilir, daha sonra oyun sonlanir.*/

void play(Board*, Player[], Last, int*, const int*); /*player vs player modunu calistiran driver kodu*/

void start_menu(const Player[], const Last, const int*, int); /*oyun modu secme ve kayit dosyasi yukleme secenklerinin oldugu ana menu ekrani*/

int* best_move(Board*, Player*, Last, int*); /*mevcut oyun tahtasinda o anda , yapay zekaya en cok puan kazandiracak hamleyi belirleyen fonksiyon. en iyi hamlenin koordinatlarini dondurur */

int** get_possible_moves(const Board, Last); /*mevcut oyun tahtasinda oynanabilecek butun hamleleri donduren fonksiyon. her biri 4 elemanli koordinat dizisi tutan bir array dondurur*/

double evaluate_state(const Player); /*en iyi hamle belirleme fonksiyonunun icinde kullanilan bu fonksiyon, yapay zekanin yemedigi veya daha az yedigi bir tasi yemesini hedefler*/

int* minimax(const Board, const Player[], const Last, int*, int, int, int*);

void greedy_cpu(Board*, Player[], Last, int*, const int*); /*yapay zekaya karsi oynama modunu calistiran driver kodu*/

void evaluate_game(const Board, const Player[], Last); /*oyunda yapilabilecek hamle kalip kalmadigina bakan fonksiyon, kalmadiysa oyunu bitirir*/

Board hard_copy(const Board); /*en iyi hamleleri ararken oyun tahtasini bozmadan makinenin oynayabilmesi icin mevcut tahtayi gecici bir tahtaya kopyalayan fonksiyondur*/

Player* hard_copy_players(const Player[]); /*en iyi hamleleri ararken oyuncularin skorlarini bozmadan makinenin oynayabilmesi icin mevcut oyuncu durumlarini gecici bir oyuncu instance'ina kopyalayan fonksiyondur*/

int main(void)
{
    srand(time(NULL));
    Last last = {0, 0, 0, 0, ' '}; /*son hamle, oyuncularin struct instance'lari, ve oyun sirasi initialize edilir ve start_menu fonksiyonu cagirilarak kullanici icin program baslar.*/
    Board board = create_board(6);
    Player players[2] = { {0, 1, {0, 0, 0, 0, 0}}, {0, 1, {0, 0, 0, 0, 0}} };
    int mode;
    int turn = 0;

    start_menu(players, last, &turn, &mode);

}

Board create_board(const int size)
{
    Board board;    /*girilen boyut degeri kullanilarak board instance'ina gerektigi kadar bellekte yer ayrilir ve yer ayrilmis board instancei dondurulur.*/
    int i;

    board.size = size;
    board.data = calloc(size, sizeof(char*));

    for (i = 0; i < size; ++i)
        board.data[i] = calloc(size, sizeof(char));

    return board;
}

void set_board(Board* board)
{
    int i, j; /*oyun tahtasina rastgele a-e arasi karakterler yerlesir. tahta boyutu ne olursa olsun ortadaki 4 hucre bostur.*/

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
    int i, min = player->skippies[0];
    for (i = 0; i < 5; ++i)
        if (player->skippies[i] < min)
            min = player->skippies[i];

    player->score = min;
}

void print_board(const Board board, const Player players[]) /*oyun tahtasi ve oyuncularin skor durumu ile yedikleri tas sayilari ekrana bastirilir*/
{
    int i, j;
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

int check_legal(const Board board, const int x, const int y) /*tas oynatmak icin girilen terminal koordinatin bos olup olmadigina bakarak hamlenin legal olup olmadigini donduren fonksiyon. legalse 1, degilse 0 dondurur.*/
{
    return board.data[x][y] == ' ' ? 1 : 0;
}

void move_skipper(int x_from, int y_from, int x_to, int y_to, Board* board, Player* player, Last* last)
{
    if (check_legal(*board, x_to, y_to)) /*girilen terminal koordinatin uygun olup olmadigi kontrol edilir, oyleyse if bloguna girilir*/
    {
        board->data[x_to][y_to] = board->data[x_from][y_from];

        board->data[x_from][y_from] = ' ';

        int t = (board->data[(x_to + x_from) / 2][(y_to + y_from) / 2]) - 'a'; /*baslangic koordinatinda bulunan tas, terminal koordinata yerlestirilir*/
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

void undo(Board* board, Player player[], Last last, const int turn) /*son yapilan hamle geri alinir*/
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

void save_game(const Board board, const Player player[], const Last last, const int* turn, const int* mode)
{
    int i;
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

    fwrite(&(board.size), sizeof(int), 1, save_file);

    for (i = 0; i < board.size; ++i)
        fwrite(board.data[i], sizeof(board.data[0][0]), board.size, save_file);

    fclose(save_file);
}

void end_game(const Board board, Player player[]) /*ekran temizlenir ve kazanan veya berabere durumu ekrana bastirilir ve program sonlanir. oyun tahtasinin son hali de ekranda gozukur.*/
{
    //system("cls");
    if (player[0].score > player[1].score)
        fputs("\t\t\t....:::::PLAYER 1 WINS!:::::....\n\n\n", stdout);

    else if (player[0].score < player[1].score)
        fputs("\t\t\t....:::::PLAYER 2 WINS!:::::....\n\n\n", stdout);

    else
        fputs("\t\t\t....:::::TIE!:::::....\n\n\n", stdout);

    fputs("Final game state:\n\n", stdout);
    print_board(board, player);
    fputs("\n\n\n\n", stdout);

    exit(0);
}

void play(Board* board, Player player[], Last last, int* turn, const int* mode)
{
    int x0, y0, x1, y1; /*iki gercek oyuncunun oynadigi moddur.*/

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
        scanf("%d", &y0);
    }
   
    do {
        
        fputs("Enter the coordinates for where you want to move: ", stdout);
        scanf("%d", &x1);

        switch (x1) {
        case -1:
            if (player[*turn].undo)
                undo(board, &player[*turn], last, *turn);

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

void start_menu(const Player players[], const Last last, const int* turn, int mode)
{
    system("cls"); /*1. secenek iki oyuncu modu, 2. secenek bilgisayara karsi mod ve 3. secenek oyun yuklemedir.*/

    int size;
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
        greedy_cpu(&board, players, last, turn, &mode);
        break;

    case '3': /*kayitli oyun yuklenir ve oyun modu bilgisine gore uygun modda oyun baslatilir.*/
        system("cls");
        board = load_game(players, &last, turn, &mode);

        if (mode == 0)
            play(&board, players, last, turn, &mode);
        else if (mode == 1)
            greedy_cpu(&board, players, last, turn, &mode);

    default:
        start_menu(players, last, turn, mode); /*1 - 2 veya 3 disinda bir input girildiginde tekrar input ister.*/
    }
}

Board load_game(Player players[], Last* last, int* turn, int* mode)
{
    int i, j, size;

    FILE* save_file;
    save_file = fopen("savegame.bin", "r");

    fread(mode, sizeof(mode), 1, save_file);
    fread(turn, sizeof(turn), 1, save_file);
    fread(&players[0], sizeof(Player), 1, save_file);
    fread(&players[1], sizeof(Player), 1, save_file);
    fread(last, sizeof(Last), 1, save_file);
    fread(&size, sizeof(int), 1, save_file);
    Board board = create_board(size);

    for (i = 0; i < board.size; ++i)
        fread(board.data[i], sizeof(board.data[0][0]), board.size, save_file);

    return board;
}

int* best_move(Board* board, Player* player, Last last, int* turn)
{
    int i;
    double bestScore = -10;

    int* best_move = calloc(4, sizeof(int));

    int** moves = get_possible_moves(*board, last);

    int depth = 3;
    int isMaximizing = 1;

    if (moves == NULL)
        return best_move;

    printf("moves:\n");
    for (i = 0; i < count; ++i)
        printf("%d %d %d %d\n", moves[i][0], moves[i][1], moves[i][2], moves[i][3]);
    
    

    for (i = 0; i < count; ++i) {
        current_move = {mo}
        
        minimax(*board, player, last, turn, depth, isMaximizing, current_move);
        double score = evaluate_state(new_players[*turn]);
        if (score > bestScore) {
            bestScore = score;
            best_move[0] = moves[i][0];
            best_move[1] = moves[i][1];
            best_move[2] = moves[i][2];
            best_move[3] = moves[i][3];
        }
        
        
    }
    free(moves);
    return best_move;
    
    
}

int** get_possible_moves(const Board board, Last last)
{
    int i, j, k, size = 50;
    int** moves = calloc(size, sizeof(int*));
    for (i = 0; i < size; ++i)
        moves[i] = calloc(4, sizeof(int));

    count = 0;

    int move1_x[4] = { 1, 0, -1, 0 }, move1_y[4] = { 0, 1, 0, -1 };
    int move2_x[4] = { 2, 0, -2, 0 }, move2_y[4] = { 0, 2, 0, -2 };

    if (!(last.x_from == 0 && last.y_from == 0 && last.x_to == 0 && last.y_to == 0)) {
        for (i = 0; i < board.size; ++i) {
            for (j = 0; j < board.size; ++j) {
                for (k = 0; k < 4; ++k) {
                    int looking1_x = i + move1_x[k];
                    int looking1_y = j + move1_y[k];
                    int looking2_x = i + move2_x[k];
                    int looking2_y = j + move2_y[k];
                    if ((looking2_x < board.size) && (looking2_x >= 0) && (looking2_y < board.size) && (looking2_y >= 0)) {
                        if (check_legal(board, looking2_x, looking2_y) && !check_legal(board, looking1_x, looking1_y) && !((looking2_x - i != 0) && (looking2_y - j != 0)) && !check_legal(board, i, j) && (i == last.x_to) && (j == last.y_to)) {
                            moves[count][0] = i;
                            moves[count][1] = j;
                            moves[count][2] = looking2_x;
                            moves[count][3] = looking2_y;
                            count++;
                        }
                    }
                }
            }
        }
    }

    else {
        for (i = 0; i < board.size; ++i) {
            for (j = 0; j < board.size; ++j) {
                for (k = 0; k < 4; ++k) {
                    int looking1_x = i + move1_x[k];
                    int looking1_y = j + move1_y[k];
                    int looking2_x = i + move2_x[k];
                    int looking2_y = j + move2_y[k];
                    if ((looking2_x < board.size) && (looking2_x >= 0) && (looking2_y < board.size) && (looking2_y >= 0)) {
                        if (check_legal(board, looking2_x, looking2_y) && !check_legal(board, looking1_x, looking1_y) && !((looking2_x - i != 0) && (looking2_y - j != 0)) && !check_legal(board, i, j)) {
                            moves[count][0] = i;
                            moves[count][1] = j;
                            moves[count][2] = looking2_x;
                            moves[count][3] = looking2_y;
                            count++;
                        }
                    }
                }
            }
        }
    }
    
    if (count == 0)
        return NULL;

    return moves;
}

double evaluate_state(const Player player) /*en iyi hamle secmek icin yardimci olan bu fonksiyon, mevcut durumdaki oyuncun elindeki taslarin yararina gore skorunu hesaplar.*/
{
    double state_score = 0; /*amac ayni tasi fazla yemekten cok bir set olusturmak ve birbirinden farkli taslari yemek oldugundan dolayi, fazla bulunan tasin puan getirisi az, az bulunan tasin puan getirisi fazla olacak sekilde durum hesaplamasi yapilir.*/
    int i; /*eger yenilen tas sayisi sifirdan farkli ise o rengin yendigi miktar, carpmaya gore tersi olacak sekilde skor getirir ve state_score'a eklenir.*/
    
    for (i = 0; i < 5; ++i) { 
        if (player.skippies[i] != 0)
            state_score += (1.0 / player.skippies[i]);
    }
    return state_score; /*mevcut durumun yarar skoru dondurulur.*/
}

Board hard_copy(const Board board) /*mevcut oyun tahtasini kaydeder ve kopya instance'ini dondurur.*/
{
    int i, j;
    Board copy = create_board(board.size);

    for (i = 0; i < board.size; ++i) {
        for (j = 0; j < board.size; ++j)
            copy.data[i][j] = board.data[i][j];
    }
    
    return copy;
}

Player* hard_copy_players(const Player player[]) /*oyuncularin verilerini kopyalar ve yeni bir oyuncu instanceina kaydedip bu instance'i dondurur*/
{
    int i;
    Player new_players[2];

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

void greedy_cpu(Board* board, Player player[], Last last, int* turn, const int* mode)
{
    int x0, y0, x1, y1;

    //system("cls");

    print_board(*board, player);

    int** moves = get_possible_moves(*board, last);

    evaluate_game(*board, player, last);

    printf("\nTurn for player %d\t\t\t\tInput -1 to undo, -2 to end turn, -3 to end game, -4 save game and exit\n", *turn + 1);
    if (player[*turn].undo)
        fputs("You can undo", stdout);
    else
        fputs("You can not undo", stdout);

    do {
        fputs("\nEnter the coordinates for the skipper you wish to move: ", stdout);

        scanf("%d", &x0);

        switch (x0) {
        case -1:
            if (player[*turn].undo)
                undo(board, &player[*turn], last, *turn);

            greedy_cpu(board, player, last, turn, mode);
            break;

        case -2:
            *turn = (*turn + 1) % 2;
            clear_last(&last);
            int** moves = get_possible_moves(*board, last);
            int* current_move = best_move(board, player, last, turn);
            //int depth = 3;
            //int isMaximizing = 1;
            if (*turn == 1) {
                while (!(current_move[0] == 0 && current_move[1] == 0 && current_move[2] == 0 && current_move[3] == 0)) {
                    int** moves = get_possible_moves(*board, last);
                    current_move = best_move(board, player, last, turn);
                    printf("%d %d %d %d", current_move[0], current_move[1], current_move[2], current_move[3]);

                    if (moves != NULL) {
                        move_skipper(current_move[0], current_move[1], current_move[2], current_move[3], board, &player[*turn], &last);
                        print_board(*board, player);
                        depth--;
                        free(current_move);
                        free(moves);
                    }
                    //evaluate_game(*board, player, last);
                } 
                clear_last(&last);
                *turn = (*turn + 1) % 2;
            }  
            
            greedy_cpu(board, player, last, turn, mode);
            break;

        case -3:
            end_game(*board, player);
            break;

        case -4:
            save_game(*board, player, last, turn, mode);
            system("cls");
            fputs("....:::::Game successfully saved!:::::....", stdout);
            exit(0);
            break;

        default:
            break;
        }

        scanf("%d", &y0);

        fputs("Enter the coordinates for where you want to move: ", stdout);
        scanf("%d %d", &x1, &y1);

        if ((x1 - x0 != 0) && (y1 - y0 != 0)) {
            fputs("Diagonal move, illegal.", stderr);
        }
    } while ((x1 - x0 != 0) && (y1 - y0 != 0));


    move_skipper(x0, y0, x1, y1, board, &player[*turn], &last);

    greedy_cpu(board, player, last, turn, mode);
}


int* minimax(const Board board, const Player player[], const Last last, int* turn, int depth, int isMaximizing, int* bestMove)
{
    int* ret = calloc(5, sizeof(int));

    if (depth == 0) {
        double score = evaluate_state(player[*turn]);
        printf("%d\n", score);
        ret[4] = score;
        //printf("played move: %d %d %d %d\n", bestMove[0], bestMove[1], bestMove[2], bestMove[3]);
        //move_skipper(bestMove[0], bestMove[1], bestMove[2], bestMove[3], &board, &player[*turn], &last);
        //*turn = (*turn + 1) % 2;
        return ret;
    }
    else if (isMaximizing) {
        int i, j;
        double bestScore = -100;
        
        int** moves = get_possible_moves(board, last);

        
        for (i = 0; i < count; ++i) {
                printf("move %d\t%d %d %d %d\n", i, moves[i][0], moves[i][1], moves[i][2], moves[i][3]);
        }
        //print_board(board, player);
        
        for (i = 0; i < count; ++i) {
            Board new_game = hard_copy(board, player, turn);
            Player* new_players = hard_copy_players(player);
            move_skipper(moves[i][0], moves[i][1], moves[i][2], moves[i][3], &new_game, &new_players[1], &last);
            int score = minimax(new_game, new_players, last, turn, depth - 1, isMaximizing, bestMove)[4];
            //printf("%d %d %d %d\n", moves[i][0], moves[i][1], moves[i][2], moves[i][3]);
            bestScore = max(score, bestScore);
            if (bestScore == score) {
                ret[0] = moves[i][0];
                ret[1] = moves[i][1];
                ret[2] = moves[i][2];
                ret[3] = moves[i][3];
            }
        }
        //isMaximizing = (isMaximizing + 1) % 2;
        //*turn = (*turn + 1) % 2;
        return ret;
    }
    else {
        int i, j;
        double worstScore = 100;
        
        int** moves = get_possible_moves(board, last);

        for (i = 0; i < count; ++i) {
            Board new_game = hard_copy(board, player, turn);
            Player* new_players = hard_copy_players(player);
            move_skipper(moves[i][0], moves[i][1], moves[i][2], moves[i][3], &new_game, &new_players[0], &last);
            int score = minimax(new_game, new_players, last, turn, depth - 1, isMaximizing, bestMove)[4];
            printf("%d", score);
            worstScore = min(score, worstScore);
            if (worstScore == score) {
                bestMove[0] = moves[i][0];
                bestMove[1] = moves[i][1];
                bestMove[2] = moves[i][2];
                bestMove[3] = moves[i][3];
            }
        }
        isMaximizing = (isMaximizing + 1) % 2;
        *turn = (*turn + 1) % 2;
        return ret;
    }
}


void evaluate_game(const Board board, const Player player[], Last last) /*mevcut hamleleri kontrol eder ve yapilacak hamle yoksa oyunu bitirir.*/
{
    int** moves = get_possible_moves(board, last);

    if (moves == NULL) {
        end_game(board, player);
    }
    free(moves);
}